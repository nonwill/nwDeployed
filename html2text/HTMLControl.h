
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
 * Thu Oct  4 21:25:07 CEST 2001: ported to g++ 3.0
 * Wed Jul  2 22:01:12 CEST 2003: ported to g++ 3.3
 */

/***************************************************************************/


#ifndef __HTMLControl_h_INCLUDED__ /* { */
#define __HTMLControl_h_INCLUDED__

/* ------------------------------------------------------------------------- */

#include "iconvstream.h"
#include <istream>

#include "HTMLParser.tab.hh"

using std::istream;

/* ------------------------------------------------------------------------- */

class HTMLControl {
	public:
		HTMLControl(iconvstream& is_,
				int& mode_,
				bool debug_scanner_,
				const char *file_name_) :
			current_line(1),
			current_column(0),
			literal_mode(false),
			next_token(EOF),
			debug_scanner(debug_scanner_),
			is(is_),
			number_of_ungotten_chars(0),
			file_name(file_name_),
			mode(mode_)
	{
	}

		void htmlparser_yyerror(const char *p);
		int htmlparser_yylex(
				html2text::HTMLParser::semantic_type *value_return);
		bool read_cdata(const char *terminal, string *value_return);
		int mode;
		int current_line;
		int current_column;
		const char *file_name;

	private:

		/*
		 * Helpers.
		 */
		int yylex2(html2text::HTMLParser::semantic_type *value_return, int *tag_type_return);
		bool literal_mode;
		int next_token;
		html2text::HTMLParser::semantic_type next_token_value;
		int next_token_tag_type;

		int get_char();
		void unget_char(int);

		bool debug_scanner;

		iconvstream &is;
		int ungotten_chars[5];
		int number_of_ungotten_chars;
};

/* ------------------------------------------------------------------------- */

#endif /* } */

/* ------------------------------------------------------------------------- */

