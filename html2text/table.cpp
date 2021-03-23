
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
 * Mon Jul 22 13:30:32 CEST 2002: fixed segfault
 */

/***************************************************************************/


/* ------------------------------------------------------------------------- */

/*
 * "Table::format()" has been taken out of "format.C" because it is way more
 * complex than the "format()" methods of the other HTML elements.
 */

/* ------------------------------------------------------------------------- */

#ident "$Id: table.C,v 1.15 1999/11/12 18:57:35 arno Exp $"

#include <iostream>
#include "html.h"
#include "auto_aptr.h"
#include "format.h"

/* ------------------------------------------------------------------------- */

// Should be local to "Table::format()", but CFRONT can't handle this.
struct LogicalCell {
	const TableCell *cell;    // Points to the parsed cell..
	int x, y;                 // Position of cell in table.
	int w, h;                 // COLSPAN/ROWSPAN of cell.
	int halign;
	int valign;
	Area::size_type width;    // Current contents width.
	bool minimized;           // Cannot be narrowed any more.
	auto_ptr<Area>  area;     // Formatted cell -- computed at a late stage.
};

/* ------------------------------------------------------------------------- */

/*
 * Correct x and y of the logical cells according to the other cells' ROWSPAN
 * and COLSPAN.
 */
static void
correct_xy(
	list<auto_ptr<LogicalCell> > *const lcs_in_out,
	int                          *const number_of_rows_in_out,
	int                          *const number_of_columns_in_out
	)
{
	list<auto_ptr<LogicalCell> >::iterator i;
	for (i = lcs_in_out->begin(); i != lcs_in_out->end(); ++i) {
		const LogicalCell *const p = (*i).get();
		if (p->w != 1 || p->h != 1) {
			list<auto_ptr<LogicalCell> >::iterator j;
			for (j = i, ++j; j != lcs_in_out->end(); ++j) {
				LogicalCell *const q = (*j).get();
				if (q->y != p->y)
					break;
				q->x += p->w - 1;
			}
			for (; j != lcs_in_out->end(); ++j) {
				LogicalCell *const q = (*j).get();
				if (q->y >= p->y + p->h)
					break;
				if (q->x >= p->x)
					q->x += p->w;
			}
		}
		if (p->x + p->w > *number_of_columns_in_out)
			*number_of_columns_in_out = p->x + p->w;
		if (p->y + p->h > *number_of_rows_in_out)
			*number_of_rows_in_out = p->y + p->h;
	}
}

static void
create_lcs(
	const Table                  &t,
	const Area::size_type w,
	const Area::size_type left_border_width,
	const Area::size_type right_border_width,
	const Area::size_type column_spacing,
	list<auto_ptr<LogicalCell> > *const lcs_return,
	int                          *const number_of_rows_return,
	int                          *const number_of_columns_return
	)
{
	*number_of_rows_return = 0;
	*number_of_columns_return = 0;

	const list<auto_ptr<TableRow> >            &rl(*t.rows);
	list<auto_ptr<TableRow> >::const_iterator ri;
	int y;
	for (ri = rl.begin(), y = 0; ri != rl.end(); ++ri, ++y) {
		if (!(*ri).get())
			continue;

		const TableRow &row(**ri);

		istr style = get_attribute(row.attributes.get(), "STYLE", "");

		int row_halign = get_attribute(
				row.attributes.get(), "ALIGN", -1,
				"LEFT", Area::LEFT,
				"CENTER", Area::CENTER,
				"RIGHT", Area::RIGHT,
				NULL
				);
		if (row_halign == -1) {
			istr align = get_style_attr(&style, "text-align", "");
			if (align.compare(0, 7, "center") == 0)
				row_halign = Area::CENTER;
			else if (align.compare(0, 6, "right") == 0)
				row_halign = Area::RIGHT;
			else
				row_halign = Area::LEFT;
		}
		int row_valign = get_attribute(
				row.attributes.get(), "VALIGN", -1,
				"TOP", Area::TOP,
				"MIDDLE", Area::MIDDLE,
				"BOTTOM", Area::BOTTOM,
				NULL
				);
		if (row_valign == -1) {
			istr align = get_style_attr(&style, "vertical-align", "");
			if (align.compare(0, 4, "top") == 0)
				row_valign = Area::TOP;
			else if (align.compare(0, 7, "bottom") == 0)
				row_valign = Area::BOTTOM;
			else
				row_valign = Area::MIDDLE;
		}

		const list<auto_ptr<TableCell> >           &cl(*row.cells);
		list<auto_ptr<TableCell> >::const_iterator ci;
		int x;
		for (ci = cl.begin(), x = 0; ci != cl.end(); ++ci, ++x) {
			if (!(*ci).get())
				continue;
			const TableCell &cell(**ci);

			auto_ptr<LogicalCell> p(new LogicalCell);
			p->cell = &cell;
			p->x = x;
			p->y = y;
			p->w = get_attribute(cell.attributes.get(), "COLSPAN", 1);
			p->h = get_attribute(cell.attributes.get(), "ROWSPAN", 1);
			if (p->w < 1)
				p->w = 1;
			if (p->h < 1)
				p->h = 1;
			if (x + p->w > *number_of_columns_return)
				*number_of_columns_return = x + p->w;
			if (y + p->h > *number_of_rows_return)
				*number_of_rows_return = y + p->h;
			istr style = get_attribute(cell.attributes.get(), "STYLE", "");

			p->halign = get_attribute(
					cell.attributes.get(), "ALIGN", -1,
					"LEFT", Area::LEFT,
					"CENTER", Area::CENTER,
					"RIGHT", Area::RIGHT,
					NULL
					);
			if (p->halign == -1) {
				istr align = get_style_attr(&style, "text-align", "");
				if (align.compare(0, 7, "center") == 0)
					p->halign = Area::CENTER;
				else if (align.compare(0, 6, "right") == 0)
					p->halign = Area::RIGHT;
				else if (align.compare(0, 5, "left") == 0)
					p->halign = Area::LEFT;
				else
					p->halign = row_halign;
			}
			p->valign = get_attribute(
					cell.attributes.get(), "VALIGN", -1,
					"TOP", Area::TOP,
					"MIDDLE", Area::MIDDLE,
					"BOTTOM", Area::BOTTOM,
					NULL
					);
			if (p->valign == -1) {
				istr align = get_style_attr(&style, "vertical-align", "");
				if (align.compare(0, 4, "top") == 0)
					p->valign = Area::TOP;
				else if (align.compare(0, 7, "bottom") == 0)
					p->valign = Area::BOTTOM;
				else if (align.compare(0, 7, "middle") == 0)
					p->valign = Area::MIDDLE;
				else
					p->valign = row_valign;
			}
			{
				auto_ptr<Area> tmp(cell.format(
						w
						- left_border_width
						- right_border_width
						- (*number_of_columns_return - 1) * (column_spacing + 0),
						Area::LEFT // Yields better results than "p->halign"!
						));
				p->width = tmp.get() ? tmp->width() : 0;
			}
			p->minimized = false;

			lcs_return->push_back(p);
		}
	}

	correct_xy(lcs_return, number_of_rows_return, number_of_columns_return);
}

static void
compute_widths(
	const list<auto_ptr<LogicalCell> > &lcs,
	const int number_of_columns,
	const Area::size_type column_spacing,
	const Area::size_type left_border_width,
	const Area::size_type right_border_width,
	Area::size_type                    *const table_width_return,
	Area::size_type                    *const column_widths_return
	)
{
	/*
	 * Compute the column widths.
	 */
	{
		for (int i = 0; i < number_of_columns; i++)
			column_widths_return[i] = 0;
	}
	for (int x = 0; x < number_of_columns; x++) {
		list<auto_ptr<LogicalCell> >::const_iterator i;
		for (i = lcs.begin(); i != lcs.end(); ++i) {
			const LogicalCell &lc(**i);
			if (x != lc.x + lc.w - 1)
				continue;                  // Cell not in column?
			Area::size_type width = lc.width;
			for (int j = lc.x; j < x; j++) {
				// Beware! "width" is unsigned!
				if (column_widths_return[j] + column_spacing >= width) {
					width = 0;
				} else {
					width -= column_widths_return[j] + column_spacing;
				}
			}
			if (width >= column_widths_return[x])
				column_widths_return[x] = width;
		}
	}

	/*
	 * Compute the table width.
	 */
	*table_width_return = (
		left_border_width
		+ (number_of_columns - 1) * column_spacing
		+ right_border_width
		);
	{
		for (int x = 0; x < number_of_columns; x++) {
			*table_width_return += column_widths_return[x];
		}
	}
}

/*
 * Examine the table for the widest column that can be narrowed. (A column
 * cannot be narrowed if its widest cell cannot be narrowed.) Return "false"
 * if none of the columns can be narrowed. Otherwise, attempt to narrow that
 * column and update "lcs_in_out", "column_widths_in_out", and
 * "table_width_in_out".
 */
static bool
narrow_table(
	list<auto_ptr<LogicalCell> > *const lcs_in_out,
	const int number_of_columns,
	const Area::size_type column_spacing,
	Area::size_type              *const column_widths_in_out,
	Area::size_type              *const table_width_in_out
	)
{
	/*
	 * Seek for the widest column that can still be narrowed.
	 */
	int widest_column = -1;
	for (int x = 0; x < number_of_columns; x++) {
		// Zero width columns cannot be narrowed.
		if (column_widths_in_out[x] == 0)
			continue;
		// Is this the widest column so far?
		if (
			widest_column != -1 &&
			column_widths_in_out[x] <= column_widths_in_out[widest_column]
			)
			continue;
		// Yes; can the widest cell(s) in this column be narrowed?
		const list<auto_ptr<LogicalCell> >           &lcl(*lcs_in_out);
		list<auto_ptr<LogicalCell> >::const_iterator i;
		for (i = lcl.begin(); i != lcl.end(); ++i) {
			const LogicalCell &lc = **i;
			if (lc.x + lc.w - 1 != x)
				continue;                   // Not in this column.
			if (!lc.minimized)
				continue;
			Area::size_type left_of_cell = 0;
			for (int j = lc.x; j < x; j++) {
				left_of_cell += column_widths_in_out[j] + column_spacing;
			}
			if (lc.width >= left_of_cell + column_widths_in_out[x]) {
				// Minimized cell is as wide as the column; cannot narrow this
				// column.
				break;
			}
		}
		if (i == lcl.end())
			widest_column = x;
	}

	/*
	 * Give up if there is no more cell that can be narrowed.
	 */
	if (widest_column == -1)
		return false;

	/*
	 * Attempt to narrow the "widest_column" by one character.
	 */
	const Area::size_type old_column_width =
		column_widths_in_out[widest_column];
	list<auto_ptr<LogicalCell> >::iterator i;
	Area::size_type new_column_width = 0;
	for (i = lcs_in_out->begin(); i != lcs_in_out->end(); ++i) {
		LogicalCell &lc = **i;
		if (lc.x + lc.w - 1 != widest_column)
			continue;                                // Not in this column.
		Area::size_type left_of_column = 0;
		for (int j = lc.x; j < widest_column; ++j) {
			left_of_column += column_widths_in_out[j] + column_spacing;
		}
		Area::size_type w = lc.width;
		if (w >= left_of_column + old_column_width) {
			auto_ptr<Area> tmp(lc.cell->format(
					left_of_column + old_column_width - 1,
					Area::LEFT // Yields better results than "lc.halign"!
					));
			w = tmp->width();
			if (w >= left_of_column + old_column_width)
				lc.minimized = true;
		}
		if (w > left_of_column + new_column_width) {
			new_column_width = w - left_of_column;
		}
	}
	column_widths_in_out[widest_column] = new_column_width;
	*table_width_in_out -= old_column_width - new_column_width;

//cerr
//  << "Narrowed column "
//  << widest_column
//  << " from "
//  << old_column_width
//  << " to "
//  << new_column_width
//  << endl;
//cerr << "table_width=" << *table_width_in_out << endl;
//for (int z = 0; z < number_of_columns; ++z) {
//  cerr << "column_widths[" << z << "]=" << column_widths_in_out[z] << endl;
//}

	return true;
}

/*
 * Compute the heights of each row. Take into account the cells of the row,
 * plus the cells above that "hang" into the row.
 *
 * As a side effect, format the table cells.
 */
static void
compute_row_heights(
	list<auto_ptr<LogicalCell> > *lcs_in_out,
	const int number_of_rows,
	const Area::size_type row_spacing,
	Area::size_type              *const row_heights_return,
	const int column_spacing,
	const Area::size_type        *column_widths
	)
{
	{
		for (int y = 0; y < number_of_rows; y++)
			row_heights_return[y] = 0;
	}

	list<auto_ptr<LogicalCell> >::reverse_iterator i;
	for (i = lcs_in_out->rbegin(); i != lcs_in_out->rend(); ++i) {
		LogicalCell &lc(**i);
		Area::size_type w = (lc.w - 1) * column_spacing;
		for (int x = lc.x; x < lc.x + lc.w; ++x)
			w += column_widths[x];
		lc.area.reset(lc.cell->format(w, lc.halign));
		if (!lc.area.get())
			continue;
//    cerr << "lc.halign=" << lc.halign << ", w=" << w << endl;
//    cerr
//      << "***" << *lc.area << "***" << endl <<
//      lc.area->width() << "x" << lc.area->height() << endl;
		Area::size_type h = (lc.h - 1) * row_spacing;
		{
			for (int y = lc.y; y < lc.y + lc.h; ++y)
				h += row_heights_return[y];
		}
		if (lc.area->height() > h) {
			row_heights_return[lc.y + lc.h - 1] += lc.area->height() - h;
		}
	}
}

/* ------------------------------------------------------------------------- */

// <TABLE> Attributes: ALIGN (processed) WIDTH (ignored) BORDER (processed)
//                     CELLSPACING CELLPADDING (ignored)
// <TR> Attributes:    ALIGN VALIGN (processed)
// <TD> Attributes:    NOWRAP (ignored) ROWSPAN COLSPAN ALIGN VALIGN
//                     (processed) WIDTH HEIGHT (ignored)
Area *
Table::format(Area::size_type w, int halign) const
{
	int ahalign = get_attribute(
			attributes.get(), "ALIGN", -1,
			"LEFT", Area::LEFT,
			"CENTER", Area::CENTER,
			"RIGHT", Area::RIGHT,
			NULL
			);
	if (ahalign == -1) {
		istr style = get_attribute(attributes.get(), "STYLE", "");
		istr align = get_style_attr(&style, "text-align", "");

		if (align.compare(0, 5, "left") == 0)
			ahalign = Area::LEFT;
		else if (align.compare(0, 7, "center") == 0)
			ahalign = Area::CENTER;
		else if (align.compare(0, 6, "right") == 0)
			ahalign = Area::RIGHT;
	}
	if (ahalign != -1)
		halign = ahalign;

	// <TABLE>          => default => no border
	// <TABLE BORDER>   => ""      => draw border
	// <TABLE BORDER=0> => "0"     => no border
	// <TABLE BORDER=1> => "1"     => draw border
	bool draw_border = get_attribute(attributes.get(), "BORDER", "0") != "0";

	static const Area::size_type column_spacing = 1;
	static const Area::size_type row_spacing = 0;
	Area::size_type left_border_width = draw_border ? 1 : 0;
	Area::size_type right_border_width = draw_border ? 1 : 0;
	static const Area::size_type top_border_width = 0;
	static const Area::size_type bottom_border_width = 0;

	/*
	 * Iterate through the table's cells and create a list of "LogicalCell"s.
	 * Compute the positions and sizes of all cells, format their contents, and
	 * compute the number of rows and columns.
	 */
	list<auto_ptr<LogicalCell> > lcs;
	int number_of_rows, number_of_columns;

	create_lcs(
			*this,
			w,
			left_border_width,
			right_border_width,
			column_spacing,
			&lcs,
			&number_of_rows,
			&number_of_columns
			);

	/*
	 * The code below relies on that a table has 1 or more rows and one or
	 * more columns. Arno Unkrig 2002-07-21.
	 */
	if (number_of_rows == 0 || number_of_columns == 0)
		return 0;

	/*
	 * Now compute the column widths and the table width.
	 */
	auto_aptr<Area::size_type> column_widths =
		new Area::size_type[number_of_columns];
	Area::size_type table_width;

	compute_widths(
			lcs,
			number_of_columns,
			column_spacing,
			left_border_width,
			right_border_width,
			&table_width,
			column_widths.get()
			);

	/*
	 * Narrow the widest column that can be narrowed, until the entire table is
	 * narrow enough.
	 */
	while (table_width > w) {
		if (!narrow_table(
				&lcs,      /* in/out */
				number_of_columns,
				column_spacing,
				column_widths.get(), /* in/out */
				&table_width /* in/out */
				))
			break;
	}

	/*
	 * At this point, all cells are formatted such that the table width fits
	 * into "w" (if possible).
	 */

	/*
	 * Compute row heights.
	 */
	auto_aptr<Area::size_type> row_heights = new Area::size_type[number_of_rows];

	compute_row_heights(
			&lcs,
			number_of_rows,
			row_spacing, row_heights.get(),
			column_spacing, column_widths.get()
			);

	Area::size_type table_height = (
		top_border_width +
		(number_of_rows - 1) * row_spacing +
		bottom_border_width
		);
	{
		for (int y = 0; y < number_of_rows; y++)
			table_height += row_heights[y];
	}

	/*
	 * Everything is prepared... start drawing!
	 */
	auto_ptr<Area> res(new Area);
	{
		static int vspace_before = Formatting::getInt("TABLE.vspace.before", 0);
		res->prepend(vspace_before);
	}

	Area::size_type x0 = 0;

	if (halign != Area::LEFT && table_width < w) {
		if (halign == Area::CENTER)
			x0 += (w - table_width) / 2;
		else if (halign == Area::RIGHT)
			x0 += w - table_width;
	}

	/*
	 * Draw the caption, if any.
	 */
	if (caption.get()) {
		auto_ptr<Area> cap(caption->format(table_width, Area::CENTER));
		if (cap.get() && cap->height() >= 1) {
			cap->add_attribute(Cell::BOLD);
			res->insert(*cap, x0, 0);
		}
	}

	/*
	 * Draw the top and the left border.
	 */
	Area::size_type y0 = res->height();
	if (draw_border) {
		if (y0 == 0)
			y0 = 1;
		res->fill('|', x0, y0, left_border_width, table_height);
		// Some trickery: The top border underline is easily masked by the bold
		// caption, so remove the boldness where possible in favor of the
		// underline.
		{
			Cell *cells = (*res)[y0 - 1];
			for (Area::size_type x = 0; x < res->width(); x++) {
				if (cells[x].character == ' ')
					cells[x].attribute = Cell::NONE;
			}
		}
		res->add_attribute(
				Cell::UNDERLINE,
				x0 + left_border_width, y0 - 1,
				table_width - left_border_width - right_border_width, 1
				);
	}

	/*
	 * Draw the cells and their bottom and right borders.
	 */
	{
		const list<auto_ptr<LogicalCell> >           &lcl(lcs);
		list<auto_ptr<LogicalCell> >::const_iterator i;
		for (i = lcl.begin(); i != lcl.end(); ++i) {
			const LogicalCell &lc = **i;

			// Calculate cell position.
			Area::size_type x = x0 + left_border_width, y = y0 + top_border_width;
			{
				for (int j = 0; j < lc.x; j++)
					x += column_widths[j] + column_spacing;
			}
			{
				for (int j = 0; j < lc.y; j++)
					y += row_heights  [j] + row_spacing;
			}

			// Calculate cell dimensions.
			Area::size_type w = (lc.w - 1) * column_spacing;
			{
				for (int j = lc.x; j < lc.x + lc.w; j++)
					w += column_widths[j];
			}
			Area::size_type h = (lc.h - 1) * row_spacing;
			{
				for (int j = lc.y; j < lc.y + lc.h; j++)
					h += row_heights[j];
			}

			// Draw cell contents and borders.
			if (lc.area.get()) {
				res->insert(*lc.area, x, y, w, h, lc.halign, lc.valign);
			}
			if (draw_border) {
				// If the right neighbor cell bottom is flush with this cell's bottom,
				// then also underline the border between the two cells.
				bool underline_column_separator = false;
				{
					int lx = lc.x + lc.w, ly = lc.y + lc.h;
					list<auto_ptr<LogicalCell> >::const_iterator j;
					for (j = lcl.begin(); j != lcl.end(); ++j) {
						const LogicalCell &lc2 = **j;
						if (lc2.x == lx && lc2.y + lc2.h == ly) {
							underline_column_separator = true;
							break;
						}
					}
				}
				res->add_attribute(
						Cell::UNDERLINE,
						x, y + h - 1,     // x, y
						w + underline_column_separator, 1 // w, h
						);
				res->fill('|', x + w, y, 1, h);
			}
		}
	}

	{
		static int vspace_after = Formatting::getInt("TABLE.vspace.after", 0);
		res->append(vspace_after);
	}

	return res.release();
}

/* ------------------------------------------------------------------------- */
