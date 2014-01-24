/*
	KernelOptions Menu

	Copyright (C) 2012 martii

	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <config.h>
#include <global.h>
#include <neutrino.h>
#include <sys/stat.h>
#include <system/debug.h>
#include <system/safe_system.h>
#include <gui/widget/messagebox.h>
#include <gui/widget/menue.h>
#include <driver/screen_max.h>
#include <gui/kerneloptions.h>

#define ONOFF_OPTION_COUNT 2
static const CMenuOptionChooser::keyval ONOFF_OPTIONS[ONOFF_OPTION_COUNT] = {
	{ 0, LOCALE_OPTIONS_OFF },
	{ 1, LOCALE_OPTIONS_ON }
};

KernelOptions_Menu::KernelOptions_Menu()
{
	width = w_max (40, 10);
}

int KernelOptions_Menu::exec(CMenuTarget* parent, const std::string & actionKey)
{
	int res = menu_return::RETURN_REPAINT;

	if (actionKey == "reset") {
		for (unsigned int i = 0; i < modules.size(); i++)
			modules[i].active = modules[i].active_orig;
		return res;
	}

	if (actionKey == "apply" || actionKey == "change") {
		bool needs_save = false;
		for (unsigned int i = 0; i < modules.size(); i++) {
			if (modules[i].active != modules[i].active_orig) {
				needs_save = true;
				char buf[80];
				if (modules[i].active)
					for (unsigned int j = 0; j < modules[i].moduleList.size(); j++) {
						snprintf(buf, sizeof(buf), "insmod /lib/modules/%s.ko %s",
						//snprintf(buf, sizeof(buf), "modprobe %s %s",
							modules[i].moduleList[j].first.c_str(), modules[i].moduleList[j].second.c_str());
						system(buf);
					}
				else
					for (unsigned int j = modules[i].moduleList.size(); j > 0; j--) {
						snprintf(buf, sizeof(buf), "rmmod %s", modules[i].moduleList[j - 1].first.c_str());
						system(buf);
					}
				modules[i].active_orig = modules[i].active;
				//break;  // why ?
			}
		}
		if (needs_save)
			save();
		if (actionKey == "change")
			return res; // whatever
	}

	if (actionKey == "apply" || actionKey == "lsmod") {
		for (unsigned int i = 0; i < modules.size(); i++)
			modules[i].installed = false;
		FILE *f = fopen("/proc/modules", "r");
		if (f) {
			char buf[200];
			while (fgets(buf, sizeof(buf), f)) {
				char name[sizeof(buf)];
				if (1 == sscanf(buf, "%s", name))
					for (unsigned int i = 0; i < modules.size(); i++) {
						if (name == modules[i].moduleList.back().first) {
							modules[i].installed = true;
							break;
						}
				}
			}
			fclose(f);
		}

		string text = "";
		for (unsigned int i = 0; i < modules.size(); i++) {
			text += modules[i].comment + " (" + modules[i].moduleList.back().first + "): ";
			text += g_Locale->getText(modules[i].active ? LOCALE_KERNELOPTIONS_ENABLED : LOCALE_KERNELOPTIONS_DISABLED);
			text += ", ";
			text += g_Locale->getText(modules[i].installed ? LOCALE_KERNELOPTIONS_LOADED : LOCALE_KERNELOPTIONS_NOT_LOADED);
			text += "\n";
		}

		ShowMsg(LOCALE_KERNELOPTIONS_LSMOD, text, CMessageBox::mbrBack, CMessageBox::mbBack, NEUTRINO_ICON_INFO);

		return res;
	}

	if (parent)
		parent->hide();

	Settings();

	return res;
}

void KernelOptions_Menu::hide()
{
}

bool KernelOptions_Menu::isEnabled(string name) {
	load();
	for (unsigned int i = 0; i < modules.size(); i++)
		if (name == modules[i].moduleList.back().first)
			return modules[i].active;
	return false;
}

bool KernelOptions_Menu::Enable(string name, bool active) {
	load();
	for (unsigned int i = 0; i < modules.size(); i++)
		if (name == modules[i].moduleList.back().first) {
				if (modules[i].active != active) {
					modules[i].active = active;
					exec(NULL, "change");
				}
				return true;
		}
	return false;
}

void KernelOptions_Menu::load() {
	modules.clear();

	FILE *f = fopen("/etc/modules.available", "r");
	// Syntax:
	//
	// # comment
	// module # description
	// module module module # description
	// module module(arguments) module # description
	//

	if (f) {
		char buf[200];
		while (fgets(buf, sizeof(buf), f)) {
			if (buf[0] == '#')
				continue;
			char *comment = strchr(buf, '#');
			if (!comment)
				continue;
			*comment++ = 0;
			while (*comment == ' ' || *comment == '\t')
				comment++;
			if (strlen(comment) < 1)
				continue;
			module m;
			m.active = m.active_orig = 0;
			m.installed = false;
			char *nl = strchr(comment, '\n');
			if (nl)
				*nl = 0;
			m.comment = string(comment);
			char *b = buf;
			while (*b) {
				if (*b == ' ' || *b == '\t') {
					b++;
					continue;
				}
				string args = "";
				string mod;
				char *e = b;
				char *a = NULL;
				while (*e && ((a && *e != ')') || (!a && *e != ' ' && *e != '\t'))) {
					if (*e == '(')
						a = e;
					e++;
				}
				if (a && *e == ')') {
					*a++ = 0;
					*e++ = 0;
					args = string (a);
					*a = 0;
					mod = string(b);
					b = e;
				} else if (*e) {
					*e++ = 0;
					mod = string(b);
					b = e;
				} else {
					mod = string(b);
					b = e;
				}
				m.moduleList.push_back(make_pair(mod, args));
			}
			if (m.moduleList.size() > 0)
				modules.push_back(m);
		}
		fclose(f);
	}

	f = fopen("/etc/modules.extra", "r");
	if (f) {
		char buf[200];
		while (fgets(buf, sizeof(buf), f)) {
			char *t = strchr(buf, '#');
			if (t)
				*t = 0;
			char name[200];
			if (1 == sscanf(buf, "%s", name)) {
				for (unsigned int i = 0; i < modules.size(); i++)
					if (modules[i].moduleList.back().first == name) {
						modules[i].active = modules[i].active_orig = 1;
						break;
					}
			}
		}
		fclose(f);
	}
}

void KernelOptions_Menu::save()
{
	FILE *f = fopen("/etc/modules.extra", "w");
	if (f) {
		chmod("/etc/modules.extra", 0644);
		for (unsigned int i = 0; i < modules.size(); i++) {
			if (modules[i].active) {
				for (unsigned int j = 0; j < modules[i].moduleList.size(); j++)
					if (modules[i].moduleList[j].second.length())
						fprintf(f, "%s %s\n",
							modules[i].moduleList[j].first.c_str(),
							modules[i].moduleList[j].second.c_str());
					else
						fprintf(f, "%s\n",
							modules[i].moduleList[j].first.c_str());
			}
		}
		fclose(f);
	}
}

void KernelOptions_Menu::Settings()
{
	CMenuWidget* menu = new CMenuWidget(LOCALE_KERNELOPTIONS_HEAD, "settings");
	menu->addItem(GenericMenuSeparator);
	menu->addItem(GenericMenuBack);
	menu->addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_KERNELOPTIONS_MODULES));

	load();

	for (unsigned int i = 0; i < modules.size(); i++)
		menu->addItem(new CMenuOptionChooser(modules[i].comment.c_str(), &modules[i].active,
				ONOFF_OPTIONS, ONOFF_OPTION_COUNT, true));

	menu->addItem(GenericMenuSeparatorLine);

	menu->addItem(new CMenuForwarder(LOCALE_KERNELOPTIONS_RESET, true, "", this,
		"reset", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	menu->addItem(new CMenuForwarder(LOCALE_KERNELOPTIONS_APPLY, true, "", this,
		"apply", CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN));
	menu->addItem(new CMenuForwarder(LOCALE_KERNELOPTIONS_LSMOD, true, "", this,
		"lsmod", CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW));
	menu->exec (NULL, "");
	menu->hide ();
	delete menu;
}
// vim:ts=4
