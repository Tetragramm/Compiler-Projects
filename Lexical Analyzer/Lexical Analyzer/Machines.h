#pragma once
#include <vector>
#include <string>
#include <functional>

#include "TokenAttr.h"
#include "SymbolTable.h"
#include <memory>

#ifndef MACHINES_H_
#define MACHINES_H_

class TAPair
{
public:
    TAPair(): token(NOT_FOUND), attribute(NONE) {}
    TAPair( const TOKEN_TYPE tok, const ATTRIBUTE_TYPE attr): token(tok), attribute(attr) {}
    TAPair( const TOKEN_TYPE tok, const int idx): token(tok), attribute( static_cast< ATTRIBUTE_TYPE >(idx) ) {}
    TAPair( const TOKEN_TYPE tok): token(tok), attribute( NONE ) {}
    TAPair( const ATTRIBUTE_TYPE error ) : token(LEXICAL_ERROR), attribute( error ) {}

    TAPair( const TAPair& other )
        : token( other.token )
        , attribute( other.attribute ) {}

    TAPair( TAPair&& other ) noexcept
        : token( other.token )
        , attribute( other.attribute ) {}

    TAPair& operator=( TAPair other )
    {
        token = other.token;
        attribute = other.attribute;
        return *this;
    }

    TOKEN_TYPE token;
    ATTRIBUTE_TYPE attribute;

    bool operator==(const TAPair& other) const
    {
        return token == other.token && attribute == other.attribute;
    }

    bool operator!=(const TAPair& other) const
    {
        return !(operator==(other));
    }

    friend std::ostream& operator<<( std::ostream& os, const TAPair& ta)
    {
        os <<"("<<getString(ta.token)<<" "<<getString(ta.attribute)<<")";
        return os;
    }
};

#define EMPTY_TOKEN (TAPair())

//typedef std::pair<std::string, TAPair> LexicalToken;
class LexicalToken
{
public:
    LexicalToken() {}
    LexicalToken(const std::string& text, const TAPair& token_attribute) : ta(token_attribute), lex(text) {}
    LexicalToken(const std::string& text, const TOKEN_TYPE& token): ta(token), lex(text) {}
    LexicalToken(const LexicalToken& lt): ta(lt.ta), lex(lt.lex) {}
    LexicalToken(LexicalToken&& lt) noexcept: ta(lt.ta), lex(lt.lex) {}

    LexicalToken& operator=( LexicalToken other )
    {
        lex = other.lex;
        ta = other.ta;
        return *this;
    }

    friend bool operator==( const LexicalToken& lhs, const LexicalToken& rhs )
    {
        return lhs.ta == rhs.ta
               && lhs.lex == rhs.lex;
    }

    friend bool operator!=( const LexicalToken& lhs, const LexicalToken& rhs ) { return !( lhs == rhs ); }

    TAPair ta;
    std::string lex;

    friend std::ostream& operator<<( std::ostream& os, const LexicalToken& lt)
    {
        os <<"("<<lt.lex<<" "<<getString(lt.ta.token)<<" "<<getString(lt.ta.attribute)<<")";
        return os;
    }
};

class Machine
{
public:
    virtual ~Machine() = default;
    virtual TAPair runMachine( const std::string& line, int& idx ) const = 0;
    virtual TOKEN_TYPE getTokenType() const = 0;
};

class LanguageMachine
{
public:
    virtual ~LanguageMachine() = default;

    virtual void setLine(const std::string& line) = 0;

    virtual LexicalToken getToken() = 0;
};

class PascalMachine : public LanguageMachine
{
    std::vector<std::shared_ptr<Machine>> _useful_machines;
    std::vector<std::shared_ptr<Machine>> _space_machines;
    int _idx;
    std::string _line;
    std::shared_ptr<SymbolTable> _s_table;

public:
    PascalMachine( const std::vector< std::string >& reserved_words, std::shared_ptr<SymbolTable>& symbol_table);

    void setLine(const std::string& line) override;

    LexicalToken getToken() override;

private:
    TAPair clearSpace();
};


#endif
