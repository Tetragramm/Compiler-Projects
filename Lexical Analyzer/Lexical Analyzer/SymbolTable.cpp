#include "stdafx.h"

#include <algorithm>

#include "SymbolTable.h"

int SymbolTable::addOrReturn( const std::string& lex )
{
    if ( !_table.empty() )
    {
        const auto iter = find_if( _table.begin(), _table.end()
                                   , [&lex]( const Symbol& s )-> bool { return lex == s.lex; } );
        if ( iter != _table.end() )
        {
            return int( iter - _table.begin() );
        }
    }

    Symbol s;
    s.idx = static_cast< int >(_table.size());
    s.lex = lex;
    _table.push_back( s );
    return s.idx;
}


Symbol & SymbolTable::operator[](const unsigned int idx)
{
    return _table[idx];
}

const Symbol & SymbolTable::operator[](const unsigned int idx) const
{
    return _table[idx];
}

Symbol & SymbolTable::get(const unsigned int idx)
{
    return _table[idx];
}

const Symbol & SymbolTable::get(const unsigned int idx) const
{
    return _table[idx];
}

