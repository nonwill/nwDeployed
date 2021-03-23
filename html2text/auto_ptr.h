
 /***************************************************************************/

/*
 * Portions Copyright (c) 1999 GMRS Software GmbH
 * Carl-von-Linde-Str. 38, D-85716 Unterschleissheim, http://www.gmrs.de
 * All rights reserved.
 *
 * Author: Arno Unkrig <arno@unkrig.de>
 *
 * All advertising materials mentioning features or use of this software
 * must display the following acknowledgement:
 * "This product includes software developed by GMRS Software GmbH."
 * The name of GMRS Software GmbH may not be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
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
 * Thu Oct  4 22:44:17 CEST 2001: ported to g++ 3.0
 */
  
 /***************************************************************************/


#ifndef __auto_ptr_h_INCLUDED__ /* { */
#define __auto_ptr_h_INCLUDED__

/* ------------------------------------------------------------------------- */

/*
 * Yet another implementation of the "auto_ptr" template... I am not sure
 * if the standard does specify "auto_ptr", and how, but this implementation
 * uses a scheme *without* an "owns" flag: When the ownership is taken away
 * from the "auto_ptr", its pointer *is set to 0*! This may seem odd, but
 * in practice, it saves you from problems because such an "auto_ptr" can
 * never be dangling, only "0", which is checked in "operator*()" and
 * "operator->()". I never found this scheme limiting.
 */

/* ------------------------------------------------------------------------- */

#include <stdlib.h>

template <class T>
class auto_ptr {

public:

  // Constructor/copy/destroy

  explicit auto_ptr(T *x = 0) : p(x) {}
  auto_ptr(const auto_ptr<T> &x) : p(x.p) { ((auto_ptr<T> *) &x)->p = 0; }
  void operator=(const auto_ptr<T> &x)
  { delete p; p = x.p; ((auto_ptr<T> *) &x)->p = 0; }
  // Would be a nice extension, but is not portable; use "reset()" instead:
//void operator=(T *x); { delete p; p = x; }
  ~auto_ptr() { delete p; }

  // Members

  T    &operator*() const  { if (!p) abort(); return *(T *) p; }
  T    *operator->() const { if (!p) abort(); return (T *) p; }
  T    *get() const        { return (T *) p; }
  T    *release()          { T *tmp = p; p = 0; return tmp; }
  void reset(T *x = 0)     { delete p; p = x; }

  // These would make a nice extension, but are not provided by many other
  // implementations.
//operator const void *() const { return p; }
//int operator!() const { return p == 0; }

private:
  T *p;
};

/* ------------------------------------------------------------------------- */

#endif /* } */

/* ------------------------------------------------------------------------- */

