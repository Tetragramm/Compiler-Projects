#pragma once
#include <memory>
#include <vector>
#include "Machines.h"
#include <variant>
#include <unordered_map>
#include <deque>

#ifndef SCOPE_H_
#define SCOPE_H_

class Scope;

#define ET_ENUM(E_TYPE) \
    E_TYPE(T_ERROR_UNDEFINDED, ) \
    E_TYPE(T_ERROR, ) \
    E_TYPE(T_INTEGER, ) \
    E_TYPE(T_REAL, ) \
    E_TYPE(T_ARRAY_INTEGER, ) \
    E_TYPE(T_ARRAY_REAL, ) \
    E_TYPE(T_BOOLEAN, ) \

DECLARE_ENUM(E_TYPE,ET_ENUM)

inline std::string getString( const E_TYPE type)
{
    return NS_E_TYPE_NS::getString( type );
}

struct TypedInfo
{
    TypedInfo() : type(T_ERROR_UNDEFINDED), memory_location(-1) {}
    explicit TypedInfo( const E_TYPE type) : type(type), memory_location(-1) {}
    E_TYPE type;
    unsigned memory_location;
};

struct VarInfo : TypedInfo
{
    VarInfo() {}
    explicit VarInfo( const E_TYPE type) : TypedInfo(type) {}
};

struct ArrayInfo : TypedInfo
{
    ArrayInfo() : TypedInfo(T_ERROR), access_offset(0), count(0) {}
    int access_offset;
    int count;
};

struct FuncInfo : TypedInfo
{
    std::vector<std::variant<VarInfo, ArrayInfo>> parameters;
    std::shared_ptr<Scope> func_scope;
};

typedef std::variant<VarInfo, ArrayInfo, FuncInfo> Info;



class Scope : public std::enable_shared_from_this<Scope>
{
public:

    void addVariable(const unsigned& tok, Info& type);
    Info& getVariable(const unsigned& tok);
    std::shared_ptr<Scope> newScope();
    std::shared_ptr<Scope> getParent() const;

private:
    std::shared_ptr<Scope> _parent;
    std::unordered_map<unsigned, Info> _map;
    unsigned _memory_offset;

};

#endif