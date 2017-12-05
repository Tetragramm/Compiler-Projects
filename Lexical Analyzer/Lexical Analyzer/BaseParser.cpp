#include "stdafx.h"
#include "BaseParser.h"
#include <iomanip>

bool BaseParser::match( const LexicalToken& m )
{
    if(check(m))
    {
        if(_tok.ta.token == END_OF_FILE)
            endParsing();
        else
        {
            _tok = getToken();
        }
        return true;
    }
    
    _output<<"Syntax Error at line "<<getLineNumber()<<": Expected "<<m<<",";
    _output<<" but Found "<<_tok<<"\n";
    _tok = getToken();
    return false;
}

bool BaseParser::match( const TAPair& m )
{
    if(check(m))
    {
        if(_tok.ta.token == END_OF_FILE)
            endParsing();
        else
        {
            _tok = getToken();
        }
        return true;
    }
    
    _output<<"Syntax Error at line "<<getLineNumber()<<": Expected "<<m<<",";
    _output<<" but Found "<<_tok.ta<<"\n";
    _tok = getToken();
    return false;
}

bool BaseParser::match( const TOKEN_TYPE& m )
{
    if(check(m))
    {
        if(_tok.ta.token == END_OF_FILE)
            endParsing();
        else
        {
            _tok = getToken();
        }
        return true;
    }
    
    _output<<"Syntax Error at line "<<getLineNumber()<<": Expected ("<<m<<"),";
    _output<<" but Found ("<<_tok.ta.token<<")\n";
    _tok = getToken();
    return false;
}

bool BaseParser::check( const LexicalToken& m ) const
{
    return (m == _tok);
}

bool BaseParser::check( const TAPair& m ) const
{
    return (m == _tok.ta);
}

bool BaseParser::check( const TOKEN_TYPE& m ) const
{
    return (m == _tok.ta.token);
}

void BaseParser::endParsing() const
{
    _output << "End of Parsing\n";
    _output.flush();
}


LexicalToken BaseParser::getToken()
{
    LexicalToken tok;
    while(!_file.eof())
    {
        do
        {
            tok = _machine->getToken();

            if(_listing_stream && tok.ta.token == LEXICAL_ERROR)
            {
                *_listing_stream<<tok<<"\n";
            }
            
            if(tok.ta != EMPTY_TOKEN)
            {
                if(_token_stream)
                {
                    *_token_stream<< std::setw(10)<<_line_number<<"  ";
                    *_token_stream<< std::setw(10)<<tok.lex<<"  ";
                    *_token_stream<< std::setw(14)<<getString(tok.ta.token)<<"  ";
                    *_token_stream<< std::setw(10)<<getString(tok.ta.attribute)<<"\n";
                }
                return tok;
            }
        } while( tok.ta != EMPTY_TOKEN);

        std::string line;
        getLine(line);

        _machine->setLine( line );
    }
    return LexicalToken("", TAPair(END_OF_FILE, NONE));
}

void BaseParser::synch( const std::vector< LexicalToken >& tokens )
{
    while(!_file.eof())
    {
        do
        {
            _tok = getToken();
            
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
    if(_listing_stream)
        *_listing_stream<<"Line Number "<<_line_number<<"\n";
}

void BaseParser::parse()
{
    std::string line;
    getLine(line);

    _machine->setLine( line );
}

void BaseParser::setTokenStream( std::shared_ptr<std::ostream> stream )
{
    _token_stream = stream;
    *_token_stream<< std::setw(10)<<"Line No."<<"  ";
    *_token_stream<< std::setw(10)<<"Lexeme"<<"  ";
    *_token_stream<< std::setw(14)<<"TOKEN_TYPE"<<"  ";
    *_token_stream<< std::setw(10)<<"ATTRIBUTE"<<"\n";
}

void BaseParser::setListingStream( std::shared_ptr<std::ostream> stream )
{
    _listing_stream = stream;
}
