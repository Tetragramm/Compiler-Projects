#include "stdafx.h"
#include "BaseParser.h"
#include <iomanip>



bool BaseParser::match( const LexicalToken& m, const std::string& current_FUNC )
{
    fillEmptyToken();
    if(check(m))
    {
        if(_tok.ta.token == END_OF_FILE)
            endParsing();
        else
        {
            setEmptyToken();
        }
        return true;
    }
    
    _output<<"Syntax Error at line "<<getLineNumber()<<" in "<<current_FUNC<<": Expected "<<m<<",";
    _output<<" but Found "<<_tok<<"\n";
    setEmptyToken();
    return false;
}

bool BaseParser::match( const TAPair& m, const std::string& current_FUNC )
{
    fillEmptyToken();
    if(check(m))
    {
        if(_tok.ta.token == END_OF_FILE)
            endParsing();
        else
        {
            setEmptyToken();
        }
        return true;
    }
    
    _output<<"Syntax Error at line "<<getLineNumber()<<" in "<<current_FUNC<<": Expected "<<m<<",";
    _output<<" but Found "<<_tok.ta<<"\n";
    setEmptyToken();
    return false;
}

bool BaseParser::match( const TOKEN_TYPE& m, const std::string& current_FUNC )
{
    fillEmptyToken();
    if(check(m))
    {
        if(_tok.ta.token == END_OF_FILE)
            endParsing();
        else
        {
            setEmptyToken();
        }
        return true;
    }
    
    _output<<"Syntax Error at line "<<getLineNumber()<<" in "<<current_FUNC<<": Expected "<<m<<",";
    _output<<" but Found ("<<_tok.ta.token<<")\n";
    setEmptyToken();
    return false;
}

bool BaseParser::check( const LexicalToken& m )
{
    fillEmptyToken();
    return (m == _tok);
}

bool BaseParser::check( const TAPair& m )
{
    fillEmptyToken();
    return (m == _tok.ta);
}

bool BaseParser::check( const TOKEN_TYPE& m )
{
    fillEmptyToken();
    return (m == _tok.ta.token);
}

bool BaseParser::getNum( int& value, const std::string& current_FUNC )
{
    if(check(INTEGER))
    {
        value = stoi( _tok.lex );
    }
    return match( INTEGER, current_FUNC );
}

bool BaseParser::getNum( double& value, const std::string& current_FUNC )
{
    if(check(REAL))
    {
        value = stod( _tok.lex );
    }
    return match( REAL, current_FUNC );
}

bool BaseParser::getIdSymbol( unsigned& symbol, const std::string& current_FUNC )
{
    if(check(ID))
    {
        symbol = static_cast< int >(_tok.ta.attribute);
    }
    return match( ID, current_FUNC );
}

void BaseParser::endParsing() const
{
    _output << "End of Parsing\n";
    _output.flush();
}

void BaseParser::fillEmptyToken()
{
    if(_tok.lex == "#")
    {
        _tok = getToken();
    }
}

void BaseParser::setEmptyToken()
{
    _tok.lex = "#";
}

LexicalToken BaseParser::getToken()
{
    LexicalToken tok;
    bool eof = false;
    while(!eof)
    {
        do
        {
            tok = _machine->getToken();

            if(tok.ta.token == LEXICAL_ERROR)
            {
                _output<<tok<<"\n";
            }
            
            if(tok.ta != EMPTY_TOKEN)
            {
                if(_token_stream)
                {
                    *_token_stream<< std::setw(5)<<_line_number<<"  ";
                    *_token_stream<< std::setw(10)<<tok.lex<<"  ";
                    *_token_stream<< std::setw(9)<<getString(tok.ta.token)<<"  ";
                    *_token_stream<< std::setw(6)<<getString(tok.ta.attribute)<<"\n";
                }
                return tok;
            }
        } while( tok.ta != EMPTY_TOKEN);

        if(!_file.eof())
        {
            std::string line;
            getLine(line);

            _machine->setLine( line );
        }
        else
        {
            eof = true;
        }
    }

    if(_token_stream)
    {
        *_token_stream<< std::setw(5)<<_line_number<<"  ";
        *_token_stream<< std::setw(10)<<"$EOF"<<"  ";
        *_token_stream<< std::setw(9)<<getString(END_OF_FILE)<<"  ";
        *_token_stream<< std::setw(6)<<getString(NONE)<<"\n";
    }
    return LexicalToken("", TAPair(END_OF_FILE, NONE));
}

void BaseParser::synch( const std::vector< LexicalToken >& tokens )
{
    while(!_file.eof())
    {
        do
        {
            if(_tok.ta != EMPTY_TOKEN)
            {
                for(const LexicalToken& t : tokens)
                {
                    //ID could be any ID, 
                    //INTEGER could be any INTEGER,
                    //# is not a valid symbol,
                    //so use it as a wildcard.
                    if(t.lex == "#")
                    {
                        if(check(t.ta.token))
                        {
                            return;
                        }
                    }
                    else
                    {
                        if(check(t))
                        {
                            return;
                        }
                    }
                }
            }
            
            _tok = getToken();
        } while( _tok.ta != EMPTY_TOKEN);
    }
}

int BaseParser::getLineNumber() const
{
    return _line_number;
}

void BaseParser::getLine(std::string& line)
{
    getline(_file, line);
    _line_number++;
    _output<<"Line Number "<<_line_number<<" "<<line<<"\n";
    _output.flush();
}

void BaseParser::parse()
{
    std::string line;
    getLine(line);

    _machine->setLine( line );
}

void BaseParser::setTokenStream( const std::shared_ptr<std::ostream> stream )
{
    _token_stream = stream;
    *_token_stream<< std::setw(5)<<"Line#"<<"  ";
    *_token_stream<< std::setw(10)<<"Lexeme"<<"  ";
    *_token_stream<< std::setw(9)<<"TOKEN_TYPE"<<"  ";
    *_token_stream<< std::setw(6)<<"ATTR"<<"\n";
}

void BaseParser::setMemoryStream( std::shared_ptr<std::ostream> stream )
{
    _memory_stream = stream;
}
