/*
	WebTV

	(c) 2012 by martii


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
#include "filebrowser.h"
#include <stdio.h>
#include <global.h>
#include <libgen.h>
#include <neutrino.h>
#include <driver/screen_max.h>
#include "movieplayer.h"
#include "webtv.h"

CWebTV::CWebTV()
{
	width = w_max (40, 10);
	selected = -1;
	parser = NULL;
}

CWebTV::~CWebTV()
{
	if (parser)
		xmlFreeDoc(parser);
}

int CWebTV::exec(CMenuTarget* parent, const std::string & actionKey)
{
	int res = menu_return::RETURN_REPAINT;

	if(actionKey != "") {
		if(parent)
			parent->hide();
		g_settings.streaming_server_url = actionKey;
		for (std::vector<std::pair<char*, char*> >::iterator i = channels.begin(); i != channels.end(); i++) {
			if (i->second == g_settings.streaming_server_url) {
				g_settings.streaming_server_name = i->first;
				break;
			}
		}
		CMoviePlayerGui::getInstance().exec(NULL, "webtv");
		return res;
	}

	if (parent)
		parent->hide();

	readXml();
	Show();

	return res;
}

void CWebTV::Show()
{
	CMenuWidget* m = new CMenuWidget(LOCALE_WEBTV_HEAD, NEUTRINO_ICON_SETTINGS, width);
	m->addItem(GenericMenuSeparator);
	m->addItem(GenericMenuBack);
	m->addItem(GenericMenuSeparatorLine);

	for (std::vector<std::pair<char*, char*> >::iterator i = channels.begin(); i != channels.end(); i++)
		m->addItem(new CMenuForwarderNonLocalized(i->first, true, NULL, this, i->second),
			!strcmp(i->second, g_settings.streaming_server_url.c_str()));

	m->exec(NULL, "");
	m->hide();
	delete m;
}

bool CWebTV::readXml()
{
	channels.clear();
	if (parser)
		xmlFreeDoc(parser);
	parser = parseXmlFile(g_settings.webtv_xml.c_str());
	if (parser) {
		xmlNodePtr l0 = NULL;
		xmlNodePtr l1 = NULL;
		l0 = xmlDocGetRootElement(parser);
		l1 = l0->xmlChildrenNode;
		if (l1) {
			while ((xmlGetNextOccurence(l1, "webtv"))) {
				char *title = xmlGetAttribute(l1, "title");
				char *url = xmlGetAttribute(l1, "url");

				if (title && url)
					channels.push_back(std::make_pair(title,url));

				l1 = l1->xmlNextNode;
			}
		}
		return true;
	}
	return false;
}
