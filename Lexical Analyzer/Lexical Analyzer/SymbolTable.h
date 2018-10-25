#pragma once
#include <vector>
#include "Enums.h"

#ifndef SYMBOL_TABLE_H_
#define SYMBOL_TABLE_H_

struct Symbol
{
    int idx{};
    std::string lex;
    E_TYPE type;
};

class SymbolTable
{
public:

    int addOrReturn( const std::string& lex );

    Symbol& operator[]( unsigned int idx );

    const Symbol& operator[]( unsigned int idx ) const;

    Symbol& get( unsigned int idx );

    const Symbol& get( unsigned int idx ) const;
    
private:
    std::vector<Symbol> _table;
};

#endif
