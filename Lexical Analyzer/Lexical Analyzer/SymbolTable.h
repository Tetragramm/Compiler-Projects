#pragma once
#include <vector>
#include <algorithm>

#ifndef SYMBOL_TABLE_H_
#define SYMBOL_TABLE_H_

struct Symbol
{
    explicit Symbol( const int idx):idx(idx) {}
    const int idx;
    std::string lex;
};

class SymbolTable
{
    std::vector<Symbol> _table;
public:
    int addOrReturn(const std::string& lex)
    {
        const auto iter = find_if(_table.begin(), _table.end(), [&lex](const Symbol& s)->bool {return lex == s.lex;});
        if(iter != _table.end())
        {
            return int(iter-_table.begin());
        }

        Symbol s(int( _table.size() ));
        s.lex = lex;
        _table.push_back( s );
        return s.idx;
    }

    Symbol& operator[]( const unsigned int idx)
    {
        return _table[idx];
    }

    const Symbol& operator[]( const unsigned int idx) const
    {
        return _table[idx];
    }
};

#endif