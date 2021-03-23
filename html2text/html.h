
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
 * Fre Jun  8 17:46:31 CEST 2001: new method
 * Thu Oct  4 21:38:47 CEST 2001: ported to g++ 3.0
 */

/***************************************************************************/


#ifndef __html_h_INCLUDED__ /* { */
#define __html_h_INCLUDED__

/* ------------------------------------------------------------------------- */


#include <string>
#include <list>
#ifdef AUTO_PTR_BROKEN /* { */
#  define auto_ptr broken_auto_ptr
#  include <memory>
#  undef auto_ptr
#  include "auto_ptr.h"
#else /* } { */
#  include <memory>
#endif /* } */
#include <utility>

#include "Area.h"
#include "iconvstream.h"
#include "istr.h"

/* ------------------------------------------------------------------------- */

using std::string;
using std::pair;
using std::list;

typedef pair<string, istr> TagAttribute;

istr get_attribute(
	const list<TagAttribute> *, const char *name, const char *dflt
	);
istr get_attribute(
	const list<TagAttribute> *, const char *name, bool *exists
	);
int get_attribute(
	const list<TagAttribute> *, const char *name, int dflt
	);
int get_attribute(
	const list<TagAttribute> *, const char *name, int dflt,
	const char *s1, int v1, ... /* ... NULL */
	);
int get_attribute(
	const list<TagAttribute> *, const char *name, const char *dflt1, int dflt2,
	const char *s1, int v1, ... /* ... NULL */
	);

istr get_style_attr(istr *style, const char *name, const char *dflt);

/* ------------------------------------------------------------------------- */

typedef char ostream_manipulator;

struct Element {
	virtual ~Element();
	virtual void unparse(iconvstream&, ostream_manipulator separator) const = 0;

	/*
	 * Attempt to line-format the element. If the element contains "Block"s,
	 * then it cannot be line-formatted, and 0 will be returned. However, it
	 * is still possible to try "format()" (see below).
	 */
	virtual Line *line_format() const
	{
		return 0;
	}

	/*
	 * Format the element into a rectangular area. Attempt to not exceed
	 * "width".
	 */
	virtual Area *format(
		Area::size_type /*width*/,
		int         /*halign*/
		) const
	{
		return 0;
	}

	virtual struct PCData *to_PCData()
	{
		return 0;
	}
};

/* ------------------------------------------------------------------------- */

struct PCData : public Element {
	istr text;

	/*virtual*/ PCData *to_PCData()
	{
		return this;
	}

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Line *line_format() const;
};

struct Font : public Element {
	int attribute;                               // TT I B U STRIKE BIG SMALL
	// SUB SUP
	auto_ptr<list<auto_ptr<Element> > > texts;

	Font(int a, list<auto_ptr<Element> > *t = 0) : attribute(a), texts(t)
	{}
	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Line *line_format() const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

struct Phrase : public Element {
	int attribute;                               // EM STRONG DFN CODE SAMP
	// KBD VAR CITE
	auto_ptr<list<auto_ptr<Element> > > texts;

	Phrase(int a, list<auto_ptr<Element> > *t = 0) : attribute(a), texts(t)
	{}
	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Line *line_format() const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

struct Font2 : public Element {
	auto_ptr<list<TagAttribute> >       attributes;// SIZE COLOR
	auto_ptr<list<auto_ptr<Element> > > elements;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Line *line_format() const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

struct Anchor : public Element {
	auto_ptr<list<TagAttribute> >       attributes;// NAME HREF REL REV TITLE
	auto_ptr<list<auto_ptr<Element> > > texts;
	mutable int                         refnum;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Line *line_format() const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

struct BaseFont : public Element {
	auto_ptr<list<TagAttribute> > attributes; // SIZE

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
};

struct LineBreak : public Element {
	auto_ptr<list<TagAttribute> > attributes; // CLEAR

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Line *line_format() const;
};

struct Map : public Element {
	auto_ptr<list<TagAttribute> >                   attributes;// NAME
	auto_ptr<list<auto_ptr<list<TagAttribute> > > > areas;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
};

struct Paragraph : public Element {
	auto_ptr<list<TagAttribute> >       attributes;// ALIGN
	auto_ptr<list<auto_ptr<Element> > > texts;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

struct Image : public Element {
	auto_ptr<list<TagAttribute> > attributes; // SRC ALT ALIGN WIDTH HEIGHT
	// BORDER HSPACE VSPACE USEMAP
	// ISMAP

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Line *line_format() const;
};

struct Applet : public Element {
	auto_ptr<list<TagAttribute> >       attributes;// CODEBASE CODE ALT NAME
	// WIDTH HEIGHT ALIGN HSPACE
	// VSPACE
	auto_ptr<list<auto_ptr<Element> > > content;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Line *line_format() const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

struct Param : public Element {
	auto_ptr<list<TagAttribute> > attributes; // NAME VALUE

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
};

struct Division : public Element {
	auto_ptr<list<TagAttribute> >       attributes;// ALIGN
	auto_ptr<list<auto_ptr<Element> > > body_content;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

struct Center : public Element {
	// No attributes specified for <CENTER>!
	auto_ptr<list<auto_ptr<Element> > > body_content;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

struct BlockQuote : public Element {
	// No attributes specified for <BLOCKQUOTE>!
	auto_ptr<list<auto_ptr<Element> > > content;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

struct Address : public Element {
	// No attributes specified for <ADDRESS>!
	auto_ptr<list<auto_ptr<Element> > > content;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

/* ------------------------------------------------------------------------- */

struct Form : public Element {
	auto_ptr<list<TagAttribute> >       attributes;// ACTION METHOD ENCTYPE
	auto_ptr<list<auto_ptr<Element> > > content;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

struct Input : public Element {
	auto_ptr<list<TagAttribute> > attributes; // TYPE NAME VALUE CHECKED SIZE
	// MAXLENGTH SRC ALIGN

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Line *line_format() const;
};

struct Option {
	auto_ptr<list<TagAttribute> > attributes; // SELECTED VALUE
	auto_ptr<PCData>              pcdata;

	void unparse(iconvstream&, ostream_manipulator separator) const;
};

struct Select : public Element {
	auto_ptr<list<TagAttribute> >      attributes;// NAME SIZE MULTIPLE
	auto_ptr<list<auto_ptr<Option> > > content;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Line *line_format() const;
};

struct TextArea : public Element {
	auto_ptr<list<TagAttribute> > attributes; // NAME ROWS COLS
	auto_ptr<PCData>              pcdata;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

/* ------------------------------------------------------------------------- */

struct Preformatted : public Element {
	auto_ptr<list<TagAttribute> >       attributes;// WIDTH
	auto_ptr<list<auto_ptr<Element> > > texts;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

struct Body {
	auto_ptr<list<TagAttribute> >       attributes;// BACKGROUND BGCOLOR TEXT
	// LINK VLINK ALINK
	auto_ptr<list<auto_ptr<Element> > > content;

	virtual ~Body()
	{}
	virtual void unparse(iconvstream&, ostream_manipulator separator) const;
	virtual Area *format(Area::size_type w, int halign) const;
	void format(
		Area::size_type indent_left,
		Area::size_type w,
		int halign,
		iconvstream& os
		) const;
};

struct Script {
	auto_ptr<list<TagAttribute> > attributes; // LANGUAGE, ???
	string text;

	void unparse(iconvstream&, ostream_manipulator separator) const;
};

struct Style {
	auto_ptr<list<TagAttribute> > attributes; // ???
	string text;

	void unparse(iconvstream&, ostream_manipulator separator) const;
};

struct Meta {
	auto_ptr<list<TagAttribute> > attributes;    // HTTP-EQUIV NAME CONTENT

	void unparse(iconvstream&, ostream_manipulator separator) const;
};

struct Head {
	auto_ptr<PCData>              title;
	auto_ptr<list<TagAttribute> > isindex_attributes; // PROMPT
	auto_ptr<list<TagAttribute> > base_attributes;  // HREF
	list<auto_ptr<Script> >       scripts;
	list<auto_ptr<Style> >        styles;
	list<auto_ptr<Meta> >         metas;
	auto_ptr<list<TagAttribute> > link_attributes;  // HREF REL REV TITLE

	void unparse(iconvstream&, ostream_manipulator separator) const;
};

struct Document {
	auto_ptr<list<TagAttribute> > attributes; // VERSION
	Head head;
	Body body;

	void unparse(iconvstream&, ostream_manipulator separator) const;
	Area *format(Area::size_type w, int halign) const;
	void format(
		Area::size_type indent_left,
		Area::size_type w,
		int halign,
		iconvstream& os
		) const;
};

struct Heading : public Element {
	int level;
	auto_ptr<list<TagAttribute> >       attributes;// ALIGN
	auto_ptr<list<auto_ptr<Element> > > content;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

struct TableCell : public Body {
	/*virtual*/
	void unparse(iconvstream&, ostream_manipulator separator) const;
};

struct TableHeadingCell : public TableCell {
	/*virtual*/
	void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

struct TableRow {
	auto_ptr<list<TagAttribute> >         attributes;// ALIGN VALIGN
	auto_ptr<list<auto_ptr<TableCell> > > cells;

	void unparse(iconvstream&, ostream_manipulator separator) const;
};

struct Caption {
	auto_ptr<list<TagAttribute> >       attributes;// ALIGN
	auto_ptr<list<auto_ptr<Element> > > texts;

	void unparse(iconvstream&, ostream_manipulator separator) const;
	Area *format(Area::size_type w, int halign) const;
};

struct Table : public Element {
	auto_ptr<list<TagAttribute> >        attributes;// ALIGN WIDTH BORDER
	// CELLSPACING CELLPADDING
	auto_ptr<Caption>                    caption;
	auto_ptr<list<auto_ptr<TableRow> > > rows;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

struct NoBreak : public Element {
	auto_ptr<list<auto_ptr<Element> > > content;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Line *line_format() const;
};

struct HorizontalRule : public Element {
	auto_ptr<list<TagAttribute> > attributes; // ALIGN NOSHADE SIZE WIDTH

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

/* ------------------------------------------------------------------------- */

struct ListItem {
	virtual ~ListItem()
	{}
	virtual void unparse(iconvstream&, ostream_manipulator separator) const = 0;
	virtual Area *format(
		Area::size_type w,
		int style,
		Area::size_type indent,
		int             *number_in_out = 0
		) const = 0;
};

struct ListNormalItem : public ListItem {
	auto_ptr<list<TagAttribute> >       attributes;// TYPE VALUE
	auto_ptr<list<auto_ptr<Element> > > flow;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Area *format(
		Area::size_type w,
		int style,
		Area::size_type indent,
		int             *number_in_out
		) const;
};

struct ListBlockItem : public ListItem {
	auto_ptr<Element> block;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Area *format(
		Area::size_type w,
		int style,
		Area::size_type indent,
		int             *
		) const;
};

struct OrderedList : public Element {
	auto_ptr<list<TagAttribute> >        attributes;// TYPE START COMPACT
	auto_ptr<list<auto_ptr<ListItem> > > items;
	int nesting;
	// Item indentation depends on on the list nesting level.

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

struct UnorderedList : public Element {
	auto_ptr<list<TagAttribute> >        attributes;// TYPE COMPACT
	auto_ptr<list<auto_ptr<ListItem> > > items;
	int nesting;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

struct Dir : public Element {
	auto_ptr<list<TagAttribute> >        attributes;// COMPACT
	auto_ptr<list<auto_ptr<ListItem> > > items;
	int nesting;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

struct Menu : public Element {
	auto_ptr<list<TagAttribute> >        attributes;// COMPACT
	auto_ptr<list<auto_ptr<ListItem> > > items;
	int nesting;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

/* ------------------------------------------------------------------------- */

struct DefinitionListItem {
	virtual ~DefinitionListItem()
	{}
	virtual void unparse(iconvstream&, ostream_manipulator separator) const = 0;
	virtual Area *format(Area::size_type w, int halign) const = 0;
};

struct TermName : public DefinitionListItem {
	auto_ptr<list<auto_ptr<Element> > > flow;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

struct TermDefinition : public DefinitionListItem {
	auto_ptr<list<auto_ptr<Element> > > flow;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

struct DefinitionList : public Element {
	auto_ptr<list<TagAttribute> >                  attributes;// COMPACT
	auto_ptr<list<auto_ptr<Element> > >            preamble;
	auto_ptr<list<auto_ptr<DefinitionListItem> > > items;

	/*virtual*/ void unparse(iconvstream&, ostream_manipulator separator) const;
	/*virtual*/ Area *format(Area::size_type w, int halign) const;
};

/* ------------------------------------------------------------------------- */

#endif /* } */

/* ------------------------------------------------------------------------- */

