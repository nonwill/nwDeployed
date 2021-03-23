
/***************************************************************************/

/*
 * Portions Copyright (c) 1999 GMRS Software GmbH
 * Carl-von-Linde-Str. 38, D-85716 Unterschleissheim, http://www.gmrs.de
 * All rights reserved.
 *
 * Author: Arno Unkrig <arno@unkrig.de>
 */

/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License in the file COPYING for more details.
 */

/***************************************************************************/

/*
 * Changes to version 1.2.2 were made by Martin Bayer <mbayer@zedat.fu-berlin.de>
 * Dates and reasons of modifications:
 * Fre Jun  8 17:24:35 CEST 2001: new method
 * Wed Jul  2 22:02:51 CEST 2003: ported to g++ 3.3
 */

/***************************************************************************/


#include <ctype.h>
#include <iostream>

#include "Properties.h"

/* ------------------------------------------------------------------------- */

const char *
Properties::getProperty(const char *key, const char *dflt) const
{
	map<string, string>::const_iterator i;
	i = property_map.find(key);
	return i == property_map.end() ? dflt : (*i).second.c_str();
}

// neue Methode fuer leere Attribute - Johannes Geiger

const char *
Properties::getProperty(const char *key) const
{
	map<string, string>::const_iterator i;
	i = property_map.find(key);
	return i == property_map.end() ? NULL : (*i).second.c_str();
}

/* ------------------------------------------------------------------------- */

void
Properties::load(istream &is)
{
	string key, value;
	while (readProperty(is, &key, &value))
		setProperty(key, value);
}

/* ------------------------------------------------------------------------- */

/*
 * Expand the escape sequence at "line[pos]". Backslash-Newline reads another
 * line from "is".
 */
static void
expandEscape(string *line_in_out, string::size_type *pos_in_out, istream &is)
{
	for (;;) {
		if (line_in_out->at(*pos_in_out) != '\\') {
			++*pos_in_out;
			return;
		}
		if (*pos_in_out != line_in_out->size() - 1)
			break;

		string tmp;
		if (!getline(is, tmp)) {
			++*pos_in_out;
			return;
		}
		int j;
		for (j = 0; j < tmp.size() && isspace(tmp[j]); ++j)
			;
		line_in_out->replace(
				*pos_in_out, string::npos,
				tmp, j, string::npos
				);
	}

	char c = line_in_out->at(*pos_in_out + 1);
	switch (c) {
	case 't': c = '\t';
		break;
	case 'n': c = '\n';
		break;
	case 'r': c = '\r';
		break;
	}
	line_in_out->replace(*pos_in_out, 2, &c, 1);
	++*pos_in_out;
}

/*static*/ bool
Properties::readProperty(istream &is, string *key_return, string *value_return)
{
	string line;
	string::size_type i, l;

	// Skip empty and comment lines.
	for (;;) {
		if (!getline(is, line))
			return false;
		l = line.size();

		// Skip leading white-space.
		for (i = 0; i < l && isspace(line[i]); ++i)
			;
		if (i == l)
			continue;                    // Line contains only white-space.

		// Ignore comment lines.
		if (line[i] == '#' || line[i] == '!')
			continue;                               // Comment line.

		break;
	}

	// Parse key.
	string::size_type bok = i;
	while (i < line.size()) {
		char c = line[i];
		if (isspace(c) || c == '=' || c == ':')
			break;
		expandEscape(&line, &i, is);
	}
	string::size_type eok = i;

	// Skip key terminator.
	while (i < l && isspace(line[i]))
		++i;
	if (i < l && (line[i] == '=' || line[i] == ':')) {
		for (++i; i < l && isspace(line[i]); ++i)
			;
	}

	// Substitute escape sequences in value.
	string::size_type bov = i;
	while (i < line.size())
		expandEscape(&line, &i, is);

	// Return key and value.
	key_return->assign(line, bok, eok - bok);
	value_return->assign(line, bov, string::npos);
	return true;
}

/* ------------------------------------------------------------------------- */

