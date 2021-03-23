
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
 * Fre Jun  8 17:49:39 CEST 2001: new method
 * Thu Oct  4 21:36:51 CEST 2001: ported to g++ 3.0
 * Wed Jul  2 22:05:32 CEST 2003: ported to g++ 3.3
 */

/***************************************************************************/


#ifndef __format_h_INCLUDED__ /* { */
#define __format_h_INCLUDED__

/* ------------------------------------------------------------------------- */

#include <string>
#include <vector>
#include <istream>

using std::string;
using std::vector;
using std::istream;


/* ------------------------------------------------------------------------- */

class Formatting {
public:
static void           setProperty(const char *key, const char *value);
static void           loadProperties(istream &is);

static const char     *getString(const char *key, const char *dflt);
// neue Methode fuer leere Attribute - Johannes Geiger
static const char     *getString(const char *key);
static vector<string> *getStringVector(const char *key, const char *dflt);
static int            getInt(const char *key, int dflt);
static vector<int> *  getIntVector(const char *key, const char *dflt);
static char           getAttributes(const char *key, char dflt);

private:
Formatting();      // Do not instantiate me!
};

/* ------------------------------------------------------------------------- */

#endif /* } */

/* ------------------------------------------------------------------------- */

