
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
 * Sun Mar 18 19:51:54 CET 2001: fixed segfault
 * Fre Jun  8 17:37:12 CEST 2001: new image handling
 * Thu Oct  4 21:34:26 CEST 2001: ported to g++ 3.0
 * Mon Jul 29 13:09:26 CEST 2002: fixed runtime increment
 * Wed Jul  2 22:04:08 CEST 2003: ported to g++ 3.3
 */

/***************************************************************************/


#include <sstream>

#include <stdlib.h>
#include <ctype.h>
#include <vector>
#include <map>

#include "html.h"
#include "HTMLParser.tab.hh"
#include "sgml.h"
#include "cmp_nocase.h"
#include "format.h"
#include "Properties.h"
#include "iconvstream.h"

#ifndef nelems
#define nelems(array) (sizeof(array) / sizeof((array)[0]))
#endif

/* ------------------------------------------------------------------------- */

static Line *line_format(const list<auto_ptr<Element> > *elements);
static Area *make_up(const Line &line, Area::size_type w, int halign);
static Area *format(
	const list<auto_ptr<Element> > *elements,
	Area::size_type w,
	int halign
	);
static void format(
	const list<auto_ptr<Element> > *elements,
	Area::size_type indent_left,
	Area::size_type w,
	int halign,
	iconvstream &os
	);

/* ------------------------------------------------------------------------- */

/*
 * Helper class that retrieves several block-formatting properties in one
 * go.
 */
struct BlockFormat {
	Area::size_type vspace_before;
	Area::size_type vspace_after;
	Area::size_type indent_left;
	Area::size_type indent_right;
	const char      *quote_char;

	BlockFormat(
		const char      *item_name,
		Area::size_type default_vspace_before = 0,
		Area::size_type default_vspace_after = 0,
		Area::size_type default_indent_left = 0,
		Area::size_type default_indent_right = 0,
		const char      *default_quote_char = NULL
		);
	Area::size_type effective_width(Area::size_type) const;
};

/*
 * Helper class that retrieves several list-formatting properties in one
 * go.
 */
struct ListFormat {
	Area::size_type vspace_before;
	Area::size_type vspace_between;
	Area::size_type vspace_after;
	auto_ptr<vector<int> >    indents;
	auto_ptr<vector<string> > default_types;
	ListFormat(
		const char      *item_name,
		Area::size_type default_vspace_before = 0,
		Area::size_type default_vspace_between = 0,
		Area::size_type default_vspace_after = 0,
		const char      *default_indents = "6",
		const char      *default_default_types = "DISC CIRCLE SQUARE"
		);
	Area::size_type get_indent(int nesting) const;
	const string    &get_default_type(int nesting) const;
	int             get_type(
		const list<TagAttribute> *attributes,
		int nesting,
		int default_default_type
		) const;
};

/* ------------------------------------------------------------------------- */

// Attributes: VERSION (ignored)
Area *
Document::format(Area::size_type w, int halign) const
{
	static BlockFormat bf("DOCUMENT");

	auto_ptr<Area> res(body.format(bf.effective_width(w), halign));
	if (!res.get())
		return 0;

	*res >>= bf.indent_left;

	res->prepend(bf.vspace_before);
	res->append(bf.vspace_after);

	return res.release();
}

void
Document::format(
	Area::size_type indent_left,
	Area::size_type w,
	int             halign,
	iconvstream     &os
	) const
{
	static BlockFormat bf("DOCUMENT");

	for (int i = 0; i < bf.vspace_before; ++i)
		os << endl;

	body.format(
			indent_left + bf.indent_left, bf.effective_width(w), halign,
			os
			);

	for (int j = 0; j < bf.vspace_after; ++j)
		os << endl;
}

/* ------------------------------------------------------------------------- */

// Attributes: BACKGROUND BGCOLOR TEXT LINK VLINK ALINK (ignored)
Area *
Body::format(Area::size_type w, int halign) const
{
	static BlockFormat bf("BODY");

	auto_ptr<Area> res(
		::format(content.get(), bf.effective_width(w), halign)
		);
	if (!res.get())
		return 0;

	*res >>= bf.indent_left;

	res->prepend(bf.vspace_before);
	res->append(bf.vspace_after);

	return res.release();
}

void
Body::format(
	Area::size_type indent_left,
	Area::size_type w,
	int halign,
	iconvstream &os
	) const
{
	static BlockFormat bf("BODY");

	for (int i = 0; i < bf.vspace_before; ++i)
		os << endl;

	::format(
			content.get(),
			indent_left + bf.indent_left, bf.effective_width(w), halign,
			os
			);

	for (int j = 0; j < bf.vspace_after; ++j)
		os << endl;
}

/* ------------------------------------------------------------------------- */

enum {
	NO_BULLET,
	ARABIC_NUMBERS, LOWER_ALPHA, UPPER_ALPHA, LOWER_ROMAN, UPPER_ROMAN,
	DISC, SQUARE, CIRCLE, CUSTOM1, CUSTOM2, CUSTOM3
};

// Attributes: TYPE (processed) COMPACT (ignored)
Area *
OrderedList::format(Area::size_type w, int /*halign*/) const
{
	if (!items.get())
		return 0;

	static ListFormat lf("OL", 0, 0, 0, "6", "1");
	int type = lf.get_type(attributes.get(), nesting, ARABIC_NUMBERS);
	auto_ptr<Area> res;

	const list<auto_ptr<ListItem> > &il(*items);
	list<auto_ptr<ListItem> >::const_iterator i;
	int number = 1;
	for (i = il.begin(); i != il.end(); ++i) {
		auto_ptr<Area> a((*i)->format(w, type, lf.get_indent(nesting), &number));
		if (a.get()) {
			if (res.get()) {
				res->append(lf.vspace_between);
			} else {
				res.reset(new Area);
				res->append(lf.vspace_before);
			}
			*res += *a;
		}
	}
	if (res.get())
		res->append(lf.vspace_after);

	return res.release();
}

/*
 * <UL>, <DIR> and <MENU> are currently formatted totally identically, because
 * this is what Netscape does, and the HTML 3.2 spec and "HTML -- The
 * Definitive Guide" give no clear indication as to how to format them.
 */

// Attributes: TYPE (processed) COMPACT (ignored)
Area *
UnorderedList::format(Area::size_type w, int /*halign*/) const
{
	if (!items.get())
		return 0;

	static ListFormat lf("UL");
	int type = lf.get_type(attributes.get(), nesting, SQUARE);
	auto_ptr<Area>    res;

	const list<auto_ptr<ListItem> > &il(*items);
	list<auto_ptr<ListItem> >::const_iterator i;
	for (i = il.begin(); i != il.end(); ++i) {
		auto_ptr<Area> a((*i)->format(w, type, lf.get_indent(nesting)));
		if (a.get()) {
			if (res.get()) {
				res->append(lf.vspace_between);
			} else {
				res.reset(new Area);
				res->append(lf.vspace_before);
			}
			*res += *a;
		}
	}
	if (res.get())
		res->append(lf.vspace_after);

	return res.release();
}

// Attributes: TYPE (extension, processed) COMPACT (ignored)
Area *
Dir::format(Area::size_type w, int /*halign*/) const
{
	if (!items.get())
		return 0;

	static ListFormat lf("DIR");
	int type = lf.get_type(attributes.get(), nesting, SQUARE);
	auto_ptr<Area>    res;

	const list<auto_ptr<ListItem> > &il(*items);
	list<auto_ptr<ListItem> >::const_iterator i;
	for (i = il.begin(); i != il.end(); ++i) {
		auto_ptr<Area> a((*i)->format(w, type, lf.get_indent(nesting)));
		if (a.get()) {
			if (res.get()) {
				res->append(lf.vspace_between);
			} else {
				res.reset(new Area);
				res->append(lf.vspace_before);
			}
			*res += *a;
		}
	}
	if (res.get())
		res->append(lf.vspace_after);

	return res.release();
}

// Attributes: TYPE (extension, processed) COMPACT (ignored)
Area *
Menu::format(Area::size_type w, int /*halign*/) const
{
	if (!items.get())
		return 0;

	static ListFormat lf("MENU", 0, 0, 0, "2", "NO_BULLET");
	int type = lf.get_type(attributes.get(), nesting, NO_BULLET);
	auto_ptr<Area>    res;

	const list<auto_ptr<ListItem> > &il(*items);
	list<auto_ptr<ListItem> >::const_iterator i;
	for (i = il.begin(); i != il.end(); ++i) {
		auto_ptr<Area> a((*i)->format(w, type, lf.get_indent(nesting)));
		if (a.get()) {
			if (res.get()) {
				res->append(lf.vspace_between);
			} else {
				res.reset(new Area);
				res->append(lf.vspace_before);
			}
			*res += *a;
		}
	}
	if (res.get())
		res->append(lf.vspace_after);

	return res.release();
}

// Attributes: TYPE VALUE (ignored)
Area *
ListNormalItem::format(
	Area::size_type w,
	int type,
	Area::size_type indent,
	int             *number_in_out/*= 0*/
	) const
{
	int number = 0;

	if (number_in_out) {
		number = *number_in_out = get_attribute(
				attributes.get(), "VALUE", *number_in_out
				);
	}

	static const char *disc_bullet =
		Formatting::getString("LI.disc_bullet", "*");
	static const char *square_bullet =
		Formatting::getString("LI.square_bullet", "#");
	static const char *circle_bullet =
		Formatting::getString("LI.circle_bullet", "o");
	static const char *custom1_bullet =
		Formatting::getString("LI.custom1_bullet", "+");
	static const char *custom2_bullet =
		Formatting::getString("LI.custom2_bullet", "-");
	static const char *custom3_bullet =
		Formatting::getString("LI.custom3_bullet", "~");

	string bullet;
	switch (type) {
	case NO_BULLET:                          break;
	case DISC:      bullet = disc_bullet;
		break;
	case SQUARE:    bullet = square_bullet;
		break;
	case CIRCLE:    bullet = circle_bullet;
		break;
	case CUSTOM1:   bullet = custom1_bullet;
		break;
	case CUSTOM2:   bullet = custom2_bullet;
		break;
	case CUSTOM3:   bullet = custom3_bullet;
		break;
	case ARABIC_NUMBERS:
	{
		std::ostringstream oss;
		oss << number << '.'; // << std::ends;
		bullet = oss.str();
		// oss.rdbuf()->freeze(0);
	}
	break;
	case LOWER_ALPHA:
		bullet = number <= 26 ? (char) (number - 1 + 'a') : 'z';
		bullet += '.';
		break;
	case UPPER_ALPHA:
		bullet = number <= 26 ? (char) (number - 1 + 'A') : 'Z';
		bullet += '.';
		break;
	case LOWER_ROMAN:
	{
		static const char *lower_roman[] = {
			"0", "i", "ii", "iii", "iv", "v", "vi", "vii", "viii", "ix",
			"x", "xi", "xii", "xiii", "xiv", "xv", "xvi", "xvii", "xviii", "xix",
			"xx", "xxi", "xxii", "xxiii", "xxiv", "xxv", "xxvi", "xxvii", "xxviii", "xxix"
		};
		const char *p = (
			number >= 0 && number < (int) nelems(lower_roman) ?
			lower_roman[number] :
			"???"
			);
		bullet = p;
		bullet += '.';
	}
	break;
	case UPPER_ROMAN:
	{
		static const char *upper_roman[] = {
			"0", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX",
			"X", "XI", "XII", "XIII", "XIV", "XV", "XVI", "XVII", "XVIII", "XIX",
			"XX", "XXI", "XXII", "XXIII", "XXIV", "XXV", "XXVI", "XXVII", "XXVIII", "XXIX"
		};
		const char *p = (
			number >= 0 && number < (int) nelems(upper_roman) ?
			upper_roman[number] :
			"???"
			);
		bullet = p;
		bullet += '.';
	}
	break;
	}

	if (bullet.length() >= indent)
		indent = bullet.length() + 1;

	auto_ptr<Area> res(::format(flow.get(), w - indent, Area::LEFT));
	// KLUDGE: Some people write "<UL> <B><LI>Bla</B>Bla </UL>", which actually
	// defines a bold and empty list item before "Bla Bla". This is very
	// difficult to handle... so... let's just ignore empty list items.
	if (!res.get())
		return 0;

	*res >>= indent;
	res->insert(bullet, indent - bullet.length() - 1, 0);
	if (number_in_out)
		(*number_in_out)++;
	return res.release();
}

Area *
ListBlockItem::format(
	Area::size_type w,
	int /*type*/,
	Area::size_type indent,
	int             */*number_in_out*/ /*= 0*/
	) const
{
	if (!block.get())
		return 0;

	auto_ptr<Area> res(block->format(w - indent, Area::LEFT));
	if (!res.get())
		return 0;

	/*
	 * Hm... shouldn't there be a bullet before the item?
	 */
	*res >>= indent;
	return res.release();
}

/* ------------------------------------------------------------------------- */

// Attributes: COMPACT (ignored)
Area *
DefinitionList::format(Area::size_type w, int halign) const
{
	static struct DefinitionListFormat {
		const Area::size_type vspace_before;
		const Area::size_type vspace_between;
		const Area::size_type vspace_after;

		DefinitionListFormat() :
			vspace_before(Formatting::getInt("DL.vspace.before", 0)),
			vspace_between(Formatting::getInt("DL.vspace.between", 0)),
			vspace_after(Formatting::getInt("DL.vspace.after", 0))
		{}
	}
	dlf;
	auto_ptr<Area> res;

	if (preamble.get()) {
		res.reset(::format(preamble.get(), w, halign));
		if (res.get())
			res->prepend(dlf.vspace_before);
	}

	if (items.get()) {
		const list<auto_ptr<DefinitionListItem> > &il(*items);
		list<auto_ptr<DefinitionListItem> >::const_iterator i;
		for (i = il.begin(); i != il.end(); ++i) {
			auto_ptr<Area> a((*i)->format(w, halign));
			if (!a.get())
				continue;
			if (res.get()) {
				res->append(dlf.vspace_between);
				*res += *a;
			} else {
				res = a;
				res->prepend(dlf.vspace_before);
			}
		}
	}

	if (res.get())
		res->append(dlf.vspace_after);
	return res.release();
}

Area *
TermName::format(Area::size_type w, int halign) const
{
	static BlockFormat bf("DT", 0, 0, 2);

	auto_ptr<Area> res(::format(flow.get(), bf.effective_width(w), halign));
	if (!res.get())
		return 0;

	*res >>= bf.indent_left;
	res->prepend(bf.vspace_before);
	res->append(bf.vspace_after);

	return res.release();
}

Area *
TermDefinition::format(Area::size_type w, int halign) const
{
	static BlockFormat bf("DD", 0, 0, 6);

	auto_ptr<Area> res(::format(flow.get(), bf.effective_width(w), halign));
	if (!res.get())
		return 0;

	*res >>= bf.indent_left;
	res->prepend(bf.vspace_before);
	res->append(bf.vspace_after);

	return res.release();
}

/* ------------------------------------------------------------------------- */

// Attributes: ALIGN NOSHADE SIZE WIDTH (ignored)
Area *
HorizontalRule::format(Area::size_type w, int /*halign*/) const
{
	static const char *marker = Formatting::getString("HR.marker", "=");
	static BlockFormat bf("HR");

	Area *res = new Area(bf.effective_width(w), 1, *marker ? *marker : ' ');
	*res >>= bf.indent_left;

	res->prepend(bf.vspace_before);
	res->append(bf.vspace_after);

	return res;
}

/* ------------------------------------------------------------------------- */

// Attributes: ALIGN (processed)
Area *
Heading::format(Area::size_type w, int halign) const
{
	halign = get_attribute(
			attributes.get(), "ALIGN", halign,
			"LEFT", Area::LEFT,
			"CENTER", Area::CENTER,
			"RIGHT", Area::RIGHT,
			NULL
			);

	static char cell_attributes[7];
	if (!cell_attributes[0]) {
		cell_attributes[0] = 1;
		cell_attributes[1] = Formatting::getAttributes("H1.attributes", Cell::BOLD);
		cell_attributes[2] = Formatting::getAttributes("H2.attributes", Cell::BOLD);
		cell_attributes[3] = Formatting::getAttributes("H3.attributes", Cell::BOLD);
		cell_attributes[4] = Formatting::getAttributes("H4.attributes", Cell::BOLD);
		cell_attributes[5] = Formatting::getAttributes("H5.attributes", Cell::BOLD);
		cell_attributes[6] = Formatting::getAttributes("H6.attributes", Cell::BOLD);
	}

	auto_ptr<Area> res;
	auto_ptr<Line> line(::line_format(content.get()));
	if (line.get()) {
		static const char *prefixes[7];
		if (!prefixes[1]) {
			prefixes[1] = Formatting::getString("H1.prefix", "****** ");
			prefixes[2] = Formatting::getString("H2.prefix", "***** ");
			prefixes[3] = Formatting::getString("H3.prefix", "**** ");
			prefixes[4] = Formatting::getString("H4.prefix", "*** ");
			prefixes[5] = Formatting::getString("H5.prefix", "** ");
			prefixes[6] = Formatting::getString("H6.prefix", "* ");
		}
		auto_ptr<Line> l(new Line(prefixes[level]));
		l->insert(*line, l->length());
		static const char *suffixes[7];
		if (!suffixes[1]) {
			suffixes[1] = Formatting::getString("H1.suffix", " ******");
			suffixes[2] = Formatting::getString("H2.suffix", " *****");
			suffixes[3] = Formatting::getString("H3.suffix", " ****");
			suffixes[4] = Formatting::getString("H4.suffix", " ***");
			suffixes[5] = Formatting::getString("H5.suffix", " **");
			suffixes[6] = Formatting::getString("H6.suffix", " *");
		}
		l->append(suffixes[level]);
		l->add_attribute(cell_attributes[level]);
		res.reset(make_up(*l, w, halign));
		if (!res.get())
			return 0;
	} else {
		/*
		 * Hm. Heading is not line-formattable...
		 */
		res.reset(::format(content.get(), w, halign));
		if (!res.get())
			return 0;
		res->add_attribute(cell_attributes[level]);
	}

	static int vspace_before[7];
	if (vspace_before[0] == 0) {
		vspace_before[0] = 1;
		vspace_before[1] = Formatting::getInt("H1.vspace.before", 0);
		vspace_before[2] = Formatting::getInt("H2.vspace.before", 0);
		vspace_before[3] = Formatting::getInt("H3.vspace.before", 0);
		vspace_before[4] = Formatting::getInt("H4.vspace.before", 0);
		vspace_before[5] = Formatting::getInt("H5.vspace.before", 0);
		vspace_before[6] = Formatting::getInt("H6.vspace.before", 0);
	}
	res->prepend(vspace_before[level]);

	static int vspace_after[7];
	if (vspace_after[0] == 0) {
		vspace_after[0] = 1;
		vspace_after[1] = Formatting::getInt("H1.vspace.after", 0);
		vspace_after[2] = Formatting::getInt("H2.vspace.after", 0);
		vspace_after[3] = Formatting::getInt("H3.vspace.after", 0);
		vspace_after[4] = Formatting::getInt("H4.vspace.after", 0);
		vspace_after[5] = Formatting::getInt("H5.vspace.after", 0);
		vspace_after[6] = Formatting::getInt("H6.vspace.after", 0);
	}
	res->append(vspace_after[level]);

	return res.release();
}

// Attributes: WIDTH (processed)
Area *
Preformatted::format(Area::size_type w, int halign) const
{
	w = get_attribute(attributes.get(), "WIDTH", w);

	static BlockFormat bf("PRE");

	/*
	 * Attempt to line-format the <PRE>.
	 */
	auto_ptr<Area> res;

	auto_ptr<Line> line(::line_format(texts.get()));
	if (line.get()) {
		res.reset(make_up(*line, bf.effective_width(w), halign));
	}

	/*
	 * Failed; block-format it.
	 */
	if (!res.get()) {
		res.reset(::format(texts.get(), bf.effective_width(w), halign));
		if (!res.get())
			return 0;
	}

	*res >>= bf.indent_left;
	res->prepend(bf.vspace_before);
	res->append(bf.vspace_after);

	return res.release();
}

// Attributes: ALIGN (processed)
Area *
Paragraph::format(Area::size_type w, int halign) const
{
	if (!texts.get())
		return 0;

	halign = get_attribute(
			attributes.get(), "ALIGN", halign,
			"LEFT", Area::LEFT,
			"CENTER", Area::CENTER,
			"RIGHT", Area::RIGHT,
			NULL
			);

	static BlockFormat bf("P");
	Area *res = ::format(texts.get(), bf.effective_width(w), halign);
	if (!res)
		return 0;

	*res >>= bf.indent_left;
	res->prepend(bf.vspace_before);
	res->append(bf.vspace_after);

	return res;
}

// Attributes: SRC ALT (processed) ALIGN HEIGHT WIDTH BORDER HSPACE VSPACE
//             USEMAP ISMAP (ignored)
Line *
Image::line_format() const
{
	// new image handling - Johannes Geiger
	static const char *repl_all =
		Formatting::getString("IMG.replace.all");
	static const char *repl_noalt =
		Formatting::getString("IMG.replace.noalt");
	static const char *alt_prefix =
		Formatting::getString("IMG.alt.prefix", "[");
	static const char *alt_suffix =
		Formatting::getString("IMG.alt.suffix", "]");

	if (repl_all) {
		return new Line(repl_all);
	}

	{
		bool ex;
		istr alt(get_attribute(attributes.get(), "ALT", &ex));
		if (ex) {
			if (!alt.empty()) {
				replace_sgml_entities(&alt);
				alt >>= alt_prefix;
				alt <<= alt_suffix;
				return new Line(alt);
			} else {
				return NULL;
			}
		}
	}

	if (repl_noalt) {
		return new Line(repl_noalt);
	}

	{
		istr src(get_attribute(attributes.get(), "SRC", ""));
		if (!src.empty())
			return new Line((src >>= '[') <<= ']');
	}

	return new Line("[Image]");
}

// Attributes: CODEBASE CODE (ignored) ALT (processed) NAME WIDTH HEIGHT
//             (ignored) ALIGN (processed) HSPACE VSPACE (ignored)
Area *
Applet::format(Area::size_type w, int /*halign*/) const
{
	if (content.get()) {
		int halign = get_attribute(
				attributes.get(), "ALIGN", Area::CENTER,
				"LEFT", Area::LEFT,
				"MIDDLE", Area::CENTER,
				"RIGHT", Area::RIGHT,
				NULL
				);
		Area *a = ::format(content.get(), w, halign);
		if (a)
			return a;
	}

	{
		istr alt(get_attribute(attributes.get(), "ALT", ""));
		if (!alt.empty())
			return new Area((alt >>= "[Java Applet: ") <<= ']');
	}

	{
		istr code(get_attribute(attributes.get(), "CODE", ""));
		if (!code.empty())
			return new Area((code >>= "[Java Applet ") <<= ']');
	}

	return new Area("[Java Applet]");
}

Line *
Applet::line_format() const
{
	if (content.get()) {
		Line *l = ::line_format(content.get());
		if (l)
			return l;
	}

	{
		istr alt(get_attribute(attributes.get(), "ALT", ""));
		if (!alt.empty())
			return new Line((alt >>= "[Java Applet: ") <<= ']');
	}

	{
		istr code(get_attribute(attributes.get(), "CODE", ""));
		if (!code.empty())
			return new Line((code >>= "[Java Applet ") <<= ']');
	}

	return new Line("[Java Applet]");
}

// Attributes: NAME HREF REL REV TITLE (ignored)
// Attributes: ALIGN (processed)
Area *
Division::format(Area::size_type w, int halign) const
{
	return ::format(body_content.get(), w, get_attribute(
			attributes.get(), "ALIGN", halign,
			"LEFT", Area::LEFT,
			"CENTER", Area::CENTER,
			"RIGHT", Area::RIGHT,
			NULL
			));
}

Area *
Center::format(Area::size_type w, int /*halign*/) const
{
	return ::format(body_content.get(), w, Area::CENTER);
}

Area *
BlockQuote::format(Area::size_type w, int halign) const
{
	static BlockFormat bf("BLOCKQUOTE", 0, 0, 5, 5);

	auto_ptr<Area> res(::format(
			content.get(),
			bf.effective_width(w), halign
			));
	if (!res.get())
		return 0;

	*res >>= bf.indent_left;
	if (bf.quote_char != NULL)
		*res >>= bf.quote_char;
	res->prepend(bf.vspace_before);
	res->append(bf.vspace_after);

	return res.release();
}

Area *
Address::format(Area::size_type w, int halign) const
{
	static BlockFormat bf("ADDRESS", 0, 0, 5, 5);

	auto_ptr<Area> res(::format(
			content.get(),
			bf.effective_width(w), halign
			));
	if (!res.get())
		return 0;

	*res >>= bf.indent_left;
	res->prepend(bf.vspace_before);
	res->append(bf.vspace_after);

	return res.release();
}

/* ------------------------------------------------------------------------- */

// Attributes: ACTION METHOD ENCTYPE (ignored)
Area *
Form::format(Area::size_type w, int halign) const
{
	return content.get() ? ::format(content.get(), w, halign) : 0;
}

// Attributes: TYPE (processed) NAME (ignored) VALUE CHECKED SIZE (processed)
//             MAXLENGTH (ignored) SRC (processed) ALIGN (ignored)
Line *
Input::line_format() const
{
	string type = get_attribute(attributes.get(), "TYPE", "TEXT").c_str();
	string name = get_attribute(attributes.get(), "NAME", "").c_str();
	string value = get_attribute(attributes.get(), "VALUE", "").c_str();
	bool checked = get_attribute(attributes.get(), "CHECKED", "0") != "0";
	int size = get_attribute(attributes.get(), "SIZE", -1);
	string src = get_attribute(attributes.get(), "SRC", "").c_str();

	string res;
	if (cmp_nocase(type, "TEXT") == 0) {
		if (size == -1)
			size = 20;
		if (value.empty())
			value = name;
//		if ((int) value.length() > size) { value.erase(size); } else
		if ((int) value.length() < size)
			value.append(size - value.length(), ' ');
		res = '[' + string(value.c_str()) + ']';
	} else if (cmp_nocase(type, "PASSWORD") == 0) {
		if (size == -1)
			size = 20;
		res = '[' + string(size, '*') + ']';
	} else if (cmp_nocase(type, "CHECKBOX") == 0) {
		res = '[' + (checked ? 'X' : ' ') + ']';
	} else if (cmp_nocase(type, "RADIO") == 0) {
		res = checked ? '#' : 'o';
	} else if (cmp_nocase(type, "SUBMIT") == 0) {
		res = value.empty() ? string("[Submit]") : '[' + value + ']';
	} else if (cmp_nocase(type, "IMAGE") == 0) {
		res = "[Submit " + src + ']';
	} else if (cmp_nocase(type, "RESET") == 0) {
		res = value.empty() ? string("[Reset]") : '[' + value + ']';
	} else if (cmp_nocase(type, "FILE") == 0) {
		res = "[File]";
	} else if (cmp_nocase(type, "HIDDEN") == 0) {
		return 0;
	} else {
		res = "[Unknown INPUT type]";
	}

	return new Line(res);
}

// Attributes: NAME SIZE (ignored) MULTIPLE (processed)
Line *
Select::line_format() const
{
	if (!content.get() || content->empty())
		return new Line("[Empty selection]");

	bool multiple = get_attribute(attributes.get(), "MULTIPLE", "0") != "0";

	auto_ptr<Line> res(new Line(multiple ? "[One or more of " : "[One of: "));
	const list<auto_ptr<Option> > &c(*content);
	list<auto_ptr<Option> >::const_iterator i;
	for (i = c.begin(); i != c.end(); ++i) {
		if (!(*i).get())
			continue;
		if (i != c.begin())
			*res += '/';
		auto_ptr<Line> l((*i)->pcdata->line_format());
		*res += *l;
	}
	*res += ']';

	return res.release();
}

// Attributes: NAME ROWS COLS
Area *
TextArea::format(Area::size_type w, int halign) const
{
	auto_ptr<Line> line(pcdata->line_format());
	return line.get() ? make_up(*line, w, halign) : 0;
}

/* ------------------------------------------------------------------------- */

Line *
PCData::line_format() const
{
	return new Line(text);
}

// Item:                        Default cell attribute:
// <TT> <I>                  => NONE
// <B>                       => BOLD
// <U>                       => UNDERLINE
// <STRIKE>                  => STRIKETHROUGH
// <BIG> <SMALL> <SUB> <SUP> => NONE
static char
get_font_cell_attributes(int attribute)
{
	if (attribute == HTMLParser_token::TT) {
		static char a = Formatting::getAttributes("TT.attributes", Cell::NONE);
		return a;
	} else if (attribute == HTMLParser_token::I) {
		static char a = Formatting::getAttributes("I.attributes", Cell::NONE);
		return a;
	} else if (attribute == HTMLParser_token::B) {
		static char a = Formatting::getAttributes("B.attributes", Cell::BOLD);
		return a;
	} else if (attribute == HTMLParser_token::U) {
		static char a = Formatting::getAttributes("U.attributes", Cell::UNDERLINE);
		return a;
	} else if (attribute == HTMLParser_token::STRIKE) {
		static char a = Formatting::getAttributes("STRIKE.attributes", Cell::STRIKETHROUGH);
		return a;
	} else if (attribute == HTMLParser_token::BIG) {
		static char a = Formatting::getAttributes("BIG.attributes", Cell::NONE);
		return a;
	} else if (attribute == HTMLParser_token::SMALL) {
		static char a = Formatting::getAttributes("SMALL.attributes", Cell::NONE);
		return a;
	} else if (attribute == HTMLParser_token::SUB) {
		static char a = Formatting::getAttributes("SUB.attributes", Cell::NONE);
		return a;
	} else if (attribute == HTMLParser_token::SUP) {
		static char a = Formatting::getAttributes("SUP.attributes", Cell::NONE);
		return a;
	}
	return Cell::NONE;
}

Line *
Font::line_format() const
{
	auto_ptr<Line> res(::line_format(texts.get()));
	if (!res.get())
		return 0;

	char a = get_font_cell_attributes(attribute);
	if (a != Cell::NONE)
		res->add_attribute(a);

	return res.release();
}

// Item:                        Default cell attribute:
Area *
Font::format(Area::size_type w, int halign) const
{
	auto_ptr<Area> res(::format(texts.get(), w, halign));
	if (!res.get())
		return 0;

	char a = get_font_cell_attributes(attribute);
	if (a != Cell::NONE)
		res->add_attribute(a);

	return res.release();
}

// Item:                                     Default cell attribute:
// <EM> <STRONG>                          => BOLD
// <DFN> <CODE> <SAMP> <KBD> <VAR> <CITE> => NONE
static char
get_phrase_cell_attributes(int attribute)
{
	if (attribute == HTMLParser_token::EM) {
		static char a = Formatting::getAttributes("EM.attributes", Cell::BOLD);
		return a;
	} else if (attribute == HTMLParser_token::STRONG) {
		static char a = Formatting::getAttributes("STRONG.attributes", Cell::BOLD);
		return a;
	} else if (attribute == HTMLParser_token::DFN) {
		static char a = Formatting::getAttributes("DFN.attributes", Cell::NONE);
		return a;
	} else if (attribute == HTMLParser_token::CODE) {
		static char a = Formatting::getAttributes("CODE.attributes", Cell::NONE);
		return a;
	} else if (attribute == HTMLParser_token::SAMP) {
		static char a = Formatting::getAttributes("SAMP.attributes", Cell::NONE);
		return a;
	} else if (attribute == HTMLParser_token::KBD) {
		static char a = Formatting::getAttributes("KBD.attributes", Cell::NONE);
		return a;
	} else if (attribute == HTMLParser_token::VAR) {
		static char a = Formatting::getAttributes("VAR.attributes", Cell::NONE);
		return a;
	} else if (attribute == HTMLParser_token::CITE) {
		static char a = Formatting::getAttributes("CITE.attributes", Cell::NONE);
		return a;
	}
	return Cell::NONE;
}

Line *
Phrase::line_format() const
{
	auto_ptr<Line> res(::line_format(texts.get()));
	if (!res.get())
		return 0;

	char a = get_phrase_cell_attributes(attribute);
	if (a != Cell::NONE)
		res->add_attribute(a);

	return res.release();
}

// EM STRONG                  => BOLD
// DFN CODE SAMP KBD VAR CITE => (nothing)
Area *
Phrase::format(Area::size_type w, int halign) const
{
	auto_ptr<Area> res(::format(texts.get(), w, halign));
	if (!res.get())
		return 0;

	char a = get_phrase_cell_attributes(attribute);
	if (a != Cell::NONE)
		res->add_attribute(a);

	return res.release();
}

// Attributes: SIZE COLOR (ignored)
Area *
Font2::format(Area::size_type w, int halign) const
{
	return ::format(elements.get(), w, halign);
}

// Attributes: SIZE COLOR (ignored)
Line *
Font2::line_format() const
{
	return ::line_format(elements.get());
}

static char
get_link_cell_attributes(const istr &href)
{
	if (href[0] == '#') {
		static const char internal_link_attributes =
			Formatting::getAttributes("A.attributes.internal_link",
					Cell::UNDERLINE);
		return internal_link_attributes;
	} else {
		static const char external_link_attributes =
			Formatting::getAttributes("A.attributes.external_link",
					Cell::UNDERLINE);
		return external_link_attributes;
	}
}

// Attributes: NAME HREF REL REV TITLE (ignored)
Line *
Anchor::line_format() const
{
	auto_ptr<Line> res(::line_format(texts.get()));
	if (!res.get())
		return 0;

	istr href(get_attribute(attributes.get(), "HREF", ""));
	if (!href.empty()) {
		res->add_attribute(get_link_cell_attributes(href));
		if (refnum > 0) {
			char refnumstr[16];
			snprintf(refnumstr, sizeof(refnumstr), "[%d]", refnum);
			res->append(refnumstr);
		}
	}
	return res.release();
}

Area *
Anchor::format(Area::size_type w, int halign) const
{
	auto_ptr<Area> res(::format(texts.get(), w, halign));
	if (!res.get())
		return 0;

	istr href(get_attribute(attributes.get(), "HREF", ""));
	if (!href.empty()) {
		res->add_attribute(get_link_cell_attributes(href));
		if (refnum > 0) {
			char refnumstr[16];
			snprintf(refnumstr, sizeof(refnumstr), "[%d]", refnum);
			auto_ptr<Area> l(new Area(refnumstr));
			*res += *l;
		}
	}
	return res.release();
}

// Attributes: CLEAR (ignored)
Line *
LineBreak::line_format() const
{
	return new Line("\n");
}

Area *
TableHeadingCell::format(Area::size_type w, int halign) const
{
	Area *a = TableCell::format(w, halign);

	if (a)
		a->add_attribute(Cell::BOLD);
	return a;
}

Area *
Caption::format(Area::size_type w, int halign) const
{
	auto_ptr<Line> l(::line_format(texts.get()));

	return l.get() ? make_up(*l, w, halign) : 0;
}

// Attributes: (none)
Line *
NoBreak::line_format() const
{
	Line *l(::line_format(content.get()));
	if (!l)
		return 0;

	for (Line::size_type i = 0; i < l->length(); ++i) {
		Cell &c((*l)[i]);
		/* we should inject nbsp UTF-8 sequence here
		if (c.character == ' ')
			c.character = (char)160;
		 */
	}
	return l;
}

/* ------------------------------------------------------------------------- */

/*
 * Make up "line" into an Area. Attempt to return an Area no wider than "w".
 */
static Area *
make_up(const Line &line, Area::size_type w, int halign)
{
//{
//  cout << "make_up(\"";
//  for (Line::size_type i = 0; i < line.length(); i++) {
//    if (isprint(line[i].character)) cout << line[i].character;
//    else cout << "[" << (int) line[i].character << "]";
//  }
//  cout << "\")" << endl;
//}

	if (line.empty())
		return 0;

	auto_ptr<Area> res(new Area);

	Line::size_type from = 0;

	while (from < line.length()) {
		/*
		 * A sole newline character has a special meaning: Append a blank line.
		 */
		if (line[from].character == '\n') {
			res->resize(res->width(), res->height() + 1);
			from++;
			continue;
		}

		Line::size_type to = from + 1;
		Line::size_type lbp = (Line::size_type) -1; // "Last break position".

		/* Determine the line break position. */
		while (to < line.length()) {
			if (line[to].character == '\n')
				break;
			char c1 = line[to].character;
			char c2 = line[to - 1].character;
			if (
					c1 == ' ' ||
					c1 == '(' ||
					c1 == '[' ||
					c1 == '{' ||
					(
						(
							c2 == '-' ||
							c2 == '/' ||
							c2 == ':'
						) &&
							c1 != ',' &&
							c1 != '.' &&
							c1 != ';' &&
							c1 != ':'
					)
			   )
			{
				lbp = to++;
				while (to < line.length() && line[to].character == ' ')
					to++;
			} else {
				to++;
			}

			if (to - from > w && lbp != (Area::size_type) -1)
			{
				to = lbp;
				break;
			}
		}

		/* Copy the "from...to" range from the "line" to the bottom of
		 * the "res" Area. */
		Area::size_type x = 0;
		Area::size_type len = to - from;
		if (halign == Area::LEFT || len >= w) {
			;
		} else if (halign == Area::CENTER) {
			x += (w - len) / 2;
		} else if (halign == Area::RIGHT) {
			x += w - len;
		}
		res->insert(line.cells() + from, len, x, res->height());

		/* Determine the beginnning of the next line. */
		if (to == line.length())
			break;

		from = to;
		if (line[from].character == '\n') {
			++from;
		} else if (line[from].character == ' ') {
			do {
				++from;
			} while (from < line.length() && line[from].character == ' ');
		}
	}

	return res.release();
}

/* ------------------------------------------------------------------------- */

/*
 * Attempt to line-format all "elements". If one of the elements can only be
 * area-formatted, return null. In that case, "::format()" (below) will
 * probably work.
 */
static Line *
line_format(const list<auto_ptr<Element> > *elements)
{
	auto_ptr<Line> res;

	if (elements) {
		list<auto_ptr<Element> >::const_iterator i;
		for (i = elements->begin(); i != elements->end(); ++i) {
			auto_ptr<Line> l((*i)->line_format());
			if (!l.get())
				return 0;
			if (res.get()) {
				*res += *l;
			} else {
				res = l;
			}
		}
	}

	return res.release();
}

/* ------------------------------------------------------------------------- */

/*
 * Basically, a list of "Text"s is a stream of words that has to be formatted
 * into an area. But... as an extension to HTML 3.2 we want to allow "Block"s
 * be embedded in "Text", e.g.
 *
 *        <FONT COLOR=red><P>Bla</P><P>Bloh</P></FONT>
 *
 * Attempt to line-format the "Text". This will fail if there is a "Block"
 * inside the "Text".
 *
 * The "Text" could not be line-formatted, so... append a line-break and
 * the area-formatted "Text".
 */

static Area *
format(
	const list<auto_ptr<Element> > *elements,
	Area::size_type w,
	int halign
	)
{
	if (!elements)
		return 0;

	auto_ptr<Area> res;
	auto_ptr<Line> line;

	list<auto_ptr<Element> >::const_iterator i;
	for (i = elements->begin(); i != elements->end(); ++i) {
		if (!(*i).get())
			continue;

		auto_ptr<Line> l((*i)->line_format());
		if (l.get()) {
			if (line.get()) {
				*line += *l;
			} else {
				line = l;
			}
			continue;
		}

		auto_ptr<Area> a((*i)->format(w, halign));
		if (a.get()) {
			if (line.get()) {
				auto_ptr<Area> a2(make_up(*line, w, halign));
				if (a2.get()) {
					if (res.get()) {
						*res += *a2;
					} else {
						res = a2;
					}
				}
				line.reset();
			}
			if (res.get()) {
				*res += *a;
			} else {
				res = a;
			}
		}
	}

	if (line.get()) {
		auto_ptr<Area> a2(make_up(*line, w, halign));
		if (a2.get()) {
			if (res.get()) {
				*res += *a2;
			} else {
				res = a2;
			}
		}
	}

	return res.release();
}

/*
 * A copy of the above function, but the formatted text is printed to "os"
 * rather than into an Area.
 */
static void
format(
	const list<auto_ptr<Element> > *elements,
	Area::size_type indent_left,
	Area::size_type w,
	int halign,
	iconvstream &os
	)
{
	if (!elements)
		return;

	auto_ptr<Line> line;

	list<auto_ptr<Element> >::const_iterator i;
	for (i = elements->begin(); i != elements->end(); ++i) {
		if (!(*i).get())
			continue;

		auto_ptr<Line> l((*i)->line_format());
		if (l.get()) {
			if (line.get()) {
				*line += *l;
			} else {
				line = l;
			}
			continue;
		}

		auto_ptr<Area> a((*i)->format(w, halign));
		if (a.get()) {
			if (line.get()) {
				auto_ptr<Area> a2(make_up(*line, w, halign));
				if (a2.get()) {
					*a2 >>= indent_left;
					os << *a2 << flush;
				}
				line.reset();
			}
			*a >>= indent_left;
			os << *a << flush;
		}
	}

	if (line.get()) {
		auto_ptr<Area> a2(make_up(*line, w, halign));
		if (a2.get()) {
			*a2 >>= indent_left;
			os << *a2 << flush;
		}
	}
}

/* ------------------------------------------------------------------------- */

static Properties formatting_properties;

/* ----------------------- */

/*static*/ void
Formatting::setProperty(const char *key, const char *value)
{
	formatting_properties.setProperty(key, value);
}

/* ----------------------- */

/*static*/ void
Formatting::loadProperties(istream &is)
{
	formatting_properties.load(is);
}

/* ----------------------- */

/*static*/ const char *
Formatting::getString(const char *key, const char *dflt)
{
	return formatting_properties.getProperty(key, dflt);
}

// neue Methode fuer leere Attribute - Johannes Geiger

const char *
Formatting::getString(const char *key)
{
	return formatting_properties.getProperty(key);
}

/* ----------------------- */

/*
 * Property not set                               => 0
 * Property contains only white-space             => 0
 * Property conains one non-white-space character => { "x" }
 */
/*static*/ vector<string> *
Formatting::getStringVector(const char *key, const char *dflt)
{
	const char *p = formatting_properties.getProperty(key, dflt);
	if (!p)
		return 0;

	vector<string> *res = 0;
	for (;;) {
		while (isspace(*p))
			++p;
		if (!*p)
			break;
		const char *q = p + 1;
		while (*q && !isspace(*q))
			++q;
		if (!res)
			res = new vector<string>;
		res->push_back(string(p, q - p));
		p = q;
	}
	return res;
}

/* ----------------------- */

/*static*/ int
Formatting::getInt(const char *key, int dflt)
{
	const char *p = formatting_properties.getProperty(key, 0);
	return p ? atoi(p) : dflt;
}

/* ----------------------- */

/*static*/ vector<int> *
Formatting::getIntVector(const char *key, const char *dflt)
{
	const char *p = formatting_properties.getProperty(key, dflt);
	if (!p)
		return 0;

	vector<int> *res = 0;
	for (;;) {
		while (isspace(*p))
			++p;
		if (!*p)
			break;
		if (!res)
			res = new vector<int>;
		res->push_back(atoi(p));
		++p;
		while (*p && !isspace(*p))
			++p;
	}
	return res;
}

/* ----------------------- */

/*static*/ char
Formatting::getAttributes(const char *key, char dflt)
{
	auto_ptr<vector<string> > v(getStringVector(key, 0));

	if (!v.get() || v->empty())
		return dflt;

	char res = Cell::NONE;
	for (vector<string>::const_iterator i = v->begin(); i != v->end(); ++i) {
		if (!cmp_nocase(*i, "NONE"))
			res = Cell::NONE;
		else if (!cmp_nocase(*i, "BOLD"))
			res |= Cell::BOLD;
		else if (!cmp_nocase(*i, "UNDERLINE"))
			res |= Cell::UNDERLINE;
		else if (!cmp_nocase(*i, "STRIKETHROUGH"))
			res |= Cell::STRIKETHROUGH;
		else
			;
	}
	return res;
}

/* ------------------------------------------------------------------------- */

BlockFormat::BlockFormat(
	const char      *item_name,
	Area::size_type default_vspace_before /* = 0 */,
	Area::size_type default_vspace_after /* = 0 */,
	Area::size_type default_indent_left /* = 0 */,
	Area::size_type default_indent_right /* = 0 */,
	const char      *default_quote_char /* = "" */
	)
{
	char lb[80];

	sprintf(lb, "%s.vspace.before", item_name);
	vspace_before = Formatting::getInt(lb, default_vspace_before);
	sprintf(lb, "%s.vspace.after", item_name);
	vspace_after = Formatting::getInt(lb, default_vspace_after);
	sprintf(lb, "%s.indent.left", item_name);
	indent_left = Formatting::getInt(lb, default_indent_left);
	sprintf(lb, "%s.indent.right", item_name);
	indent_right = Formatting::getInt(lb, default_indent_right);
	sprintf(lb, "%s.prefix", item_name);
	quote_char = Formatting::getString(lb, default_quote_char);
}

Area::size_type
BlockFormat::effective_width(Area::size_type w) const
{
	/*
	 * No problem if "w" is wide enough...
	 */
	if (indent_left + 10 + indent_right <= w) {
		return w - indent_left - indent_right;
	}

	/*
	 * Does reducing the right indent help?
	 */
	if (indent_left + 10 <= w)
		return 10;

	/*
	 * Do it with right indent == 0.
	 */
	if (indent_left + 1 <= w)
		return w - indent_left;

	/*
	 * Even that doesn't help, return "1".
	 */
	return 1;
}

/* ------------------------------------------------------------------------- */

ListFormat::ListFormat(
	const char      *item_name,
	Area::size_type default_vspace_before /* = 0 */,
	Area::size_type default_vspace_between /* = 0 */,
	Area::size_type default_vspace_after /* = 0 */,
	const char      *default_indents /* = "6" */,
	const char      *default_default_types/* = "DISC CIRCLE SQUARE" */
	)
{
	char lb[80];

	sprintf(lb, "%s.vspace.before", item_name);
	vspace_before = Formatting::getInt(lb, default_vspace_before);
	sprintf(lb, "%s.vspace.between", item_name);
	vspace_between = Formatting::getInt(lb, default_vspace_between);
	sprintf(lb, "%s.vspace.after", item_name);
	vspace_after = Formatting::getInt(lb, default_vspace_after);
	sprintf(lb, "%s.indents", item_name);
	indents.reset(Formatting::getIntVector(lb, default_indents));
	sprintf(lb, "%s.default_types", item_name);
	default_types.reset(Formatting::getStringVector(lb, default_default_types));
}

Area::size_type
ListFormat::get_indent(int nesting) const
{
	return
		(!indents.get() || indents.get()->empty()) ? 6 :
		nesting < indents->size() ? (*indents)[nesting] :
		indents->back()
	;
}

int
ListFormat::get_type(
	const list<TagAttribute> *attributes,
	int nesting,
	int default_default_type
	) const
{
	const char *default_type = (
		!default_types.get() || default_types->empty() ? 0 :
		nesting < default_types->size() ?
		(*default_types)[nesting].c_str() :
		default_types->back().c_str()
		);

	return get_attribute(
			attributes, "TYPE",
			default_type, // dflt1
			default_default_type, // dflt2, if dflt1 fails
			"NO_BULLET", NO_BULLET,
			"DISC", DISC,
			"SQUARE", SQUARE,
			"CIRCLE", CIRCLE,
			"CUSTOM1", CUSTOM1,
			"CUSTOM2", CUSTOM2,
			"CUSTOM3", CUSTOM3,
			"1", ARABIC_NUMBERS,
			"a", LOWER_ALPHA,
			"A", UPPER_ALPHA,
			"i", LOWER_ROMAN,
			"I", UPPER_ROMAN,
			NULL
			);
}

/* ------------------------------------------------------------------------- */

