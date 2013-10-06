/*
	FileBrowser Setup

	(c) 2013 by martii


	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#define __USE_FILE_OFFSET64 1
#include <stdio.h>
#include <global.h>
#include <neutrino.h>
#include <driver/screen_max.h>
#include "mymenu.h"
#include "filebrowser_setup.h"

CFileBrowserSetup::CFileBrowserSetup()
{
	width = w_max (40, 10);
	selected = -1;
}


int CFileBrowserSetup::exec(CMenuTarget* parent, const std::string & /*actionKey*/)
{
	int res = menu_return::RETURN_REPAINT;

	if (parent)
		parent->hide();

	Show();

	return res;
}

void CFileBrowserSetup::Show()
{
	int shortcut = 1;

	CMenuWidget m(LOCALE_MOVIEPLAYER_FILEPLAYBACK, NEUTRINO_ICON_MOVIEPLAYER, width);
	m.addIntroItems(LOCALE_EPGPLUS_OPTIONS);
	m.setSelected(selected);
	int multi_select = g_settings.filebrowser_multi_select;
	m.addItem(new CMenuOptionChooser(LOCALE_FILEBROWSER_MULTI_SELECT, &multi_select,
				OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, NULL,
				CRCInput::convertDigitToKey(shortcut++)));
	m.exec(NULL, "");
	m.hide();
	g_settings.filebrowser_multi_select = multi_select;
}
// vim:ts=4
