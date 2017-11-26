#pragma once

#include <string>

#ifndef ENUM_FACTORY_H
#define ENUM_FACTORY_H

//Took MACROS from https://stackoverflow.com/questions/147267/easy-way-to-use-variables-of-enum-types-as-string-in-c#202511
//because why re-write all of this?

// expansion macro for enum value definition
#define ENUM_VALUE(name,assign) name assign,

// expansion macro for enum to string conversion
#define ENUM_CASE(name,assign) case name: return #name;

// expansion macro for string to enum conversion

#define ENUM_STRCMP(name,assign) if (!str.compare(#name)) return name;

/// declare the access function and define enum values
#define DECLARE_ENUM(EnumType,ENUM_DEF) \
  enum EnumType { \
    ENUM_DEF(ENUM_VALUE) \
  }; \
  \
  namespace NS_##EnumType##_NS {\
  const std::string GetString(EnumType dummy); \
  EnumType Get##EnumType##Value(const std::string& string); \
  };\

/// define the access function names
#define DEFINE_ENUM(EnumType,ENUM_DEF) \
  namespace NS_##EnumType##_NS {\
  const std::string GetString(EnumType value) \
  { \
    switch(value) \
    { \
      ENUM_DEF(ENUM_CASE) \
      default: return ""; /* handle input error */ \
    } \
  } \
  EnumType Get##EnumType##Value(const std::string& str) \
  { \
    ENUM_DEF(ENUM_STRCMP) \
    return (EnumType)0; /* handle input error */ \
  } \
  };\

#endif
