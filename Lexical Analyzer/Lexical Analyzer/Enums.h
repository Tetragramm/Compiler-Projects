#pragma once
#include "EnumFactory.h"

#ifndef ENUMS_H_
#define ENUMS_H_

#define TT_ENUM(TOKEN_TYPE) \
    TOKEN_TYPE(LEXICAL_ERROR, = -99999) \
    TOKEN_TYPE(NOT_FOUND, = 0) \
    TOKEN_TYPE(RESERVED, = 1) \
    TOKEN_TYPE(WHITE_SPACE, ) \
    TOKEN_TYPE(ADD_OP, ) \
    TOKEN_TYPE(MUL_OP, ) \
    TOKEN_TYPE(REL_OP, ) \
    TOKEN_TYPE(ASSIGN_OP, ) \
    TOKEN_TYPE(COMMENT, ) \
    TOKEN_TYPE(INTEGER, ) \
    TOKEN_TYPE(REAL, ) \
    TOKEN_TYPE(SYMBOL, ) \
    TOKEN_TYPE(ID, ) \
    TOKEN_TYPE(TYPE, ) \
    TOKEN_TYPE(END_OF_FILE, )\

DECLARE_ENUM(TOKEN_TYPE,TT_ENUM)

inline std::string getString( const TOKEN_TYPE type)
{
    return NS_TOKEN_TYPE_NS::getString( type );
}

#define AT_ENUM(ATTRIBUTE_TYPE)\
    ATTRIBUTE_TYPE(UNKNOWN_SYMBOL, = -99999)\
    ATTRIBUTE_TYPE(LONG_ID,)\
    ATTRIBUTE_TYPE(LEADING_ZERO,)\
    ATTRIBUTE_TYPE(TRAILING_ZERO,)\
    ATTRIBUTE_TYPE(LONG_INTEGER,)\
    ATTRIBUTE_TYPE(LONG_REAL,)\
    ATTRIBUTE_TYPE(LONG_REAL_FRACTIONAL,)\
    ATTRIBUTE_TYPE(LONG_EXPONENT,)\
    ATTRIBUTE_TYPE(MISSING_EXPONENT,)\
    ATTRIBUTE_TYPE(LEADING_ZERO_EXPONENT,)\
    ATTRIBUTE_TYPE(UNCLOSED_COMMENT,)\
    ATTRIBUTE_TYPE(NONE, = -1)\

DECLARE_ENUM(ATTRIBUTE_TYPE,AT_ENUM)

inline std::string getString( const ATTRIBUTE_TYPE attr)
{
    if(attr >= 0)
        return "loc "+std::to_string( static_cast< int >(attr) );
    return NS_ATTRIBUTE_TYPE_NS::getString( attr );
}

inline std::ostream& operator<<( std::ostream& os, const TOKEN_TYPE& t)
{
    os<<getString(t);
    return os;
}

inline std::ostream& operator<<( std::ostream& os, const ATTRIBUTE_TYPE& a)
{
    os<<getString(a);
    return os;
}

#define ET_ENUM(E_TYPE) \
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

#endif