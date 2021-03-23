
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
 * Fre Jun  8 19:00:26 CEST 2001: new image handling
 * Thu Oct  4 21:42:24 CEST 2001: ported to g++ 3.0, bugfix for '-' as synonym for STDIN
 * Mon Jul 22 13:48:26 CEST 2002: Made finaly reading from STDIN work.
 * Sat Sep 14 15:04:09 CEST 2002: Added plain ASCII output patch by Bela Lubkin
 * Wed Jul  2 22:08:45 CEST 2003: ported to g++ 3.3
 */

/***************************************************************************/


#include <iostream>
#include <string.h>
#include <stdlib.h>

#include "html.h"
#include "HTMLControl.h"
#include "HTMLDriver.h"
#include "iconvstream.h"
#include "format.h"

#define stringify(x) stringify2(x)
#define stringify2(x) #x

/* ------------------------------------------------------------------------- */

static const char *usage = "\
Usage:\n\
  html2text -help\n\
  html2text -version\n\
  html2text [ -unparse | -check ] [ -debug-scanner ] [ -debug-parser ] \\\n\
     [ -rcfile <file> ] [ -width <w> ] [ -nobs ] [ -links ]\\\n\
     [ -from_encoding ] [ -to_encoding ] [ -ascii ]\\\n\
     [ -o <file> ] [ <input-file> ] ...\n\
Formats HTML document(s) read from <input-file> or STDIN and generates ASCII\n\
text.\n\
  -help          Print this text and exit\n\
  -version       Print program version and copyright notice\n\
  -unparse       Generate HTML instead of ASCII output\n\
  -check         Do syntax checking only\n\
  -debug-scanner Report parsed tokens on STDERR (debugging)\n\
  -debug-parser  Report parser activity on STDERR (debugging)\n\
  -rcfile <file> Read <file> instead of \"$HOME/.html2textrc\"\n\
  -width <w>     Optimize for screen widths other than 79\n\
  -nobs          Do not render boldface and underlining (using backspaces)\n\
  -links         Generate reference list with link targets\n\
  -from_encoding Treat input encoded as given encoding\n\
  -to_encoding   Output using given encoding\n\
  -ascii         Use plain ASCII for output instead of UTF-8\n\
                 alias for: -to_encoding ASCII//TRANSLIT \n\
  -o <file>      Redirect output into <file>\n\
";

int
main(int argc, char **argv)
{
	if (argc == 2 && !strcmp(argv[1], "-help")) {
		std::cout
			<< "This is html2text, version " stringify(VERSION) << std::endl
			<< std::endl
			<< usage;
		exit(0);
	}

	if (argc == 2 && !strcmp(argv[1], "-version")) {
		std::cout
			<< "This is html2text, version " stringify(VERSION) << std::endl
			<< std::endl
			<< "The latest version can be found at "
			<< "https://github.com/grobian/html2text" << std::endl
			<< std::endl
			<< "This program is distributed in the hope that it will "
			<< "be useful, but WITHOUT" << std::endl
			<< "ANY WARRANTY; without even the implied warranty of "
			<< "MERCHANTABILITY or FITNESS" << std::endl
			<< "FOR A PARTICULAR PURPOSE. See the GNU General Public "
			<< "License for more details." << std::endl
			<< std::endl;
		exit(0);
	}

	int mode = HTMLDriver::PRINT_AS_ASCII;
	bool debug_scanner = false;
	bool debug_parser = false;
	const char *home = getenv("HOME");
	const char *rcfile = NULL;
	int width = 79;
	const char *output_file_name = "-";
	bool use_backspaces = true;
	bool enable_links = false;
	const char *from_encoding = NULL;
	const char *to_encoding = NULL;
	const char *widthstr = NULL;
	const char **extarg = NULL;

	int i;
	for (i = 1; i < argc && argv[i][0] == '-' && argv[i][1]; i++) {
		const char *arg = argv[i];

		if (!strcmp(arg, "-unparse")) {
			mode = HTMLDriver::UNPARSE;
		} else if (!strcmp(arg, "-check")) {
			mode = HTMLDriver::SYNTAX_CHECK;
		} else if (!strcmp(arg, "-debug-scanner")) {
			debug_scanner = true;
		} else if (!strcmp(arg, "-debug-parser")) {
			debug_parser = true;
		} else if (!strcmp(arg, "-rcfile")) {
			extarg = &rcfile;
		} else if (!strcmp(arg, "-links")) {
			enable_links = true;
		} else if (!strcmp(arg, "-width")) {
			extarg = &widthstr;
		} else if (!strcmp(arg, "-o")) {
			extarg = &output_file_name;
		} else if (!strcmp(arg, "-nobs")) {
			use_backspaces = false;
		} else if (!strcmp(arg, "-from_encoding")) {
			extarg = &from_encoding;
		} else if (!strcmp(arg, "-to_encoding")) {
			extarg = &to_encoding;
		} else if (!strcmp(arg, "-ascii")) {
			to_encoding = "ASCII//TRANSLIT";  /* create things like (c) */
		} else {
			std::cerr
				<< "Unrecognized command line option \""
				<< arg
				<< "\", try \"-help\"."
				<< std::endl;
			exit(1);
		}

		if (extarg != NULL) {
			if (++i >= argc) {
				std::cerr
					<< "Option \"" << arg << "\" needs an additional "
					<< "argument."
					<< std::endl;
				exit(1);
			}
			*extarg = argv[i];

			/* handle the only integer argument inline */
			if (extarg == &widthstr) {
				int nwidth = atoi(widthstr);
				if (nwidth > 10) {
					width = nwidth;
				} else {
					std::cerr
						<< "width '" << nwidth << "' invalid, must be >10"
						<< std::endl;
					exit(1);
				}
			}
			extarg = NULL;
		}
	}
	if (i > argc) {
		std::cerr
			<< "Error: Required parameter after \""
			<< argv[argc - 1]
			<< "\" missing."
			<< std::endl;
		exit(1);
	}

	/* historical default */
	if (from_encoding == NULL)
		from_encoding = "ISO-8859-1";

	/* this is probably the output we want on 99% of all terminals */
	if (to_encoding == NULL)
		to_encoding = "UTF-8";

	const char *const *input_files;
	int number_of_input_files;

	if (i >= argc) {
		static const char *const x = "-";
		input_files = &x;
		number_of_input_files = 1;
	} else {
		input_files = argv + i;
		number_of_input_files = argc - i;
	}

	{
		std::ifstream ifs;

		if (rcfile == NULL && home != NULL)
			rcfile = (string(home) + "/.html2textrc").c_str();
		if (rcfile != NULL)
			ifs.open(rcfile);
		if (rcfile == NULL || !ifs.rdbuf()->is_open())
			ifs.open("/etc/html2textrc");
		if (ifs.rdbuf()->is_open())
			Formatting::loadProperties(ifs);
	}

	/*
	 * Set up printing.
	 */
	Area::use_backspaces = use_backspaces;

	iconvstream is;

	is.open_os(output_file_name, to_encoding);
	if (!is.os_open()) {
		std::cerr
			<< "Could not open output file \""
			<< output_file_name
			<< "\": "
			<< is.open_error_msg()
			<< std::endl;
		exit(1);
	}

	for (i = 0; i < number_of_input_files; ++i) {
		const char *input_file = input_files[i];

		if (number_of_input_files != 1)
			is << "###### " << input_file << " ######" << endl;

		is.open_is(input_file, from_encoding);
		if (!is.is_open()) {
			std::cerr
				<< "Opening input file \""
				<< input_file
				<< "\": "
				<< is.open_error_msg()
				<< std::endl;
			exit(1);
		}

		HTMLControl control(is, mode, debug_scanner, input_file);
		HTMLDriver driver(control, is, enable_links,
				width, mode, debug_parser);

		if (driver.parse() != 0)
			exit(1);
	}

	is.close();

	return 0;
}

/* ------------------------------------------------------------------------- */
