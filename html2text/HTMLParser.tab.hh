// A Bison parser, made by GNU Bison 3.6.3.

// Skeleton interface for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2020 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.


/**
 ** \file HTMLParser.tab.hh
 ** Define the html2text::parser class.
 */

// C++ LALR(1) parser skeleton written by Akim Demaille.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.

#ifndef YY_YY_HTMLPARSER_TAB_HH_INCLUDED
# define YY_YY_HTMLPARSER_TAB_HH_INCLUDED
// "%code requires" blocks.
#line 43 "HTMLParser.yy"

#include <string>
#define HTMLParser_token html2text::HTMLParser::token
#include "html.h"
#include "istr.h"
class HTMLDriver;

#line 57 "HTMLParser.tab.hh"


# include <cstdlib> // std::abort
# include <iostream>
# include <stdexcept>
# include <string>
# include <vector>

#if defined __cplusplus
# define YY_CPLUSPLUS __cplusplus
#else
# define YY_CPLUSPLUS 199711L
#endif

// Support move semantics when possible.
#if 201103L <= YY_CPLUSPLUS
# define YY_MOVE           std::move
# define YY_MOVE_OR_COPY   move
# define YY_MOVE_REF(Type) Type&&
# define YY_RVREF(Type)    Type&&
# define YY_COPY(Type)     Type
#else
# define YY_MOVE
# define YY_MOVE_OR_COPY   copy
# define YY_MOVE_REF(Type) Type&
# define YY_RVREF(Type)    const Type&
# define YY_COPY(Type)     const Type&
#endif

// Support noexcept when possible.
#if 201103L <= YY_CPLUSPLUS
# define YY_NOEXCEPT noexcept
# define YY_NOTHROW
#else
# define YY_NOEXCEPT
# define YY_NOTHROW throw ()
#endif

// Support constexpr when possible.
#if 201703 <= YY_CPLUSPLUS
# define YY_CONSTEXPR constexpr
#else
# define YY_CONSTEXPR
#endif



#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

#line 40 "HTMLParser.yy"
namespace html2text {
#line 187 "HTMLParser.tab.hh"




  /// A Bison parser.
  class HTMLParser
  {
  public:
#ifndef YYSTYPE
    /// Symbol semantic values.
    union semantic_type
    {
#line 67 "HTMLParser.yy"

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

#line 225 "HTMLParser.tab.hh"

    };
#else
    typedef YYSTYPE semantic_type;
#endif

    /// Syntax errors thrown from user actions.
    struct syntax_error : std::runtime_error
    {
      syntax_error (const std::string& m)
        : std::runtime_error (m)
      {}

      syntax_error (const syntax_error& s)
        : std::runtime_error (s.what ())
      {}

      ~syntax_error () YY_NOEXCEPT YY_NOTHROW;
    };

    /// Token kinds.
    struct token
    {
      enum token_kind_type
      {
        YYEMPTY = -2,
    END = 0,                       // "end of file"
    YYerror = 256,                 // error
    YYUNDEF = 257,                 // "invalid token"
    DOCTYPE = 258,                 // DOCTYPE
    PCDATA = 259,                  // PCDATA
    SCAN_ERROR = 260,              // SCAN_ERROR
    A = 261,                       // A
    ADDRESS = 262,                 // ADDRESS
    APPLET = 263,                  // APPLET
    AREA = 264,                    // AREA
    B = 265,                       // B
    BASE = 266,                    // BASE
    BASEFONT = 267,                // BASEFONT
    BIG = 268,                     // BIG
    BLOCKQUOTE = 269,              // BLOCKQUOTE
    BODY = 270,                    // BODY
    BR = 271,                      // BR
    CAPTION = 272,                 // CAPTION
    CENTER = 273,                  // CENTER
    CITE = 274,                    // CITE
    CODE = 275,                    // CODE
    DD = 276,                      // DD
    DFN = 277,                     // DFN
    DIR = 278,                     // DIR
    DIV = 279,                     // DIV
    DL = 280,                      // DL
    DT = 281,                      // DT
    EM = 282,                      // EM
    FONT = 283,                    // FONT
    FORM = 284,                    // FORM
    H1 = 285,                      // H1
    H2 = 286,                      // H2
    H3 = 287,                      // H3
    H4 = 288,                      // H4
    H5 = 289,                      // H5
    H6 = 290,                      // H6
    HEAD = 291,                    // HEAD
    HR = 292,                      // HR
    HTML = 293,                    // HTML
    I = 294,                       // I
    IMG = 295,                     // IMG
    INPUT = 296,                   // INPUT
    ISINDEX = 297,                 // ISINDEX
    KBD = 298,                     // KBD
    LI = 299,                      // LI
    LINK = 300,                    // LINK
    MAP = 301,                     // MAP
    MENU = 302,                    // MENU
    META = 303,                    // META
    NOBR = 304,                    // NOBR
    OL = 305,                      // OL
    OPTION = 306,                  // OPTION
    P = 307,                       // P
    PARAM = 308,                   // PARAM
    PRE = 309,                     // PRE
    SAMP = 310,                    // SAMP
    SCRIPT = 311,                  // SCRIPT
    SELECT = 312,                  // SELECT
    SMALL = 313,                   // SMALL
    STRIKE = 314,                  // STRIKE
    STRONG = 315,                  // STRONG
    STYLE = 316,                   // STYLE
    SUB = 317,                     // SUB
    SUP = 318,                     // SUP
    TABLE = 319,                   // TABLE
    TD = 320,                      // TD
    TEXTAREA = 321,                // TEXTAREA
    TH = 322,                      // TH
    TITLE = 323,                   // TITLE
    TR = 324,                      // TR
    TT = 325,                      // TT
    U = 326,                       // U
    UL = 327,                      // UL
    VAR = 328,                     // VAR
    END_A = 329,                   // END_A
    END_ADDRESS = 330,             // END_ADDRESS
    END_APPLET = 331,              // END_APPLET
    END_B = 332,                   // END_B
    END_BIG = 333,                 // END_BIG
    END_BLOCKQUOTE = 334,          // END_BLOCKQUOTE
    END_BODY = 335,                // END_BODY
    END_CAPTION = 336,             // END_CAPTION
    END_CENTER = 337,              // END_CENTER
    END_CITE = 338,                // END_CITE
    END_CODE = 339,                // END_CODE
    END_DD = 340,                  // END_DD
    END_DFN = 341,                 // END_DFN
    END_DIR = 342,                 // END_DIR
    END_DIV = 343,                 // END_DIV
    END_DL = 344,                  // END_DL
    END_DT = 345,                  // END_DT
    END_EM = 346,                  // END_EM
    END_FONT = 347,                // END_FONT
    END_FORM = 348,                // END_FORM
    END_H1 = 349,                  // END_H1
    END_H2 = 350,                  // END_H2
    END_H3 = 351,                  // END_H3
    END_H4 = 352,                  // END_H4
    END_H5 = 353,                  // END_H5
    END_H6 = 354,                  // END_H6
    END_HEAD = 355,                // END_HEAD
    END_HTML = 356,                // END_HTML
    END_I = 357,                   // END_I
    END_KBD = 358,                 // END_KBD
    END_LI = 359,                  // END_LI
    END_MAP = 360,                 // END_MAP
    END_MENU = 361,                // END_MENU
    END_NOBR = 362,                // END_NOBR
    END_OL = 363,                  // END_OL
    END_OPTION = 364,              // END_OPTION
    END_P = 365,                   // END_P
    END_PRE = 366,                 // END_PRE
    END_SAMP = 367,                // END_SAMP
    END_SCRIPT = 368,              // END_SCRIPT
    END_SELECT = 369,              // END_SELECT
    END_SMALL = 370,               // END_SMALL
    END_STRIKE = 371,              // END_STRIKE
    END_STRONG = 372,              // END_STRONG
    END_STYLE = 373,               // END_STYLE
    END_SUB = 374,                 // END_SUB
    END_SUP = 375,                 // END_SUP
    END_TABLE = 376,               // END_TABLE
    END_TD = 377,                  // END_TD
    END_TEXTAREA = 378,            // END_TEXTAREA
    END_TH = 379,                  // END_TH
    END_TITLE = 380,               // END_TITLE
    END_TR = 381,                  // END_TR
    END_TT = 382,                  // END_TT
    END_U = 383,                   // END_U
    END_UL = 384,                  // END_UL
    END_VAR = 385                  // END_VAR
      };
      /// Backward compatibility alias (Bison 3.6).
      typedef token_kind_type yytokentype;
    };

    /// Token kind, as returned by yylex.
    typedef token::yytokentype token_kind_type;

    /// Backward compatibility alias (Bison 3.6).
    typedef token_kind_type token_type;

    /// Symbol kinds.
    struct symbol_kind
    {
      enum symbol_kind_type
      {
        YYNTOKENS = 131, ///< Number of tokens.
        S_YYEMPTY = -2,
        S_YYEOF = 0,                             // "end of file"
        S_YYerror = 1,                           // error
        S_YYUNDEF = 2,                           // "invalid token"
        S_DOCTYPE = 3,                           // DOCTYPE
        S_PCDATA = 4,                            // PCDATA
        S_SCAN_ERROR = 5,                        // SCAN_ERROR
        S_A = 6,                                 // A
        S_ADDRESS = 7,                           // ADDRESS
        S_APPLET = 8,                            // APPLET
        S_AREA = 9,                              // AREA
        S_B = 10,                                // B
        S_BASE = 11,                             // BASE
        S_BASEFONT = 12,                         // BASEFONT
        S_BIG = 13,                              // BIG
        S_BLOCKQUOTE = 14,                       // BLOCKQUOTE
        S_BODY = 15,                             // BODY
        S_BR = 16,                               // BR
        S_CAPTION = 17,                          // CAPTION
        S_CENTER = 18,                           // CENTER
        S_CITE = 19,                             // CITE
        S_CODE = 20,                             // CODE
        S_DD = 21,                               // DD
        S_DFN = 22,                              // DFN
        S_DIR = 23,                              // DIR
        S_DIV = 24,                              // DIV
        S_DL = 25,                               // DL
        S_DT = 26,                               // DT
        S_EM = 27,                               // EM
        S_FONT = 28,                             // FONT
        S_FORM = 29,                             // FORM
        S_H1 = 30,                               // H1
        S_H2 = 31,                               // H2
        S_H3 = 32,                               // H3
        S_H4 = 33,                               // H4
        S_H5 = 34,                               // H5
        S_H6 = 35,                               // H6
        S_HEAD = 36,                             // HEAD
        S_HR = 37,                               // HR
        S_HTML = 38,                             // HTML
        S_I = 39,                                // I
        S_IMG = 40,                              // IMG
        S_INPUT = 41,                            // INPUT
        S_ISINDEX = 42,                          // ISINDEX
        S_KBD = 43,                              // KBD
        S_LI = 44,                               // LI
        S_LINK = 45,                             // LINK
        S_MAP = 46,                              // MAP
        S_MENU = 47,                             // MENU
        S_META = 48,                             // META
        S_NOBR = 49,                             // NOBR
        S_OL = 50,                               // OL
        S_OPTION = 51,                           // OPTION
        S_P = 52,                                // P
        S_PARAM = 53,                            // PARAM
        S_PRE = 54,                              // PRE
        S_SAMP = 55,                             // SAMP
        S_SCRIPT = 56,                           // SCRIPT
        S_SELECT = 57,                           // SELECT
        S_SMALL = 58,                            // SMALL
        S_STRIKE = 59,                           // STRIKE
        S_STRONG = 60,                           // STRONG
        S_STYLE = 61,                            // STYLE
        S_SUB = 62,                              // SUB
        S_SUP = 63,                              // SUP
        S_TABLE = 64,                            // TABLE
        S_TD = 65,                               // TD
        S_TEXTAREA = 66,                         // TEXTAREA
        S_TH = 67,                               // TH
        S_TITLE = 68,                            // TITLE
        S_TR = 69,                               // TR
        S_TT = 70,                               // TT
        S_U = 71,                                // U
        S_UL = 72,                               // UL
        S_VAR = 73,                              // VAR
        S_END_A = 74,                            // END_A
        S_END_ADDRESS = 75,                      // END_ADDRESS
        S_END_APPLET = 76,                       // END_APPLET
        S_END_B = 77,                            // END_B
        S_END_BIG = 78,                          // END_BIG
        S_END_BLOCKQUOTE = 79,                   // END_BLOCKQUOTE
        S_END_BODY = 80,                         // END_BODY
        S_END_CAPTION = 81,                      // END_CAPTION
        S_END_CENTER = 82,                       // END_CENTER
        S_END_CITE = 83,                         // END_CITE
        S_END_CODE = 84,                         // END_CODE
        S_END_DD = 85,                           // END_DD
        S_END_DFN = 86,                          // END_DFN
        S_END_DIR = 87,                          // END_DIR
        S_END_DIV = 88,                          // END_DIV
        S_END_DL = 89,                           // END_DL
        S_END_DT = 90,                           // END_DT
        S_END_EM = 91,                           // END_EM
        S_END_FONT = 92,                         // END_FONT
        S_END_FORM = 93,                         // END_FORM
        S_END_H1 = 94,                           // END_H1
        S_END_H2 = 95,                           // END_H2
        S_END_H3 = 96,                           // END_H3
        S_END_H4 = 97,                           // END_H4
        S_END_H5 = 98,                           // END_H5
        S_END_H6 = 99,                           // END_H6
        S_END_HEAD = 100,                        // END_HEAD
        S_END_HTML = 101,                        // END_HTML
        S_END_I = 102,                           // END_I
        S_END_KBD = 103,                         // END_KBD
        S_END_LI = 104,                          // END_LI
        S_END_MAP = 105,                         // END_MAP
        S_END_MENU = 106,                        // END_MENU
        S_END_NOBR = 107,                        // END_NOBR
        S_END_OL = 108,                          // END_OL
        S_END_OPTION = 109,                      // END_OPTION
        S_END_P = 110,                           // END_P
        S_END_PRE = 111,                         // END_PRE
        S_END_SAMP = 112,                        // END_SAMP
        S_END_SCRIPT = 113,                      // END_SCRIPT
        S_END_SELECT = 114,                      // END_SELECT
        S_END_SMALL = 115,                       // END_SMALL
        S_END_STRIKE = 116,                      // END_STRIKE
        S_END_STRONG = 117,                      // END_STRONG
        S_END_STYLE = 118,                       // END_STYLE
        S_END_SUB = 119,                         // END_SUB
        S_END_SUP = 120,                         // END_SUP
        S_END_TABLE = 121,                       // END_TABLE
        S_END_TD = 122,                          // END_TD
        S_END_TEXTAREA = 123,                    // END_TEXTAREA
        S_END_TH = 124,                          // END_TH
        S_END_TITLE = 125,                       // END_TITLE
        S_END_TR = 126,                          // END_TR
        S_END_TT = 127,                          // END_TT
        S_END_U = 128,                           // END_U
        S_END_UL = 129,                          // END_UL
        S_END_VAR = 130,                         // END_VAR
        S_YYACCEPT = 131,                        // $accept
        S_document = 132,                        // document
        S_document_ = 133,                       // document_
        S_pcdata = 134,                          // pcdata
        S_body_content = 135,                    // body_content
        S_heading = 136,                         // heading
        S_block = 137,                           // block
        S_paragraph_content = 138,               // paragraph_content
        S_block_except_p = 139,                  // block_except_p
        S_list = 140,                            // list
        S_141_1 = 141,                           // $@1
        S_142_2 = 142,                           // $@2
        S_143_3 = 143,                           // $@3
        S_144_4 = 144,                           // $@4
        S_list_content = 145,                    // list_content
        S_list_item = 146,                       // list_item
        S_definition_list = 147,                 // definition_list
        S_definition_list_content = 148,         // definition_list_content
        S_term_name = 149,                       // term_name
        S_term_definition = 150,                 // term_definition
        S_flow = 151,                            // flow
        S_flow_ = 152,                           // flow_
        S_preformatted = 153,                    // preformatted
        S_caption = 154,                         // caption
        S_table_rows = 155,                      // table_rows
        S_table_cells = 156,                     // table_cells
        S_address = 157,                         // address
        S_texts = 158,                           // texts
        S_text = 159,                            // text
        S_font = 160,                            // font
        S_phrase = 161,                          // phrase
        S_special = 162,                         // special
        S_applet_content = 163,                  // applet_content
        S_map_content = 164,                     // map_content
        S_form = 165,                            // form
        S_select_content = 166,                  // select_content
        S_option = 167,                          // option
        S_HX = 168,                              // HX
        S_END_HX = 169,                          // END_HX
        S_opt_pcdata = 170,                      // opt_pcdata
        S_opt_caption = 171,                     // opt_caption
        S_opt_texts = 172,                       // opt_texts
        S_opt_flow = 173,                        // opt_flow
        S_opt_LI = 174,                          // opt_LI
        S_opt_P = 175,                           // opt_P
        S_opt_END_A = 176,                       // opt_END_A
        S_opt_END_B = 177,                       // opt_END_B
        S_opt_END_BLOCKQUOTE = 178,              // opt_END_BLOCKQUOTE
        S_opt_END_BIG = 179,                     // opt_END_BIG
        S_opt_END_CENTER = 180,                  // opt_END_CENTER
        S_opt_END_CITE = 181,                    // opt_END_CITE
        S_opt_END_CODE = 182,                    // opt_END_CODE
        S_opt_END_DFN = 183,                     // opt_END_DFN
        S_opt_END_DIV = 184,                     // opt_END_DIV
        S_opt_END_DL = 185,                      // opt_END_DL
        S_opt_END_EM = 186,                      // opt_END_EM
        S_opt_END_FONT = 187,                    // opt_END_FONT
        S_opt_END_FORM = 188,                    // opt_END_FORM
        S_opt_END_I = 189,                       // opt_END_I
        S_opt_END_KBD = 190,                     // opt_END_KBD
        S_opt_END_LI = 191,                      // opt_END_LI
        S_opt_END_OPTION = 192,                  // opt_END_OPTION
        S_opt_END_P = 193,                       // opt_END_P
        S_opt_END_PRE = 194,                     // opt_END_PRE
        S_opt_END_SAMP = 195,                    // opt_END_SAMP
        S_opt_END_SMALL = 196,                   // opt_END_SMALL
        S_opt_END_STRIKE = 197,                  // opt_END_STRIKE
        S_opt_END_STRONG = 198,                  // opt_END_STRONG
        S_opt_END_SUB = 199,                     // opt_END_SUB
        S_opt_END_SUP = 200,                     // opt_END_SUP
        S_opt_END_TABLE = 201,                   // opt_END_TABLE
        S_opt_END_TD = 202,                      // opt_END_TD
        S_opt_END_TH = 203,                      // opt_END_TH
        S_opt_END_TITLE = 204,                   // opt_END_TITLE
        S_opt_END_TR = 205,                      // opt_END_TR
        S_opt_END_TT = 206,                      // opt_END_TT
        S_opt_END_U = 207,                       // opt_END_U
        S_opt_END_UL = 208,                      // opt_END_UL
        S_opt_END_VAR = 209,                     // opt_END_VAR
        S_opt_error = 210                        // opt_error
      };
    };

    /// (Internal) symbol kind.
    typedef symbol_kind::symbol_kind_type symbol_kind_type;

    /// The number of tokens.
    static const symbol_kind_type YYNTOKENS = symbol_kind::YYNTOKENS;

    /// A complete symbol.
    ///
    /// Expects its Base type to provide access to the symbol kind
    /// via kind ().
    ///
    /// Provide access to semantic value.
    template <typename Base>
    struct basic_symbol : Base
    {
      /// Alias to Base.
      typedef Base super_type;

      /// Default constructor.
      basic_symbol ()
        : value ()
      {}

#if 201103L <= YY_CPLUSPLUS
      /// Move constructor.
      basic_symbol (basic_symbol&& that)
        : Base (std::move (that))
        , value (std::move (that.value))
      {}
#endif

      /// Copy constructor.
      basic_symbol (const basic_symbol& that);
      /// Constructor for valueless symbols.
      basic_symbol (typename Base::kind_type t);

      /// Constructor for symbols with semantic value.
      basic_symbol (typename Base::kind_type t,
                    YY_RVREF (semantic_type) v);

      /// Destroy the symbol.
      ~basic_symbol ()
      {
        clear ();
      }

      /// Destroy contents, and record that is empty.
      void clear ()
      {
        Base::clear ();
      }

      /// The user-facing name of this symbol.
      std::string name () const YY_NOEXCEPT
      {
        return HTMLParser::symbol_name (this->kind ());
      }

      /// Backward compatibility (Bison 3.6).
      symbol_kind_type type_get () const YY_NOEXCEPT;

      /// Whether empty.
      bool empty () const YY_NOEXCEPT;

      /// Destructive move, \a s is emptied into this.
      void move (basic_symbol& s);

      /// The semantic value.
      semantic_type value;

    private:
#if YY_CPLUSPLUS < 201103L
      /// Assignment operator.
      basic_symbol& operator= (const basic_symbol& that);
#endif
    };

    /// Type access provider for token (enum) based symbols.
    struct by_kind
    {
      /// Default constructor.
      by_kind ();

#if 201103L <= YY_CPLUSPLUS
      /// Move constructor.
      by_kind (by_kind&& that);
#endif

      /// Copy constructor.
      by_kind (const by_kind& that);

      /// The symbol kind as needed by the constructor.
      typedef token_kind_type kind_type;

      /// Constructor from (external) token numbers.
      by_kind (kind_type t);

      /// Record that this symbol is empty.
      void clear ();

      /// Steal the symbol kind from \a that.
      void move (by_kind& that);

      /// The (internal) type number (corresponding to \a type).
      /// \a empty when empty.
      symbol_kind_type kind () const YY_NOEXCEPT;

      /// Backward compatibility (Bison 3.6).
      symbol_kind_type type_get () const YY_NOEXCEPT;

      /// The symbol kind.
      /// \a S_YYEMPTY when empty.
      symbol_kind_type kind_;
    };

    /// Backward compatibility for a private implementation detail (Bison 3.6).
    typedef by_kind by_type;

    /// "External" symbols: returned by the scanner.
    struct symbol_type : basic_symbol<by_kind>
    {};

    /// Build a parser object.
    HTMLParser (HTMLDriver &drv_yyarg);
    virtual ~HTMLParser ();

#if 201103L <= YY_CPLUSPLUS
    /// Non copyable.
    HTMLParser (const HTMLParser&) = delete;
    /// Non copyable.
    HTMLParser& operator= (const HTMLParser&) = delete;
#endif

    /// Parse.  An alias for parse ().
    /// \returns  0 iff parsing succeeded.
    int operator() ();

    /// Parse.
    /// \returns  0 iff parsing succeeded.
    virtual int parse ();

#if YYDEBUG
    /// The current debugging stream.
    std::ostream& debug_stream () const YY_ATTRIBUTE_PURE;
    /// Set the current debugging stream.
    void set_debug_stream (std::ostream &);

    /// Type for debugging levels.
    typedef int debug_level_type;
    /// The current debugging level.
    debug_level_type debug_level () const YY_ATTRIBUTE_PURE;
    /// Set the current debugging level.
    void set_debug_level (debug_level_type l);
#endif

    /// Report a syntax error.
    /// \param msg    a description of the syntax error.
    virtual void error (const std::string& msg);

    /// Report a syntax error.
    void error (const syntax_error& err);

    /// The user-facing name of the symbol whose (internal) number is
    /// YYSYMBOL.  No bounds checking.
    static std::string symbol_name (symbol_kind_type yysymbol);



    class context
    {
    public:
      context (const HTMLParser& yyparser, const symbol_type& yyla);
      const symbol_type& lookahead () const { return yyla_; }
      symbol_kind_type token () const { return yyla_.kind (); }
      /// Put in YYARG at most YYARGN of the expected tokens, and return the
      /// number of tokens stored in YYARG.  If YYARG is null, return the
      /// number of expected tokens (guaranteed to be less than YYNTOKENS).
      int expected_tokens (symbol_kind_type yyarg[], int yyargn) const;

    private:
      const HTMLParser& yyparser_;
      const symbol_type& yyla_;
    };

  private:
#if YY_CPLUSPLUS < 201103L
    /// Non copyable.
    HTMLParser (const HTMLParser&);
    /// Non copyable.
    HTMLParser& operator= (const HTMLParser&);
#endif


    /// Stored state numbers (used for stacks).
    typedef short state_type;

    /// The arguments of the error message.
    int yy_syntax_error_arguments_ (const context& yyctx,
                                    symbol_kind_type yyarg[], int yyargn) const;

    /// Generate an error message.
    /// \param yyctx     the context in which the error occurred.
    virtual std::string yysyntax_error_ (const context& yyctx) const;
    /// Compute post-reduction state.
    /// \param yystate   the current state
    /// \param yysym     the nonterminal to push on the stack
    static state_type yy_lr_goto_state_ (state_type yystate, int yysym);

    /// Whether the given \c yypact_ value indicates a defaulted state.
    /// \param yyvalue   the value to check
    static bool yy_pact_value_is_default_ (int yyvalue);

    /// Whether the given \c yytable_ value indicates a syntax error.
    /// \param yyvalue   the value to check
    static bool yy_table_value_is_error_ (int yyvalue);

    static const short yypact_ninf_;
    static const short yytable_ninf_;

    /// Convert a scanner token kind \a t to a symbol kind.
    /// In theory \a t should be a token_kind_type, but character literals
    /// are valid, yet not members of the token_type enum.
    static symbol_kind_type yytranslate_ (int t);

    /// Convert the symbol name \a n to a form suitable for a diagnostic.
    static std::string yytnamerr_ (const char *yystr);

    /// For a symbol, its name in clear.
    static const char* const yytname_[];


    // Tables.
    // YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
    // STATE-NUM.
    static const short yypact_[];

    // YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
    // Performed when YYTABLE does not specify something else to do.  Zero
    // means the default is an error.
    static const unsigned char yydefact_[];

    // YYPGOTO[NTERM-NUM].
    static const short yypgoto_[];

    // YYDEFGOTO[NTERM-NUM].
    static const short yydefgoto_[];

    // YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
    // positive, shift that token.  If negative, reduce the rule whose
    // number is the opposite.  If YYTABLE_NINF, syntax error.
    static const short yytable_[];

    static const short yycheck_[];

    // YYSTOS[STATE-NUM] -- The (internal number of the) accessing
    // symbol of state STATE-NUM.
    static const unsigned char yystos_[];

    // YYR1[YYN] -- Symbol number of symbol that rule YYN derives.
    static const unsigned char yyr1_[];

    // YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.
    static const signed char yyr2_[];


#if YYDEBUG
    // YYRLINE[YYN] -- Source line where rule number YYN was defined.
    static const short yyrline_[];
    /// Report on the debug stream that the rule \a r is going to be reduced.
    virtual void yy_reduce_print_ (int r) const;
    /// Print the state stack on the debug stream.
    virtual void yy_stack_print_ () const;

    /// Debugging level.
    int yydebug_;
    /// Debug stream.
    std::ostream* yycdebug_;

    /// \brief Display a symbol kind, value and location.
    /// \param yyo    The output stream.
    /// \param yysym  The symbol.
    template <typename Base>
    void yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const;
#endif

    /// \brief Reclaim the memory associated to a symbol.
    /// \param yymsg     Why this token is reclaimed.
    ///                  If null, print nothing.
    /// \param yysym     The symbol.
    template <typename Base>
    void yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const;

  private:
    /// Type access provider for state based symbols.
    struct by_state
    {
      /// Default constructor.
      by_state () YY_NOEXCEPT;

      /// The symbol kind as needed by the constructor.
      typedef state_type kind_type;

      /// Constructor.
      by_state (kind_type s) YY_NOEXCEPT;

      /// Copy constructor.
      by_state (const by_state& that) YY_NOEXCEPT;

      /// Record that this symbol is empty.
      void clear () YY_NOEXCEPT;

      /// Steal the symbol kind from \a that.
      void move (by_state& that);

      /// The symbol kind (corresponding to \a state).
      /// \a S_YYEMPTY when empty.
      symbol_kind_type kind () const YY_NOEXCEPT;

      /// The state number used to denote an empty symbol.
      /// We use the initial state, as it does not have a value.
      enum { empty_state = 0 };

      /// The state.
      /// \a empty when empty.
      state_type state;
    };

    /// "Internal" symbol: element of the stack.
    struct stack_symbol_type : basic_symbol<by_state>
    {
      /// Superclass.
      typedef basic_symbol<by_state> super_type;
      /// Construct an empty symbol.
      stack_symbol_type ();
      /// Move or copy construction.
      stack_symbol_type (YY_RVREF (stack_symbol_type) that);
      /// Steal the contents from \a sym to build this.
      stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) sym);
#if YY_CPLUSPLUS < 201103L
      /// Assignment, needed by push_back by some old implementations.
      /// Moves the contents of that.
      stack_symbol_type& operator= (stack_symbol_type& that);

      /// Assignment, needed by push_back by other implementations.
      /// Needed by some other old implementations.
      stack_symbol_type& operator= (const stack_symbol_type& that);
#endif
    };

    /// A stack with random access from its top.
    template <typename T, typename S = std::vector<T> >
    class stack
    {
    public:
      // Hide our reversed order.
      typedef typename S::iterator iterator;
      typedef typename S::const_iterator const_iterator;
      typedef typename S::size_type size_type;
      typedef typename std::ptrdiff_t index_type;

      stack (size_type n = 200)
        : seq_ (n)
      {}

#if 201103L <= YY_CPLUSPLUS
      /// Non copyable.
      stack (const stack&) = delete;
      /// Non copyable.
      stack& operator= (const stack&) = delete;
#endif

      /// Random access.
      ///
      /// Index 0 returns the topmost element.
      const T&
      operator[] (index_type i) const
      {
        return seq_[size_type (size () - 1 - i)];
      }

      /// Random access.
      ///
      /// Index 0 returns the topmost element.
      T&
      operator[] (index_type i)
      {
        return seq_[size_type (size () - 1 - i)];
      }

      /// Steal the contents of \a t.
      ///
      /// Close to move-semantics.
      void
      push (YY_MOVE_REF (T) t)
      {
        seq_.push_back (T ());
        operator[] (0).move (t);
      }

      /// Pop elements from the stack.
      void
      pop (std::ptrdiff_t n = 1) YY_NOEXCEPT
      {
        for (; 0 < n; --n)
          seq_.pop_back ();
      }

      /// Pop all elements from the stack.
      void
      clear () YY_NOEXCEPT
      {
        seq_.clear ();
      }

      /// Number of elements on the stack.
      index_type
      size () const YY_NOEXCEPT
      {
        return index_type (seq_.size ());
      }

      /// Iterator on top of the stack (going downwards).
      const_iterator
      begin () const YY_NOEXCEPT
      {
        return seq_.begin ();
      }

      /// Bottom of the stack.
      const_iterator
      end () const YY_NOEXCEPT
      {
        return seq_.end ();
      }

      /// Present a slice of the top of a stack.
      class slice
      {
      public:
        slice (const stack& stack, index_type range)
          : stack_ (stack)
          , range_ (range)
        {}

        const T&
        operator[] (index_type i) const
        {
          return stack_[range_ - i];
        }

      private:
        const stack& stack_;
        index_type range_;
      };

    private:
#if YY_CPLUSPLUS < 201103L
      /// Non copyable.
      stack (const stack&);
      /// Non copyable.
      stack& operator= (const stack&);
#endif
      /// The wrapped container.
      S seq_;
    };


    /// Stack type.
    typedef stack<stack_symbol_type> stack_type;

    /// The stack.
    stack_type yystack_;

    /// Push a new state on the stack.
    /// \param m    a debug message to display
    ///             if null, no trace is output.
    /// \param sym  the symbol
    /// \warning the contents of \a s.value is stolen.
    void yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym);

    /// Push a new look ahead token on the state on the stack.
    /// \param m    a debug message to display
    ///             if null, no trace is output.
    /// \param s    the state
    /// \param sym  the symbol (for its value and location).
    /// \warning the contents of \a sym.value is stolen.
    void yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym);

    /// Pop \a n symbols from the stack.
    void yypop_ (int n = 1);

    /// Constants.
    enum
    {
      yylast_ = 2229,     ///< Last index in yytable_.
      yynnts_ = 80,  ///< Number of nonterminal symbols.
      yyfinal_ = 3 ///< Termination state number.
    };


    // User arguments.
    HTMLDriver &drv;

  };


#line 40 "HTMLParser.yy"
} // html2text
#line 1123 "HTMLParser.tab.hh"





#endif // !YY_YY_HTMLPARSER_TAB_HH_INCLUDED
