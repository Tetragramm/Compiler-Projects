#include "stdafx.h"

#include <cctype>

#include "Machines.h"

using namespace std;

class TemplateMachine : public Machine
{
protected:
    vector< string > _words;

public:
    TemplateMachine() = default;

    explicit TemplateMachine( const vector< string >& words ) : _words( words ) {}

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
    ReservedWordMachine() {}
    ReservedWordMachine(const vector< string >& reserved_words) : TemplateMachine( reserved_words ) {}

    void pushBack( const string& str )
    {
        _words.push_back( str );
    }
    
    TOKEN_TYPE getTokenType() const override
    {
        return RESERVED_WORD;
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
    RelOpMachine() : TemplateMachine( vector< string >{ ">=", "<=", "<>",  "=", "<", ">" } )
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

        bool leadingZero = false;
        while(idx < line.length() && isdigit(line[idx]))
        {
            //Leading Zero Check
            if(idx == start_idx && line[idx] == '0')
            {
                leadingZero = true;
            }
            
            idx++;
            digit_count++;
        }

        if(digit_count > 1 && leadingZero)
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
        bool leadingZero = false;
        bool trailingZero = false;
        bool longReal = false;
        bool longFraction = false;
        bool longExponent = false;
        bool missingExponent = false;
        bool zeroExponent = false;

        while(idx < line.length() && isdigit(line[idx]))
        {
            //Leading Zero Check
            if(idx == start_idx && line[idx] == '0')
            {
                leadingZero = true;
            }

            idx++;
            digit_count++;
        }

        if(!(digit_count > 1 && leadingZero))
            leadingZero = false;

        //If it's a real instead of an int
        //And actually is a number...
        if(idx < line.length() && digit_count != 0 && (line[idx] == '.' || line[idx] == 'E'))
        {
            if(digit_count > 5)
            {    
                longReal = true;
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
                    trailingZero = true;
                }
            
                if(digit_count > 5)
                {    
                    longFraction = true;
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
                        zeroExponent = true;
                    }
                    idx++;
                    digit_count++;
                }
                
                if(digit_count > 2)
                {    
                    longExponent = true;
                }
                else if(digit_count == 0)
                {
                    missingExponent = true;
                }
            }

            //Error Handling
            if(leadingZero)
                return LEADING_ZERO;
            if(longReal)
                return LONG_REAL;
            if(trailingZero)
                return TRAILING_ZERO;
            if(longFraction)
                return LONG_REAL_FRACTIONAL;
            if(longExponent)
                return LONG_EXPONENT;
            if(missingExponent)
                return MISSING_EXPONENT;
            if(zeroExponent)
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
    SymbolTable& sTable;
public:
    IDMachine(SymbolTable& symbol_table): sTable(symbol_table) {}

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

            const int sTableIdx = sTable.addOrReturn( line.substr( start_idx, idx-start_idx ) );
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
    const vector< string > wsChars;
    
public:
    CatchAllMachine() : wsChars( { " ", "\n", "\t" } ) {}

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
        for ( auto& str : wsChars )
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

    TOKEN_TYPE getTokenType() const override
    {
        return TYPE;
    }
};

PascalMachine::PascalMachine( const vector< string >& reserved_words, SymbolTable& symbol_table):_idx(-1), _s_table(symbol_table)
{
    _space_machines.push_back( make_unique<WhiteSpaceMachine>() );
    _space_machines.push_back( make_unique<CommentMachine>() );

    _useful_machines.push_back( make_unique<ReservedWordMachine>(reserved_words) );
    _useful_machines.push_back( make_unique<TypeMachine>() );
    _useful_machines.push_back( make_unique<AddOpMachine>() );
    _useful_machines.push_back( make_unique<MulOpMachine>() );
    _useful_machines.push_back( make_unique<RelOpMachine>() );
    _useful_machines.push_back( make_unique<AssignOpMachine>() );
    _useful_machines.push_back( make_unique<RealMachine>() );
    _useful_machines.push_back( make_unique<IntMachine>() );
    _useful_machines.push_back( make_unique<SymbolMachine>() );
    _useful_machines.push_back( make_unique<IDMachine>(_s_table) );
    _useful_machines.push_back( make_unique<CatchAllMachine>() );
}

void PascalMachine::setLine(const string& line)
{
    _line = line;
    _idx = 0;
}

LexicalToken PascalMachine::getToken()
{
    const int space_idx = _idx;
    TAPair space = clearSpace();
    if(space != EMPTY_TOKEN)
    {
        return LexicalToken(_line.substr(space_idx, _idx-space_idx), space);
    }
        
    if(_idx != _line.length())
    {
        for(const auto& m : _useful_machines)
        {
            const int start_idx = _idx;
            TAPair p = m->runMachine( _line, _idx );
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
            TAPair res = m->runMachine( _line, _idx );
            if(res.token == LEXICAL_ERROR)
                return res;
        }
    } while(temp_idx != _idx && _idx != _line.length());

    return EMPTY_TOKEN;
}
