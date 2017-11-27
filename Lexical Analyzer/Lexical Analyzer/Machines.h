#pragma once
#include <vector>
#include <string>
#include <functional>

#include "TokenAttr.h"
#include "SymbolTable.h"

#ifndef MACHINES_H_
#define MACHINES_H_

class TAPair
{
public:
    TAPair(): token(NOT_FOUND), attribute(NONE) {}
    TAPair( const TOKEN_TYPE tok, const ATTRIBUTE_TYPE attr): token(tok), attribute(attr) {}
    TAPair( const TOKEN_TYPE tok, const int idx): token(tok), attribute( static_cast< ATTRIBUTE_TYPE >(idx) ) {}
    TAPair( const ATTRIBUTE_TYPE error ) : token(LEXICAL_ERROR), attribute( error ) {}

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
};

#define EMPTY_TOKEN (TAPair())

typedef std::pair<std::string, TAPair> LexicalToken;

class Machine
{
public:
    virtual ~Machine() = default;
    virtual TAPair runMachine( const std::string& line, int& idx ) const = 0;
    virtual TOKEN_TYPE getTokenType() const = 0;
};

class PascalMachine
{
    std::vector<std::unique_ptr<Machine>> _useful_machines;
    std::vector<std::unique_ptr<Machine>> _space_machines;
    int _idx;
    std::string _line;
    SymbolTable& _s_table;

public:
    PascalMachine( const std::vector< std::string >& reserved_words, SymbolTable& symbol_table);

    void setLine(const std::string& line);

    LexicalToken getToken();

private:
    TAPair clearSpace();
};


#endif
