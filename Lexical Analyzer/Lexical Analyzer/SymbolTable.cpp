#include "stdafx.h"

#include <algorithm>
#include <iostream>

#include "SymbolTable.h"

int SymbolTable::addOrReturn( const std::string& lex )
{
    if ( _table.size() > 0 )
    {
        const auto iter = find_if( _table.begin(), _table.end()
                                   , [&lex]( const Symbol& s )-> bool { return lex == s.lex; } );
        if ( iter != _table.end() )
        {
            return int( iter - _table.begin() );
        }
    }

    Symbol s;
    s.idx = _table.size();
    s.lex = lex;
    _table.push_back( s );
    return s.idx;
}

inline Symbol& SymbolTable::operator[]( const unsigned idx )
{
    return _table[idx];
}

inline const Symbol& SymbolTable::operator[]( const unsigned idx ) const
{
    return _table[idx];
}

inline Symbol& SymbolTable::get( const unsigned idx)
{
    return _table[idx];
}

inline const Symbol& SymbolTable::get( const unsigned idx) const
{
    return _table[idx];
}

