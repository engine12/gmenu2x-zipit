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

#ifndef LINKAPP_H
#define LINKAPP_H

#include "link.h"

#include <string>

class GMenu2X;
class InputManager;
class Touchscreen;

/**
Parses links files.

	@author Massimiliano Torromeo <massimiliano.torromeo@gmail.com>
*/
class LinkApp : public Link {
private:
	InputManager &inputMgr;
	std::string sclock;
	int iclock;
	std::string exec, params, workdir, manual, selectordir, selectorfilter, selectorscreens;
	bool selectorbrowser, useRamTimings;
	void drawRun();

	std::string aliasfile;
	std::string file;

	bool wrapper;
	bool dontleave;
	bool newTerm;

	void launch(
			const std::string &selectedFile = "",
			const std::string &selectedDir = "");

public:
	LinkApp(GMenu2X *gmenu2x, Touchscreen &ts, InputManager &inputMgr,
			const char* linkfile);
	virtual const std::string &searchIcon();
	virtual void run();

	const std::string &getExec();
	void setExec(const std::string &exec);
	const std::string &getParams();
	void setParams(const std::string &params);
	const std::string &getManual();
	void setManual(const std::string &manual);
	const std::string &getSelectorDir();
	void setSelectorDir(const std::string &selectordir);
	bool getSelectorBrowser();
	void setSelectorBrowser(bool value);
	bool getUseRamTimings();
	void setUseRamTimings(bool value);
	const std::string &getSelectorScreens();
	void setSelectorScreens(const std::string &selectorscreens);
	const std::string &getSelectorFilter();
	void setSelectorFilter(const std::string &selectorfilter);
	const std::string &getAliasFile();
	void setAliasFile(const std::string &aliasfile);

	int clock();
	const std::string &clockStr(int maxClock);
	void setClock(int mhz);

	bool save();
	void showManual();
	void selector(int startSelection=0, const std::string &selectorDir="");
	bool targetExists();

	const std::string &getFile() { return file; }
	void renameFile(const std::string &name);
	bool &needsWrapperRef() { return wrapper; }
	bool &runsInBackgroundRef() { return dontleave; }
	bool &runsOnNewTerm() { return newTerm; }
};

#endif
