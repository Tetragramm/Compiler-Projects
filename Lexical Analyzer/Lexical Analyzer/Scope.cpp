#include "stdafx.h"
#include "Scope.h"

void Scope::addVariable(const unsigned& tok, Info& type)
{
    _map[tok] = type;
}

Info& Scope::getVariable(const unsigned& tok)
{
    if(_map.count(tok))
    {
        return _map[tok];
    }
    
    if(_parent)
        return _parent->getVariable( tok );

    return _map[tok];
}

std::shared_ptr<Scope> Scope::newScope()
{
    std::shared_ptr<Scope> s = std::make_shared<Scope>();
    s->_parent = shared_from_this();
    return s;
}

std::shared_ptr<Scope> Scope::getParent() const
{
    return _parent;
}
