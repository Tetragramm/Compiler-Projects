#include "stdafx.h"
#include "Scope.h"

bool Scope::addVariable( const unsigned& tok, const Info& type )
{
    if(_map.count(tok) != 0)
        return false;
    if ( std::holds_alternative< VarInfo >( type ) )
    {
        VarInfo var = std::get<VarInfo>(type);
        addVariable( tok, ParInfo(var) ); 
    }
    else if(std::holds_alternative<ArrayInfo>(type))
    {
        ArrayInfo arr = std::get<ArrayInfo>(type);
        addVariable( tok, ParInfo(arr) ); 
    }
    else
    {
        _map[tok] = type;
    }
    return true;
}

bool Scope::addVariable( const unsigned& tok, const ParInfo& type )
{
    if(_map.count(tok) != 0)
        return false;
    if ( std::holds_alternative< VarInfo >( type ) )
    {
        VarInfo var = std::get<VarInfo>(type);
        if(var.memory_location >= 0)
            var.memory_location = _memory_offset;
        switch(var.type)
        {
        case T_INTEGER:
            _memory_offset += 4;
            break;
        case T_REAL:
            _memory_offset += 8;
            break;
        default:
            var.memory_location = 1000000;
        }
        _map[tok] = var;
    }
    else if(std::holds_alternative<ArrayInfo>(type))
    {
        ArrayInfo arr = std::get<ArrayInfo>(type);
        arr.memory_location = _memory_offset;
        switch(arr.type)
        {
        case T_INTEGER:
            _memory_offset += 4*arr.count;
            break;
        case T_REAL:
            _memory_offset += 8*arr.count;
            break;
        default:
            arr.memory_location = 1000000;
        }
        _map[tok] = arr;
    }
    return true;
}

Info Scope::getVariable( const unsigned& tok )
{
    if ( _map.count( tok ) )
    {
        return _map[tok];
    }

    if ( _parent )
        return _parent->getVariable( tok );

    throw tok;
}

std::shared_ptr< Scope > Scope::newScope( const std::string& name )
{
    std::shared_ptr< Scope > s = std::make_shared< Scope >( name );
    s->_parent = shared_from_this();
    return s;
}

std::shared_ptr< Scope > Scope::getParent() const
{
    return _parent;
}

void Scope::print( std::ostream& os, const SymbolTable& table )
{
    os << "Scope Name: " << _name << "\n";
    os << "Variable Name    Address\n";
    for ( auto& v : _map )
    {
        os << table.get( v.first ).lex << "    ";
        if ( std::holds_alternative< VarInfo >( v.second ) )
        {
            const VarInfo var = std::get<VarInfo>(v.second);
            os<<var.memory_location<<"\n";
        }
        else if(std::holds_alternative<ArrayInfo>( v.second))
        {
            const ArrayInfo arr = std::get<ArrayInfo>(v.second);
            os<<arr.memory_location<<"\n";
        }
        else
        {
            os<<"Function\n";
        }
    }
    os <<"\n\n";
}

void Scope::setName(const std::string name)
{
    _name = name;
}
