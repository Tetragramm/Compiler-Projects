#pragma once
#include "Parser.h"
#include "SymbolTable.h"
#include "Machines.h"

#ifndef _BASE_PARSER_H
#define _BASE_PARSER_H

class BaseParser : public Parser
{
public:
    explicit BaseParser( std::ifstream& file, std::ostream& output) :
    _output( output ), _file(file), _line_number(0)
    {
        _table = std::make_shared<SymbolTable>();
    }
    
    void parse() override;

    void setTokenStream(std::shared_ptr<std::ostream> stream) override;

protected:
    std::shared_ptr<SymbolTable> _table;
    LexicalToken _tok;

    LexicalToken getToken();

    std::shared_ptr<LanguageMachine> _machine;

    bool match(const LexicalToken& m, const std::string& current_func);
    bool match(const TAPair& m, const std::string& current_func);
    bool match(const TOKEN_TYPE& m, const std::string& current_func);

    bool check(const LexicalToken& m) const;
    bool check(const TAPair& m) const;
    bool check(const TOKEN_TYPE& m) const;

    void endParsing() const;
    void synch(const std::vector<LexicalToken>& tokens);

    int getLineNumber() const;
    void getLine(std::string& line);

    std::ostream& _output;
    std::shared_ptr< std::ostream > _token_stream;

private:
    std::ifstream& _file;
    int _line_number;
};

#endif
