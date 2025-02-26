/*
 * MRustC - Rust Compiler
 * - By John Hodge (Mutabah/thePowersGang)
 *
 * parse/token.hpp
 * - Lexer Tokens
 */
#pragma once

#include <rc_string.hpp>
#include <tagged_union.hpp>
#include "../coretypes.hpp"
#include <ident.hpp>
#include <memory>
#include <int128.h>
#include "span.hpp"

enum eTokenType
{
    #define _(t)    t,
    #include "eTokenType.enum.h"
    #undef _
};


class Position
{
public:
    Span    span;
    RcString    filename;
    unsigned int    line;
    unsigned int    ofs;

    Position():
        filename(""),
        line(0),
        ofs(0)
    {}
    Position(Span sp)
        : span(std::move(sp))
        , filename("")
        , line(0)
        , ofs(0)
    {}
    Position(RcString filename, unsigned int line, unsigned int ofs):
        filename(filename),
        line(line),
        ofs(ofs)
    {
    }
};
extern ::std::ostream& operator<<(::std::ostream& os, const Position& p);

class TypeRef;
class TokenTree;
namespace AST {
    class Visibility;
    class Pattern;
    class Path;
    class ExprNode;
    class Attribute;
    class Item;

    template<typename T>
    struct Named;
};

class InterpolatedFragment;

class Token
{
    friend class HirSerialiser;
    friend class HirDeserialiser;

    TAGGED_UNION(Data, None,
    (None, struct {}),
    (Ident, Ident),
    (String, ::std::string),
    (Integer, struct {
        enum eCoreType  m_datatype;
        U128    m_intval;
        }),
    (Float, struct {
        enum eCoreType  m_datatype;
        double  m_floatval;
        }),
    (Fragment, void*)
    );

    enum eTokenType m_type;
    Data    m_data;
    Position    m_pos;
    Ident::Hygiene  m_hygiene;  // Only for strings, for formatting

    Token(enum eTokenType t, Data d, Position p):
        m_type(t),
        m_data( ::std::move(d) ),
        m_pos( ::std::move(p) )
    {
    }
public:
    virtual ~Token();
    Token();
    Token& operator=(Token&& t)
    {
        m_type = t.m_type;  t.m_type = TOK_NULL;
        m_data = ::std::move(t.m_data);
        m_pos = ::std::move(t.m_pos);
        m_hygiene = ::std::move(t.m_hygiene);
        return *this;
    }
    Token(Token&& t):
        m_type(t.m_type),
        m_data( ::std::move(t.m_data) ),
        m_pos( ::std::move(t.m_pos) ),
        m_hygiene( std::move(t.m_hygiene) )
    {
        t.m_type = TOK_NULL;
    }
    Token& operator=(const Token& t) {
        this->~Token();
        new (this) Token(t);
        return *this;
    }
    Token(const Token& t);
    Token clone() const;

    Token(enum eTokenType type);
    Token(enum eTokenType type, ::std::string str, Ident::Hygiene h);
    Token(enum eTokenType type, Ident i);
    Token(U128 val, enum eCoreType datatype);
    static Token make_float(double val, enum eCoreType datatype);
    Token(const InterpolatedFragment& );
    struct TagTakeIP {};
    Token(TagTakeIP, InterpolatedFragment );

    enum eTokenType type() const { return m_type; }
    bool has_data() const { return !m_data.is_None(); }

    const Ident& ident() const { return m_data.as_Ident(); }
          ::std::string& str()       { return m_data.as_String(); }
    const ::std::string& str() const { return m_data.as_String(); }
    const Ident::Hygiene& str_hygiene() const { return m_hygiene; }
    enum eCoreType  datatype() const { TU_MATCH_DEF(Data, (m_data), (e), (assert(!"Getting datatype of invalid token type");), (Integer, return e.m_datatype;), (Float, return e.m_datatype;)) throw ""; }
    U128 intval() const { return m_data.as_Integer().m_intval; }
    double floatval() const { return m_data.as_Float().m_floatval; }

    // TODO: Replace these with a way of getting a InterpolatedFragment&
    TypeRef& frag_type() { assert(m_type == TOK_INTERPOLATED_TYPE); return *reinterpret_cast<TypeRef*>( m_data.as_Fragment() ); }
    AST::Path& frag_path() { assert(m_type == TOK_INTERPOLATED_PATH); return *reinterpret_cast<AST::Path*>( m_data.as_Fragment() ); }
    AST::Pattern& frag_pattern() { assert(m_type == TOK_INTERPOLATED_PATTERN); return *reinterpret_cast<AST::Pattern*>( m_data.as_Fragment() ); }
    AST::Attribute& frag_meta() { assert(m_type == TOK_INTERPOLATED_META); return *reinterpret_cast<AST::Attribute*>( m_data.as_Fragment() ); }
    AST::ExprNode& frag_node();

    ::std::unique_ptr<AST::ExprNode> take_frag_node();
    ::AST::Named<AST::Item> take_frag_item();
    ::AST::Visibility take_frag_vis();

    bool operator==(eTokenType tty) const {
        return type() == tty;
    }
    bool operator!=(eTokenType tty) const { return !(*this == tty); }
    bool operator==(const Token& r) const {
        if(type() != r.type())
            return false;
        TU_MATCH(Data, (m_data, r.m_data), (e, re),
        (None, return true;),
        (Ident, return e == re; ),
        (String, return e == re; ),
        (Integer, return e.m_datatype == re.m_datatype && e.m_intval == re.m_intval;),
        (Float, return e.m_datatype == re.m_datatype && e.m_floatval == re.m_floatval;),
        (Fragment, assert(!"Token equality on Fragment");)
        )
        throw "";
    }
    bool operator!=(const Token& r) const { return !(*this == r); }

    /// Return a re-parseable version of the token
    ::std::string to_str() const;

    void set_pos(Position pos) { m_pos = pos; }
    const Position& get_pos() const { return m_pos; }

    static bool type_is_rword(enum eTokenType type) {
        return type >= TOK_RWORD_PUB;
    }
    static const char* typestr(enum eTokenType type);
    static eTokenType typefromstr(const ::std::string& s);

    friend ::std::ostream&  operator<<(::std::ostream& os, const Token& tok);
};
extern ::std::ostream&  operator<<(::std::ostream& os, const Token& tok);

