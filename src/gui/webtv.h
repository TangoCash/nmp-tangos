/*
	WebTV menue

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

#ifndef __webtv_setup_h__
#define __webtv_setup_h__

#include <sys/types.h>
#include <string.h>
#include <vector>
#include <xmltree/xmlinterface.h>

class CWebTV : public CMenuTarget
{
	private:
		int width;
		bool fileSelected;
		xmlDocPtr parser;
		bool readXml();
		struct web_channel {
			char *url;
			std::string name;
		};
		std::vector<web_channel> channels;
		CMenuWidget* m;
		int menu_offset;
	public:
		CWebTV();
		~CWebTV();
		void Show();
		int exec(CMenuTarget* parent, const std::string & actionKey);
		bool getFile(std::string &file_name, std::string &full_name);
};
#endif
