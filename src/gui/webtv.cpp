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
#include <gui/filebrowser.h>
#include <stdio.h>
#include <global.h>
#include <libgen.h>
#include <neutrino.h>
#include <driver/screen_max.h>
#include <ctype.h>
#include <gui/movieplayer.h>
#include <gui/webtv.h>
#include <gui/widget/hintbox.h>

CWebTV::CWebTV()
{
	width = w_max (40, 10);
	parser = NULL;
	m = NULL;
	menu_offset = 0;
}

CWebTV::~CWebTV()
{
	if (parser)
		xmlFreeDoc(parser);
}

bool CWebTV::getFile(std::string &file_name, std::string &full_name)
{
	fileSelected = false;
	exec(NULL, "");
	if (fileSelected) {
		file_name = g_settings.streaming_server_name;
		full_name = g_settings.streaming_server_url;
		return true;
	}
	return false;
}

int CWebTV::exec(CMenuTarget* parent, const std::string & actionKey)
{
	int res = menu_return::RETURN_REPAINT;

	if (actionKey == "rc_info") {
		int selected = m->getSelected() - menu_offset;
		if (selected < 0)
			return menu_return::RETURN_NONE;
		ShowHint(channels[selected].name.c_str(), channels[selected].url);
		return res;
	}

	if(!actionKey.empty()) {
		if(parent)
			parent->hide();
		std::string streaming_server_url = actionKey;
		std::string streaming_server_name;

		for (std::vector<web_channel>::iterator i = channels.begin(); i != channels.end(); i++) {
			if (i->url == streaming_server_url) {
				streaming_server_name = i->name;
				break;
			}
		}

		if (g_settings.parentallock_prompt != PARENTALLOCK_PROMPT_NEVER) {
			int age = -1;
			const char *ages[] = { "18+", "16+", "12+", "6+", "0+", NULL };
			int agen[] = { 18, 16, 12, 6, 0 };
			for (int i = 0; ages[i] && age < 0; i++) {
				const char *n = streaming_server_name.c_str();
				char *h = (char *) n;
				while ((age < 0) && (h = strstr(h, ages[i])))
					if ((h == n) || !isdigit(*(h - 1)))
						age = agen[i];
			}
			if (age > -1 && age >= g_settings.parentallock_lockage) {
				CZapProtection zapProtection (g_settings.parentallock_pincode, age);
				if (!zapProtection.check())
					return res;
			}
		}

		g_settings.streaming_server_name = streaming_server_name;
		g_settings.streaming_server_url = streaming_server_url;
		fileSelected = true;
		return menu_return::RETURN_EXIT_ALL;
	}
	if (m)
		return res;

	if (parent)
		parent->hide();

	readXml();
	Show();

	return res;
}

void CWebTV::Show()
{
	m = new CMenuWidget(LOCALE_WEBTV_HEAD, NEUTRINO_ICON_MOVIEPLAYER, width);
	m->addItem(GenericMenuSeparator);
	m->addItem(GenericMenuBack);
	m->addItem(GenericMenuSeparatorLine);
	m->addKey(CRCInput::RC_info, this, "rc_info");
	menu_offset = m->getItemsCount();

	for (std::vector<web_channel>::iterator i = channels.begin(); i != channels.end(); i++)
		m->addItem(new CMenuForwarder(i->name.c_str(), true, NULL, this, i->url),
			!strcmp(i->url, g_settings.streaming_server_url.c_str()));

	m->exec(NULL, "");
	m->hide();
	delete m;
	m = NULL;
}

bool CWebTV::readXml()
{
	if (!channels.empty())
		return true;
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
				char *desc = xmlGetAttribute(l1, "description");

				if (title && url) {
					std::string t = string(title);
					if (desc && strlen(desc))
						t += " (" + string(desc) + ")";
					web_channel w;
					w.url = url;
					w.name = t;
					channels.push_back(w);
				}

				l1 = l1->xmlNextNode;
			}
		}
		return true;
	}
	return false;
}
