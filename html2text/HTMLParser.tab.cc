// A Bison parser, made by GNU Bison 3.6.3.

// Skeleton implementation for Bison LALR(1) parsers in C++

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

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.





#include "HTMLParser.tab.hh"


// Unqualified %code blocks.
#line 56 "HTMLParser.yy"

#include "HTMLDriver.h"
// call the lex function of HTMLDriver instead of plain yylex
#undef yylex
#define yylex drv.lex
#undef yyerror
#define yyerror drv.yyerror

#line 55 "HTMLParser.tab.cc"


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif


// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif



// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yy_stack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

#line 40 "HTMLParser.yy"
namespace html2text {
#line 129 "HTMLParser.tab.cc"

  /// Build a parser object.
  HTMLParser::HTMLParser (HTMLDriver &drv_yyarg)
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      drv (drv_yyarg)
  {}

  HTMLParser::~HTMLParser ()
  {}

  HTMLParser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------------.
  | symbol kinds.  |
  `---------------*/

  // basic_symbol.
  template <typename Base>
  HTMLParser::basic_symbol<Base>::basic_symbol (const basic_symbol& that)
    : Base (that)
    , value (that.value)
  {}


  /// Constructor for valueless symbols.
  template <typename Base>
  HTMLParser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t)
    : Base (t)
    , value ()
  {}

  template <typename Base>
  HTMLParser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, YY_RVREF (semantic_type) v)
    : Base (t)
    , value (YY_MOVE (v))
  {}

  template <typename Base>
  HTMLParser::symbol_kind_type
  HTMLParser::basic_symbol<Base>::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }

  template <typename Base>
  bool
  HTMLParser::basic_symbol<Base>::empty () const YY_NOEXCEPT
  {
    return this->kind () == symbol_kind::S_YYEMPTY;
  }

  template <typename Base>
  void
  HTMLParser::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move (s);
    value = YY_MOVE (s.value);
  }

  // by_kind.
  HTMLParser::by_kind::by_kind ()
    : kind_ (symbol_kind::S_YYEMPTY)
  {}

#if 201103L <= YY_CPLUSPLUS
  HTMLParser::by_kind::by_kind (by_kind&& that)
    : kind_ (that.kind_)
  {
    that.clear ();
  }
#endif

  HTMLParser::by_kind::by_kind (const by_kind& that)
    : kind_ (that.kind_)
  {}

  HTMLParser::by_kind::by_kind (token_kind_type t)
    : kind_ (yytranslate_ (t))
  {}

  void
  HTMLParser::by_kind::clear ()
  {
    kind_ = symbol_kind::S_YYEMPTY;
  }

  void
  HTMLParser::by_kind::move (by_kind& that)
  {
    kind_ = that.kind_;
    that.clear ();
  }

  HTMLParser::symbol_kind_type
  HTMLParser::by_kind::kind () const YY_NOEXCEPT
  {
    return kind_;
  }

  HTMLParser::symbol_kind_type
  HTMLParser::by_kind::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }


  // by_state.
  HTMLParser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  HTMLParser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  HTMLParser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  HTMLParser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  HTMLParser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  HTMLParser::symbol_kind_type
  HTMLParser::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

  HTMLParser::stack_symbol_type::stack_symbol_type ()
  {}

  HTMLParser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.value))
  {
#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  HTMLParser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.value))
  {
    // that is emptied.
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
  HTMLParser::stack_symbol_type&
  HTMLParser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    value = that.value;
    return *this;
  }

  HTMLParser::stack_symbol_type&
  HTMLParser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    value = that.value;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  HTMLParser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);

    // User destructor.
    YYUSE (yysym.kind ());
  }

#if YYDEBUG
  template <typename Base>
  void
  HTMLParser::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    if (yysym.empty ())
      yyo << "empty symbol";
    else
      {
        symbol_kind_type yykind = yysym.kind ();
        yyo << (yykind < YYNTOKENS ? "token" : "nterm")
            << ' ' << yysym.name () << " (";
        YYUSE (yykind);
        yyo << ')';
      }
  }
#endif

  void
  HTMLParser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  HTMLParser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  HTMLParser::yypop_ (int n)
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  HTMLParser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  HTMLParser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  HTMLParser::debug_level_type
  HTMLParser::debug_level () const
  {
    return yydebug_;
  }

  void
  HTMLParser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  HTMLParser::state_type
  HTMLParser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
  HTMLParser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  HTMLParser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  HTMLParser::operator() ()
  {
    return parse ();
  }

  int
  HTMLParser::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The return value of parse ().
    int yyresult;

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';
    YY_STACK_PRINT ();

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            yyla.kind_ = yytranslate_ (yylex (&yyla.value));
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    if (yyla.kind () == symbol_kind::S_YYerror)
    {
      // The scanner already issued an error message, process directly
      // to error recovery.  But do not keep the error token as
      // lookahead, it is too special and may lead us to an endless
      // loop in error recovery. */
      yyla.kind_ = symbol_kind::S_YYUNDEF;
      goto yyerrlab1;
    }

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.kind ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.kind ())
      {
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* If YYLEN is nonzero, implement the default value of the
         action: '$$ = $1'.  Otherwise, use the top of the stack.

         Otherwise, the following line sets YYLHS.VALUE to garbage.
         This behavior is undocumented and Bison users should not rely
         upon it.  */
      if (yylen)
        yylhs.value = yystack_[yylen - 1].value;
      else
        yylhs.value = yystack_[0].value;


      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2:
#line 274 "HTMLParser.yy"
            {
    drv.process(*(yystack_[0].value.document));
    delete (yystack_[0].value.document);
  }
#line 583 "HTMLParser.tab.cc"
    break;

  case 3:
#line 305 "HTMLParser.yy"
              {
    (yylhs.value.document) = new Document;
    (yylhs.value.document)->body.content.reset(new list<auto_ptr<Element> >);
  }
#line 592 "HTMLParser.tab.cc"
    break;

  case 4:
#line 309 "HTMLParser.yy"
                    {
    (yylhs.value.document) = (yystack_[1].value.document);
  }
#line 600 "HTMLParser.tab.cc"
    break;

  case 5:
#line 312 "HTMLParser.yy"
                      {
    (yylhs.value.document) = (yystack_[1].value.document);
  }
#line 608 "HTMLParser.tab.cc"
    break;

  case 6:
#line 315 "HTMLParser.yy"
                   {
    (yylhs.value.document)->attributes.reset((yystack_[0].value.tag_attributes));
    (yylhs.value.document) = (yystack_[1].value.document);
  }
#line 617 "HTMLParser.tab.cc"
    break;

  case 7:
#line 319 "HTMLParser.yy"
                       {
    (yylhs.value.document) = (yystack_[1].value.document);
  }
#line 625 "HTMLParser.tab.cc"
    break;

  case 8:
#line 322 "HTMLParser.yy"
                   {
    delete (yystack_[0].value.tag_attributes);
    (yylhs.value.document) = (yystack_[1].value.document);
  }
#line 634 "HTMLParser.tab.cc"
    break;

  case 9:
#line 326 "HTMLParser.yy"
                       {
    (yylhs.value.document) = (yystack_[1].value.document);
  }
#line 642 "HTMLParser.tab.cc"
    break;

  case 10:
#line 329 "HTMLParser.yy"
                                             {
    delete (yystack_[2].value.tag_attributes); // Ignore <TITLE> attributes
    ((yylhs.value.document) = (yystack_[3].value.document))->head.title.reset((yystack_[1].value.pcdata));
  }
#line 651 "HTMLParser.tab.cc"
    break;

  case 11:
#line 333 "HTMLParser.yy"
                      {
    ((yylhs.value.document) = (yystack_[1].value.document))->head.isindex_attributes.reset((yystack_[0].value.tag_attributes));
  }
#line 659 "HTMLParser.tab.cc"
    break;

  case 12:
#line 336 "HTMLParser.yy"
                   {
    ((yylhs.value.document) = (yystack_[1].value.document))->head.base_attributes.reset((yystack_[0].value.tag_attributes));
  }
#line 667 "HTMLParser.tab.cc"
    break;

  case 13:
#line 339 "HTMLParser.yy"
                   {
    auto_ptr<Meta> s(new Meta);
    s->attributes.reset((yystack_[0].value.tag_attributes));
    ((yylhs.value.document) = (yystack_[1].value.document))->head.metas.push_back(s);
  }
#line 677 "HTMLParser.tab.cc"
    break;

  case 14:
#line 344 "HTMLParser.yy"
                   {
    ((yylhs.value.document) = (yystack_[1].value.document))->head.link_attributes.reset((yystack_[0].value.tag_attributes));
  }
#line 685 "HTMLParser.tab.cc"
    break;

  case 15:
#line 347 "HTMLParser.yy"
                     {
    auto_ptr<Script> s(new Script);
    s->attributes.reset((yystack_[0].value.tag_attributes));
    if (!drv.read_cdata("</SCRIPT>", &s->text)) {
      yyerror("CDATA terminal not found");
    }
    ((yylhs.value.document) = (yystack_[1].value.document))->head.scripts.push_back(s);
  }
#line 698 "HTMLParser.tab.cc"
    break;

  case 16:
#line 355 "HTMLParser.yy"
                    {
    auto_ptr<Style> s(new Style);
    s->attributes.reset((yystack_[0].value.tag_attributes));
    if (!drv.read_cdata("</STYLE>", &s->text)) {
      yyerror("CDATA terminal not found");
    }
    ((yylhs.value.document) = (yystack_[1].value.document))->head.styles.push_back(s);
  }
#line 711 "HTMLParser.tab.cc"
    break;

  case 17:
#line 363 "HTMLParser.yy"
                   {
    delete (yystack_[0].value.tag_attributes);
    (yylhs.value.document) = (yystack_[1].value.document);
  }
#line 720 "HTMLParser.tab.cc"
    break;

  case 18:
#line 367 "HTMLParser.yy"
                       {
    (yylhs.value.document) = (yystack_[1].value.document);
  }
#line 728 "HTMLParser.tab.cc"
    break;

  case 19:
#line 370 "HTMLParser.yy"
                    {
    Paragraph *p = new Paragraph;
    p->texts.reset((yystack_[0].value.element_list));
    ((yylhs.value.document) = (yystack_[1].value.document))->body.content->push_back(auto_ptr<Element>(p));
  }
#line 738 "HTMLParser.tab.cc"
    break;

  case 20:
#line 375 "HTMLParser.yy"
                      {
    ((yylhs.value.document) = (yystack_[1].value.document))->body.content->push_back(auto_ptr<Element>((yystack_[0].value.heading)));
  }
#line 746 "HTMLParser.tab.cc"
    break;

  case 21:
#line 378 "HTMLParser.yy"
                    {
    ((yylhs.value.document) = (yystack_[1].value.document))->body.content->push_back(auto_ptr<Element>((yystack_[0].value.element)));
  }
#line 754 "HTMLParser.tab.cc"
    break;

  case 22:
#line 381 "HTMLParser.yy"
                      {
    ((yylhs.value.document) = (yystack_[1].value.document))->body.content->push_back(auto_ptr<Element>((yystack_[0].value.address)));
  }
#line 762 "HTMLParser.tab.cc"
    break;

  case 23:
#line 387 "HTMLParser.yy"
         {
    (yylhs.value.pcdata) = new PCData;
    (yylhs.value.pcdata)->text = *(yystack_[0].value.strinG);
    delete (yystack_[0].value.strinG);
  }
#line 772 "HTMLParser.tab.cc"
    break;

  case 24:
#line 395 "HTMLParser.yy"
              {
    (yylhs.value.element_list) = new list<auto_ptr<Element>>;
  }
#line 780 "HTMLParser.tab.cc"
    break;

  case 25:
#line 398 "HTMLParser.yy"
                       {
    (yylhs.value.element_list) = (yystack_[1].value.element_list);
  }
#line 788 "HTMLParser.tab.cc"
    break;

  case 26:
#line 401 "HTMLParser.yy"
                        {
    auto_ptr<Script> s(new Script);
    s->attributes.reset((yystack_[0].value.tag_attributes));
    if (!drv.read_cdata("</SCRIPT>", &s->text)) {
      yyerror("CDATA terminal not found");
    }
//    ($$ = $1)->head.scripts.push_back(s);
  }
#line 801 "HTMLParser.tab.cc"
    break;

  case 27:
#line 409 "HTMLParser.yy"
                       {
    auto_ptr<Style> s(new Style);
    s->attributes.reset((yystack_[0].value.tag_attributes));
    if (!drv.read_cdata("</STYLE>", &s->text)) {
      yyerror("CDATA terminal not found");
    }
//    ($$ = $1)->head.styles.push_back(s);
  }
#line 814 "HTMLParser.tab.cc"
    break;

  case 28:
#line 417 "HTMLParser.yy"
                      {
    /* This seems to happen for instance by Mozilla Thunderbird in its
     * replies, a blockquote is followed by a meta tag having content
     * encoding.  Don't error out, just ignore this */
    (yylhs.value.element_list) = new list<auto_ptr<Element>>;
  }
#line 825 "HTMLParser.tab.cc"
    break;

  case 29:
#line 423 "HTMLParser.yy"
                       {
    Paragraph *p = new Paragraph;
    p->texts = auto_ptr<list<auto_ptr<Element> > >((yystack_[0].value.element_list));
    ((yylhs.value.element_list) = (yystack_[1].value.element_list))->push_back(auto_ptr<Element>(p));
  }
#line 835 "HTMLParser.tab.cc"
    break;

  case 30:
#line 428 "HTMLParser.yy"
                         {
    ((yylhs.value.element_list) = (yystack_[1].value.element_list))->push_back(auto_ptr<Element>((yystack_[0].value.heading)));
  }
#line 843 "HTMLParser.tab.cc"
    break;

  case 31:
#line 431 "HTMLParser.yy"
                       {
    ((yylhs.value.element_list) = (yystack_[1].value.element_list))->push_back(auto_ptr<Element>((yystack_[0].value.element)));
  }
#line 851 "HTMLParser.tab.cc"
    break;

  case 32:
#line 434 "HTMLParser.yy"
                         {
    ((yylhs.value.element_list) = (yystack_[1].value.element_list))->push_back(auto_ptr<Element>((yystack_[0].value.address)));
  }
#line 859 "HTMLParser.tab.cc"
    break;

  case 33:
#line 440 "HTMLParser.yy"
                              {
            /* EXTENSION: Allow paragraph content in heading, not only texts */
    if ((yystack_[2].value.heading)->level != (yystack_[0].value.inT)) {
      yyerror ("Levels of opening and closing headings don't match");
    }
    (yylhs.value.heading) = (yystack_[2].value.heading);
    (yylhs.value.heading)->content.reset((yystack_[1].value.element_list));
  }
#line 872 "HTMLParser.tab.cc"
    break;

  case 34:
#line 451 "HTMLParser.yy"
                 {
    (yylhs.value.element) = (yystack_[0].value.element);
  }
#line 880 "HTMLParser.tab.cc"
    break;

  case 35:
#line 454 "HTMLParser.yy"
                                  {
    Paragraph *p = new Paragraph;
    p->attributes.reset((yystack_[2].value.tag_attributes));
    p->texts.reset((yystack_[1].value.element_list));
    (yylhs.value.element) = p;
  }
#line 891 "HTMLParser.tab.cc"
    break;

  case 36:
#line 463 "HTMLParser.yy"
              {
    (yylhs.value.element_list) = new list<auto_ptr<Element> >;
  }
#line 899 "HTMLParser.tab.cc"
    break;

  case 37:
#line 466 "HTMLParser.yy"
                            {
    (yylhs.value.element_list) = (yystack_[1].value.element_list);
  }
#line 907 "HTMLParser.tab.cc"
    break;

  case 38:
#line 469 "HTMLParser.yy"
                            {
    (yylhs.value.element_list) = (yystack_[1].value.element_list);
    (yylhs.value.element_list)->splice((yylhs.value.element_list)->end(), *(yystack_[0].value.element_list));
    delete (yystack_[0].value.element_list);
  }
#line 917 "HTMLParser.tab.cc"
    break;

  case 39:
#line 474 "HTMLParser.yy"
                                     {
    ((yylhs.value.element_list) = (yystack_[1].value.element_list))->push_back(auto_ptr<Element>((yystack_[0].value.element)));
  }
#line 925 "HTMLParser.tab.cc"
    break;

  case 40:
#line 480 "HTMLParser.yy"
       {
    (yylhs.value.element) = (yystack_[0].value.element);
  }
#line 933 "HTMLParser.tab.cc"
    break;

  case 41:
#line 483 "HTMLParser.yy"
                 {
    (yylhs.value.element) = (yystack_[0].value.preformatted);
  }
#line 941 "HTMLParser.tab.cc"
    break;

  case 42:
#line 486 "HTMLParser.yy"
                    {
    (yylhs.value.element) = (yystack_[0].value.definition_list);
  }
#line 949 "HTMLParser.tab.cc"
    break;

  case 43:
#line 489 "HTMLParser.yy"
                                 {
    Division *p = new Division;
    p->attributes.reset((yystack_[2].value.tag_attributes));
    p->body_content.reset((yystack_[1].value.element_list));
    (yylhs.value.element) = p;
  }
#line 960 "HTMLParser.tab.cc"
    break;

  case 44:
#line 495 "HTMLParser.yy"
                                       {
    Center *p = new Center;
    delete (yystack_[2].value.tag_attributes);       // CENTER has no attributes.
    p->body_content.reset((yystack_[1].value.element_list));
    (yylhs.value.element) = p;
  }
#line 971 "HTMLParser.tab.cc"
    break;

  case 45:
#line 501 "HTMLParser.yy"
                                               {
    delete (yystack_[2].value.tag_attributes); // BLOCKQUOTE has no attributes!
    BlockQuote *bq = new BlockQuote;
    bq->content.reset((yystack_[1].value.element_list));
    (yylhs.value.element) = bq;
  }
#line 982 "HTMLParser.tab.cc"
    break;

  case 46:
#line 507 "HTMLParser.yy"
                                   {
    Form *f = new Form;
    f->attributes.reset((yystack_[2].value.tag_attributes));
    f->content.reset((yystack_[1].value.element_list));
    (yylhs.value.element) = f;
  }
#line 993 "HTMLParser.tab.cc"
    break;

  case 47:
#line 513 "HTMLParser.yy"
       {
    HorizontalRule *h = new HorizontalRule;
    h->attributes.reset((yystack_[0].value.tag_attributes));
    (yylhs.value.element) = h;
  }
#line 1003 "HTMLParser.tab.cc"
    break;

  case 48:
#line 518 "HTMLParser.yy"
                                               {
    Table *t = new Table;
    t->attributes.reset((yystack_[3].value.tag_attributes));
    t->caption.reset((yystack_[2].value.caption));
    t->rows.reset((yystack_[1].value.table_rows));
    (yylhs.value.element) = t;
  }
#line 1015 "HTMLParser.tab.cc"
    break;

  case 49:
#line 528 "HTMLParser.yy"
     { ++drv.list_nesting; }
#line 1021 "HTMLParser.tab.cc"
    break;

  case 50:
#line 528 "HTMLParser.yy"
                                                 {
    OrderedList *ol = new OrderedList;
    ol->attributes.reset((yystack_[3].value.tag_attributes));
    ol->items.reset((yystack_[1].value.list_items));
    ol->nesting = --drv.list_nesting;
    (yylhs.value.element) = ol;
  }
#line 1033 "HTMLParser.tab.cc"
    break;

  case 51:
#line 535 "HTMLParser.yy"
       { ++drv.list_nesting; }
#line 1039 "HTMLParser.tab.cc"
    break;

  case 52:
#line 535 "HTMLParser.yy"
                                                       {
    UnorderedList *ul = new UnorderedList;
    ul->attributes.reset((yystack_[3].value.tag_attributes));
    ul->items.reset((yystack_[1].value.list_items));
    ul->nesting = --drv.list_nesting;
    (yylhs.value.element) = ul;
  }
#line 1051 "HTMLParser.tab.cc"
    break;

  case 53:
#line 542 "HTMLParser.yy"
        { ++drv.list_nesting; }
#line 1057 "HTMLParser.tab.cc"
    break;

  case 54:
#line 542 "HTMLParser.yy"
                                                     {
    Dir *d = new Dir;
    d->attributes.reset((yystack_[3].value.tag_attributes));
    d->items.reset((yystack_[1].value.list_items));
    d->nesting = --drv.list_nesting;
    (yylhs.value.element) = d;
  }
#line 1069 "HTMLParser.tab.cc"
    break;

  case 55:
#line 549 "HTMLParser.yy"
         { ++drv.list_nesting; }
#line 1075 "HTMLParser.tab.cc"
    break;

  case 56:
#line 549 "HTMLParser.yy"
                                                       {
    Menu *m = new Menu;
    m->attributes.reset((yystack_[3].value.tag_attributes));
    m->items.reset((yystack_[1].value.list_items));
    m->nesting = --drv.list_nesting;
    (yylhs.value.element) = m;
  }
#line 1087 "HTMLParser.tab.cc"
    break;

  case 57:
#line 559 "HTMLParser.yy"
              {
    (yylhs.value.list_items) = 0;
  }
#line 1095 "HTMLParser.tab.cc"
    break;

  case 58:
#line 562 "HTMLParser.yy"
                       {
    (yylhs.value.list_items) = (yystack_[1].value.list_items);
  }
#line 1103 "HTMLParser.tab.cc"
    break;

  case 59:
#line 565 "HTMLParser.yy"
                           {
    (yylhs.value.list_items) = (yystack_[1].value.list_items) ? (yystack_[1].value.list_items) : new list<auto_ptr<ListItem> >;
    (yylhs.value.list_items)->push_back(auto_ptr<ListItem>((yystack_[0].value.list_item)));
  }
#line 1112 "HTMLParser.tab.cc"
    break;

  case 60:
#line 572 "HTMLParser.yy"
                         {
    ListNormalItem *lni = new ListNormalItem;
    lni->attributes.reset((yystack_[2].value.tag_attributes));
    lni->flow.reset((yystack_[1].value.element_list));
    (yylhs.value.list_item) = lni;
  }
#line 1123 "HTMLParser.tab.cc"
    break;

  case 61:
#line 578 "HTMLParser.yy"
          {   /* EXTENSION: Handle a "block" in a list as an indented block. */
    ListBlockItem *lbi = new ListBlockItem;
    lbi->block.reset((yystack_[0].value.element));
    (yylhs.value.list_item) = lbi;
  }
#line 1133 "HTMLParser.tab.cc"
    break;

  case 62:
#line 583 "HTMLParser.yy"
          {              /* EXTENSION: Treat "texts" in a list as an "<LI>". */
    ListNormalItem *lni = new ListNormalItem;
    lni->flow.reset((yystack_[0].value.element_list));
    (yylhs.value.list_item) = lni;
  }
#line 1143 "HTMLParser.tab.cc"
    break;

  case 63:
#line 593 "HTMLParser.yy"
                                                   {
    delete (yystack_[4].value.tag_attributes);
    delete (yystack_[3].value.element_list); /* Kludge */
    (yylhs.value.definition_list) = (yystack_[1].value.definition_list);
  }
#line 1153 "HTMLParser.tab.cc"
    break;

  case 64:
#line 599 "HTMLParser.yy"
                                                         {
    DefinitionList *dl = new DefinitionList;
    dl->attributes.reset((yystack_[4].value.tag_attributes));
    dl->preamble.reset((yystack_[3].value.element_list));
    dl->items.reset((yystack_[1].value.definition_list_item_list));
    (yylhs.value.definition_list) = dl;
  }
#line 1165 "HTMLParser.tab.cc"
    break;

  case 65:
#line 609 "HTMLParser.yy"
              {
    (yylhs.value.definition_list_item_list) = 0;
  }
#line 1173 "HTMLParser.tab.cc"
    break;

  case 66:
#line 612 "HTMLParser.yy"
                            {
    (yylhs.value.definition_list_item_list) = (yystack_[0].value.definition_list_item_list);
  }
#line 1181 "HTMLParser.tab.cc"
    break;

  case 67:
#line 615 "HTMLParser.yy"
                                      {
    (yylhs.value.definition_list_item_list) = (yystack_[1].value.definition_list_item_list) ? (yystack_[1].value.definition_list_item_list) : new list<auto_ptr<DefinitionListItem> >;
    (yylhs.value.definition_list_item_list)->push_back(auto_ptr<DefinitionListItem>((yystack_[0].value.term_name)));
  }
#line 1190 "HTMLParser.tab.cc"
    break;

  case 68:
#line 619 "HTMLParser.yy"
                                            {
    (yylhs.value.definition_list_item_list) = (yystack_[1].value.definition_list_item_list) ? (yystack_[1].value.definition_list_item_list) : new list<auto_ptr<DefinitionListItem> >;
    (yylhs.value.definition_list_item_list)->push_back(auto_ptr<DefinitionListItem>((yystack_[0].value.term_definition)));
  }
#line 1199 "HTMLParser.tab.cc"
    break;

  case 69:
#line 626 "HTMLParser.yy"
                        {      /* EXTENSION: Allow "flow" instead of "texts" */
    delete (yystack_[2].value.tag_attributes);
    (yylhs.value.term_name) = new TermName;
    (yylhs.value.term_name)->flow.reset((yystack_[1].value.element_list));
  }
#line 1209 "HTMLParser.tab.cc"
    break;

  case 70:
#line 631 "HTMLParser.yy"
                                       {/* EXTENSION: Ignore <P> after </DT> */
    delete (yystack_[4].value.tag_attributes);
    delete (yystack_[1].value.tag_attributes);
    (yylhs.value.term_name) = new TermName;
    (yylhs.value.term_name)->flow.reset((yystack_[3].value.element_list));
  }
#line 1220 "HTMLParser.tab.cc"
    break;

  case 71:
#line 640 "HTMLParser.yy"
                        {
    delete (yystack_[2].value.tag_attributes);
    (yylhs.value.term_definition) = new TermDefinition;
    (yylhs.value.term_definition)->flow.reset((yystack_[1].value.element_list));
  }
#line 1230 "HTMLParser.tab.cc"
    break;

  case 72:
#line 645 "HTMLParser.yy"
                                       {/* EXTENSION: Ignore <P> after </DD> */
    delete (yystack_[4].value.tag_attributes);
    delete (yystack_[1].value.tag_attributes);
    (yylhs.value.term_definition) = new TermDefinition;
    (yylhs.value.term_definition)->flow.reset((yystack_[3].value.element_list));
  }
#line 1241 "HTMLParser.tab.cc"
    break;

  case 73:
#line 654 "HTMLParser.yy"
        {
    (yylhs.value.element_list) = new list<auto_ptr<Element> >;
    (yylhs.value.element_list)->push_back(auto_ptr<Element>((yystack_[0].value.element)));
  }
#line 1250 "HTMLParser.tab.cc"
    break;

  case 74:
#line 658 "HTMLParser.yy"
               {
    (yylhs.value.element_list) = (yystack_[1].value.element_list);
  }
#line 1258 "HTMLParser.tab.cc"
    break;

  case 75:
#line 661 "HTMLParser.yy"
               {
    ((yylhs.value.element_list) = (yystack_[1].value.element_list))->push_back(auto_ptr<Element>((yystack_[0].value.element)));
  }
#line 1266 "HTMLParser.tab.cc"
    break;

  case 76:
#line 667 "HTMLParser.yy"
       {
    (yylhs.value.element) = (yystack_[0].value.element);
  }
#line 1274 "HTMLParser.tab.cc"
    break;

  case 77:
#line 670 "HTMLParser.yy"
            {          /* EXTENSION: Allow headings in "flow", i.e. in lists */
    (yylhs.value.element) = (yystack_[0].value.heading);
  }
#line 1282 "HTMLParser.tab.cc"
    break;

  case 78:
#line 673 "HTMLParser.yy"
          {
    (yylhs.value.element) = (yystack_[0].value.element);
  }
#line 1290 "HTMLParser.tab.cc"
    break;

  case 79:
#line 679 "HTMLParser.yy"
                            {
    (yylhs.value.preformatted) = new Preformatted;
    (yylhs.value.preformatted)->attributes.reset((yystack_[2].value.tag_attributes));
    (yylhs.value.preformatted)->texts.reset((yystack_[1].value.element_list));
  }
#line 1300 "HTMLParser.tab.cc"
    break;

  case 80:
#line 687 "HTMLParser.yy"
                                {
    (yylhs.value.caption) = new Caption;
    (yylhs.value.caption)->attributes.reset((yystack_[2].value.tag_attributes));
    (yylhs.value.caption)->texts.reset((yystack_[1].value.element_list));
  }
#line 1310 "HTMLParser.tab.cc"
    break;

  case 81:
#line 695 "HTMLParser.yy"
              {
    (yylhs.value.table_rows) = new list<auto_ptr<TableRow> >;
  }
#line 1318 "HTMLParser.tab.cc"
    break;

  case 82:
#line 698 "HTMLParser.yy"
                     {
    (yylhs.value.table_rows) = (yystack_[1].value.table_rows);
  }
#line 1326 "HTMLParser.tab.cc"
    break;

  case 83:
#line 701 "HTMLParser.yy"
                                         {
    TableRow *tr = new TableRow;
    tr->attributes.reset((yystack_[2].value.tag_attributes));
    tr->cells.reset((yystack_[1].value.table_cells));
    ((yylhs.value.table_rows) = (yystack_[3].value.table_rows))->push_back(auto_ptr<TableRow>(tr));
  }
#line 1337 "HTMLParser.tab.cc"
    break;

  case 84:
#line 710 "HTMLParser.yy"
              {
    (yylhs.value.table_cells) = new list<auto_ptr<TableCell> >;
  }
#line 1345 "HTMLParser.tab.cc"
    break;

  case 85:
#line 713 "HTMLParser.yy"
                      {
    (yylhs.value.table_cells) = (yystack_[1].value.table_cells);
  }
#line 1353 "HTMLParser.tab.cc"
    break;

  case 86:
#line 716 "HTMLParser.yy"
                                           {
    TableCell *tc = new TableCell;
    tc->attributes.reset((yystack_[2].value.tag_attributes));
    tc->content.reset((yystack_[1].value.element_list));
    ((yylhs.value.table_cells) = (yystack_[3].value.table_cells))->push_back(auto_ptr<TableCell>(tc));
  }
#line 1364 "HTMLParser.tab.cc"
    break;

  case 87:
#line 722 "HTMLParser.yy"
                                                      {
                            /* EXTENSION: Allow "</TD>" in place of "</TH>". */
    TableHeadingCell *thc = new TableHeadingCell;
    thc->attributes.reset((yystack_[3].value.tag_attributes));
    thc->content.reset((yystack_[2].value.element_list));
    ((yylhs.value.table_cells) = (yystack_[4].value.table_cells))->push_back(auto_ptr<TableCell>(thc));
  }
#line 1376 "HTMLParser.tab.cc"
    break;

  case 88:
#line 729 "HTMLParser.yy"
                      {    /* EXTENSION: Ignore <INPUT> between table cells. */
    delete (yystack_[0].value.tag_attributes);
    (yylhs.value.table_cells) = (yystack_[1].value.table_cells);
  }
#line 1385 "HTMLParser.tab.cc"
    break;

  case 89:
#line 736 "HTMLParser.yy"
                                { /* Should be "address_content"... */
    delete (yystack_[2].value.tag_attributes);
    (yylhs.value.address) = new Address;
    (yylhs.value.address)->content.reset((yystack_[1].value.element_list));
  }
#line 1395 "HTMLParser.tab.cc"
    break;

  case 90:
#line 746 "HTMLParser.yy"
       {
    (yylhs.value.element_list) = new list<auto_ptr<Element> >;
    (yylhs.value.element_list)->push_back(auto_ptr<Element>((yystack_[0].value.element)));
  }
#line 1404 "HTMLParser.tab.cc"
    break;

  case 91:
#line 750 "HTMLParser.yy"
               {
    ((yylhs.value.element_list) = (yystack_[1].value.element_list))->push_back(auto_ptr<Element>((yystack_[0].value.element)));
  }
#line 1412 "HTMLParser.tab.cc"
    break;

  case 92:
#line 756 "HTMLParser.yy"
                                      { (yylhs.value.element) = (yystack_[1].value.pcdata); }
#line 1418 "HTMLParser.tab.cc"
    break;

  case 93:
#line 757 "HTMLParser.yy"
                                      { (yylhs.value.element) = (yystack_[1].value.element); }
#line 1424 "HTMLParser.tab.cc"
    break;

  case 94:
#line 758 "HTMLParser.yy"
                                      { (yylhs.value.element) = (yystack_[1].value.element); }
#line 1430 "HTMLParser.tab.cc"
    break;

  case 95:
#line 759 "HTMLParser.yy"
                                      { (yylhs.value.element) = (yystack_[1].value.element); }
#line 1436 "HTMLParser.tab.cc"
    break;

  case 96:
#line 760 "HTMLParser.yy"
                                      { (yylhs.value.element) = (yystack_[1].value.element); }
#line 1442 "HTMLParser.tab.cc"
    break;

  case 97:
#line 761 "HTMLParser.yy"
                                      { /* EXTENSION: NS 1.1 / IE 2.0 */
    NoBreak *nb = new NoBreak;
    delete (yystack_[3].value.tag_attributes);
    nb->content.reset((yystack_[2].value.element_list));
    (yylhs.value.element) = nb;
  }
#line 1453 "HTMLParser.tab.cc"
    break;

  case 98:
#line 770 "HTMLParser.yy"
                                    { delete (yystack_[2].value.tag_attributes); (yylhs.value.element) = new Font(token::TT,     (yystack_[1].value.element_list)); }
#line 1459 "HTMLParser.tab.cc"
    break;

  case 99:
#line 771 "HTMLParser.yy"
                                    { delete (yystack_[2].value.tag_attributes); (yylhs.value.element) = new Font(token::I,      (yystack_[1].value.element_list)); }
#line 1465 "HTMLParser.tab.cc"
    break;

  case 100:
#line 772 "HTMLParser.yy"
                                    { delete (yystack_[2].value.tag_attributes); (yylhs.value.element) = new Font(token::B,      (yystack_[1].value.element_list)); }
#line 1471 "HTMLParser.tab.cc"
    break;

  case 101:
#line 773 "HTMLParser.yy"
                                    { delete (yystack_[2].value.tag_attributes); (yylhs.value.element) = new Font(token::U,      (yystack_[1].value.element_list)); }
#line 1477 "HTMLParser.tab.cc"
    break;

  case 102:
#line 774 "HTMLParser.yy"
                                    { delete (yystack_[2].value.tag_attributes); (yylhs.value.element) = new Font(token::STRIKE, (yystack_[1].value.element_list)); }
#line 1483 "HTMLParser.tab.cc"
    break;

  case 103:
#line 775 "HTMLParser.yy"
                                    { delete (yystack_[2].value.tag_attributes); (yylhs.value.element) = new Font(token::BIG,    (yystack_[1].value.element_list)); }
#line 1489 "HTMLParser.tab.cc"
    break;

  case 104:
#line 776 "HTMLParser.yy"
                                    { delete (yystack_[2].value.tag_attributes); (yylhs.value.element) = new Font(token::SMALL,  (yystack_[1].value.element_list)); }
#line 1495 "HTMLParser.tab.cc"
    break;

  case 105:
#line 777 "HTMLParser.yy"
                                    { delete (yystack_[2].value.tag_attributes); (yylhs.value.element) = new Font(token::SUB,    (yystack_[1].value.element_list)); }
#line 1501 "HTMLParser.tab.cc"
    break;

  case 106:
#line 778 "HTMLParser.yy"
                                    { delete (yystack_[2].value.tag_attributes); (yylhs.value.element) = new Font(token::SUP,    (yystack_[1].value.element_list)); }
#line 1507 "HTMLParser.tab.cc"
    break;

  case 107:
#line 782 "HTMLParser.yy"
                                    { delete (yystack_[2].value.tag_attributes); (yylhs.value.element) = new Phrase(token::EM,     (yystack_[1].value.element_list)); }
#line 1513 "HTMLParser.tab.cc"
    break;

  case 108:
#line 783 "HTMLParser.yy"
                                    { delete (yystack_[2].value.tag_attributes); (yylhs.value.element) = new Phrase(token::STRONG, (yystack_[1].value.element_list)); }
#line 1519 "HTMLParser.tab.cc"
    break;

  case 109:
#line 784 "HTMLParser.yy"
                                    { delete (yystack_[2].value.tag_attributes); (yylhs.value.element) = new Phrase(token::DFN,    (yystack_[1].value.element_list)); }
#line 1525 "HTMLParser.tab.cc"
    break;

  case 110:
#line 785 "HTMLParser.yy"
                                    { delete (yystack_[2].value.tag_attributes); (yylhs.value.element) = new Phrase(token::CODE,   (yystack_[1].value.element_list)); }
#line 1531 "HTMLParser.tab.cc"
    break;

  case 111:
#line 786 "HTMLParser.yy"
                                    { delete (yystack_[2].value.tag_attributes); (yylhs.value.element) = new Phrase(token::SAMP,   (yystack_[1].value.element_list)); }
#line 1537 "HTMLParser.tab.cc"
    break;

  case 112:
#line 787 "HTMLParser.yy"
                                    { delete (yystack_[2].value.tag_attributes); (yylhs.value.element) = new Phrase(token::KBD,    (yystack_[1].value.element_list)); }
#line 1543 "HTMLParser.tab.cc"
    break;

  case 113:
#line 788 "HTMLParser.yy"
                                    { delete (yystack_[2].value.tag_attributes); (yylhs.value.element) = new Phrase(token::VAR,    (yystack_[1].value.element_list)); }
#line 1549 "HTMLParser.tab.cc"
    break;

  case 114:
#line 789 "HTMLParser.yy"
                                    { delete (yystack_[2].value.tag_attributes); (yylhs.value.element) = new Phrase(token::CITE,   (yystack_[1].value.element_list)); }
#line 1555 "HTMLParser.tab.cc"
    break;

  case 115:
#line 796 "HTMLParser.yy"
                              {
    delete (yystack_[2].value.tag_attributes);
    Anchor *a = new Anchor;
    a->attributes.reset((yystack_[3].value.tag_attributes));
    a->texts.reset((yystack_[1].value.element_list));
    a->refnum = 0;
    (yylhs.value.element) = a;

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
#line 1580 "HTMLParser.tab.cc"
    break;

  case 116:
#line 816 "HTMLParser.yy"
        {
	auto_ptr<list<TagAttribute>> attr;
	attr.reset((yystack_[0].value.tag_attributes));
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

		(yylhs.value.element) = a;
	} else {
		Image *i = new Image;
		i->attributes = attr;
		(yylhs.value.element) = i;
	}
  }
#line 1625 "HTMLParser.tab.cc"
    break;

  case 117:
#line 856 "HTMLParser.yy"
                                     {
    Applet *a = new Applet;
    a->attributes.reset((yystack_[2].value.tag_attributes));
    a->content.reset((yystack_[1].value.element_list));
    (yylhs.value.element) = a;
  }
#line 1636 "HTMLParser.tab.cc"
    break;

  case 118:
#line 864 "HTMLParser.yy"
                               {
    Font2 *f2 = new Font2;
    f2->attributes.reset((yystack_[2].value.tag_attributes));
    f2->elements.reset((yystack_[1].value.element_list));
    (yylhs.value.element) = f2;
  }
#line 1647 "HTMLParser.tab.cc"
    break;

  case 119:
#line 870 "HTMLParser.yy"
             {
    BaseFont *bf = new BaseFont;
    bf->attributes.reset((yystack_[0].value.tag_attributes));
    (yylhs.value.element) = bf;
  }
#line 1657 "HTMLParser.tab.cc"
    break;

  case 120:
#line 875 "HTMLParser.yy"
       {
    LineBreak *lb = new LineBreak;
    lb->attributes.reset((yystack_[0].value.tag_attributes));
    (yylhs.value.element) = lb;
  }
#line 1667 "HTMLParser.tab.cc"
    break;

  case 121:
#line 880 "HTMLParser.yy"
                            {
    Map *m = new Map;
    m->attributes.reset((yystack_[2].value.tag_attributes));
    m->areas.reset((yystack_[1].value.tag_attributes_list));
    (yylhs.value.element) = m;
  }
#line 1678 "HTMLParser.tab.cc"
    break;

  case 122:
#line 889 "HTMLParser.yy"
              {
    (yylhs.value.element_list) = 0;
  }
#line 1686 "HTMLParser.tab.cc"
    break;

  case 123:
#line 892 "HTMLParser.yy"
                        {
    (yylhs.value.element_list) = (yystack_[1].value.element_list) ? (yystack_[1].value.element_list) : new list<auto_ptr<Element> >;
    (yylhs.value.element_list)->push_back(auto_ptr<Element>((yystack_[0].value.element)));
  }
#line 1695 "HTMLParser.tab.cc"
    break;

  case 124:
#line 896 "HTMLParser.yy"
                         {
    (yylhs.value.element_list) = (yystack_[1].value.element_list) ? (yystack_[1].value.element_list) : new list<auto_ptr<Element> >;
    Param *p = new Param;
    p->attributes.reset((yystack_[0].value.tag_attributes));
    (yylhs.value.element_list)->push_back(auto_ptr<Element>(p));
  }
#line 1706 "HTMLParser.tab.cc"
    break;

  case 125:
#line 905 "HTMLParser.yy"
              {
    (yylhs.value.tag_attributes_list) = 0;
  }
#line 1714 "HTMLParser.tab.cc"
    break;

  case 126:
#line 908 "HTMLParser.yy"
                      {
    (yylhs.value.tag_attributes_list) = (yystack_[1].value.tag_attributes_list);
  }
#line 1722 "HTMLParser.tab.cc"
    break;

  case 127:
#line 911 "HTMLParser.yy"
                     {
    (yylhs.value.tag_attributes_list) = (yystack_[1].value.tag_attributes_list) ? (yystack_[1].value.tag_attributes_list) : new list<auto_ptr<list<TagAttribute> > >;
    (yylhs.value.tag_attributes_list)->push_back(auto_ptr<list<TagAttribute> >((yystack_[0].value.tag_attributes)));
  }
#line 1731 "HTMLParser.tab.cc"
    break;

  case 128:
#line 918 "HTMLParser.yy"
        {
    Input *i = new Input;
    i->attributes.reset((yystack_[0].value.tag_attributes));
    (yylhs.value.element) = i;
  }
#line 1741 "HTMLParser.tab.cc"
    break;

  case 129:
#line 923 "HTMLParser.yy"
                                     {
    Select *s = new Select;
    s->attributes.reset((yystack_[2].value.tag_attributes));
    s->content.reset((yystack_[1].value.option_list));
    (yylhs.value.element) = s;
  }
#line 1752 "HTMLParser.tab.cc"
    break;

  case 130:
#line 929 "HTMLParser.yy"
                                 {
    TextArea *ta = new TextArea;
    ta->attributes.reset((yystack_[2].value.tag_attributes));
    ta->pcdata.reset((yystack_[1].value.pcdata));
    (yylhs.value.element) = ta;
  }
#line 1763 "HTMLParser.tab.cc"
    break;

  case 131:
#line 938 "HTMLParser.yy"
         {
    (yylhs.value.option_list) = new list<auto_ptr<Option> >;
    (yylhs.value.option_list)->push_back(auto_ptr<Option>((yystack_[0].value.option)));
  }
#line 1772 "HTMLParser.tab.cc"
    break;

  case 132:
#line 942 "HTMLParser.yy"
                          {
    ((yylhs.value.option_list) = (yystack_[1].value.option_list))->push_back(auto_ptr<Option>((yystack_[0].value.option)));
  }
#line 1780 "HTMLParser.tab.cc"
    break;

  case 133:
#line 948 "HTMLParser.yy"
                               {
    (yylhs.value.option) = new Option;
    (yylhs.value.option)->attributes.reset((yystack_[2].value.tag_attributes));
    (yylhs.value.option)->pcdata.reset((yystack_[1].value.pcdata));
  }
#line 1790 "HTMLParser.tab.cc"
    break;

  case 134:
#line 958 "HTMLParser.yy"
       { (yylhs.value.heading) = new Heading; (yylhs.value.heading)->level = 1; (yylhs.value.heading)->attributes.reset((yystack_[0].value.tag_attributes)); }
#line 1796 "HTMLParser.tab.cc"
    break;

  case 135:
#line 959 "HTMLParser.yy"
       { (yylhs.value.heading) = new Heading; (yylhs.value.heading)->level = 2; (yylhs.value.heading)->attributes.reset((yystack_[0].value.tag_attributes)); }
#line 1802 "HTMLParser.tab.cc"
    break;

  case 136:
#line 960 "HTMLParser.yy"
       { (yylhs.value.heading) = new Heading; (yylhs.value.heading)->level = 3; (yylhs.value.heading)->attributes.reset((yystack_[0].value.tag_attributes)); }
#line 1808 "HTMLParser.tab.cc"
    break;

  case 137:
#line 961 "HTMLParser.yy"
       { (yylhs.value.heading) = new Heading; (yylhs.value.heading)->level = 4; (yylhs.value.heading)->attributes.reset((yystack_[0].value.tag_attributes)); }
#line 1814 "HTMLParser.tab.cc"
    break;

  case 138:
#line 962 "HTMLParser.yy"
       { (yylhs.value.heading) = new Heading; (yylhs.value.heading)->level = 5; (yylhs.value.heading)->attributes.reset((yystack_[0].value.tag_attributes)); }
#line 1820 "HTMLParser.tab.cc"
    break;

  case 139:
#line 963 "HTMLParser.yy"
       { (yylhs.value.heading) = new Heading; (yylhs.value.heading)->level = 6; (yylhs.value.heading)->attributes.reset((yystack_[0].value.tag_attributes)); }
#line 1826 "HTMLParser.tab.cc"
    break;

  case 140:
#line 967 "HTMLParser.yy"
           { (yylhs.value.inT) = 1; }
#line 1832 "HTMLParser.tab.cc"
    break;

  case 141:
#line 968 "HTMLParser.yy"
           { (yylhs.value.inT) = 2; }
#line 1838 "HTMLParser.tab.cc"
    break;

  case 142:
#line 969 "HTMLParser.yy"
           { (yylhs.value.inT) = 3; }
#line 1844 "HTMLParser.tab.cc"
    break;

  case 143:
#line 970 "HTMLParser.yy"
           { (yylhs.value.inT) = 4; }
#line 1850 "HTMLParser.tab.cc"
    break;

  case 144:
#line 971 "HTMLParser.yy"
           { (yylhs.value.inT) = 5; }
#line 1856 "HTMLParser.tab.cc"
    break;

  case 145:
#line 972 "HTMLParser.yy"
           { (yylhs.value.inT) = 6; }
#line 1862 "HTMLParser.tab.cc"
    break;

  case 146:
#line 977 "HTMLParser.yy"
                            { (yylhs.value.pcdata) = 0; }
#line 1868 "HTMLParser.tab.cc"
    break;

  case 147:
#line 977 "HTMLParser.yy"
                                                  { (yylhs.value.pcdata) = (yystack_[0].value.pcdata); }
#line 1874 "HTMLParser.tab.cc"
    break;

  case 148:
#line 978 "HTMLParser.yy"
                            { (yylhs.value.caption) = 0; }
#line 1880 "HTMLParser.tab.cc"
    break;

  case 149:
#line 978 "HTMLParser.yy"
                                                  { (yylhs.value.caption) = (yystack_[0].value.caption); }
#line 1886 "HTMLParser.tab.cc"
    break;

  case 150:
#line 979 "HTMLParser.yy"
                            { (yylhs.value.element_list) = 0; }
#line 1892 "HTMLParser.tab.cc"
    break;

  case 151:
#line 979 "HTMLParser.yy"
                                                  { (yylhs.value.element_list) = (yystack_[0].value.element_list); }
#line 1898 "HTMLParser.tab.cc"
    break;

  case 152:
#line 980 "HTMLParser.yy"
                            { (yylhs.value.element_list) = 0; }
#line 1904 "HTMLParser.tab.cc"
    break;

  case 153:
#line 980 "HTMLParser.yy"
                                                  { (yylhs.value.element_list) = (yystack_[0].value.element_list); }
#line 1910 "HTMLParser.tab.cc"
    break;

  case 154:
#line 982 "HTMLParser.yy"
                            { (yylhs.value.tag_attributes) = 0; }
#line 1916 "HTMLParser.tab.cc"
    break;

  case 155:
#line 982 "HTMLParser.yy"
                                                  { (yylhs.value.tag_attributes) = (yystack_[0].value.tag_attributes); }
#line 1922 "HTMLParser.tab.cc"
    break;

  case 156:
#line 983 "HTMLParser.yy"
                            { (yylhs.value.tag_attributes) = 0; }
#line 1928 "HTMLParser.tab.cc"
    break;

  case 157:
#line 983 "HTMLParser.yy"
                                                  { (yylhs.value.tag_attributes) = (yystack_[0].value.tag_attributes); }
#line 1934 "HTMLParser.tab.cc"
    break;


#line 1938 "HTMLParser.tab.cc"

            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        context yyctx (*this, yyla);
        std::string msg = yysyntax_error_ (yyctx);
        error (YY_MOVE (msg));
      }


    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.kind () == symbol_kind::S_YYEOF)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    // Pop stack until we find a state that shifts the error token.
    for (;;)
      {
        yyn = yypact_[+yystack_[0].state];
        if (!yy_pact_value_is_default_ (yyn))
          {
            yyn += symbol_kind::S_YYerror;
            if (0 <= yyn && yyn <= yylast_
                && yycheck_[yyn] == symbol_kind::S_YYerror)
              {
                yyn = yytable_[yyn];
                if (0 < yyn)
                  break;
              }
          }

        // Pop the current state because it cannot handle the error token.
        if (yystack_.size () == 1)
          YYABORT;

        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;


      // Shift the error token.
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    YY_STACK_PRINT ();
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  HTMLParser::error (const syntax_error& yyexc)
  {
    error (yyexc.what ());
  }

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  HTMLParser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr;
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              else
                goto append;

            append:
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }

  std::string
  HTMLParser::symbol_name (symbol_kind_type yysymbol)
  {
    return yytnamerr_ (yytname_[yysymbol]);
  }



  // HTMLParser::context.
  HTMLParser::context::context (const HTMLParser& yyparser, const symbol_type& yyla)
    : yyparser_ (yyparser)
    , yyla_ (yyla)
  {}

  int
  HTMLParser::context::expected_tokens (symbol_kind_type yyarg[], int yyargn) const
  {
    // Actual number of expected tokens
    int yycount = 0;

    int yyn = yypact_[+yyparser_.yystack_[0].state];
    if (!yy_pact_value_is_default_ (yyn))
      {
        /* Start YYX at -YYN if negative to avoid negative indexes in
           YYCHECK.  In other words, skip the first -YYN actions for
           this state because they are default actions.  */
        int yyxbegin = yyn < 0 ? -yyn : 0;
        // Stay within bounds of both yycheck and yytname.
        int yychecklim = yylast_ - yyn + 1;
        int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
        for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
          if (yycheck_[yyx + yyn] == yyx && yyx != symbol_kind::S_YYerror
              && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
            {
              if (!yyarg)
                ++yycount;
              else if (yycount == yyargn)
                return 0;
              else
                yyarg[yycount++] = YY_CAST (symbol_kind_type, yyx);
            }
      }

    if (yyarg && yycount == 0 && 0 < yyargn)
      yyarg[0] = symbol_kind::S_YYEMPTY;
    return yycount;
  }



  int
  HTMLParser::yy_syntax_error_arguments_ (const context& yyctx,
                                                 symbol_kind_type yyarg[], int yyargn) const
  {
    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state merging
         (from LALR or IELR) and default reductions corrupt the expected
         token list.  However, the list is correct for canonical LR with
         one exception: it will still contain any token that will not be
         accepted due to an error action in a later state.
    */

    if (!yyctx.lookahead ().empty ())
      {
        if (yyarg)
          yyarg[0] = yyctx.token ();
        int yyn = yyctx.expected_tokens (yyarg ? yyarg + 1 : yyarg, yyargn - 1);
        return yyn + 1;
      }
    return 0;
  }

  // Generate an error message.
  std::string
  HTMLParser::yysyntax_error_ (const context& yyctx) const
  {
    // Its maximum.
    enum { YYARGS_MAX = 5 };
    // Arguments of yyformat.
    symbol_kind_type yyarg[YYARGS_MAX];
    int yycount = yy_syntax_error_arguments_ (yyctx, yyarg, YYARGS_MAX);

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    std::ptrdiff_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += symbol_name (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const short HTMLParser::yypact_ninf_ = -171;

  const short HTMLParser::yytable_ninf_ = -227;

  const short
  HTMLParser::yypact_[] =
  {
    -171,    24,  1748,  -171,  -171,  -171,  -171,   -26,  2156,  -171,
    2156,  -171,  -171,  2156,  -171,  -171,  -171,  -171,  2156,  2156,
    2156,  -171,  -171,  2086,  2156,  2086,  -171,  -171,  -171,  -171,
    -171,  -171,  -171,  -171,  -171,  -171,  2156,  -171,  -171,  -171,
    2156,  -171,  -171,  -171,  -171,  2156,  -171,  -171,  2156,  2156,
    -171,   -29,  2156,  2156,  2156,  -171,  2156,  2156,    13,    27,
      27,  2156,  2156,  -171,  2156,  -171,  -171,  -171,   287,  -171,
    -171,  -171,  -171,  -171,  -171,  -171,  2156,  -171,   287,   287,
     287,   287,  -171,  -171,  2086,  2156,   -41,  2015,   -40,   -43,
     418,   549,   -45,   -44,   -31,  -171,   680,  -171,  -171,   811,
    -171,  -171,    25,   -49,   -48,   942,   -55,   -47,    18,  -171,
     -33,  -171,  1073,   -53,   -50,    27,   -42,  -171,   -52,   -51,
     -38,   -36,   -56,  2156,  -171,  -171,   -57,  -171,   -35,   -46,
     -28,  -171,   -37,  -171,  -171,  -171,  -171,  -171,  -171,  -171,
    1846,     3,  -171,  -171,  -171,  -171,  -171,  -171,  -171,  -171,
    -171,  -171,  -171,  -171,  -171,  -171,  -171,  -171,  2156,  -171,
    -171,  -171,  -171,  -171,  -171,  -171,  -171,  -171,  1945,  -171,
    -171,  -171,  -171,    50,  -171,  -171,  -171,  -171,  -171,  -171,
    -171,  -171,  -171,  -171,  -171,  -171,  -171,  1668,   287,  1595,
    -171,  -171,  -171,  2156,  -171,  -171,  -171,  -171,  -171,   -25,
    -171,  -171,  -171,  -171,  -171,  -171,  -171,  -171,  -171,  -171,
    -171,  -171,    -3,    16,  -171,  -171,  -171,  -171,  -171,  -171,
    -171,  1204,  -171,  -171,  -171,  -171,  -171,  -171,  -171,  -171,
    -171,  -171,  -171,  -171,  2086,  -171,  -171,  -171,  2156,    -9,
       7,  -171,  -171,  -171,  -171,  -171,  -171,  -171,  -171,  -171,
    -171,  -171,  -171,  -171,   -18,  -171,  -171,  2086,  2086,  -171,
    -171,  -171,     4,  -171,  -171,    31,    22,  -171,  -171,  -171,
    -171,  -171,  -171,    35,  -171,    35,  -171,  1462,  1335,  -171,
      28,    28,  -171,  -171,  -171,   -27,  -171,  -171,  -171
  };

  const unsigned char
  HTMLParser::yydefact_[] =
  {
       3,     0,     0,     1,     4,     5,    23,   154,   150,   122,
     150,    12,   119,   150,    24,    17,   120,    24,   150,   150,
     150,    53,    24,   152,   150,   152,    24,   134,   135,   136,
     137,   138,   139,     8,    47,     6,   150,   116,   128,    11,
     150,    14,   125,    55,    13,   150,    49,    36,   150,   150,
      15,     0,   150,   150,   150,    16,   150,   150,   148,     0,
     146,   150,   150,    51,   150,    18,     9,     7,     0,    20,
      21,    34,    40,    42,    41,    22,    19,    90,     0,     0,
       0,     0,    36,   155,   152,   151,     0,     0,   160,   164,
       0,     0,   168,   170,   172,    57,     0,    77,    78,     0,
      73,    76,     0,   178,   180,     0,   184,   186,     0,    57,
       0,    57,     0,   194,   196,     0,     0,   131,   198,   200,
     202,   204,   206,   150,   149,    81,     0,   147,   214,   218,
     220,    57,   224,   227,    92,    91,    93,    94,    95,    96,
       0,   158,    89,   124,   117,   123,   161,   100,   165,   103,
      25,    28,    26,    27,   163,    30,    31,    32,    29,    45,
     167,    44,   169,   114,   171,   110,   173,   109,     0,   175,
      43,    74,    75,    65,   179,   107,   181,   118,   183,    46,
     185,    99,   187,   112,   126,   127,   121,     0,     0,     0,
      37,   193,    39,    38,    35,   195,    79,   197,   111,   190,
     129,   132,   199,   104,   201,   102,   203,   108,   205,   105,
     207,   106,     0,     0,   130,   215,    10,   219,    98,   221,
     101,     0,   225,   113,   140,   141,   142,   143,   144,   145,
      33,   159,   115,    58,   152,    54,    61,    59,    62,   176,
       0,    56,    97,    50,   191,   133,    80,   208,    82,    84,
     209,    48,   223,    52,   188,   177,    63,   152,   152,    64,
      67,    68,     0,   189,    60,     0,     0,    85,    88,    24,
      24,   217,    83,   156,    71,   156,    69,     0,     0,   157,
       0,     0,   211,    86,   213,   210,    72,    70,    87
  };

  const short
  HTMLParser::yypgoto_[] =
  {
    -171,  -171,  -171,   -39,   -15,     8,     1,    19,   -87,  -171,
    -171,  -171,  -171,  -171,   -70,  -171,   -71,  -171,  -171,  -171,
    -171,     6,  -171,  -171,  -171,  -171,   105,    -2,   -17,  -171,
    -171,  -171,  -171,  -171,  -171,  -171,    -8,  -171,  -171,  -171,
    -171,  1533,   -24,  -171,  -166,  -171,  -171,  -171,  -171,  -171,
    -171,  -171,  -171,  -171,  -171,  -171,  -171,  -171,  -171,  -171,
    -171,  -171,  -171,  -171,  -171,  -171,  -171,  -171,  -171,  -171,
    -171,  -170,  -171,  -171,  -171,  -171,  -171,  -171,  -171,   -66
  };

  const short
  HTMLParser::yydefgoto_[] =
  {
      -1,     1,     2,    68,    90,    97,    98,   112,    71,    72,
     111,   131,    95,   109,   168,   237,    73,   240,   260,   261,
      99,   100,    74,   124,   213,   262,   157,    85,    77,    78,
      79,    80,    87,   108,    81,   116,   117,    82,   230,   128,
     125,    86,   102,    84,   280,   232,   147,   159,   149,   161,
     163,   165,   167,   170,   256,   175,   177,   179,   181,   183,
     264,   245,   194,   196,   198,   203,   205,   207,   209,   211,
     251,   283,   285,   216,   272,   218,   220,   253,   223,   134
  };

  const short
  HTMLParser::yytable_[] =
  {
      76,   104,    91,    70,  -216,   267,   101,    96,   101,   115,
      69,   105,   136,   137,   138,   139,   247,   248,    83,   184,
     126,   127,   115,   133,     3,   192,   133,   185,   257,   133,
     123,     6,   133,   258,   142,   148,   173,   146,   162,   187,
     164,   189,   174,  -226,   176,   268,  -226,   180,  -226,  -226,
    -226,  -226,  -226,   192,  -226,   166,   182,  -226,   195,   135,
     141,   221,   197,   202,   210,   204,   214,   101,   135,   269,
     145,   270,   200,  -216,   188,    23,   199,   231,   246,   206,
     255,   217,   101,   208,   244,   249,   263,   279,   158,   158,
     215,   156,   156,   222,   158,   282,   259,   156,   155,   155,
     219,   140,   239,   158,   155,   172,   156,    75,   201,   281,
     193,  -226,   275,   155,  -226,   288,   273,  -226,     0,     0,
    -226,     0,   242,   186,     0,  -216,     0,     0,     0,     0,
     271,     0,     0,     0,     0,     0,     0,   250,   193,     0,
       0,   135,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   238,     0,     0,   236,
       0,     0,     0,     0,     0,     0,   135,     0,     0,     0,
       0,     0,     0,     0,     0,   238,     0,   238,   236,     0,
     236,     0,     0,     0,     0,     0,     0,     0,     0,   274,
     276,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     254,     0,     0,     0,   286,   287,     0,   101,     0,   238,
       0,   135,   236,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   265,   266,     0,     0,     0,     0,     0,
     101,   101,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   277,   278,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   158,   158,     0,   156,   156,
       0,     0,     0,     0,     0,   155,   155,  -226,   133,     0,
    -226,  -226,     0,  -226,  -226,  -226,     0,  -226,  -226,  -226,
    -226,  -226,  -226,  -226,     0,  -226,  -226,  -226,  -226,  -226,
    -226,  -226,  -226,  -226,  -226,  -226,  -226,  -226,  -226,  -226,
    -226,  -226,  -226,  -226,  -226,  -226,  -226,  -226,  -226,  -226,
    -226,  -226,  -226,  -226,  -226,  -226,  -226,  -226,     0,  -226,
    -226,  -226,  -226,  -226,  -226,  -226,  -226,  -226,  -226,  -226,
    -226,  -226,  -226,  -226,  -226,  -226,  -226,  -226,  -226,  -226,
    -226,  -226,  -226,  -226,  -226,  -226,  -226,  -226,  -226,  -226,
    -226,  -226,  -226,  -226,  -226,  -226,  -226,  -226,  -226,  -226,
    -226,  -226,  -226,  -226,  -226,  -226,  -226,  -226,  -226,  -226,
    -226,  -226,     0,  -226,  -226,  -226,     0,  -226,  -226,  -226,
       0,     0,  -226,  -226,  -226,     0,  -226,  -226,  -226,  -226,
       0,  -226,     0,  -226,  -226,  -226,  -226,  -226,  -162,   150,
       0,  -162,     6,     0,     7,     8,     9,     0,    10,  -162,
      12,    13,    14,  -162,    16,     0,    17,    18,    19,  -162,
      20,    21,    22,    23,  -162,    24,    25,    26,    27,    28,
      29,    30,    31,    32,  -162,    34,  -162,    36,    37,    38,
    -162,    40,  -162,  -162,    42,    43,   151,    45,    46,     0,
      47,  -162,    48,    49,   152,    51,    52,    53,    54,   153,
      56,    57,    58,  -162,    59,  -162,  -162,  -162,    61,    62,
      63,    64,  -162,  -162,  -162,  -162,  -162,   154,  -162,  -162,
    -162,  -162,  -162,  -162,  -162,  -162,  -162,  -162,  -162,  -162,
    -162,  -162,  -162,  -162,  -162,  -162,  -162,  -162,  -162,  -162,
    -162,  -162,  -162,     0,  -162,  -162,  -162,     0,  -162,  -162,
    -162,     0,     0,  -162,  -162,  -162,     0,  -162,  -162,  -162,
    -162,     0,  -162,     0,  -162,  -162,  -162,  -162,  -162,  -166,
     150,     0,  -166,     6,     0,     7,     8,     9,     0,    10,
    -166,    12,    13,    14,  -166,    16,     0,    17,    18,    19,
    -166,    20,    21,    22,    23,  -166,    24,    25,    26,    27,
      28,    29,    30,    31,    32,  -166,    34,  -166,    36,    37,
      38,  -166,    40,  -166,  -166,    42,    43,   151,    45,    46,
       0,    47,  -166,    48,    49,   152,    51,    52,    53,    54,
     153,    56,    57,    58,  -166,    59,  -166,  -166,  -166,    61,
      62,    63,    64,  -166,  -166,  -166,  -166,  -166,  -166,  -166,
    -166,   160,  -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,
    -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,
    -166,  -166,  -166,  -166,     0,  -166,  -166,  -166,     0,  -166,
    -166,  -166,     0,     0,  -166,  -166,  -166,     0,  -166,  -166,
    -166,  -166,     0,  -166,     0,  -166,  -166,  -166,  -166,  -166,
    -174,   150,     0,  -174,     6,     0,     7,     8,     9,     0,
      10,  -174,    12,    13,    14,  -174,    16,     0,    17,    18,
      19,  -174,    20,    21,    22,    23,  -174,    24,    25,    26,
      27,    28,    29,    30,    31,    32,  -174,    34,  -174,    36,
      37,    38,  -174,    40,  -174,  -174,    42,    43,   151,    45,
      46,     0,    47,  -174,    48,    49,   152,    51,    52,    53,
      54,   153,    56,    57,    58,  -174,    59,  -174,  -174,  -174,
      61,    62,    63,    64,  -174,  -174,  -174,  -174,  -174,  -174,
    -174,  -174,  -174,  -174,  -174,  -174,  -174,  -174,   169,  -174,
    -174,  -174,  -174,  -174,  -174,  -174,  -174,  -174,  -174,  -174,
    -174,  -174,  -174,  -174,  -174,     0,  -174,  -174,  -174,     0,
    -174,  -174,  -174,     0,     0,  -174,  -174,  -174,     0,  -174,
    -174,  -174,  -174,     0,  -174,     0,  -174,  -174,  -174,  -174,
    -174,  -153,   171,     0,  -153,     6,     0,     7,  -153,     9,
       0,    10,  -153,    12,    13,    14,  -153,    16,     0,    17,
      18,    19,  -153,    20,    21,    22,    23,  -153,    24,    25,
      26,    27,    28,    29,    30,    31,    32,  -153,    34,  -153,
      36,    37,    38,  -153,    40,  -153,  -153,    42,    43,  -153,
      45,    46,     0,    47,  -153,    48,    49,  -153,    51,    52,
      53,    54,  -153,    56,    57,    58,  -153,    59,  -153,  -153,
    -153,    61,    62,    63,    64,  -153,  -153,  -153,  -153,  -153,
    -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,
    -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,
    -153,  -153,  -153,  -153,  -153,  -153,     0,  -153,  -153,  -153,
       0,  -153,  -153,  -153,     0,     0,  -153,  -153,  -153,     0,
    -153,  -153,  -153,  -153,     0,  -153,     0,  -153,  -153,  -153,
    -153,  -153,  -182,   150,     0,  -182,     6,     0,     7,     8,
       9,     0,    10,  -182,    12,    13,    14,  -182,    16,     0,
      17,    18,    19,  -182,    20,    21,    22,    23,  -182,    24,
      25,    26,    27,    28,    29,    30,    31,    32,  -182,    34,
    -182,    36,    37,    38,  -182,    40,  -182,  -182,    42,    43,
     151,    45,    46,     0,    47,  -182,    48,    49,   152,    51,
      52,    53,    54,   153,    56,    57,    58,  -182,    59,  -182,
    -182,  -182,    61,    62,    63,    64,  -182,  -182,  -182,  -182,
    -182,  -182,  -182,  -182,  -182,  -182,  -182,  -182,  -182,  -182,
    -182,  -182,  -182,  -182,  -182,   178,  -182,  -182,  -182,  -182,
    -182,  -182,  -182,  -182,  -182,  -182,  -182,     0,  -182,  -182,
    -182,     0,  -182,  -182,  -182,     0,     0,  -182,  -182,  -182,
       0,  -182,  -182,  -182,  -182,     0,  -182,     0,  -182,  -182,
    -182,  -182,  -182,  -192,   190,     0,  -192,     6,     0,     7,
    -192,     9,     0,    10,  -192,    12,    13,    14,  -192,    16,
       0,    17,    18,    19,  -192,    20,    21,    22,    23,  -192,
      24,    25,    26,  -192,  -192,  -192,  -192,  -192,  -192,  -192,
      34,  -192,    36,    37,    38,  -192,    40,  -192,  -192,    42,
      43,  -192,    45,    46,     0,  -192,  -192,    48,    49,  -192,
      51,    52,    53,    54,  -192,    56,    57,    58,  -192,    59,
    -192,  -192,  -192,    61,    62,    63,    64,  -192,  -192,  -192,
    -192,  -192,  -192,  -192,  -192,  -192,  -192,  -192,  -192,  -192,
    -192,  -192,  -192,  -192,  -192,  -192,  -192,  -192,  -192,  -192,
    -192,  -192,  -192,  -192,  -192,  -192,  -192,  -192,     0,  -192,
    -192,  -192,     0,   191,  -192,  -192,     0,     0,  -192,  -192,
    -192,     0,  -192,  -192,  -192,  -192,     0,  -192,     0,  -192,
    -192,  -192,  -192,  -192,  -222,   233,     0,  -222,     6,     0,
       7,  -222,     9,     0,    10,  -222,    12,    13,    14,  -222,
      16,     0,    17,    18,    19,  -222,    20,    21,    22,    23,
    -222,    24,    25,    26,  -222,  -222,  -222,  -222,  -222,  -222,
    -222,    34,  -222,    36,    37,    38,  -222,    40,   234,  -222,
      42,    43,  -222,    45,    46,     0,    47,  -222,    48,    49,
    -222,    51,    52,    53,    54,  -222,    56,    57,    58,  -222,
      59,  -222,  -222,  -222,    61,    62,    63,    64,  -222,  -222,
    -222,  -222,  -222,  -222,  -222,  -222,  -222,  -222,  -222,  -222,
    -222,  -222,  -222,  -222,  -222,  -222,  -222,  -222,  -222,  -222,
    -222,  -222,  -222,  -222,  -222,  -222,  -222,  -222,  -222,     0,
    -222,  -222,  -222,     0,  -222,  -222,  -222,     0,     0,  -222,
    -222,  -222,     0,  -222,  -222,  -222,  -222,     0,  -222,     0,
    -222,  -222,  -222,   252,  -222,  -212,   150,     0,     0,     6,
       0,     7,     8,     9,     0,    10,     0,    12,    13,    14,
       0,    16,     0,    17,    18,    19,     0,    20,    21,    22,
      23,     0,    24,    25,    26,    27,    28,    29,    30,    31,
      32,     0,    34,     0,    36,    37,    38,     0,    40,     0,
       0,    42,    43,   151,    45,    46,     0,    47,     0,    48,
      49,   152,    51,    52,    53,    54,   153,    56,    57,    58,
    -212,    59,  -212,     0,  -212,    61,    62,    63,    64,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  -212,  -212,     0,   284,
       0,  -212,  -210,   150,     0,     0,     6,     0,     7,     8,
       9,     0,    10,     0,    12,    13,    14,     0,    16,     0,
      17,    18,    19,     0,    20,    21,    22,    23,     0,    24,
      25,    26,    27,    28,    29,    30,    31,    32,     0,    34,
       0,    36,    37,    38,     0,    40,     0,     0,    42,    43,
     151,    45,    46,     0,    47,     0,    48,    49,   152,    51,
      52,    53,    54,   153,    56,    57,    58,  -210,    59,  -210,
       0,  -210,    61,    62,    63,    64,     0,     0,     0,     0,
       0,     0,     0,    88,     0,     0,    89,     0,     0,     0,
       0,    92,    93,    94,     0,     0,     0,   103,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   106,
       0,     0,     0,   107,     0,     0,     0,     0,   110,     0,
       0,   113,   114,  -210,   282,   118,   119,   120,  -210,   121,
     122,     0,     0,     0,   129,   130,   233,   132,     0,     6,
       0,     7,     0,     9,     0,    10,     0,    12,    13,    14,
       0,    16,     0,    17,    18,    19,     0,    20,    21,    22,
      23,     0,    24,    25,    26,     0,     0,     0,     0,     0,
       0,     0,    34,     0,    36,    37,    38,     0,    40,   234,
       0,    42,    43,     0,    45,    46,     0,    47,     0,    48,
      49,     0,    51,    52,    53,    54,   212,    56,    57,    58,
       0,    59,     0,     0,     0,    61,    62,    63,    64,   233,
       0,     0,     6,     0,     7,     0,     9,     0,    10,     0,
      12,    13,    14,     0,    16,     0,    17,    18,    19,     0,
      20,    21,    22,    23,     0,    24,    25,    26,     0,     0,
       0,     0,     0,   243,     0,    34,     0,    36,    37,    38,
       0,    40,   234,     0,    42,    43,     0,    45,    46,     0,
      47,     0,    48,    49,     0,    51,    52,    53,    54,     0,
      56,    57,    58,     0,    59,     0,     0,     0,    61,    62,
      63,    64,     0,     0,     0,     0,     0,     0,    -2,     4,
       0,     5,     6,     0,     7,     8,     9,     0,    10,    11,
      12,    13,    14,    15,    16,     0,    17,    18,    19,     0,
      20,    21,    22,    23,   241,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,    43,    44,    45,    46,     0,
      47,     0,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,     0,    59,     0,    60,     0,    61,    62,
      63,    64,     0,     0,     0,     0,     0,     0,    65,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   190,    66,    67,
       6,     0,     7,     0,     9,     0,    10,     0,    12,    13,
      14,     0,    16,     0,    17,    18,    19,     0,    20,    21,
      22,    23,     0,    24,    25,    26,     0,     0,     0,     0,
       0,     0,     0,    34,     0,    36,    37,    38,     0,    40,
       0,     0,    42,    43,     0,    45,    46,     0,     0,     0,
      48,    49,     0,    51,    52,    53,    54,     0,    56,    57,
      58,     0,    59,     0,     0,     0,    61,    62,    63,    64,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     224,   225,   226,   227,   228,   229,   233,     0,     0,     6,
       0,     7,     0,     9,     0,    10,     0,    12,    13,    14,
       0,    16,     0,    17,    18,    19,     0,    20,    21,    22,
      23,     0,    24,    25,    26,     0,     0,     0,     0,     0,
       0,     0,    34,     0,    36,    37,    38,     0,    40,   234,
       0,    42,    43,     0,    45,    46,     0,    47,     0,    48,
      49,     0,    51,    52,    53,    54,     0,    56,    57,    58,
       0,    59,     0,     0,     0,    61,    62,    63,    64,     6,
       0,     7,     0,     9,     0,    10,     0,    12,    13,     0,
       0,    16,   235,     0,    18,    19,     0,    20,     0,     0,
       0,     0,    24,    25,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    36,    37,    38,     0,    40,     0,
       0,    42,     0,     0,    45,     0,     0,     0,   143,     0,
      49,     0,    51,    52,    53,    54,     0,    56,    57,     0,
       0,    59,     0,     0,     0,    61,    62,     0,    64,     0,
       6,   144,     7,     0,     9,     0,    10,     0,    12,    13,
      14,     0,    16,     0,    17,    18,    19,     0,    20,    21,
      22,    23,     0,    24,    25,    26,    27,    28,    29,    30,
      31,    32,     0,    34,     0,    36,    37,    38,     0,    40,
       0,     0,    42,    43,     0,    45,    46,     0,    47,     0,
      48,    49,     0,    51,    52,    53,    54,     0,    56,    57,
      58,     0,    59,     0,     0,     0,    61,    62,    63,    64,
       6,     0,     7,     0,     9,     0,    10,     0,    12,    13,
       0,     0,    16,     0,     0,    18,    19,     0,    20,     0,
       0,     0,     0,    24,    25,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    36,    37,    38,     0,    40,
       0,     0,    42,     0,     0,    45,     0,     0,     0,     0,
       0,    49,     0,    51,    52,    53,    54,     0,    56,    57,
       0,     0,    59,     0,     0,     0,    61,    62,     0,    64
  };

  const short
  HTMLParser::yycheck_[] =
  {
       2,    25,    17,     2,     0,     1,    23,    22,    25,    51,
       2,    26,    78,    79,    80,    81,     0,     1,    44,     1,
      59,    60,    51,     1,     0,   112,     1,     9,    21,     1,
      17,     4,     1,    26,    75,    78,   102,    77,    83,   109,
      84,   111,    91,    21,    92,    41,    21,   102,    26,    21,
      25,    26,    21,   140,    26,    86,   103,    26,   111,    76,
      84,   131,   112,   115,   120,   116,   123,    84,    85,    65,
      87,    67,   114,    69,   107,    25,   115,    74,    81,   117,
      89,   127,    99,   119,   109,    69,   104,    52,    90,    91,
     125,    90,    91,   130,    96,   122,    89,    96,    90,    91,
     128,    82,   173,   105,    96,    99,   105,     2,   116,   275,
     112,    89,    90,   105,    89,   285,    85,    89,    -1,    -1,
      89,    -1,   188,   105,    -1,   121,    -1,    -1,    -1,    -1,
     126,    -1,    -1,    -1,    -1,    -1,    -1,   121,   140,    -1,
      -1,   158,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   168,    -1,    -1,   168,
      -1,    -1,    -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   187,    -1,   189,   187,    -1,
     189,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   265,
     266,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     234,    -1,    -1,    -1,   280,   281,    -1,   234,    -1,   221,
      -1,   238,   221,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   257,   258,    -1,    -1,    -1,    -1,    -1,
     257,   258,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   269,   270,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   277,   278,    -1,   277,   278,
      -1,    -1,    -1,    -1,    -1,   277,   278,     0,     1,    -1,
       3,     4,    -1,     6,     7,     8,    -1,    10,    11,    12,
      13,    14,    15,    16,    -1,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    -1,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,    -1,   106,   107,   108,    -1,   110,   111,   112,
      -1,    -1,   115,   116,   117,    -1,   119,   120,   121,   122,
      -1,   124,    -1,   126,   127,   128,   129,   130,     0,     1,
      -1,     3,     4,    -1,     6,     7,     8,    -1,    10,    11,
      12,    13,    14,    15,    16,    -1,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    -1,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,    -1,   106,   107,   108,    -1,   110,   111,
     112,    -1,    -1,   115,   116,   117,    -1,   119,   120,   121,
     122,    -1,   124,    -1,   126,   127,   128,   129,   130,     0,
       1,    -1,     3,     4,    -1,     6,     7,     8,    -1,    10,
      11,    12,    13,    14,    15,    16,    -1,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      -1,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,    -1,   106,   107,   108,    -1,   110,
     111,   112,    -1,    -1,   115,   116,   117,    -1,   119,   120,
     121,   122,    -1,   124,    -1,   126,   127,   128,   129,   130,
       0,     1,    -1,     3,     4,    -1,     6,     7,     8,    -1,
      10,    11,    12,    13,    14,    15,    16,    -1,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    -1,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,    -1,   106,   107,   108,    -1,
     110,   111,   112,    -1,    -1,   115,   116,   117,    -1,   119,
     120,   121,   122,    -1,   124,    -1,   126,   127,   128,   129,
     130,     0,     1,    -1,     3,     4,    -1,     6,     7,     8,
      -1,    10,    11,    12,    13,    14,    15,    16,    -1,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    -1,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,    -1,   106,   107,   108,
      -1,   110,   111,   112,    -1,    -1,   115,   116,   117,    -1,
     119,   120,   121,   122,    -1,   124,    -1,   126,   127,   128,
     129,   130,     0,     1,    -1,     3,     4,    -1,     6,     7,
       8,    -1,    10,    11,    12,    13,    14,    15,    16,    -1,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    -1,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,    -1,   106,   107,
     108,    -1,   110,   111,   112,    -1,    -1,   115,   116,   117,
      -1,   119,   120,   121,   122,    -1,   124,    -1,   126,   127,
     128,   129,   130,     0,     1,    -1,     3,     4,    -1,     6,
       7,     8,    -1,    10,    11,    12,    13,    14,    15,    16,
      -1,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    -1,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,    -1,   106,
     107,   108,    -1,   110,   111,   112,    -1,    -1,   115,   116,
     117,    -1,   119,   120,   121,   122,    -1,   124,    -1,   126,
     127,   128,   129,   130,     0,     1,    -1,     3,     4,    -1,
       6,     7,     8,    -1,    10,    11,    12,    13,    14,    15,
      16,    -1,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    -1,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,    -1,
     106,   107,   108,    -1,   110,   111,   112,    -1,    -1,   115,
     116,   117,    -1,   119,   120,   121,   122,    -1,   124,    -1,
     126,   127,   128,   129,   130,     0,     1,    -1,    -1,     4,
      -1,     6,     7,     8,    -1,    10,    -1,    12,    13,    14,
      -1,    16,    -1,    18,    19,    20,    -1,    22,    23,    24,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    -1,    37,    -1,    39,    40,    41,    -1,    43,    -1,
      -1,    46,    47,    48,    49,    50,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    -1,    69,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   121,   122,    -1,   124,
      -1,   126,     0,     1,    -1,    -1,     4,    -1,     6,     7,
       8,    -1,    10,    -1,    12,    13,    14,    -1,    16,    -1,
      18,    19,    20,    -1,    22,    23,    24,    25,    -1,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    -1,    37,
      -1,    39,    40,    41,    -1,    43,    -1,    -1,    46,    47,
      48,    49,    50,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    18,    19,    20,    -1,    -1,    -1,    24,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,
      -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,    45,    -1,
      -1,    48,    49,   121,   122,    52,    53,    54,   126,    56,
      57,    -1,    -1,    -1,    61,    62,     1,    64,    -1,     4,
      -1,     6,    -1,     8,    -1,    10,    -1,    12,    13,    14,
      -1,    16,    -1,    18,    19,    20,    -1,    22,    23,    24,
      25,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    37,    -1,    39,    40,    41,    -1,    43,    44,
      -1,    46,    47,    -1,    49,    50,    -1,    52,    -1,    54,
      55,    -1,    57,    58,    59,    60,   123,    62,    63,    64,
      -1,    66,    -1,    -1,    -1,    70,    71,    72,    73,     1,
      -1,    -1,     4,    -1,     6,    -1,     8,    -1,    10,    -1,
      12,    13,    14,    -1,    16,    -1,    18,    19,    20,    -1,
      22,    23,    24,    25,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,   108,    -1,    37,    -1,    39,    40,    41,
      -1,    43,    44,    -1,    46,    47,    -1,    49,    50,    -1,
      52,    -1,    54,    55,    -1,    57,    58,    59,    60,    -1,
      62,    63,    64,    -1,    66,    -1,    -1,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    -1,    -1,     0,     1,
      -1,     3,     4,    -1,     6,     7,     8,    -1,    10,    11,
      12,    13,    14,    15,    16,    -1,    18,    19,    20,    -1,
      22,    23,    24,    25,   106,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    -1,    45,    46,    47,    48,    49,    50,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    -1,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    -1,    -1,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,   100,   101,
       4,    -1,     6,    -1,     8,    -1,    10,    -1,    12,    13,
      14,    -1,    16,    -1,    18,    19,    20,    -1,    22,    23,
      24,    25,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    37,    -1,    39,    40,    41,    -1,    43,
      -1,    -1,    46,    47,    -1,    49,    50,    -1,    -1,    -1,
      54,    55,    -1,    57,    58,    59,    60,    -1,    62,    63,
      64,    -1,    66,    -1,    -1,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,     1,    -1,    -1,     4,
      -1,     6,    -1,     8,    -1,    10,    -1,    12,    13,    14,
      -1,    16,    -1,    18,    19,    20,    -1,    22,    23,    24,
      25,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    37,    -1,    39,    40,    41,    -1,    43,    44,
      -1,    46,    47,    -1,    49,    50,    -1,    52,    -1,    54,
      55,    -1,    57,    58,    59,    60,    -1,    62,    63,    64,
      -1,    66,    -1,    -1,    -1,    70,    71,    72,    73,     4,
      -1,     6,    -1,     8,    -1,    10,    -1,    12,    13,    -1,
      -1,    16,    87,    -1,    19,    20,    -1,    22,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    39,    40,    41,    -1,    43,    -1,
      -1,    46,    -1,    -1,    49,    -1,    -1,    -1,    53,    -1,
      55,    -1,    57,    58,    59,    60,    -1,    62,    63,    -1,
      -1,    66,    -1,    -1,    -1,    70,    71,    -1,    73,    -1,
       4,    76,     6,    -1,     8,    -1,    10,    -1,    12,    13,
      14,    -1,    16,    -1,    18,    19,    20,    -1,    22,    23,
      24,    25,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    -1,    37,    -1,    39,    40,    41,    -1,    43,
      -1,    -1,    46,    47,    -1,    49,    50,    -1,    52,    -1,
      54,    55,    -1,    57,    58,    59,    60,    -1,    62,    63,
      64,    -1,    66,    -1,    -1,    -1,    70,    71,    72,    73,
       4,    -1,     6,    -1,     8,    -1,    10,    -1,    12,    13,
      -1,    -1,    16,    -1,    -1,    19,    20,    -1,    22,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    39,    40,    41,    -1,    43,
      -1,    -1,    46,    -1,    -1,    49,    -1,    -1,    -1,    -1,
      -1,    55,    -1,    57,    58,    59,    60,    -1,    62,    63,
      -1,    -1,    66,    -1,    -1,    -1,    70,    71,    -1,    73
  };

  const unsigned char
  HTMLParser::yystos_[] =
  {
       0,   132,   133,     0,     1,     3,     4,     6,     7,     8,
      10,    11,    12,    13,    14,    15,    16,    18,    19,    20,
      22,    23,    24,    25,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    45,    46,    47,    48,    49,    50,    52,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    66,
      68,    70,    71,    72,    73,    80,   100,   101,   134,   136,
     137,   139,   140,   147,   153,   157,   158,   159,   160,   161,
     162,   165,   168,    44,   174,   158,   172,   163,   172,   172,
     135,   135,   172,   172,   172,   143,   135,   136,   137,   151,
     152,   159,   173,   172,   173,   135,   172,   172,   164,   144,
     172,   141,   138,   172,   172,    51,   166,   167,   172,   172,
     172,   172,   172,    17,   154,   171,   134,   134,   170,   172,
     172,   142,   172,     1,   210,   159,   210,   210,   210,   210,
     138,   173,    75,    53,    76,   159,    77,   177,    78,   179,
       1,    48,    56,    61,    79,   136,   137,   157,   158,   178,
      82,   180,    83,   181,    84,   182,    86,   183,   145,    88,
     184,     1,   152,   210,    91,   186,    92,   187,    93,   188,
     102,   189,   103,   190,     1,     9,   105,   145,   107,   145,
       1,   110,   139,   158,   193,   111,   194,   112,   195,   134,
     114,   167,   115,   196,   116,   197,   117,   198,   119,   199,
     120,   200,   172,   155,   123,   125,   204,   127,   206,   128,
     207,   145,   130,   209,    94,    95,    96,    97,    98,    99,
     169,    74,   176,     1,    44,    87,   137,   146,   158,   147,
     148,   106,   210,   108,   109,   192,    81,     0,     1,    69,
     121,   201,   129,   208,   173,    89,   185,    21,    26,    89,
     149,   150,   156,   104,   191,   173,   173,     1,    41,    65,
      67,   126,   205,    85,   210,    90,   210,   135,   135,    52,
     175,   175,   122,   202,   124,   203,   210,   210,   202
  };

  const unsigned char
  HTMLParser::yyr1_[] =
  {
       0,   131,   132,   133,   133,   133,   133,   133,   133,   133,
     133,   133,   133,   133,   133,   133,   133,   133,   133,   133,
     133,   133,   133,   134,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   136,   137,   137,   138,   138,   138,   138,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   141,
     140,   142,   140,   143,   140,   144,   140,   145,   145,   145,
     146,   146,   146,   147,   147,   148,   148,   148,   148,   149,
     149,   150,   150,   151,   151,   151,   152,   152,   152,   153,
     154,   155,   155,   155,   156,   156,   156,   156,   156,   157,
     158,   158,   159,   159,   159,   159,   159,   159,   160,   160,
     160,   160,   160,   160,   160,   160,   160,   161,   161,   161,
     161,   161,   161,   161,   161,   162,   162,   162,   162,   162,
     162,   162,   163,   163,   163,   164,   164,   164,   165,   165,
     165,   166,   166,   167,   168,   168,   168,   168,   168,   168,
     169,   169,   169,   169,   169,   169,   170,   170,   171,   171,
     172,   172,   173,   173,   174,   174,   175,   175,   176,   176,
     177,   177,   178,   178,   179,   179,   180,   180,   181,   181,
     182,   182,   183,   183,   184,   184,   185,   185,   186,   186,
     187,   187,   188,   188,   189,   189,   190,   190,   191,   191,
     192,   192,   193,   193,   194,   194,   195,   195,   196,   196,
     197,   197,   198,   198,   199,   199,   200,   200,   201,   201,
     202,   202,   203,   203,   204,   204,   205,   205,   206,   206,
     207,   207,   208,   208,   209,   209,   210,   210
  };

  const signed char
  HTMLParser::yyr2_[] =
  {
       0,     2,     1,     0,     2,     2,     2,     2,     2,     2,
       4,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     1,     0,     2,     2,     2,     2,     2,
       2,     2,     2,     3,     1,     3,     0,     2,     2,     2,
       1,     1,     1,     3,     3,     3,     3,     1,     4,     0,
       4,     0,     4,     0,     4,     0,     4,     0,     2,     2,
       3,     1,     1,     5,     5,     0,     1,     2,     2,     3,
       5,     3,     5,     1,     2,     2,     1,     1,     1,     3,
       3,     0,     2,     4,     0,     2,     4,     5,     2,     3,
       1,     2,     2,     2,     2,     2,     2,     4,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     4,     1,     3,     3,     1,
       1,     3,     0,     2,     2,     0,     2,     2,     1,     3,
       3,     1,     2,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     0,     1,     0,     1,
       0,     1,     0,     1,     0,     1,     0,     1,     0,     1,
       0,     1,     0,     1,     0,     1,     0,     1,     0,     1,
       0,     1,     0,     1,     0,     1,     0,     1,     0,     1,
       0,     1,     0,     1,     0,     1,     0,     1,     0,     1,
       0,     1,     0,     1,     0,     1,     0,     1,     0,     1,
       0,     1,     0,     1,     0,     1,     0,     1,     1,     1,
       0,     1,     0,     1,     0,     1,     0,     1,     0,     1,
       0,     1,     0,     1,     0,     1,     0,     1
  };


#if YYDEBUG || 1
  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a YYNTOKENS, nonterminals.
  const char*
  const HTMLParser::yytname_[] =
  {
  "\"end of file\"", "error", "\"invalid token\"", "DOCTYPE", "PCDATA",
  "SCAN_ERROR", "A", "ADDRESS", "APPLET", "AREA", "B", "BASE", "BASEFONT",
  "BIG", "BLOCKQUOTE", "BODY", "BR", "CAPTION", "CENTER", "CITE", "CODE",
  "DD", "DFN", "DIR", "DIV", "DL", "DT", "EM", "FONT", "FORM", "H1", "H2",
  "H3", "H4", "H5", "H6", "HEAD", "HR", "HTML", "I", "IMG", "INPUT",
  "ISINDEX", "KBD", "LI", "LINK", "MAP", "MENU", "META", "NOBR", "OL",
  "OPTION", "P", "PARAM", "PRE", "SAMP", "SCRIPT", "SELECT", "SMALL",
  "STRIKE", "STRONG", "STYLE", "SUB", "SUP", "TABLE", "TD", "TEXTAREA",
  "TH", "TITLE", "TR", "TT", "U", "UL", "VAR", "END_A", "END_ADDRESS",
  "END_APPLET", "END_B", "END_BIG", "END_BLOCKQUOTE", "END_BODY",
  "END_CAPTION", "END_CENTER", "END_CITE", "END_CODE", "END_DD", "END_DFN",
  "END_DIR", "END_DIV", "END_DL", "END_DT", "END_EM", "END_FONT",
  "END_FORM", "END_H1", "END_H2", "END_H3", "END_H4", "END_H5", "END_H6",
  "END_HEAD", "END_HTML", "END_I", "END_KBD", "END_LI", "END_MAP",
  "END_MENU", "END_NOBR", "END_OL", "END_OPTION", "END_P", "END_PRE",
  "END_SAMP", "END_SCRIPT", "END_SELECT", "END_SMALL", "END_STRIKE",
  "END_STRONG", "END_STYLE", "END_SUB", "END_SUP", "END_TABLE", "END_TD",
  "END_TEXTAREA", "END_TH", "END_TITLE", "END_TR", "END_TT", "END_U",
  "END_UL", "END_VAR", "$accept", "document", "document_", "pcdata",
  "body_content", "heading", "block", "paragraph_content",
  "block_except_p", "list", "$@1", "$@2", "$@3", "$@4", "list_content",
  "list_item", "definition_list", "definition_list_content", "term_name",
  "term_definition", "flow", "flow_", "preformatted", "caption",
  "table_rows", "table_cells", "address", "texts", "text", "font",
  "phrase", "special", "applet_content", "map_content", "form",
  "select_content", "option", "HX", "END_HX", "opt_pcdata", "opt_caption",
  "opt_texts", "opt_flow", "opt_LI", "opt_P", "opt_END_A", "opt_END_B",
  "opt_END_BLOCKQUOTE", "opt_END_BIG", "opt_END_CENTER", "opt_END_CITE",
  "opt_END_CODE", "opt_END_DFN", "opt_END_DIV", "opt_END_DL", "opt_END_EM",
  "opt_END_FONT", "opt_END_FORM", "opt_END_I", "opt_END_KBD", "opt_END_LI",
  "opt_END_OPTION", "opt_END_P", "opt_END_PRE", "opt_END_SAMP",
  "opt_END_SMALL", "opt_END_STRIKE", "opt_END_STRONG", "opt_END_SUB",
  "opt_END_SUP", "opt_END_TABLE", "opt_END_TD", "opt_END_TH",
  "opt_END_TITLE", "opt_END_TR", "opt_END_TT", "opt_END_U", "opt_END_UL",
  "opt_END_VAR", "opt_error", YY_NULLPTR
  };
#endif


#if YYDEBUG
  const short
  HTMLParser::yyrline_[] =
  {
       0,   274,   274,   305,   309,   312,   315,   319,   322,   326,
     329,   333,   336,   339,   344,   347,   355,   363,   367,   370,
     375,   378,   381,   387,   395,   398,   401,   409,   417,   423,
     428,   431,   434,   440,   451,   454,   463,   466,   469,   474,
     480,   483,   486,   489,   495,   501,   507,   513,   518,   528,
     528,   535,   535,   542,   542,   549,   549,   559,   562,   565,
     572,   578,   583,   593,   599,   609,   612,   615,   619,   626,
     631,   640,   645,   654,   658,   661,   667,   670,   673,   679,
     687,   695,   698,   701,   710,   713,   716,   722,   729,   736,
     746,   750,   756,   757,   758,   759,   760,   761,   770,   771,
     772,   773,   774,   775,   776,   777,   778,   782,   783,   784,
     785,   786,   787,   788,   789,   796,   816,   856,   864,   870,
     875,   880,   889,   892,   896,   905,   908,   911,   918,   923,
     929,   938,   942,   948,   958,   959,   960,   961,   962,   963,
     967,   968,   969,   970,   971,   972,   977,   977,   978,   978,
     979,   979,   980,   980,   982,   982,   983,   983,   985,   985,
     986,   986,   987,   987,   988,   988,   989,   989,   990,   990,
     991,   991,   992,   992,   993,   993,   994,   994,   995,   995,
     996,   996,   997,   997,   998,   998,   999,   999,  1000,  1000,
    1001,  1001,  1002,  1002,  1003,  1003,  1004,  1004,  1005,  1005,
    1006,  1006,  1007,  1007,  1008,  1008,  1009,  1009,  1010,  1010,
    1011,  1011,  1012,  1012,  1013,  1013,  1014,  1014,  1015,  1015,
    1016,  1016,  1017,  1017,  1018,  1018,  1020,  1020
  };

  void
  HTMLParser::yy_stack_print_ () const
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  void
  HTMLParser::yy_reduce_print_ (int yyrule) const
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG

  HTMLParser::symbol_kind_type
  HTMLParser::yytranslate_ (int t)
  {
    // YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to
    // TOKEN-NUM as returned by yylex.
    static
    const unsigned char
    translate_table[] =
    {
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130
    };
    const int user_token_number_max_ = 385;

    if (t <= 0)
      return symbol_kind::S_YYEOF;
    else if (t <= user_token_number_max_)
      return YY_CAST (symbol_kind_type, translate_table[t]);
    else
      return symbol_kind::S_YYUNDEF;
  }

#line 40 "HTMLParser.yy"
} // html2text
#line 3090 "HTMLParser.tab.cc"

#line 1022 "HTMLParser.yy"
 /* } */

void
html2text::HTMLParser::error(const std::string& msg)
{
	yyerror(msg.c_str());
}

