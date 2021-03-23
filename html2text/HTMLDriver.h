/* Copyright 2020 Fabian Groffen <grobian@gentoo.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License in the file COPYING for more details.
 */

#ifndef HTML_DRIVER_H
#define HTML_DRIVER_H 1

#include "HTMLParser.tab.hh"
#include "HTMLControl.h"
#include "iconvstream.h"

class HTMLDriver {
	public:
		HTMLDriver(HTMLControl &c,
				iconvstream& os_,
				bool& enable_links_,
				int& width_,
				int& mode_,
				bool& debug_parser);

		int parse();
		int lex(html2text::HTMLParser::semantic_type * const lval);
		void yyerror(const char *msg);
		void process(const Document&);
		bool read_cdata(const char *terminal, string *);
		int list_nesting = 0;
		bool enable_links;
		OrderedList *links = nullptr;

		enum {
			PRINT_AS_ASCII, UNPARSE, SYNTAX_CHECK
		};

	private:
		HTMLControl& control;
		bool trace_parsing;
		int width;
		int mode;
		iconvstream& os;

		html2text::HTMLParser::semantic_type *yylval = nullptr;
};

#endif
