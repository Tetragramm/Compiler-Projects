#include "stdafx.h"

#include <cctype>
#include <locale>
#include <utility>

#include "Machines.h"

using namespace std;

class TemplateMachine : public Machine
{
protected:
    vector< string > _words;

public:
    TemplateMachine() = default;

    explicit TemplateMachine( vector< string > words ) : _words( std::move( words ) ) {}

    TAPair runMachine( const string& line, int& idx ) const override
    {
        const int start_idx = idx;
        for ( auto& str : _words )
        {
            idx = start_idx;
            bool match = true;
            for ( auto& c : str )
            {
                if ( c != line[idx] )
                {
                    match = false;
                    break;
                }
                idx++;
            }
            if ( match )
                return TAPair(getTokenType(), NONE);
        }
        idx = start_idx;
        return EMPTY_TOKEN;
    }
    
    TOKEN_TYPE getTokenType() const override
    {
        return ID;
    }
};

class ReservedWordMachine : public TemplateMachine
{
public:
    ReservedWordMachine() = default;
    explicit ReservedWordMachine(const vector< string >& reserved_words) : TemplateMachine( reserved_words ) {}

    TAPair runMachine(const std::string& line, int& idx) const override
    {
        const int start_idx = idx;
        const TAPair temp = TemplateMachine::runMachine( line, idx );
        if(idx == line.length() || !isalnum(line[idx]))
        {
            return temp;
        }

        if(temp != TAPair())
        {
            idx = start_idx;
        }
        return TAPair();
    }

    void pushBack( const string& str )
    {
        _words.push_back( str );
    }
    
    TOKEN_TYPE getTokenType() const override
    {
        return RESERVED;
    }
};

class WhiteSpaceMachine : public TemplateMachine
{
public:
    WhiteSpaceMachine() : TemplateMachine( vector< string >{ " ", "\n", "\t" } )
    {}

    TAPair runMachine( const string& line, int& idx ) const override
    {
        bool found_ws = false;
        int my_idx = idx;
        while(TemplateMachine::runMachine( line, my_idx ) != EMPTY_TOKEN)
        {
            found_ws = true;
        }

        if(found_ws)
            idx = my_idx;

        return TAPair(getTokenType(), NONE);
    }
    
    TOKEN_TYPE getTokenType() const override
    {
        return WHITE_SPACE;
    }
};

class AddOpMachine : public TemplateMachine
{
public:
    AddOpMachine() : TemplateMachine( vector< string >{ "+", "-", "or" } )
    {}
    
    TOKEN_TYPE getTokenType() const override
    {
        return ADD_OP;
    }
};

class MulOpMachine : public TemplateMachine
{
public:
    MulOpMachine() : TemplateMachine( vector< string >{ "*", "/", "div", "mod", "and" } )
    {}
    
    TOKEN_TYPE getTokenType() const override
    {
        return MUL_OP;
    }
};

class RelOpMachine : public TemplateMachine
{
public:
    RelOpMachine() : TemplateMachine( vector< string >{ ">=", "<=", "<>",  "=", "<", ">", "not" } )
    {}
    
    TOKEN_TYPE getTokenType() const override
    {
        return REL_OP;
    }
};

class AssignOpMachine : public TemplateMachine
{
public:
    AssignOpMachine() : TemplateMachine( vector< string >{ ":=" } )
    {}
    
    TOKEN_TYPE getTokenType() const override
    {
        return ASSIGN_OP;
    }
};

class CommentMachine : public Machine
{
public:
    TAPair runMachine( const string& line, int& idx ) const override
    {
        if ( line[idx] == '{' )
        {
            for ( ; idx < line.length(); ++idx )
            {
                if ( line[idx] == '}' )
                {
                    idx++;
                    return TAPair(getTokenType(), NONE);
                }
            }
            return UNCLOSED_COMMENT;
        }
        return EMPTY_TOKEN;
    }
    
    TOKEN_TYPE getTokenType() const override
    {
        return COMMENT;
    }
};

class IntMachine : public Machine
{
public:
    TAPair runMachine( const string& line, int& idx ) const override
    {
        const int start_idx = idx;
        int digit_count = 0;

        bool leading_zero = false;
        while(idx < line.length() && isdigit(line[idx]))
        {
            //Leading Zero Check
            if(idx == start_idx && line[idx] == '0')
            {
                leading_zero = true;
            }
            
            idx++;
            digit_count++;
        }

        if(digit_count > 1 && leading_zero)
            return LEADING_ZERO;

        //If it's a real instead...
        if(idx < line.length() && line[idx] == '.')
        {
            idx = start_idx;
            return EMPTY_TOKEN;
        }

        if(digit_count > 10)
        {
            return LONG_INTEGER;
        }
        
        if(idx != start_idx)
            return TAPair(getTokenType(), NONE);

        idx = start_idx;
        return EMPTY_TOKEN;
    }
    
    TOKEN_TYPE getTokenType() const override
    {
        return INTEGER;
    }
};

class RealMachine : public Machine
{
public:
    TAPair runMachine( const string& line, int& idx ) const override
    {
        const int start_idx = idx;
        int digit_count = 0;
        bool leading_zero = false;
        bool trailing_zero = false;
        bool long_real = false;
        bool long_fraction = false;
        bool long_exponent = false;
        bool missing_exponent = false;
        bool zero_exponent = false;

        while(idx < line.length() && isdigit(line[idx]))
        {
            //Leading Zero Check
            if(idx == start_idx && line[idx] == '0')
            {
                leading_zero = true;
            }

            idx++;
            digit_count++;
        }

        if(!(digit_count > 1 && leading_zero))
            leading_zero = false;

        //If it's a real instead of an int
        //And actually is a number...
        if(idx < line.length() && digit_count != 0 && (line[idx] == '.' || line[idx] == 'E'))
        {
            if(digit_count > 5)
            {    
                long_real = true;
            }

            if(line[idx] == '.')
            {
                digit_count = 0;
                idx++;
            
                while(idx < line.length() && isdigit(line[idx]))
                {
                    idx++;
                    digit_count++;
                }
            
                //Trailing Zero check
                if(digit_count > 1 && line[idx-1] == '0')
                {
                    trailing_zero = true;
                }
            
                if(digit_count > 5)
                {    
                    long_fraction = true;
                }
            }

            if(idx < line.length() && line[idx] == 'E')
            {
                digit_count = 0;
                idx++;
                if(idx < line.length() && (line[idx] == '+' || line[idx] == '-'))
                {
                    idx++;
                }
                
                while(idx < line.length() && isdigit(line[idx]))
                {
                    if(digit_count == 0 && line[idx] == '0')
                    {
                        zero_exponent = true;
                    }
                    idx++;
                    digit_count++;
                }
                
                if(digit_count > 2)
                {    
                    long_exponent = true;
                }
                else if(digit_count == 0)
                {
                    missing_exponent = true;
                }
            }

            //Error Handling
            if(leading_zero)
                return LEADING_ZERO;
            if(long_real)
                return LONG_REAL;
            if(trailing_zero)
                return TRAILING_ZERO;
            if(long_fraction)
                return LONG_REAL_FRACTIONAL;
            if(long_exponent)
                return LONG_EXPONENT;
            if(missing_exponent)
                return MISSING_EXPONENT;
            if(zero_exponent)
                return LEADING_ZERO_EXPONENT;

            //No errors
            return TAPair(getTokenType(), NONE);
        }

        idx = start_idx;
        return EMPTY_TOKEN;
    }

    TOKEN_TYPE getTokenType() const override
    {
        return REAL;
    }
};

class SymbolMachine : public Machine
{
public:
    TAPair runMachine( const string& line, int& idx ) const override
    {
        switch(line[idx])
        {
        case '(':
        case ')':
        case ';':
        case ':':
        case '[':
        case ']':
        case ',':
        case '.':
            idx++;
            return TAPair(getTokenType(), NONE);
        default:
            return EMPTY_TOKEN;
        }
    }

    TOKEN_TYPE getTokenType() const override
    {
        return SYMBOL;
    }
};

class IDMachine : public Machine
{
    shared_ptr<SymbolTable>& _s_table;
public:
    IDMachine(shared_ptr<SymbolTable>& symbol_table): _s_table(symbol_table)
    {
    }

    TAPair runMachine( const string& line, int& idx ) const override
    {
        const int start_idx = idx;
        
        if(isalpha( line[idx] ))
        {
            idx++;
            while(idx < line.length() &&
                (isalpha( line[idx] ) || isdigit( line[idx] )))
            {
                idx++;
            }
            //Check ID Length
            if(idx-start_idx > 10)
            {
                return LONG_ID;
            }

            const int sTableIdx = _s_table->addOrReturn( line.substr( start_idx, idx-start_idx ) );
            return TAPair(getTokenType(), sTableIdx);
        }
        //Invalid ID
        return EMPTY_TOKEN;
    }

    TOKEN_TYPE getTokenType() const override
    {
        return ID;
    }
};

class CatchAllMachine : public Machine
{
    const vector< string > _ws_chars;
    
public:
    CatchAllMachine() : _ws_chars( { " ", "\n", "\t" } ) {}

    TAPair runMachine(const string& line, int& idx) const override
    {
        idx++;
        return UNKNOWN_SYMBOL;
    }

    TOKEN_TYPE getTokenType() const override
    {
        return LEXICAL_ERROR;
    }

private:
    bool matchesTemplate(const string& line, int idx) const
    {
        const int start_idx = idx;
        for ( auto& str : _ws_chars )
        {
            idx = start_idx;
            bool match = true;
            for ( auto& c : str )
            {
                if ( c != line[idx] )
                {
                    match = false;
                    break;
                }
                idx++;
            }
            if ( match )
                return false;
        }
        return true;
    }
};

class TypeMachine : public TemplateMachine
{
public:
    TypeMachine() : TemplateMachine({"integer", "real"}) {}

    TAPair runMachine(const std::string& line, int& idx) const override
    {
        const int start_idx = idx;
        const TAPair temp = TemplateMachine::runMachine( line, idx );
        if(idx == line.length() || !isalnum(line[idx]))
        {
            return temp;
        }

        if(temp != TAPair())
        {
            idx = start_idx;
        }
        return TAPair();
    }

    TOKEN_TYPE getTokenType() const override
    {
        return TYPE;
    }
};

PascalMachine::PascalMachine( const vector< string >& reserved_words, shared_ptr<SymbolTable>& symbol_table):_idx(-1), _s_table(symbol_table)
{
    _space_machines.push_back( make_shared<WhiteSpaceMachine>() );
    _space_machines.push_back( make_shared<CommentMachine>() );

    _useful_machines.push_back( make_shared<ReservedWordMachine>(reserved_words) );
    _useful_machines.push_back( make_shared<TypeMachine>() );
    _useful_machines.push_back( make_shared<AddOpMachine>() );
    _useful_machines.push_back( make_shared<MulOpMachine>() );
    _useful_machines.push_back( make_shared<RelOpMachine>() );
    _useful_machines.push_back( make_shared<AssignOpMachine>() );
    _useful_machines.push_back( make_shared<RealMachine>() );
    _useful_machines.push_back( make_shared<IntMachine>() );
    _useful_machines.push_back( make_shared<SymbolMachine>() );
    _useful_machines.push_back( make_shared<IDMachine>(_s_table) );
    _useful_machines.push_back( make_shared<CatchAllMachine>() );
}

void PascalMachine::setLine(const string& line)
{
    _line = line;
    _idx = 0;
}

LexicalToken PascalMachine::getToken()
{
    const int space_idx = _idx;
    const TAPair space = clearSpace();
    if(space != EMPTY_TOKEN)
    {
        return LexicalToken(_line.substr(space_idx, _idx-space_idx), space);
    }
        
    if(_idx != _line.length())
    {
        for(const auto& m : _useful_machines)
        {
            const int start_idx = _idx;
            const TAPair p = m->runMachine( _line, _idx );
            if(p != EMPTY_TOKEN)
            {
                return LexicalToken(_line.substr(start_idx, _idx-start_idx), p);
            }
        }
    }
    return LexicalToken("", EMPTY_TOKEN);
}

TAPair PascalMachine::clearSpace()
{
    int temp_idx;
    do
    {
        temp_idx = _idx;
        //Clear out comments and empty space
        for(const auto& m : _space_machines)
        {
            const TAPair res = m->runMachine( _line, _idx );
            if(res.token == LEXICAL_ERROR)
                return res;
        }
    } while(temp_idx != _idx && _idx != _line.length());

    return EMPTY_TOKEN;
}
