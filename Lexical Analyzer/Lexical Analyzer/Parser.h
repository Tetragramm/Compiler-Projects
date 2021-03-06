#pragma once
#include <memory>
#include <fstream>

#ifndef PARSER_H_
#define PARSER_H_

class Parser
{
public:
    virtual ~Parser() = default;

    virtual void parse() = 0;
    virtual void testTable() = 0;
    
    virtual void setTokenStream(std::shared_ptr<std::ostream> stream) = 0;
    virtual void setMemoryStream(std::shared_ptr<std::ostream> stream) = 0;

    static std::shared_ptr<Parser> createPascalParser(
        std::ifstream& r_words_file,
        std::ifstream& file,
        std::ostream& output );
};

#endif
