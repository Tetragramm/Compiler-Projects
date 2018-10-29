#pragma once
#include <memory>
#include <utility>
#include <vector>
#include <variant>
#include <unordered_map>

#include "Machines.h"

#ifndef SCOPE_H_
#define SCOPE_H_

class Scope;

struct TypedInfo
{
    TypedInfo() : type(T_ERROR), memory_location(-1) {}
    explicit TypedInfo( const E_TYPE type) : type(type), memory_location(-1) {}

    TypedInfo( const TypedInfo& other ) = default;
    TypedInfo( TypedInfo&& other ) noexcept = default;
    TypedInfo& operator=( const TypedInfo& other ) = default;
    TypedInfo& operator=( TypedInfo&& other ) noexcept = default;
    ~TypedInfo() = default;
    E_TYPE type;
    unsigned memory_location;
    unsigned id = -1;
};

struct VarInfo : TypedInfo
{
    VarInfo() = default;
    VarInfo( const VarInfo& other ) = default;
    VarInfo( VarInfo&& other ) noexcept = default;
    VarInfo& operator=( const VarInfo& other ) = default;
    VarInfo& operator=( VarInfo&& other ) noexcept = default;
    explicit VarInfo( const E_TYPE type) : TypedInfo(type) {}
    ~VarInfo() = default;
};

struct ArrayInfo : TypedInfo
{
    ArrayInfo() : TypedInfo(T_ERROR), access_offset(0), count(0) {}
    int access_offset;
    int count;
    ArrayInfo( const ArrayInfo& other ) = default;
    ArrayInfo( ArrayInfo&& other ) noexcept = default;
    ArrayInfo& operator=( const ArrayInfo& other ) = default;
    ArrayInfo& operator=( ArrayInfo&& other ) noexcept = default;
    ~ArrayInfo() = default;
};

struct FuncInfo : TypedInfo
{
    std::vector<std::variant<VarInfo, ArrayInfo>> parameters;

    FuncInfo() = default;
    FuncInfo( const FuncInfo& other ) = default;
    FuncInfo( FuncInfo&& other ) noexcept = default;
    FuncInfo& operator=( const FuncInfo& other ) = default;
    FuncInfo& operator=( FuncInfo&& other ) noexcept = default;
    ~FuncInfo() = default;
};

typedef std::variant<VarInfo, ArrayInfo, FuncInfo> Info;

typedef std::variant< VarInfo, ArrayInfo > ParInfo;


class Scope : public std::enable_shared_from_this<Scope>
{
public:
    Scope( std::string name): _name( std::move( name ) ), _memory_offset(0) {}

    Scope( const Scope& other ) = default;
    Scope( Scope&& other ) = default;
    Scope& operator=( const Scope& other ) = default;
    Scope& operator=( Scope&& other ) noexcept = default;
    ~Scope() = default;

    bool addVariable(const unsigned& tok, const Info& type);
    bool addVariable(const unsigned& tok, const ParInfo& type);
    Info getVariable(const unsigned& tok);
    bool checkVariable(const unsigned& tok) const;
    std::shared_ptr<Scope> newScope(const std::string& name);
    std::shared_ptr<Scope> getParent() const;

    void print(std::ostream& os, const SymbolTable& table);

    void setName( std::string name);

private:
    std::string _name;
    std::shared_ptr<Scope> _parent;
    std::unordered_map<unsigned, Info> _map;
    unsigned _memory_offset;

};

#endif