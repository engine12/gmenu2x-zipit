/***************************************************************************
 *   Copyright (C) 2006 by Massimiliano Torromeo                           *
 *   massimiliano.torromeo@gmail.com                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "surface.h"

#include "debug.h"
#include "imageio.h"
#include "surfacecollection.h"
#include "utilities.h"

#include <SDL_gfxPrimitives.h>

#include <iostream>

using namespace std;

RGBAColor strtorgba(const string &strColor) {
	RGBAColor c = {0,0,0,255};
	c.r = constrain( strtol( strColor.substr(0,2).c_str(), NULL, 16 ), 0, 255 );
	c.g = constrain( strtol( strColor.substr(2,2).c_str(), NULL, 16 ), 0, 255 );
	c.b = constrain( strtol( strColor.substr(4,2).c_str(), NULL, 16 ), 0, 255 );
	c.a = constrain( strtol( strColor.substr(6,2).c_str(), NULL, 16 ), 0, 255 );
	return c;
}

Surface *Surface::openOutputSurface(int width, int height, int bitsperpixel) {
	SDL_ShowCursor(SDL_DISABLE);
	SDL_Surface *raw = SDL_SetVideoMode(
		width, height, bitsperpixel, SDL_HWSURFACE | SDL_DOUBLEBUF);
	return raw ? new Surface(raw, false) : NULL;
}

Surface *Surface::emptySurface(int width, int height) {
	SDL_Surface *raw = SDL_CreateRGBSurface(
		SDL_SWSURFACE, width, height, 32, 0, 0, 0, 0);
	if (!raw) return NULL;
	SDL_FillRect(raw, NULL, SDL_MapRGB(raw->format, 0, 0, 0));
	return new Surface(raw, true);
}

Surface *Surface::loadImage(const string &img, const string &skin) {
	string skinpath;
	if (!skin.empty() && !img.empty() && img[0]!='/')
	  skinpath = SurfaceCollection::getSkinFilePath(skin, img);
	else
	  skinpath = img;

	SDL_Surface *raw = loadPNG(skinpath);
	if (!raw) {
		ERROR("Couldn't load surface '%s'\n", img.c_str());
		return NULL;
	}

	return new Surface(raw, true);
}

Surface::Surface(SDL_Surface *raw_, bool freeWhenDone_)
	: raw(raw_)
	, freeWhenDone(freeWhenDone_)
{
	halfW = raw->w/2;
	halfH = raw->h/2;
}

Surface::Surface(Surface *s) {
	raw = SDL_ConvertSurface(s->raw, s->raw->format, SDL_SWSURFACE);
	// Note: A bug in SDL_ConvertSurface() leaves the per-surface alpha
	//       undefined when converting from RGBA to RGBA. This can cause
	//       problems if the surface is later converted to a format without
	//       an alpha channel, such as the display format.
	raw->format->alpha = s->raw->format->alpha;
	freeWhenDone = true;
	halfW = raw->w/2;
	halfH = raw->h/2;
}

Surface::~Surface() {
	if (freeWhenDone) {
		SDL_FreeSurface(raw);
	}
}

void Surface::convertToDisplayFormat() {
	SDL_Surface *newSurface = SDL_DisplayFormat(raw);
	if (newSurface) {
		if (freeWhenDone) {
			SDL_FreeSurface(raw);
		}
		raw = newSurface;
		freeWhenDone = true;
	}
}

void Surface::flip() {
	SDL_Flip(raw);
}

bool Surface::blit(SDL_Surface *destination, int x, int y, int w, int h, int a) {
	if (destination == NULL || a==0) return false;

	SDL_Rect src = {0,0,w,h};
	SDL_Rect dest;
	dest.x = x;
	dest.y = y;
	if (a>0 && a!=raw->format->alpha)
		SDL_SetAlpha(raw, SDL_SRCALPHA|SDL_RLEACCEL, a);
	return SDL_BlitSurface(raw, (w==0 || h==0) ? NULL : &src, destination, &dest);
}
bool Surface::blit(Surface *destination, int x, int y, int w, int h, int a) {
	return blit(destination->raw,x,y,w,h,a);
}

bool Surface::blitCenter(SDL_Surface *destination, int x, int y, int w, int h, int a) {
	int oh, ow;
	if (w==0) ow = halfW; else ow = min(halfW,w/2);
	if (h==0) oh = halfH; else oh = min(halfH,h/2);
	return blit(destination,x-ow,y-oh,w,h,a);
}
bool Surface::blitCenter(Surface *destination, int x, int y, int w, int h, int a) {
	return blitCenter(destination->raw,x,y,w,h,a);
}

bool Surface::blitRight(SDL_Surface *destination, int x, int y, int w, int h, int a) {
	if (!w) w = raw->w;
	return blit(destination,x-min(raw->w,w),y,w,h,a);
}
bool Surface::blitRight(Surface *destination, int x, int y, int w, int h, int a) {
	if (!w) w = raw->w;
	return blitRight(destination->raw,x,y,w,h,a);
}

int Surface::box(Sint16 x, Sint16 y, Sint16 w, Sint16 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	return boxRGBA(raw,x,y,x+w-1,y+h-1,r,g,b,a);
}
int Surface::box(Sint16 x, Sint16 y, Sint16 w, Sint16 h, Uint8 r, Uint8 g, Uint8 b) {
	SDL_Rect re = {x,y,w,h};
	return SDL_FillRect(raw, &re, SDL_MapRGBA(raw->format,r,g,b,255));
}
int Surface::box(Sint16 x, Sint16 y, Sint16 w, Sint16 h, RGBAColor c) {
	return box(x,y,w,h,c.r,c.g,c.b,c.a);
}
int Surface::box(SDL_Rect re, RGBAColor c) {
	return boxRGBA(
		raw, re.x, re.y, re.x + re.w - 1, re.y + re.h - 1, c.r, c.g, c.b, c.a
		);
}

int Surface::rectangle(Sint16 x, Sint16 y, Sint16 w, Sint16 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	return rectangleRGBA(raw,x,y,x+w-1,y+h-1,r,g,b,a);
}
int Surface::rectangle(Sint16 x, Sint16 y, Sint16 w, Sint16 h, RGBAColor c) {
	return rectangle(x,y,w,h,c.r,c.g,c.b,c.a);
}
int Surface::rectangle(SDL_Rect re, RGBAColor c) {
	return rectangle(re.x,re.y,re.w,re.h,c.r,c.g,c.b,c.a);
}

int Surface::hline(Sint16 x, Sint16 y, Sint16 w, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	return hlineRGBA(raw,x,x+w-1,y,r,g,b,a);
}

int Surface::line(Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	return lineRGBA(raw,x1,y1,x2,y2,r,g,b,a);
}

int Surface::circle(Sint16 xCenter, Sint16 yCenter, Sint16 radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	return circleRGBA(raw,xCenter,yCenter,radius,r,g,b,a);
}

void Surface::clearClipRect() {
	SDL_SetClipRect(raw,NULL);
}

void Surface::setClipRect(int x, int y, int w, int h) {
	SDL_Rect rect = {x,y,w,h};
	setClipRect(rect);
}

void Surface::setClipRect(SDL_Rect rect) {
	SDL_SetClipRect(raw,&rect);
}

bool Surface::blit(Surface *destination, SDL_Rect container, ASFont::HAlign halign, ASFont::VAlign valign) {
	switch (halign) {
	case ASFont::HAlignLeft:
		break;
	case ASFont::HAlignCenter:
		container.x += container.w/2-halfW;
		break;
	case ASFont::HAlignRight:
		container.x += container.w-raw->w;
		break;
	}

	switch (valign) {
	case ASFont::VAlignTop:
		break;
	case ASFont::VAlignMiddle:
		container.y += container.h/2-halfH;
		break;
	case ASFont::VAlignBottom:
		container.y += container.h-raw->h;
		break;
	}

	return blit(destination,container.x,container.y);
}
