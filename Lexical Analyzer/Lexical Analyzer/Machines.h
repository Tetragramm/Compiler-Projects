#pragma once
#include <utility>
#include <vector>
#include <string>
#include <memory>

#include "Enums.h"
#include "SymbolTable.h"

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



    TOKEN_TYPE token;
    ATTRIBUTE_TYPE attribute;

    bool operator==(const TAPair& other) const
    {
        return token == other.token && attribute == other.attribute;
    }

    TAPair( const TAPair& other ) = default;

    TAPair( TAPair&& other ) noexcept = default;

    TAPair& operator=( const TAPair& other ) = default;

    TAPair& operator=( TAPair&& other ) noexcept = default;

    ~TAPair() = default;

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
    LexicalToken() = default;
    LexicalToken( std::string text, const TAPair& token_attribute) : ta(token_attribute), lex( std::move( text ) ) {}
    LexicalToken( std::string text, const TOKEN_TYPE& token): ta(token), lex( std::move( text ) ) {}
    LexicalToken( const LexicalToken& other ) = default;
    LexicalToken( LexicalToken&& other ) noexcept = default;
    LexicalToken& operator=( const LexicalToken& other ) = default;
    LexicalToken& operator=( LexicalToken&& other ) noexcept = default;
    ~LexicalToken() = default;

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
public:
    PascalMachine( const std::vector< std::string >& reserved_words, std::shared_ptr<SymbolTable>& symbol_table);
    PascalMachine( const PascalMachine& other ) = default;
    PascalMachine( PascalMachine&& other ) noexcept = default;
    PascalMachine& operator=( const PascalMachine& other ) = default;
    PascalMachine& operator=( PascalMachine&& other ) noexcept = default;
    virtual ~PascalMachine() = default;

    void setLine(const std::string& line) override;

    LexicalToken getToken() override;

private:
    TAPair clearSpace();
    std::vector<std::shared_ptr<Machine>> _useful_machines;
    std::vector<std::shared_ptr<Machine>> _space_machines;
    int _idx;
    std::string _line;
    std::shared_ptr<SymbolTable> _s_table;
};


#endif
