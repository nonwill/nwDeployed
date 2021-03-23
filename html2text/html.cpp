
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
 * Fre Jun  8 17:43:02 CEST 2001: new method
 * Wed Jul  2 22:07:12 CEST 2003: ported to g++ 3.3
 */

/***************************************************************************/


#include <stdlib.h>
#include <stdarg.h>

#include "html.h"
#include "HTMLParser.tab.hh"
#include "cmp_nocase.h"
#include "iconvstream.h"

/* ------------------------------------------------------------------------- */

/*
 * Define some helpers.
 */

#define define_foreach(T, args, action) \
	void foreach args { \
		for (T::const_iterator i = l.begin(); i != l.end(); ++i) { \
			action; \
		} \
	}

#define pack(T) \
	define_foreach(list<auto_ptr<T> >, ( \
			const list<auto_ptr<T> > &l, \
			iconvstream& os, \
			ostream_manipulator separator \
			), (*i)->unparse(os, separator))

static pack(Element)
static pack(TableCell)
static pack(TableRow)
static pack(ListItem)
static pack(Option)
static pack(DefinitionListItem)
static pack(Script)
static pack(Style)
static pack(Meta)

#undef pack

/*
 * Special helper for "const auto_ptr<list<TagAttribute> > &".
 */
static iconvstream &operator<<(iconvstream& os,
		const auto_ptr<list<TagAttribute> > &a)
{
	if (a.get()) {
		const list<TagAttribute> &al(*a);
		list<TagAttribute>::const_iterator i;
		for (i = al.begin(); i != al.end(); ++i) {
			os << " " << (*i).first << "=\"" << (*i).second.c_str() << "\"";
		}
	}
	return os;
}

/* ------------------------------------------------------------------------- */

/*
 * Some C++ compilers (e.g. EGCS 2.91.66) have problems if all virtual
 * methods of a class are inline or pure virtual, so we define the virtual
 * "Element::~Element()", which is the only virtual method, non-inline,
 * although it is empty.
 */
Element::~Element()
{}

/* ------------------------------------------------------------------------- */

void
Document::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<HTML" << attributes << ">" << separator;
	head.unparse(os, separator);
	body.unparse(os, separator);
	os << "</HTML>" << separator;
}

void
Head::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<HEAD>" << separator;
	if (title.get()) {
		os << "<TITLE>" << separator;
		title->unparse(os, separator);
		os << "</TITLE>" << separator;
	}
	if (isindex_attributes.get()) {
		os << "<ISINDEX" << isindex_attributes << ">" << endl;
	}
	if (base_attributes.get())
		os << "<BASE" << base_attributes << ">" << endl;
	foreach(scripts, os, separator);
	foreach(styles, os, separator);
	foreach(metas, os, separator);
	if (link_attributes.get())
		os << "<LINK" << link_attributes << ">" << endl;
	os << "</HEAD>" << separator;
}

void
Meta::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<META" << attributes << ">" << separator;
}

void
Script::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os
		<< "<SCRIPT" << attributes << ">" << separator
		<< text
		<< "</SCRIPT>" << separator;
}

void
Style::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os
		<< "<STYLE" << attributes << ">" << separator
		<< text
		<< "</STYLE>" << separator;
}

void
Body::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<BODY" << attributes << ">" << separator;
	if (content.get())
		foreach(*content, os, separator);
	os << "</BODY>" << separator;
}

void
PCData::unparse(iconvstream& os, ostream_manipulator separator) const
{
	for (string::size_type j = 0; j < text.length(); ++j) {
		char c = text[j];
		switch (((int) c) & 255) {
			case '&':         os << "&amp;";
							  break;
			case '<':         os << "&lt;";
							  break;
			case '>':         os << "&gt;";
							  break;
			case '"':         os << "&quot;";
							  break;
			default:
							  if (c & 0x80) {
								  os << "&#" << (((int) c) & 255) << ";";
							  } else {
								  os << c;
							  }
							  break;
		}
	}

	os << separator;
}

void
Heading::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<H" << level << attributes << ">" << separator;
	if (content.get())
		foreach(*content, os, separator);
	os << "</H" << level << ">" << separator;
}

void
Paragraph::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<P" << attributes << ">" << separator;
	if (texts.get())
		foreach(*texts, os, separator);
	os << "</P>" << separator;
}

void
Image::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<IMG" << attributes << ">" << separator;
}

void
Applet::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<APPLET" << attributes << ">" << separator;
	if (content.get())
		foreach(*content, os, separator);
	os << "</APPLET>" << separator;
}

void
Param::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<PARAM" << attributes << ">" << separator;
}

void
Division::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<DIV" << attributes << ">" << separator;
	if (body_content.get())
		foreach(*body_content, os, separator);
	os << "</DIV>" << separator;
}

void
Center::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<CENTER>" << separator;
	if (body_content.get())
		foreach(*body_content, os, separator);
	os << "</CENTER>" << separator;
}

void
BlockQuote::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<BLOCKQUOTE>" << separator;
	if (content.get())
		foreach(*content, os, separator);
	os << "</BLOCKQUOTE>" << separator;
}

void
Address::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<ADDRESS>" << separator;
	if (content.get())
		foreach(*content, os, separator);
	os << "</ADDRESS>" << separator;
}

void
Form::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<FORM" << attributes << ">" << separator;
	if (content.get())
		foreach(*content, os, separator);
	os << "</FORM>" << separator;
}

void
Preformatted::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<PRE" << attributes << ">" << separator;
	if (texts.get()) {
		foreach(*texts, os, '\0');
		os << separator;
	}
	os << "</PRE>" << separator;
}

void
HorizontalRule::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<HR" << attributes << ">" << separator;
}

void
Input::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<INPUT" << attributes << ">" << separator;
}

void
Option::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<OPTION" << attributes << ">" << separator;
	if (pcdata.get())
		pcdata->unparse(os, separator);
	os << "</OPTION>" << endl;
}

void
Select::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<SELECT" << attributes << ">" << separator;
	if (content.get())
		foreach(*content, os, separator);
	os << "</SELECT>" << endl;
}

void
TextArea::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<TEXTAREA" << attributes << ">" << separator;
	if (pcdata.get())
		pcdata->unparse(os, separator);
	os << "</TEXTAREA>" << endl;
}

/* ------------------------------------------------------------------------- */

void
OrderedList::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<OL" << attributes << ">" << separator;
	if (items.get())
		foreach(*items, os, separator);
	os << "</OL>" << separator;
}

void
UnorderedList::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<UL" << attributes << ">" << separator;
	if (items.get())
		foreach(*items, os, separator);
	os << "</UL>" << separator;
}

void
Dir::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<DIR" << attributes << ">" << separator;
	if (items.get())
		foreach(*items, os, separator);
	os << "</DIR>" << separator;
}

void
Menu::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<MENU" << attributes << ">" << separator;
	if (items.get())
		foreach(*items, os, separator);
	os << "</MENU>" << separator;
}

void
ListNormalItem::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<LI" << attributes << ">" << separator;
	if (flow.get())
		foreach(*flow, os, separator);
	os << "</LI>" << separator;
}

void
ListBlockItem::unparse(iconvstream& os, ostream_manipulator separator) const
{
	if (block.get())
		block->unparse(os, separator);
}

/* ------------------------------------------------------------------------- */

void
DefinitionList::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<DL" << attributes << ">" << separator;
	if (items.get())
		foreach(*items, os, separator);
	os << "</DL>" << separator;
}

void
TermName::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<DT>" << separator;
	if (flow.get())
		foreach(*flow, os, separator);
	os << "</DT>" << separator;
}

void
TermDefinition::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<DD>" << separator;
	if (flow.get())
		foreach(*flow, os, separator);
	os << "</DD>" << separator;
}

/* ------------------------------------------------------------------------- */

void
Table::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<TABLE" << attributes << ">" << separator;
	if (caption.get())
		caption->unparse(os, separator);
	if (rows.get())
		foreach(*rows, os, separator);
	os << "</TABLE>" << separator;
}

void
NoBreak::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<NOBR>" << separator;
	if (content.get())
		foreach(*content, os, separator);
	os << "</NOBR>" << separator;
}

void
Font::unparse(iconvstream& os, ostream_manipulator separator) const
{
	const char *tag = (
		attribute == HTMLParser_token::TT     ? "TT" :
		attribute == HTMLParser_token::I      ? "I" :
		attribute == HTMLParser_token::B      ? "B" :
		attribute == HTMLParser_token::U      ? "U" :
		attribute == HTMLParser_token::STRIKE ? "STRIKE" :
		attribute == HTMLParser_token::BIG    ? "BIG" :
		attribute == HTMLParser_token::SMALL  ? "SMALL" :
		attribute == HTMLParser_token::SUB    ? "SUB" :
		attribute == HTMLParser_token::SUP    ? "SUP" :
		"???"
		);

	os << "<" << tag << ">" << separator;
	if (texts.get())
		foreach(*texts, os, separator);
	os << "</" << tag << ">" << separator;
}

void
Phrase::unparse(iconvstream& os, ostream_manipulator separator) const
{
	const char *tag = (
		attribute == HTMLParser_token::EM     ? "EM" :
		attribute == HTMLParser_token::STRONG ? "STRONG" :
		attribute == HTMLParser_token::DFN    ? "DFN" :
		attribute == HTMLParser_token::CODE   ? "CODE" :
		attribute == HTMLParser_token::SAMP   ? "SAMP" :
		attribute == HTMLParser_token::KBD    ? "KBD" :
		attribute == HTMLParser_token::VAR    ? "VAR" :
		attribute == HTMLParser_token::CITE   ? "CITE" :
		"???"
		);

	os << "<" << tag << ">" << separator;
	if (texts.get())
		foreach(*texts, os, separator);
	os << "</" << tag << ">" << separator;
}

void
Anchor::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<A" << attributes << ">" << separator;
	if (texts.get())
		foreach(*texts, os, separator);
	os << "</A>" << separator;
}

void
BaseFont::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<BASEFONT" << attributes << ">" << separator;
}

void
LineBreak::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<BR" << attributes << ">" << separator;
}

void
Map::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<MAP" << attributes << ">" << separator;
	if (areas.get()) {
		const list<auto_ptr<list<TagAttribute> > >           &al(*areas);
		list<auto_ptr<list<TagAttribute> > >::const_iterator i;
		for (i = al.begin(); i != al.end(); ++i) {
			os << "<AREA" << *i << ">" << separator;
		}
	}
	os << "</MAP>" << separator;
}

void
Font2::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<FONT" << attributes << ">" << separator;
	if (elements.get())
		foreach(*elements, os, separator);
	os << "</FONT>" << separator;
}

void
TableRow::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<TR" << attributes << ">" << separator;
	if (cells.get())
		foreach(*cells, os, separator);
	os << "</TR>" << separator;
}

void
TableCell::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<TD" << attributes << ">" << separator;
	if (content.get())
		foreach(*content, os, separator);
	os << "</TD>" << separator;
}

void
TableHeadingCell::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<TH" << attributes << ">" << separator;
	if (content.get())
		foreach(*content, os, separator);
	os << "</TH>" << separator;
}

void
Caption::unparse(iconvstream& os, ostream_manipulator separator) const
{
	os << "<CAPTION" << attributes << ">" << separator;
	if (texts.get())
		foreach(*texts, os, separator);
	os << "</CAPTION>" << separator;
}

/* ------------------------------------------------------------------------- */

istr
get_attribute(
	const list<TagAttribute> *as,
	const char               *name,
	const char               *dflt
	)
{
	if (as) {
		list<TagAttribute>::const_iterator i;
		for (i = as->begin(); i != as->end(); ++i) {
			if (cmp_nocase((*i).first, name) == 0)
				return (*i).second;
		}
	}
	return istr(dflt);
}

// *exists is set to false if attribute *name does not exist - Johannes Geiger

istr
get_attribute(
	const list<TagAttribute> *as,
	const char               *name,
	bool                     *exists
	)
{
	*exists = true;
	if (as) {
		list<TagAttribute>::const_iterator i;
		for (i = as->begin(); i != as->end(); ++i) {
			if (cmp_nocase((*i).first, name) == 0)
				return (*i).second;
		}
	}
	*exists = false;
	return istr("");
}


int
get_attribute(
	const list<TagAttribute> *as,
	const char               *name,
	int dflt
	)
{
	if (as) {
		list<TagAttribute>::const_iterator i;
		for (i = as->begin(); i != as->end(); ++i) {
			if (cmp_nocase((*i).first, name) == 0)
				return atoi((*i).second.c_str());
		}
	}
	return dflt;
}

int
get_attribute(
	const list<TagAttribute> *as,
	const char               *name,
	int dflt,
	const char               *s1,
	int v1,
	...
	)
{
	if (as) {
		list<TagAttribute>::const_iterator i;
		for (i = as->begin(); i != as->end(); ++i) {
			if (cmp_nocase((*i).first, name) == 0) {
				const char *s = s1;
				int v = v1;

				va_list va;
				va_start(va, v1);
				for (;;) {
					if (cmp_nocase(s, (*i).second.c_str()) == 0)
						break;
					s = va_arg(va, const char *);
					if (!s) {
						v = dflt;
						break;
					}
					v = va_arg(va, int);
				}
				va_end(va);
				return v;
			}
		}
	}
	return dflt;
}

int
get_attribute(
	const list<TagAttribute> *as,
	const char               *name, // Attribute name
	const char               *dflt1,// If attribute not specified
	int dflt2,                      // If string value does not match s1, ...
	const char               *s1,
	int v1,
	...
	)
{
	if (as) {
		list<TagAttribute>::const_iterator i;
		for (i = as->begin(); i != as->end(); ++i) {
			if (cmp_nocase((*i).first, name) == 0) {
				dflt1 = (*i).second.c_str();
				break;
			}
		}
	}

	if (!dflt1)
		return dflt2;

	const char *s = s1;
	int v = v1;

	va_list va;
	va_start(va, v1);
	for (;;) {
		if (cmp_nocase(s, dflt1) == 0)
			break;
		s = va_arg(va, const char *);
		if (!s)
			break;
		v = va_arg(va, int);
	}
	va_end(va);

	return s ? v : dflt2;
}

/* ------------------------------------------------------------------------- */

istr
get_style_attr(istr *style, const char *name, const char *dflt)
{
	bool iskey = true;
	size_t keystart = 0;
	size_t valstart = 0;
	size_t t;

	if (style != NULL) {
		for (size_t i = 0; i < style->length(); i++) {
			switch ((*style)[i]) {
				case ':':
					/* keystart:i = key */
					for (t = i - 1; (*style)[t] == ' '; t--)
						;
					t++;
					iskey = false;
					valstart = i + 1;
					break;
				case ';':
					/* end of value */
					if (style->compare(keystart, t - keystart, name) == 0) {
						for (t = i - 1; (*style)[t] == ' '; t--)
							;
						t++;
						return style->slice(valstart, t - valstart);
					}
					keystart = i + 1;
					iskey = true;
					break;
				case ' ':
					if (keystart == i)
						keystart++;
					if (valstart == i)
						valstart++;
					break;
			}
		}
	}

	return istr(dflt);
}
