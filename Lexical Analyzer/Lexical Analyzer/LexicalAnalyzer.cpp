// LexicalAnalyzer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <iostream>
#include <cctype>
#include <memory>

class TokenMachine
{
private:
    std::regex _regex;
    std::string _token_id;
public:
    TokenMachine( std::string regex, std::string token_id ): _regex( regex )
                                                           , _token_id( token_id )
    {}
    TokenMachine( std::regex regex, std::string token_id ): _regex( regex )
                                                           , _token_id( token_id )
    {}

    std::string filter( std::string str )
    {
        return std::regex_replace( str, _regex, _token_id, std::regex_constants::format_default );
    }
};

std::vector<TokenMachine> machines(
{
    TokenMachine("\\{.*\\}", "")
    , TokenMachine("\\,|;|\\(|\\)|\\[|\\]|:|=|<|>|\\*|/", " $& ")
    , TokenMachine("([^E])(\\+|\\-)", "$1 $2 ")
});

std::string cleanInput( std::string str )
{
    for(auto& m : machines)
    {
        str = m.filter( str );
    }
    return str;
}

class Machine
{
public:
    virtual ~Machine() = default;
    virtual bool run_machine( const std::string& line, int& idx ) const = 0;
    virtual int unit_test() const = 0;

protected:
    static int run_tests(Machine& machine, 
        std::vector<std::pair<std::string,int>>& test_input, 
        std::vector<std::pair<bool, int>>& test_results)
    {
        for(int i = 0; i<test_input.size(); ++i)
        {
            const bool resb = machine.run_machine( test_input[i].first, test_input[i].second);
            if(resb != test_results[i].first && test_input[i].second != test_results[i].second)
                return i;
        }

        return -1;
    }
};

class TemplateMachine : public Machine
{
protected:
    std::vector< std::string > _words;

public:
    TemplateMachine() = default;

    explicit TemplateMachine( const std::vector< std::string >& words ) : _words( words ) {}

    bool run_machine( const std::string& line, int& idx ) const override
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
                return true;
        }
        idx = start_idx;
        return false;
    }
};

class ReservedWordMachine : public TemplateMachine
{
public:
    void push_back( const std::string& str )
    {
        _words.push_back( str );
    }

    int unit_test() const override
    {
        using namespace std;
        ReservedWordMachine test;
        test.push_back( "program" );
        vector<pair<string, int>> test_input({
            pair<string, int>("program   ", 0),
            pair<string, int>("progra m  ", 0),
            pair<string, int>("  program", 0),
            pair<string, int>("testprogram", 0),
            pair<string, int>("programtest", 0),
            pair<string, int>("testprogram", 4),
            pair<string, int>("programtest", 4),
            });
        vector<pair<bool, int>> test_results({
            pair<bool, int>(true, 7),
            pair<bool, int>(false, 0),
            pair<bool, int>(false, 0),
            pair<bool, int>(false, 0),
            pair<bool, int>(true, 7),
            pair<bool, int>(true, 11),
            pair<bool, int>(false, 4),
            });
        return run_tests( test, test_input, test_results );
    }
};

class WhiteSpaceMachine : public TemplateMachine
{
public:
    WhiteSpaceMachine() : TemplateMachine( std::vector< std::string >{ " ", "\n", "\t" } )
    {}

    bool run_machine(const std::string& line, int& idx) const override
    {
        bool foundWS = false;
        int myIdx = idx;
        while(TemplateMachine::run_machine( line, myIdx ))
        {
            foundWS = true;
        }

        if(foundWS)
            idx = myIdx;

        return foundWS;
    }

    int unit_test() const override
    {
        using namespace std;
        WhiteSpaceMachine test;
        vector<pair<string, int>> test_input({
            pair<string, int>("program   ", 0),
            pair<string, int>("progra m  ", 0),
            pair<string, int>("  program", 0),
            pair<string, int>("\testprogram", 0),
            pair<string, int>("\n  programtest", 0),
            pair<string, int>("test program", 4),
            pair<string, int>("programtest", 4),
            });
        vector<pair<bool, int>> test_results({
            pair<bool, int>(false, 0),
            pair<bool, int>(false, 0),
            pair<bool, int>(true, 2),
            pair<bool, int>(true, 1),
            pair<bool, int>(true, 3),
            pair<bool, int>(true, 5),
            pair<bool, int>(false, 4),
            });
        return run_tests( test, test_input, test_results );
    }
};

class AddOpMachine : public TemplateMachine
{
public:
    AddOpMachine() : TemplateMachine( std::vector< std::string >{ "+", "-", "or" } )
    {}

    int unit_test() const override
    {
        using namespace std;
        AddOpMachine test;
        vector<pair<string, int>> test_input({
            pair<string, int>("+words   ", 0),
            pair<string, int>("+ 123  ", 0),
            pair<string, int>("123 - 5", 0),
            pair<string, int>(" +123", 0),
            pair<string, int>("or number", 0),
            pair<string, int>("123 or 567", 4),
            pair<string, int>("123+567", 4),
            });
        vector<pair<bool, int>> test_results({
            pair<bool, int>(true, 1),
            pair<bool, int>(true, 1),
            pair<bool, int>(false, 0),
            pair<bool, int>(false, 0),
            pair<bool, int>(true, 2),
            pair<bool, int>(true, 6),
            pair<bool, int>(false, 4),
            });
        return run_tests( test, test_input, test_results );
    }
};

class MulOpMachine : public TemplateMachine
{
public:
    MulOpMachine() : TemplateMachine( std::vector< std::string >{ "*", "/", "div", "mod", "and" } )
    {}

    int unit_test() const override
    {
        using namespace std;
        MulOpMachine test;
        vector<pair<string, int>> test_input({
            pair<string, int>("*words   ", 0),
            pair<string, int>("* 123  ", 0),
            pair<string, int>("123 / 5", 0),
            pair<string, int>(" *123", 0),
            pair<string, int>("div number", 0),
            pair<string, int>("123 and 567", 4),
            pair<string, int>("123+567", 3),
            });
        vector<pair<bool, int>> test_results({
            pair<bool, int>(true, 1),
            pair<bool, int>(true, 1),
            pair<bool, int>(false, 0),
            pair<bool, int>(false, 0),
            pair<bool, int>(true, 3),
            pair<bool, int>(true, 7),
            pair<bool, int>(false, 3),
            });
        return run_tests( test, test_input, test_results );
    }
};

class RelOpMachine : public TemplateMachine
{
public:
    RelOpMachine() : TemplateMachine( std::vector< std::string >{ "=", "<", ">", ">=", "<=", "<>" } )
    {}

    int unit_test() const override
    {
        using namespace std;
        RelOpMachine test;
        vector<pair<string, int>> test_input({
            pair<string, int>("=words   ", 0),
            pair<string, int>("< 123  ", 0),
            pair<string, int>("123 ? 5", 0),
            pair<string, int>(" <=123", 0),
            pair<string, int>(">= number", 0),
            pair<string, int>("123 <> 567", 4),
            pair<string, int>("123+567", 3),
            });
        vector<pair<bool, int>> test_results({
            pair<bool, int>(true, 1),
            pair<bool, int>(true, 1),
            pair<bool, int>(false, 0),
            pair<bool, int>(false, 0),
            pair<bool, int>(true, 2),
            pair<bool, int>(true, 6),
            pair<bool, int>(false, 3),
            });
        return run_tests( test, test_input, test_results );
    }
};

class AssignOpMachine : public TemplateMachine
{
public:
    AssignOpMachine() : TemplateMachine( std::vector< std::string >{ ":=" } )
    {}

    int unit_test() const override
    {
        using namespace std;
        AssignOpMachine test;
        vector<pair<string, int>> test_input({
            pair<string, int>(":=words   ", 0),
            pair<string, int>("< 123  ", 0),
            pair<string, int>("word := number", 5),
            pair<string, int>(" <=123", 0),
            pair<string, int>(":>= number", 0),
            pair<string, int>("123 := 567", 4),
            pair<string, int>("123=567", 3),
            });
        vector<pair<bool, int>> test_results({
            pair<bool, int>(true, 2),
            pair<bool, int>(false, 0),
            pair<bool, int>(true, 7),
            pair<bool, int>(false, 0),
            pair<bool, int>(false, 0),
            pair<bool, int>(true, 6),
            pair<bool, int>(false, 3),
            });
        return run_tests( test, test_input, test_results );
    }
};

class CommentMachine : public Machine
{
public:
    bool run_machine( const std::string& line, int& idx ) const override
    {
        if ( line[idx] == '{' )
        {
            for ( ; idx < line.length(); ++idx )
            {
                if ( line[idx] == '}' )
                {
                    idx++;
                    return true;
                }
            }
            //TODO: Throw error unclosed comment
        }
        return false;
    }

    int unit_test() const override
    {
        using namespace std;
        CommentMachine test;
        vector<pair<string, int>> test_input({
            pair<string, int>("{testing}   ", 0),
            pair<string, int>("{< 123  ", 0),
            pair<string, int>("123{} ? 5", 0),
            pair<string, int>("} <=123", 0),
            pair<string, int>("{}>= number", 0),
            pair<string, int>("123 {}<> 567", 4),
            pair<string, int>("1{23+5}67", 3),
            });
        vector<pair<bool, int>> test_results({
            pair<bool, int>(true, 9),
            pair<bool, int>(true, 8),
            pair<bool, int>(false, 0),
            pair<bool, int>(false, 0),
            pair<bool, int>(true, 2),
            pair<bool, int>(true, 6),
            pair<bool, int>(false, 3),
            });
        return run_tests( test, test_input, test_results );
    }
};

class IntMachine : public Machine
{
public:
    bool run_machine( const std::string& line, int& idx ) const override
    {
        const int start_idx = idx;
        int digit_count = 0;

        while(idx < line.length() && std::isdigit(line[idx]))
        {
            //TODO: Leading Zero Check
            idx++;
            digit_count++;
        }
        //If it's a real instead...
        if(idx < line.length() && line[idx] == '.')
        {
            idx = start_idx;
            return false;
        }

        if(digit_count > 10)
        {
            //TODO: Throw error int too long
        }
        
        if(idx != start_idx)
            return true;

        idx = start_idx;
        return false;
    }

    int unit_test() const override
    {
        using namespace std;
        IntMachine test;
        vector<pair<string, int>> test_input({
            pair<string, int>("123456789 ", 0),
            pair<string, int>("1234  ", 0),
            pair<string, int>("12345.", 0),
            pair<string, int>(" 1234", 0),
            pair<string, int>("123.456", 0),
            pair<string, int>("123456789", 4),
            pair<string, int>("1  {23+5}67", 3),
            pair<string, int>("1123456789123456789", 0),
            });
        vector<pair<bool, int>> test_results({
            pair<bool, int>(true, 9),
            pair<bool, int>(true, 4),
            pair<bool, int>(false, 0),
            pair<bool, int>(false, 0),
            pair<bool, int>(false, 0),
            pair<bool, int>(true, 5),
            pair<bool, int>(false, 3),
            pair<bool, int>(true, 19),
            });
        return run_tests( test, test_input, test_results );
    }
};

class RealMachine : public Machine
{
public:
    bool run_machine( const std::string& line, int& idx ) const override
    {
        const int start_idx = idx;
        int digit_count = 0;

        while(idx < line.length() && std::isdigit(line[idx]))
        {
            //TODO: Leading Zero Check
            idx++;
            digit_count++;
        }

        if(digit_count > 5)
        {    
            //TODO: Throw error number too long
        }

        //If it's a real instead...
        if(idx < line.length() && line[idx] == '.')
        {
            digit_count = 0;
            idx++;
            
            while(idx < line.length() && std::isdigit(line[idx]))
            {
                //TODO: Trailing Zero Check
                idx++;
                digit_count++;
            }
            
            if(digit_count > 5)
            {    
                //TODO: Throw error number too long
            }

            if(idx < line.length() && line[idx] == 'E')
            {
                digit_count = 0;
                idx++;
                if(idx < line.length() && (line[idx] == '+' || line[idx] == '-'))
                {
                    idx++;
                }
                
                while(idx < line.length() && std::isdigit(line[idx]))
                {
                    idx++;
                    digit_count++;
                }
                
                if(digit_count > 3)
                {    
                    //TODO: Throw error number too long
                }
                else if(digit_count == 0)
                {
                    idx = start_idx;
                    return false;
                }
            }

            return true;
        }

        idx = start_idx;
        return false;
    }

    int unit_test() const override
    {
        using namespace std;
        RealMachine test;
        vector<pair<string, int>> test_input({
            pair<string, int>("12.34 ", 0),
            pair<string, int>("12.300  ", 0),
            pair<string, int>("12345", 0),
            pair<string, int>(" 12.34", 0),
            pair<string, int>("12.45E6", 0),
            pair<string, int>("123456.789", 4),
            pair<string, int>("1 {23+5}67", 3),
            pair<string, int>("12.34E-5", 0),
            });
        vector<pair<bool, int>> test_results({
            pair<bool, int>(true, 5),
            pair<bool, int>(true, 6),
            pair<bool, int>(false, 0),
            pair<bool, int>(false, 0),
            pair<bool, int>(true, 7),
            pair<bool, int>(false, 10),
            pair<bool, int>(false, 3),
            pair<bool, int>(true, 8),
            });
        return run_tests( test, test_input, test_results );
    }
};

int main()
{
    using namespace std;
    vector<unique_ptr<Machine>> tests;

    tests.push_back(std::make_unique< WhiteSpaceMachine >());
    tests.push_back(std::make_unique< ReservedWordMachine >());
    tests.push_back(std::make_unique< AddOpMachine >());
    tests.push_back(std::make_unique< MulOpMachine >());
    tests.push_back(std::make_unique< RelOpMachine >());
    tests.push_back(std::make_unique< AssignOpMachine >());
    tests.push_back(std::make_unique< CommentMachine >());
    tests.push_back(std::make_unique< IntMachine >());
    tests.push_back(std::make_unique< RealMachine >());

	for(auto& m : tests)
	{
		cout<<m->unit_test()<<"\n";
	}

    return 0;
}
