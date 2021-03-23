
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
 * Sun Apr  7 11:54:06 CEST 2002: Make some closing tags optional
 * Mon Jul 22 13:42:13 CEST 2002: Don't insert Paragraphs to other block elements
 * Mon Aug 12 17:14:57 CEST 2002: Make even more closing tags optional
 */
  
 /***************************************************************************/


%skeleton "lalr1.cc"
%require "3.5"
%language "c++"
%defines
%define api.namespace {html2text}
%define api.parser.class {HTMLParser}

%code requires {
#include <string>
#define HTMLParser_token html2text::HTMLParser::token
#include "html.h"
#include "istr.h"
class HTMLDriver;
}

%parse-param {HTMLDriver &drv}

%define parse.error verbose
%debug

%code {
#include "HTMLDriver.h"
// call the lex function of HTMLDriver instead of plain yylex
#undef yylex
#define yylex drv.lex
#undef yyerror
#define yyerror drv.yyerror
}

/* ------------------------------------------------------------------------- */

%union {
  Document                           *document;
  Element                            *element;
  list<auto_ptr<Element>>            *element_list;
  PCData                             *pcdata;
  istr                               *strinG;
  list<TagAttribute>                 *tag_attributes;
  int                                inT;
  list<auto_ptr<TableRow>>           *table_rows;
  list<auto_ptr<TableCell>>          *table_cells;
  ListItem                           *list_item;
  list<auto_ptr<ListItem>>           *list_items;
  Caption                            *caption;
  Heading                            *heading;
  list<auto_ptr<Option>>             *option_list;
  Option                             *option;
  DefinitionList                     *definition_list;
  list<auto_ptr<DefinitionListItem>> *definition_list_item_list;
  TermName                           *term_name;
  TermDefinition                     *term_definition;
  Preformatted                       *preformatted;
  Address                            *address;
  list<auto_ptr<list<TagAttribute>>> *tag_attributes_list;
}

%type  <document>                 document_
%type  <pcdata>                   pcdata
%type  <pcdata>                   opt_pcdata
%type  <element_list>             body_content
%type  <heading>                  heading
%type  <heading>                  HX
%type  <inT>                      END_HX
%type  <element>                  block
%type  <element>                  block_except_p
%type  <element>                  text
%type  <element_list>             texts
%type  <element_list>             opt_texts
%type  <element>                  font
%type  <element>                  phrase
%type  <element>                  special
%type  <element>                  form
%type  <table_rows>               table_rows
%type  <table_cells>              table_cells
%type  <caption>                  caption
%type  <caption>                  opt_caption
%type  <element_list>             applet_content
%type  <definition_list>          definition_list
%type  <definition_list_item_list>definition_list_content
%type  <term_name>                term_name
%type  <term_definition>          term_definition
%type  <option_list>              select_content
%type  <option>                   option
%type  <element>                  list
%type  <list_items>               list_content
%type  <list_item>                list_item
%type  <preformatted>             preformatted
%type  <element_list>             opt_flow
%type  <element_list>             flow
%type  <element>                  flow_
%type  <element_list>             paragraph_content
%type  <address>                  address
%type  <tag_attributes_list>      map_content

%type  <tag_attributes> opt_LI
%type  <tag_attributes> opt_P

%token                  DOCTYPE
%token <strinG>         PCDATA
%token                  SCAN_ERROR


%token <tag_attributes> A
%token <tag_attributes> ADDRESS
%token <tag_attributes> APPLET
%token <tag_attributes> AREA
%token <tag_attributes> B
%token <tag_attributes> BASE
%token <tag_attributes> BASEFONT
%token <tag_attributes> BIG
%token <tag_attributes> BLOCKQUOTE
%token <tag_attributes> BODY
%token <tag_attributes> BR
%token <tag_attributes> CAPTION
%token <tag_attributes> CENTER
%token <tag_attributes> CITE
%token <tag_attributes> CODE
%token <tag_attributes> DD
%token <tag_attributes> DFN
%token <tag_attributes> DIR
%token <tag_attributes> DIV
%token <tag_attributes> DL
%token <tag_attributes> DT
%token <tag_attributes> EM
%token <tag_attributes> FONT
%token <tag_attributes> FORM
%token <tag_attributes> H1
%token <tag_attributes> H2
%token <tag_attributes> H3
%token <tag_attributes> H4
%token <tag_attributes> H5
%token <tag_attributes> H6
%token <tag_attributes> HEAD
%token <tag_attributes> HR
%token <tag_attributes> HTML
%token <tag_attributes> I
%token <tag_attributes> IMG
%token <tag_attributes> INPUT
%token <tag_attributes> ISINDEX
%token <tag_attributes> KBD
%token <tag_attributes> LI
%token <tag_attributes> LINK
%token <tag_attributes> MAP
%token <tag_attributes> MENU
%token <tag_attributes> META
%token <tag_attributes> NOBR
%token <tag_attributes> OL
%token <tag_attributes> OPTION
%token <tag_attributes> P
%token <tag_attributes> PARAM
%token <tag_attributes> PRE
%token <tag_attributes> SAMP
%token <tag_attributes> SCRIPT
%token <tag_attributes> SELECT
%token <tag_attributes> SMALL
%token <tag_attributes> STRIKE
%token <tag_attributes> STRONG
%token <tag_attributes> STYLE
%token <tag_attributes> SUB
%token <tag_attributes> SUP
%token <tag_attributes> TABLE
%token <tag_attributes> TD
%token <tag_attributes> TEXTAREA
%token <tag_attributes> TH
%token <tag_attributes> TITLE
%token <tag_attributes> TR
%token <tag_attributes> TT
%token <tag_attributes> U
%token <tag_attributes> UL
%token <tag_attributes> VAR

%token                  END_A
%token                  END_ADDRESS
%token                  END_APPLET
%token                  END_B
%token                  END_BIG
%token                  END_BLOCKQUOTE
%token                  END_BODY
%token                  END_CAPTION
%token                  END_CENTER
%token                  END_CITE
%token                  END_CODE
%token                  END_DD
%token                  END_DFN
%token                  END_DIR
%token                  END_DIV
%token                  END_DL
%token                  END_DT
%token                  END_EM
%token                  END_FONT
%token                  END_FORM
%token                  END_H1
%token                  END_H2
%token                  END_H3
%token                  END_H4
%token                  END_H5
%token                  END_H6
%token                  END_HEAD
%token                  END_HTML
%token                  END_I
%token                  END_KBD
%token                  END_LI
%token                  END_MAP
%token                  END_MENU
%token                  END_NOBR
%token                  END_OL
%token                  END_OPTION
%token                  END_P
%token                  END_PRE
%token                  END_SAMP
%token                  END_SCRIPT
%token                  END_SELECT
%token                  END_SMALL
%token                  END_STRIKE
%token                  END_STRONG
%token                  END_STYLE
%token                  END_SUB
%token                  END_SUP
%token                  END_TABLE
%token                  END_TD
%token                  END_TEXTAREA
%token                  END_TH
%token                  END_TITLE
%token                  END_TR
%token                  END_TT
%token                  END_U
%token                  END_UL
%token                  END_VAR

%token                  END               0  "end of file"

/* ------------------------------------------------------------------------- */

%start document

%% /* { */

document:
  document_ {
    drv.process(*$1);
    delete $1;
  }
  ;

/*
 * Well... actually, an HTML document should look like
 *
 * <!DOCTYPE ...>
 * <HTML>
 *   <HEAD>
 *   ...
 *   </HEAD>
 *   <BODY>
 *   ...
 *   </BODY>
 * </HTML>
 *
 * but...
 *
 * (A) All seven tags are optional
 * (B) The contents of the HEAD and the BODY section can be distinuished
 * (C) Most people out there do not know which element to put before, into,
 *     or after which section...
 *
 * so... let's just forget about the structure of an HTML document, discard
 * the seven tags, and process the remainder as a series of sections.
 */

document_:
  /* empty */ {
    $$ = new Document;
    $$->body.content.reset(new list<auto_ptr<Element> >);
  }
  | document_ error {
    $$ = $1;
  }
  | document_ DOCTYPE {
    $$ = $1;
  }
  | document_ HTML {
    $$->attributes.reset($2);
    $$ = $1;
  }
  | document_ END_HTML {
    $$ = $1;
  }
  | document_ HEAD {
    delete $2;
    $$ = $1;
  }
  | document_ END_HEAD {
    $$ = $1;
  }
  | document_ TITLE opt_pcdata opt_END_TITLE {
    delete $2; // Ignore <TITLE> attributes
    ($$ = $1)->head.title.reset($3);
  }
  | document_ ISINDEX {
    ($$ = $1)->head.isindex_attributes.reset($2);
  }
  | document_ BASE {
    ($$ = $1)->head.base_attributes.reset($2);
  }
  | document_ META {
    auto_ptr<Meta> s(new Meta);
    s->attributes.reset($2);
    ($$ = $1)->head.metas.push_back(s);
  }
  | document_ LINK {
    ($$ = $1)->head.link_attributes.reset($2);
  }
  | document_ SCRIPT {
    auto_ptr<Script> s(new Script);
    s->attributes.reset($2);
    if (!drv.read_cdata("</SCRIPT>", &s->text)) {
      yyerror("CDATA terminal not found");
    }
    ($$ = $1)->head.scripts.push_back(s);
  }
  | document_ STYLE {
    auto_ptr<Style> s(new Style);
    s->attributes.reset($2);
    if (!drv.read_cdata("</STYLE>", &s->text)) {
      yyerror("CDATA terminal not found");
    }
    ($$ = $1)->head.styles.push_back(s);
  }
  | document_ BODY {
    delete $2;
    $$ = $1;
  }
  | document_ END_BODY {
    $$ = $1;
  }
  | document_ texts {
    Paragraph *p = new Paragraph;
    p->texts.reset($2);
    ($$ = $1)->body.content->push_back(auto_ptr<Element>(p));
  }
  | document_ heading {
    ($$ = $1)->body.content->push_back(auto_ptr<Element>($2));
  }
  | document_ block {
    ($$ = $1)->body.content->push_back(auto_ptr<Element>($2));
  }
  | document_ address {
    ($$ = $1)->body.content->push_back(auto_ptr<Element>($2));
  }
  ;

pcdata:
  PCDATA {
    $$ = new PCData;
    $$->text = *$1;
    delete $1;
  }
  ;

body_content:
  /* empty */ {
    $$ = new list<auto_ptr<Element>>;
  }
  | body_content error {
    $$ = $1;
  }
  | body_content SCRIPT {
    auto_ptr<Script> s(new Script);
    s->attributes.reset($2);
    if (!drv.read_cdata("</SCRIPT>", &s->text)) {
      yyerror("CDATA terminal not found");
    }
//    ($$ = $1)->head.scripts.push_back(s);
  }
  | body_content STYLE {
    auto_ptr<Style> s(new Style);
    s->attributes.reset($2);
    if (!drv.read_cdata("</STYLE>", &s->text)) {
      yyerror("CDATA terminal not found");
    }
//    ($$ = $1)->head.styles.push_back(s);
  }
  | body_content META {
    /* This seems to happen for instance by Mozilla Thunderbird in its
     * replies, a blockquote is followed by a meta tag having content
     * encoding.  Don't error out, just ignore this */
    $$ = new list<auto_ptr<Element>>;
  }
  | body_content texts {
    Paragraph *p = new Paragraph;
    p->texts = auto_ptr<list<auto_ptr<Element> > >($2);
    ($$ = $1)->push_back(auto_ptr<Element>(p));
  }
  | body_content heading {
    ($$ = $1)->push_back(auto_ptr<Element>($2));
  }
  | body_content block {
    ($$ = $1)->push_back(auto_ptr<Element>($2));
  }
  | body_content address {
    ($$ = $1)->push_back(auto_ptr<Element>($2));
  }
  ;

heading:
  HX paragraph_content END_HX {
            /* EXTENSION: Allow paragraph content in heading, not only texts */
    if ($1->level != $3) {
      yyerror ("Levels of opening and closing headings don't match");
    }
    $$ = $1;
    $$->content.reset($2);
  }
  ;

block:
  block_except_p {
    $$ = $1;
  }
  | P paragraph_content opt_END_P {
    Paragraph *p = new Paragraph;
    p->attributes.reset($1);
    p->texts.reset($2);
    $$ = p;
  }
  ;

paragraph_content:  /* EXTENSION: Allow blocks (except "<P>") in paragraphs. */
  /* empty */ {
    $$ = new list<auto_ptr<Element> >;
  }
  | paragraph_content error {
    $$ = $1;
  }
  | paragraph_content texts {
    $$ = $1;
    $$->splice($$->end(), *$2);
    delete $2;
  }
  | paragraph_content block_except_p {
    ($$ = $1)->push_back(auto_ptr<Element>($2));
  }
  ;

block_except_p:
  list {
    $$ = $1;
  }
  | preformatted {
    $$ = $1;
  }
  | definition_list {
    $$ = $1;
  }
  | DIV body_content opt_END_DIV {
    Division *p = new Division;
    p->attributes.reset($1);
    p->body_content.reset($2);
    $$ = p;
  }
  | CENTER body_content opt_END_CENTER {
    Center *p = new Center;
    delete $1;       // CENTER has no attributes.
    p->body_content.reset($2);
    $$ = p;
  }
  | BLOCKQUOTE body_content opt_END_BLOCKQUOTE {
    delete $1; // BLOCKQUOTE has no attributes!
    BlockQuote *bq = new BlockQuote;
    bq->content.reset($2);
    $$ = bq;
  }
  | FORM body_content opt_END_FORM {
    Form *f = new Form;
    f->attributes.reset($1);
    f->content.reset($2);
    $$ = f;
  }
  | HR {
    HorizontalRule *h = new HorizontalRule;
    h->attributes.reset($1);
    $$ = h;
  }
  | TABLE opt_caption table_rows opt_END_TABLE {
    Table *t = new Table;
    t->attributes.reset($1);
    t->caption.reset($2);
    t->rows.reset($3);
    $$ = t;
  }
  ;

list:
  OL { ++drv.list_nesting; } list_content END_OL {
    OrderedList *ol = new OrderedList;
    ol->attributes.reset($1);
    ol->items.reset($3);
    ol->nesting = --drv.list_nesting;
    $$ = ol;
  }
  | UL { ++drv.list_nesting; } list_content opt_END_UL {
    UnorderedList *ul = new UnorderedList;
    ul->attributes.reset($1);
    ul->items.reset($3);
    ul->nesting = --drv.list_nesting;
    $$ = ul;
  }
  | DIR { ++drv.list_nesting; } list_content END_DIR {
    Dir *d = new Dir;
    d->attributes.reset($1);
    d->items.reset($3);
    d->nesting = --drv.list_nesting;
    $$ = d;
  }
  | MENU { ++drv.list_nesting; } list_content END_MENU {
    Menu *m = new Menu;
    m->attributes.reset($1);
    m->items.reset($3);
    m->nesting = --drv.list_nesting;
    $$ = m;
  }
  ;

list_content:
  /* empty */ {
    $$ = 0;
  }
  | list_content error {
    $$ = $1;
  }
  | list_content list_item {
    $$ = $1 ? $1 : new list<auto_ptr<ListItem> >;
    $$->push_back(auto_ptr<ListItem>($2));
  }
  ;

list_item:
  LI opt_flow opt_END_LI {
    ListNormalItem *lni = new ListNormalItem;
    lni->attributes.reset($1);
    lni->flow.reset($2);
    $$ = lni;
  }
  | block {   /* EXTENSION: Handle a "block" in a list as an indented block. */
    ListBlockItem *lbi = new ListBlockItem;
    lbi->block.reset($1);
    $$ = lbi;
  }
  | texts {              /* EXTENSION: Treat "texts" in a list as an "<LI>". */
    ListNormalItem *lni = new ListNormalItem;
    lni->flow.reset($1);
    $$ = lni;
  }
  ;

definition_list:
                                           /* EXTENSION: Allow nested <DL>s. */
                                             /* EXTENSION: "</DL>" optional. */
  DL opt_flow opt_error definition_list opt_END_DL {
    delete $1;
    delete $2; /* Kludge */
    $$ = $4;
  }
                                 /* EXTENSION: Accept a "preamble" in the DL */
  | DL opt_flow opt_error definition_list_content END_DL {
    DefinitionList *dl = new DefinitionList;
    dl->attributes.reset($1);
    dl->preamble.reset($2);
    dl->items.reset($4);
    $$ = dl;
  }
  ;

definition_list_content:
  /* empty */ {
    $$ = 0;
  }
  | definition_list_content {
    $$ = $1;
  }
  | definition_list_content term_name {
    $$ = $1 ? $1 : new list<auto_ptr<DefinitionListItem> >;
    $$->push_back(auto_ptr<DefinitionListItem>($2));
  }
  | definition_list_content term_definition {
    $$ = $1 ? $1 : new list<auto_ptr<DefinitionListItem> >;
    $$->push_back(auto_ptr<DefinitionListItem>($2));
  }
  ;

term_name:
  DT opt_flow opt_error {      /* EXTENSION: Allow "flow" instead of "texts" */
    delete $1;
    $$ = new TermName;
    $$->flow.reset($2);
  }
  | DT opt_flow END_DT opt_P opt_error {/* EXTENSION: Ignore <P> after </DT> */
    delete $1;
    delete $4;
    $$ = new TermName;
    $$->flow.reset($2);
  }
  ;

term_definition:
  DD opt_flow opt_error {
    delete $1;
    $$ = new TermDefinition;
    $$->flow.reset($2);
  }
  | DD opt_flow END_DD opt_P opt_error {/* EXTENSION: Ignore <P> after </DD> */
    delete $1;
    delete $4;
    $$ = new TermDefinition;
    $$->flow.reset($2);
  }
  ;

flow:
  flow_ {
    $$ = new list<auto_ptr<Element> >;
    $$->push_back(auto_ptr<Element>($1));
  }
  | flow error {
    $$ = $1;
  }
  | flow flow_ {
    ($$ = $1)->push_back(auto_ptr<Element>($2));
  }
  ;

flow_:
  text {
    $$ = $1;
  }
  | heading {          /* EXTENSION: Allow headings in "flow", i.e. in lists */
    $$ = $1;
  }
  | block {
    $$ = $1;
  }
  ;

preformatted:
  PRE opt_texts opt_END_PRE {
    $$ = new Preformatted;
    $$->attributes.reset($1);
    $$->texts.reset($2);
  }
  ;

caption:
  CAPTION opt_texts END_CAPTION {
    $$ = new Caption;
    $$->attributes.reset($1);
    $$->texts.reset($2);
  }
  ;

table_rows:
  /* empty */ {
    $$ = new list<auto_ptr<TableRow> >;
  }
  | table_rows error {
    $$ = $1;
  }
  | table_rows TR table_cells opt_END_TR {
    TableRow *tr = new TableRow;
    tr->attributes.reset($2);
    tr->cells.reset($3);
    ($$ = $1)->push_back(auto_ptr<TableRow>(tr));
  }
  ;

table_cells:
  /* empty */ {
    $$ = new list<auto_ptr<TableCell> >;
  }
  | table_cells error {
    $$ = $1;
  }
  | table_cells TD body_content opt_END_TD {
    TableCell *tc = new TableCell;
    tc->attributes.reset($2);
    tc->content.reset($3);
    ($$ = $1)->push_back(auto_ptr<TableCell>(tc));
  }
  | table_cells TH body_content opt_END_TH opt_END_TD {
                            /* EXTENSION: Allow "</TD>" in place of "</TH>". */
    TableHeadingCell *thc = new TableHeadingCell;
    thc->attributes.reset($2);
    thc->content.reset($3);
    ($$ = $1)->push_back(auto_ptr<TableCell>(thc));
  }
  | table_cells INPUT {    /* EXTENSION: Ignore <INPUT> between table cells. */
    delete $2;
    $$ = $1;
  }
  ;

address:
  ADDRESS opt_texts END_ADDRESS { /* Should be "address_content"... */
    delete $1;
    $$ = new Address;
    $$->content.reset($2);
  }
  ;

/* ------------------------------------------------------------------------- */

texts:
  text {
    $$ = new list<auto_ptr<Element> >;
    $$->push_back(auto_ptr<Element>($1));
  }
  | texts text {
    ($$ = $1)->push_back(auto_ptr<Element>($2));
  }
  ;

text:
  pcdata                    opt_error { $$ = $1; }
  | font                    opt_error { $$ = $1; }
  | phrase                  opt_error { $$ = $1; }
  | special                 opt_error { $$ = $1; }
  | form                    opt_error { $$ = $1; }
  | NOBR opt_texts END_NOBR opt_error { /* EXTENSION: NS 1.1 / IE 2.0 */
    NoBreak *nb = new NoBreak;
    delete $1;
    nb->content.reset($2);
    $$ = nb;
  }
  ;

font:
  TT       opt_texts opt_END_TT     { delete $1; $$ = new Font(token::TT,     $2); }
  | I      opt_texts opt_END_I      { delete $1; $$ = new Font(token::I,      $2); }
  | B      opt_texts opt_END_B      { delete $1; $$ = new Font(token::B,      $2); }
  | U      opt_texts opt_END_U      { delete $1; $$ = new Font(token::U,      $2); }
  | STRIKE opt_texts opt_END_STRIKE { delete $1; $$ = new Font(token::STRIKE, $2); }
  | BIG    opt_texts opt_END_BIG    { delete $1; $$ = new Font(token::BIG,    $2); }
  | SMALL  opt_texts opt_END_SMALL  { delete $1; $$ = new Font(token::SMALL,  $2); }
  | SUB    opt_texts opt_END_SUB    { delete $1; $$ = new Font(token::SUB,    $2); }
  | SUP    opt_texts opt_END_SUP    { delete $1; $$ = new Font(token::SUP,    $2); }
  ;

phrase:
  EM       opt_texts opt_END_EM     { delete $1; $$ = new Phrase(token::EM,     $2); }
  | STRONG opt_texts opt_END_STRONG { delete $1; $$ = new Phrase(token::STRONG, $2); }
  | DFN    opt_texts opt_END_DFN    { delete $1; $$ = new Phrase(token::DFN,    $2); }
  | CODE   opt_texts opt_END_CODE   { delete $1; $$ = new Phrase(token::CODE,   $2); }
  | SAMP   opt_texts opt_END_SAMP   { delete $1; $$ = new Phrase(token::SAMP,   $2); }
  | KBD    opt_texts opt_END_KBD    { delete $1; $$ = new Phrase(token::KBD,    $2); }
  | VAR    opt_texts opt_END_VAR    { delete $1; $$ = new Phrase(token::VAR,    $2); }
  | CITE   opt_texts opt_END_CITE   { delete $1; $$ = new Phrase(token::CITE,   $2); }
  ;

special:
                        /* EXTENSION: Allow "flow" in <A>, not only "texts". */
                                 /* EXTENSION: Allow useless <LI> in anchor. */
                                               /* EXTENSION: "</A>" optional.*/
  A opt_LI opt_flow opt_END_A {
    delete $2;
    Anchor *a = new Anchor;
    a->attributes.reset($1);
    a->texts.reset($3);
    a->refnum = 0;
    $$ = a;

	istr href = get_attribute(a->attributes.get(), "HREF", "");
	if (drv.enable_links && !href.empty() && href[0] != '#') {
        ListNormalItem *lni = new ListNormalItem;
        PCData *d = new PCData;
        d->text = href;
        list<auto_ptr<Element>> *data = new list<auto_ptr<Element>>;
        data->push_back(auto_ptr<Element>(d));
        lni->flow.reset(data);
        drv.links->items->push_back(auto_ptr<ListItem>(lni));
        a->refnum = drv.links->items->size();
    }
  }
  | IMG {
	auto_ptr<list<TagAttribute>> attr;
	attr.reset($1);
	istr src = get_attribute(attr.get(), "SRC", "");
	istr alt = get_attribute(attr.get(), "ALT", "");
	/* when ALT is empty, and we have SRC, replace it with a link */
	if (drv.enable_links && !src.empty() && alt.empty()) {
		PCData *d = new PCData;
		string nothing = "";
        d->text = nothing;
        list<auto_ptr<Element>> *data = new list<auto_ptr<Element>>;
        data->push_back(auto_ptr<Element>(d));

		TagAttribute attribute;
		string href = "HREF";
		attribute.first = href;
		attribute.second = src;
		attr->push_back(attribute);

		Anchor *a = new Anchor;
		a->attributes = attr;
		a->texts.reset(data);
		a->refnum = 0;

        ListNormalItem *lni = new ListNormalItem;
		d = new PCData;
        d->text = src;
        data = new list<auto_ptr<Element>>;
        data->push_back(auto_ptr<Element>(d));
        lni->flow.reset(data);
        drv.links->items->push_back(auto_ptr<ListItem>(lni));
        a->refnum = drv.links->items->size();

		$$ = a;
	} else {
		Image *i = new Image;
		i->attributes = attr;
		$$ = i;
	}
  }
  | APPLET applet_content END_APPLET {
    Applet *a = new Applet;
    a->attributes.reset($1);
    a->content.reset($2);
    $$ = a;
  }
                   /* EXTENSION: "flow" in <FONT> allowed, not only "texts". */
                                           /* EXTENSION: "</FONT>" optional. */
  | FONT opt_flow opt_END_FONT {
    Font2 *f2 = new Font2;
    f2->attributes.reset($1);
    f2->elements.reset($2);
    $$ = f2;
  }
  | BASEFONT {
    BaseFont *bf = new BaseFont;
    bf->attributes.reset($1);
    $$ = bf;
  }
  | BR {
    LineBreak *lb = new LineBreak;
    lb->attributes.reset($1);
    $$ = lb;
  }
  | MAP map_content END_MAP {
    Map *m = new Map;
    m->attributes.reset($1);
    m->areas.reset($2);
    $$ = m;
  }
  ;

applet_content:
  /* empty */ {
    $$ = 0;
  }
  | applet_content text {
    $$ = $1 ? $1 : new list<auto_ptr<Element> >;
    $$->push_back(auto_ptr<Element>($2));
  }
  | applet_content PARAM {
    $$ = $1 ? $1 : new list<auto_ptr<Element> >;
    Param *p = new Param;
    p->attributes.reset($2);
    $$->push_back(auto_ptr<Element>(p));
  }
  ;

map_content:
  /* empty */ {
    $$ = 0;
  }
  | map_content error {
    $$ = $1;
  }
  | map_content AREA {
    $$ = $1 ? $1 : new list<auto_ptr<list<TagAttribute> > >;
    $$->push_back(auto_ptr<list<TagAttribute> >($2));
  }
  ;

form:
  INPUT {
    Input *i = new Input;
    i->attributes.reset($1);
    $$ = i;
  }
  | SELECT select_content END_SELECT {
    Select *s = new Select;
    s->attributes.reset($1);
    s->content.reset($2);
    $$ = s;
  }
  | TEXTAREA pcdata END_TEXTAREA {
    TextArea *ta = new TextArea;
    ta->attributes.reset($1);
    ta->pcdata.reset($2);
    $$ = ta;
  }
  ;

select_content:
  option {
    $$ = new list<auto_ptr<Option> >;
    $$->push_back(auto_ptr<Option>($1));
  }
  | select_content option {
    ($$ = $1)->push_back(auto_ptr<Option>($2));
  }
  ;

option:
  OPTION pcdata opt_END_OPTION {
    $$ = new Option;
    $$->attributes.reset($1);
    $$->pcdata.reset($2);
  }
  ;

/* ------------------------------------------------------------------------- */

HX:
  H1   { $$ = new Heading; $$->level = 1; $$->attributes.reset($1); }
  | H2 { $$ = new Heading; $$->level = 2; $$->attributes.reset($1); }
  | H3 { $$ = new Heading; $$->level = 3; $$->attributes.reset($1); }
  | H4 { $$ = new Heading; $$->level = 4; $$->attributes.reset($1); }
  | H5 { $$ = new Heading; $$->level = 5; $$->attributes.reset($1); }
  | H6 { $$ = new Heading; $$->level = 6; $$->attributes.reset($1); }
  ;

END_HX:
  END_H1   { $$ = 1; }
  | END_H2 { $$ = 2; }
  | END_H3 { $$ = 3; }
  | END_H4 { $$ = 4; }
  | END_H5 { $$ = 5; }
  | END_H6 { $$ = 6; }
  ;

/* ------------------------------------------------------------------------- */

opt_pcdata:     /* empty */ { $$ = 0; } | pcdata  { $$ = $1; };
opt_caption:    /* empty */ { $$ = 0; } | caption { $$ = $1; };
opt_texts:      /* empty */ { $$ = 0; } | texts   { $$ = $1; };
opt_flow:       /* empty */ { $$ = 0; } | flow    { $$ = $1; };

opt_LI:         /* empty */ { $$ = 0; } | LI      { $$ = $1; };
opt_P:          /* empty */ { $$ = 0; } | P       { $$ = $1; };

opt_END_A:      /* empty */ | END_A;
opt_END_B:      /* empty */ | END_B;
opt_END_BLOCKQUOTE: /* empty */ | END_BLOCKQUOTE;
opt_END_BIG:    /* empty */ | END_BIG;
opt_END_CENTER: /* empty */ | END_CENTER;
opt_END_CITE:   /* empty */ | END_CITE;
opt_END_CODE:   /* empty */ | END_CODE;
opt_END_DFN:    /* empty */ | END_DFN;
opt_END_DIV:    /* empty */ | END_DIV;
opt_END_DL:     /* empty */ | END_DL;
opt_END_EM:     /* empty */ | END_EM;
opt_END_FONT:   /* empty */ | END_FONT;
opt_END_FORM:   /* empty */ | END_FORM;
opt_END_I:      /* empty */ | END_I;
opt_END_KBD:    /* empty */ | END_KBD;
opt_END_LI:     /* empty */ | END_LI;
opt_END_OPTION: /* empty */ | END_OPTION;
opt_END_P:      /* empty */ | END_P;
opt_END_PRE:    /* empty */ | END_PRE;
opt_END_SAMP:   /* empty */ | END_SAMP;
opt_END_SMALL:  /* empty */ | END_SMALL;
opt_END_STRIKE: /* empty */ | END_STRIKE;
opt_END_STRONG: /* empty */ | END_STRONG;
opt_END_SUB:    /* empty */ | END_SUB;
opt_END_SUP:    /* empty */ | END_SUP;
opt_END_TABLE:  END         | END_TABLE;
opt_END_TD:     /* empty */ | END_TD;
opt_END_TH:     /* empty */ | END_TH;
opt_END_TITLE:  /* empty */ | END_TITLE;
opt_END_TR:     /* empty */ | END_TR;
opt_END_TT:     /* empty */ | END_TT;
opt_END_U:      /* empty */ | END_U;
opt_END_UL:     /* empty */ | END_UL;
opt_END_VAR:    /* empty */ | END_VAR;

opt_error:      /* empty */ | error;

%% /* } */

void
html2text::HTMLParser::error(const std::string& msg)
{
	yyerror(msg.c_str());
}

