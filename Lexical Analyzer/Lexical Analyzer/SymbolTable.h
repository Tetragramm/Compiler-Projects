#pragma once
#include <iostream>
#include <vector>
#include "Scope.h"

#ifndef SYMBOL_TABLE_H_
#define SYMBOL_TABLE_H_

struct Symbol
{
    int idx;
    std::string lex;
    E_TYPE type;
};

class SymbolTable
{
    std::vector<Symbol> _table;
public:
    SymbolTable()
    {
        std::cout<<"Constructing ST\n";
    }
    int addOrReturn( const std::string& lex );

    Symbol& operator[]( const unsigned int idx );

    const Symbol& operator[]( const unsigned int idx ) const;
};

#endif
