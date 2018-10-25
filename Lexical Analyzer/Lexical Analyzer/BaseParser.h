#pragma once
#include "Parser.h"
#include "Machines.h"
#include "Scope.h"

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
    void setMemoryStream( std::shared_ptr<std::ostream> stream ) override;

protected:
    std::shared_ptr<SymbolTable> _table;
    LexicalToken _tok;

    LexicalToken getToken();

    std::shared_ptr<LanguageMachine> _machine;

    bool match(const LexicalToken& m, const std::string& current_FUNC);
    bool match(const TAPair& m, const std::string& current_FUNC);
    bool match(const TOKEN_TYPE& m, const std::string& current_FUNC);

    bool check(const LexicalToken& m) const;
    bool check(const TAPair& m) const;
    bool check(const TOKEN_TYPE& m) const;

    bool getNum(int& value, const std::string& current_FUNC);
    bool getNum(double& value, const std::string& current_FUNC);
    bool getIdSymbol(unsigned& symbol, const std::string& current_FUNC);

    void endParsing() const;
    void synch(const std::vector<LexicalToken>& tokens);

    int getLineNumber() const;
    void getLine(std::string& line);

protected:
    std::ostream& _output;
    std::shared_ptr< std::ostream > _token_stream;
    std::shared_ptr< std::ostream > _memory_stream;

    std::shared_ptr<Scope> _scope;

private:
    std::ifstream& _file;
    int _line_number;
};

#endif
