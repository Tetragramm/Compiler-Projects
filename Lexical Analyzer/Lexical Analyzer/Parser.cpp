#include "stdafx.h"

#include "Parser.h"
#include "Machines.h"
#include "BaseParser.h"
#include "Scope.h"

using std::ifstream;
using std::string;
using std::make_shared;
using std::variant;
using std::vector;
using std::monostate;
using std::holds_alternative;
using std::get;
using std::get_if;
using std::shared_ptr;

#define EOF_TOK LexicalToken("", TAPair(END_OF_FILE,NONE))

class PascalParser : public BaseParser
{
public:
    PascalParser( ifstream& r_words_file, ifstream& file, std::ostream& output ) :
                                                                                 BaseParser( file, output )
    {
        vector< string > r_words;
        while ( !r_words_file.eof() )
        {
            string word;
            getline( r_words_file, word );
            r_words.push_back( word );
        }
        r_words_file.close();

        _machine = make_shared< PascalMachine >( r_words, _table );
    }

    void testTable() override
    {
        _table->addOrReturn( "Test" );
    }

    void parse() override;

private:

    struct Type_Ext
    {
        ParInfo info;
    };

    struct Standard_Type_Ext
    {
        E_TYPE type;
    };

    struct SP_Head_Ext
    {
        FuncInfo f_info;
    };

    struct Arg_Ext
    {
        vector< ParInfo > list;
    };

    struct Par_List_Ext
    {
        vector< ParInfo > list;
    };

    struct Exp_Ext
    {
        ParInfo info;
    };

    void program();
    void program_2();
    void program_3();
    void program_4();
    void identifier_list();
    void identifier_list_2();
    void declarations();
    void declarations_2();
    void subprogram_declarations();
    void subprogram_declarations_2();
    void compound_statement();
    void compound_statement_2();
    Type_Ext type();
    Standard_Type_Ext standard_type();
    void subprogram_declaration();
    void subprogram_declaration_2();
    void subprogram_declaration_3();

    struct Sub_Head_Ext
    {
        unsigned int id{};
        FuncInfo info;
    };

    Sub_Head_Ext subprogram_head();
    SP_Head_Ext subprogram_head_2();
    void optional_statements();
    Arg_Ext arguments();
    void statement_list();
    void statement_list_2();
    Par_List_Ext parameter_list();
    Par_List_Ext parameter_list_2();
    void statement();
    void statement_2();
    VarInfo variable();
    VarInfo variable_2( const Info& intr );
    Exp_Ext expression();
    Exp_Ext expression_2( Exp_Ext& intr );
    Exp_Ext simple_expression();
    Exp_Ext simple_expression_2( Exp_Ext& intr );
    Exp_Ext term();
    Exp_Ext term_2( Exp_Ext intr );
    void sign();
    bool checkParameters( vector< ParInfo >& actual, vector< ParInfo >& found ) const;

    Exp_Ext factor();

    typedef vector<ParInfo> Exp_List_Ext;

    typedef variant<monostate,Exp_List_Ext,VarInfo> Fac_2_Ext;

    Exp_Ext factor_prod_1( unsigned id_idx );
    Fac_2_Ext factor_2();

    Exp_List_Ext expression_list();
    Exp_List_Ext expression_list_2();
};

void PascalParser::parse()
{
    BaseParser::parse();
    //_output<<"Begin Parse\n";
    _tok = getToken();
    program();
    match( END_OF_FILE, __FUNCTION__ );
    _scope->print( *_memory_stream, *_table );
}

void PascalParser::program()
{
    _scope = make_shared< Scope >( "Program Scope" );
    //_output<<"Program -> program id ( identifier_list ) ; program_2\n";
    if ( check( LexicalToken( "program", RESERVED ) ) )
    {
        if ( match( LexicalToken( "program", RESERVED ), __FUNCTION__ )
             && match( ID, __FUNCTION__ )
             && match( LexicalToken( "(", SYMBOL ), __FUNCTION__ ) )
        {
            identifier_list();
            if ( match( LexicalToken( ")", SYMBOL ), __FUNCTION__ )
                 && match( LexicalToken( ";", SYMBOL ), __FUNCTION__ ) )
            {
                program_2();
                //I want to short-circuit the other symbols if an early one breaks.
                return;
            }
        }
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                LexicalToken( "program", RESERVED ) << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
    } );
}

void PascalParser::program_2()
{
    //_output<<"program_2 -> ";
    if ( check( LexicalToken( "var", RESERVED ) ) )
    {
        //_output<<"declarations \\n program_3 \n";
        declarations();
        program_3();
        return;
    }
    if ( check( LexicalToken( "function", RESERVED ) )
         || check( LexicalToken( "begin", RESERVED ) ) )
    {
        //_output<<"program_3\n";
        program_3();
        return;
    }

    _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
            LexicalToken( "var", RESERVED ) << ", " <<
            LexicalToken( "function", RESERVED ) << ", " <<
            LexicalToken( "begin", RESERVED ) << ", " <<
            "but found " << _tok << "\n";

    synch( {
        EOF_TOK,
    } );
}

void PascalParser::program_3()
{
    //_output<<"program_3 -> ";
    if ( check( LexicalToken( "function", RESERVED ) ) )
    {
        //_output<<"subprogram_declarations \\n program_4\n";
        subprogram_declarations();
        program_4();
        return;
    }
    if ( check( LexicalToken( "begin", RESERVED ) ) )
    {
        //_output<<"program_4\n";
        program_4();
        return;
    }

    _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
            LexicalToken( "function", RESERVED ) << ", " <<
            LexicalToken( "begin", RESERVED ) << ", " <<
            "but found " << _tok << "\n";

    synch( {
        EOF_TOK,
    } );
}

void PascalParser::program_4()
{
    //_output<<"program_4 -> ";
    if ( check( LexicalToken( "begin", RESERVED ) ) )
    {
        //_output<<"compound_statement \\n .\n";
        compound_statement();
        if ( match( LexicalToken( ".", SYMBOL ), __FUNCTION__ ) )
        {
            return;
        }
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                LexicalToken( "begin", RESERVED ) << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
    } );
}

void PascalParser::identifier_list()
{
    if ( check( ID ) )
    {
        //_output<<"identifier_list -> id identifier_list_2\n";
        if ( match( ID, __FUNCTION__ ) )
        {
            identifier_list_2();
            return;
        }
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                ID << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
        LexicalToken( ")", SYMBOL ),
    } );
}

void PascalParser::identifier_list_2()
{
    //_output<<"identifier_list_2 -> ";
    if ( check( LexicalToken( ",", SYMBOL ) ) )
    {
        //_output<<", id identifier_list_2\n";
        if ( match( LexicalToken( ",", SYMBOL ), __FUNCTION__ )
             && match( ID, __FUNCTION__ ) )
        {
            identifier_list_2();
            return;
        }
    }
    else if ( check( LexicalToken( ")", SYMBOL ) ) )
    {
        //_output<<"epsilon\n";
        return;
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                LexicalToken( ",", SYMBOL ) << ", " <<
                LexicalToken( ")", SYMBOL ) << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
        LexicalToken( ")", SYMBOL ),
    } );
}

void PascalParser::declarations()
{
    if ( check( LexicalToken( "var", RESERVED ) ) )
    {
        unsigned id;
        auto ln = getLineNumber();
        //_output<<"declarations -> var id : info ; declarations_2\n";
        if ( match( LexicalToken( "var", RESERVED ), __FUNCTION__ )
             && getIdSymbol( id, __FUNCTION__ )
             && match( LexicalToken( ":", SYMBOL ), __FUNCTION__ ) )
        {
            bool reDeclare = false;
            if(_scope->checkVariable(id))
            {
                reDeclare = true;
                _output << "Re-declaring variable "<<_table->get( id).lex<< " at line "<< ln << " in " <<__FUNCTION__<<"\n";
            }
            Type_Ext type_ext = type();
            if ( match( LexicalToken( ";", SYMBOL ), __FUNCTION__ ) )
            {
                if( const auto errq = get_if<VarInfo>(&type_ext.info))
                {
                    if(errq->type == T_ERROR)
                        type_ext.info = VarInfo(T_ERROR);
                }

                if(!reDeclare)
                    _scope->addVariable( id, type_ext.info );

                declarations_2();
                //var id : info ; declarations_2
                return;
            }
        }
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                LexicalToken( ",", SYMBOL ) << ", " <<
                LexicalToken( ")", SYMBOL ) << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
        LexicalToken( "function", RESERVED ),
        LexicalToken( "begin", RESERVED ),
    } );
}

void PascalParser::declarations_2()
{
    //_output<<"declarations_2 -> ";
    if ( check( LexicalToken( "var", RESERVED ) ) )
    {
        //_output<<"declarations\n";
        declarations();
        return;
    }
    if ( check( LexicalToken( "function", RESERVED ) )
         || check( LexicalToken( "begin", RESERVED ) ) )
    {
        //_output<<"epsilon\n";
        return;
    }

    _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
            LexicalToken( "var", RESERVED ) << ", " <<
            LexicalToken( "function", RESERVED ) << ", " <<
            LexicalToken( "begin", RESERVED ) << ", " <<
            "but found " << _tok << "\n";

    synch( {
        EOF_TOK,
        LexicalToken( "function", RESERVED ),
        LexicalToken( "begin", RESERVED ),
    } );
}

void PascalParser::subprogram_declarations()
{
    //_output<<"subprogram_declarations -> subprogram_declaration ; subprogram_declarations_2\n";
    if ( check( LexicalToken( "function", RESERVED ) ) )
    {
        //subprogram_declaration ; subprogram_declarations_2
        subprogram_declaration();
        if ( match( LexicalToken( ";", SYMBOL ), __FUNCTION__ ) )
        {
            subprogram_declarations_2();
            return;
        }
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                LexicalToken( "function", RESERVED ) << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
        LexicalToken( "begin", RESERVED ),
    } );
}

void PascalParser::subprogram_declarations_2()
{
    //_output<<"subprogram_declarations_2 -> ";
    if ( check( LexicalToken( "function", RESERVED ) ) )
    {
        //_output<<"subprogram_declarations\n";
        subprogram_declarations();
        return;
    }
    if ( check( LexicalToken( "begin", RESERVED ) ) )
    {
        //_output<<"epsilon\n";
        return;
    }

    _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
            LexicalToken( "function", RESERVED ) << ", " <<
            LexicalToken( "begin", RESERVED ) << ", " <<
            "but found " << _tok << "\n";

    synch( {
        EOF_TOK,
        LexicalToken( "begin", RESERVED ),
    } );
}

void PascalParser::compound_statement()
{
    //_output<<"compound_statement -> begin \\n compound_statement_2 \n";
    if ( check( LexicalToken( "begin", RESERVED ) ) )
    {
        if ( match( LexicalToken( "begin", RESERVED ), __FUNCTION__ ) )
        {
            compound_statement_2();
            return;
        }
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                LexicalToken( "begin", RESERVED ) << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
        LexicalToken( ";", SYMBOL ),
        LexicalToken( "end", RESERVED ),
        LexicalToken( ".", SYMBOL ),
        LexicalToken( "else", RESERVED ),
    } );
}

void PascalParser::compound_statement_2()
{
    //_output<<"compound_statement_2 -> ";
    //optional_statements \n end | \n end
    if ( check( ID )
         || check( LexicalToken( "begin", RESERVED ) )
         || check( LexicalToken( "if", RESERVED ) )
         || check( LexicalToken( "while", RESERVED ) ) )
    {
        //_output<<"optional_statements \\n end\n";
        optional_statements();
        if ( match( LexicalToken( "end", RESERVED ), __FUNCTION__ ) )
        {
            return;
        }
    }
    else if( check(LexicalToken("end", RESERVED)))
    {
        match(LexicalToken("end", RESERVED), __FUNCTION__ );
        //_output<<"\\n end\n";
        return;
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                ID << ", " <<
                LexicalToken( "begin", RESERVED ) << ", " <<
                LexicalToken( "if", RESERVED ) << ", " <<
                LexicalToken( "while", RESERVED ) << ", " <<
                LexicalToken( "end", RESERVED ) << ", " <<
                "but found " << _tok << "\n";
    }
    synch( {
        EOF_TOK,
        LexicalToken( ";", SYMBOL ),
        LexicalToken( "end", RESERVED ),
        LexicalToken( ".", SYMBOL ),
        LexicalToken( "else", RESERVED ),
    } );
}

PascalParser::Type_Ext PascalParser::type()
{
    Type_Ext ext;
    //_output<<"info -> ";
    //standard_type | array [ num .. num ] of standard_type
    if ( check( LexicalToken( "integer", TYPE ) )
         || check( LexicalToken( "real", TYPE ) ) )
    {
        //_output<<"standard_type\n";
        const Standard_Type_Ext st_ext = standard_type();
        ext.info = VarInfo( st_ext.type );
        return ext;
    }
    if ( check( LexicalToken( "array", RESERVED ) ) )
    {
        ArrayInfo info;
        int start, stop;
        if ( match( LexicalToken( "array", RESERVED ), __FUNCTION__ )
             && match( LexicalToken( "[", SYMBOL ), __FUNCTION__ )
             && getNum( start, __FUNCTION__ )
             && match( LexicalToken( ".", SYMBOL ), __FUNCTION__ )
             && match( LexicalToken( ".", SYMBOL ), __FUNCTION__ )
             && getNum( stop, __FUNCTION__ )
             && match( LexicalToken( "]", SYMBOL ), __FUNCTION__ )
             && match( LexicalToken( "of", RESERVED ), __FUNCTION__ ) )
        {
            const Standard_Type_Ext st_ext = standard_type();
            //_output<<"array [ num .. num ] of standard_type\n";
            info.access_offset = start;
            info.count = stop - start;
            info.type = st_ext.type;
            ext.info = info;
            return ext;
        }
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                LexicalToken( "integer", TYPE ) << ", " <<
                LexicalToken( "real", TYPE ) << ", " <<
                LexicalToken( "array", RESERVED ) << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
        LexicalToken( ";", SYMBOL ),
        LexicalToken( ")", SYMBOL ),
    } );
    ext.info = VarInfo( T_ERROR );
    return ext;
}

PascalParser::Standard_Type_Ext PascalParser::standard_type()
{
    Standard_Type_Ext ext{T_ERROR};
    //_output<<"standard_type -> ";
    //integer | real
    if ( check( LexicalToken( "integer", TYPE ) ) )
    {
        //_output<<"integer\n";
        if ( match( LexicalToken( "integer", TYPE ), __FUNCTION__ ) )
        {
            ext.type = T_INTEGER;
            return ext;
        }
    }
    else if ( check( LexicalToken( "real", TYPE ) ) )
    {
        //_output<<"real\n";
        if ( match( LexicalToken( "real", TYPE ), __FUNCTION__ ) )
        {
            ext.type = T_REAL;
            return ext;
        }
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                LexicalToken( "integer", TYPE ) << ", " <<
                LexicalToken( "real", TYPE ) << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
        LexicalToken( ";", SYMBOL ),
        LexicalToken( ")", SYMBOL ),
    } );
    return ext;
}

void PascalParser::subprogram_declaration()
{
    //_output<<"subprogram_declaration -> ";
    //subprogram_head subprogram_declaration_2
    if ( check( LexicalToken( "function", RESERVED ) ) )
    {
        //_output<<"subprogram_head subprogram_declaration_2\n";
        auto ln = getLineNumber();
        _scope = _scope->newScope( "Temp Scope" );
        const Sub_Head_Ext head = subprogram_head();

        const bool reDeclare = _scope->getParent()->checkVariable( head.id);
        if(reDeclare)
        {
            _output << "Re-declaring function "<<_table->get( head.id ).lex<< " at line "<< ln << " in " <<__FUNCTION__<<"\n";
        }

        _scope->setName(_table->get( head.id ).lex + " Scope");
        subprogram_declaration_2();
        if(!reDeclare)
        {
            _scope->getParent()->addVariable( head.id, head.info );
            _scope->print( *_memory_stream, *_table );
        }
        _scope = _scope->getParent();
        return;
    }

    _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
            LexicalToken( "function", RESERVED ) << ", " <<
            "but found " << _tok << "\n";

    synch( {
        EOF_TOK,
        LexicalToken( ";", SYMBOL ),
    } );
}

void PascalParser::subprogram_declaration_2()
{
    //_output<<"subprogram_declaration_2 -> ";
    //declarations subprogram_declaration_3 | subprogram_declaration_3
    if ( check( LexicalToken( "var", RESERVED ) ) )
    {
        //_output<<"declarations subprogram_declaration_3\n";
        declarations();
        subprogram_declaration_3();
        return;
    }
    if ( check( LexicalToken( "begin", RESERVED ) )
         || check( LexicalToken( "function", RESERVED ) ) )
    {
        //_output<<"subprogram_declaration_3\n";
        subprogram_declaration_3();
        return;
    }

    _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
            LexicalToken( "var", RESERVED ) << ", " <<
            LexicalToken( "begin", RESERVED ) << ", " <<
            LexicalToken( "function", RESERVED ) << ", " <<
            "but found " << _tok << "\n";

    synch( {
        EOF_TOK,
        LexicalToken( ";", SYMBOL ),
    } );
}

void PascalParser::subprogram_declaration_3()
{
    //_output<<"subprogram_declaration_3 -> ";
    //subprogram_declarations compound_statement | compound_statement
    if ( check( LexicalToken( "begin", RESERVED ) ) )
    {
        //_output<<"compound_statement\n";
        compound_statement();
        return;
    }

    if ( check( LexicalToken( "function", RESERVED ) ) )
    {
        //_output<<"subprogram_declarations compound_statement\n";
        subprogram_declarations();
        compound_statement();
        return;
    }

    _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
            LexicalToken( "begin", RESERVED ) << ", " <<
            LexicalToken( "function", RESERVED ) << ", " <<
            "but found " << _tok << "\n";

    synch( {
        EOF_TOK,
        LexicalToken( ";", SYMBOL ),
    } );
}

PascalParser::Sub_Head_Ext PascalParser::subprogram_head()
{
    Sub_Head_Ext ext;
    if ( check( LexicalToken( "function", RESERVED ) ) )
    {
        //_output<<"subprogram_head -> function id subprogram_head_2\n";
        unsigned id;
        if ( match( LexicalToken( "function", RESERVED ), __FUNCTION__ )
             && getIdSymbol( id, __FUNCTION__ ) )
        {
            auto ln = getLineNumber();
            SP_Head_Ext sp2_ext = subprogram_head_2();
            const Info f_info = sp2_ext.f_info;
            if(!_scope->addVariable( id, f_info ))
            {
                _output << "Re-declaring function "<<_table->get( id).lex<< " at line "<< ln << " in " <<__FUNCTION__<<"\n";
            }
            ext.info = sp2_ext.f_info;
            ext.id = id;
            return ext;
        }
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                LexicalToken( "function", RESERVED ) << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
        LexicalToken( "var", RESERVED ),
        LexicalToken( "function", RESERVED ),
        LexicalToken( "begin", RESERVED ),
    } );
    ext.id = 999;
    ext.info = FuncInfo();
    return ext;
}

PascalParser::SP_Head_Ext PascalParser::subprogram_head_2()
{
    SP_Head_Ext ext;
    //_output<<"subprogram_head_2 -> ";
    //arguments : standard_type ; | : standard_type ;
    if ( check( LexicalToken( "(", SYMBOL ) ) )
    {
        //_output<<"arguments : standard_type ;\n";
        const Arg_Ext args = arguments();
        bool isErr = false;
        for(auto& arg : args.list)
        {
            if( const auto var = get_if<VarInfo>(&arg))
            {
                if(var->type == T_ERROR)
                    isErr = true;
            }
            else if( const auto arr = get_if<ArrayInfo>(&arg))
            {
                if(arr->type == T_ERROR)
                    isErr = true;
            }
        }

        if ( match( LexicalToken( ":", SYMBOL ), __FUNCTION__ ) )
        {
            const Standard_Type_Ext st_ext = standard_type();
            if ( match( LexicalToken( ";", SYMBOL ), __FUNCTION__ ) )
            {
                if(!isErr)
                    ext.f_info.type = st_ext.type;
                else
                    ext.f_info.type = T_ERROR;
                ext.f_info.parameters = args.list;
                return ext;
            }
        }
    }
    else if ( check( LexicalToken( ":", SYMBOL ) ) )
    {
        //_output<<": standard_type ;\n";
        if ( match( LexicalToken( ":", SYMBOL ), __FUNCTION__ ) )
        {
            const Standard_Type_Ext st_ext = standard_type();
            if ( match( LexicalToken( ";", SYMBOL ), __FUNCTION__ ) )
            {
                ext.f_info.type = st_ext.type;
                return ext;
            }
        }
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                LexicalToken( "(", SYMBOL ) << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
        LexicalToken( "var", RESERVED ),
        LexicalToken( "function", RESERVED ),
        LexicalToken( "begin", RESERVED ),
    } );
    ext.f_info.type = T_ERROR;
    const ParInfo err = VarInfo( T_ERROR );
    ext.f_info.parameters.push_back( err );
    return ext;
}

void PascalParser::optional_statements()
{
    //_output<<"optional_statements -> statement_list\n";
    if ( check( ID )
         || check( LexicalToken( "begin", RESERVED ) )
         || check( LexicalToken( "if", RESERVED ) )
         || check( LexicalToken( "while", RESERVED ) ) )
    {
        statement_list();
        return;
    }
    _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
            ID << ", " <<
            LexicalToken( "begin", RESERVED ) << ", " <<
            LexicalToken( "if", RESERVED ) << ", " <<
            LexicalToken( "while", RESERVED ) << ", " <<
            "but found " << _tok << "\n";

    synch( {
        EOF_TOK,
        LexicalToken( "end", RESERVED ),
    } );
}

PascalParser::Arg_Ext PascalParser::arguments()
{
    Arg_Ext ext;
    if ( check( LexicalToken( "(", SYMBOL ) ) )
    {
        //_output<<"arguments -> ( parameter_list ) \n";
        if ( match( LexicalToken( "(", SYMBOL ), __FUNCTION__ ) )
        {
            const Par_List_Ext params = parameter_list();
            if ( match( LexicalToken( ")", SYMBOL ), __FUNCTION__ ) )
            {
                ext.list = params.list;
                return ext;
            }
        }
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                LexicalToken( "(", SYMBOL ) << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
        LexicalToken( "end", RESERVED ),
    } );
    const ParInfo err = VarInfo( T_ERROR );
    ext.list.push_back( err );
    return ext;
}

void PascalParser::statement_list()
{
    //_output<<"statement_list -> ";
    //statement statement_list_2
    if ( check( ID )
         || check( LexicalToken( "begin", RESERVED ) )
         || check( LexicalToken( "if", RESERVED ) )
         || check( LexicalToken( "while", RESERVED ) ) )
    {
        //_output<<"statement statement_list_2\n";
        statement();
        statement_list_2();
        return;
    }

    _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
            ID << ", " <<
            LexicalToken( "begin", RESERVED ) << ", " <<
            LexicalToken( "if", RESERVED ) << ", " <<
            LexicalToken( "while", RESERVED ) << ", " <<
            "but found " << _tok << "\n";

    synch( {
        EOF_TOK,
        LexicalToken( "end", RESERVED ),
    } );
}

void PascalParser::statement_list_2()
{
    //_output<<"statement_list_2 -> ";
    //; statement statement_list_2 | \epsilon
    if ( check( LexicalToken( ";", SYMBOL ) ) )
    {
        //_output<<"; statement statement_list_2\n";
        if ( match( LexicalToken( ";", SYMBOL ), __FUNCTION__ ) )
        {
            statement();
            statement_list_2();
            return;
        }
    }
    else if ( check( LexicalToken( "end", RESERVED ) ) )
    {
        //_output<<"epsilon\n";
        return;
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                LexicalToken( ";", SYMBOL ) << ", " <<
                LexicalToken( "end", RESERVED ) << ", " <<
                "but found " << _tok << "\n";
    }
    synch( {
        EOF_TOK,
        LexicalToken( "end", RESERVED ),
    } );
}

PascalParser::Par_List_Ext PascalParser::parameter_list()
{
    Par_List_Ext ext;
    if ( check( ID ) )
    {
        unsigned id;
        //_output<<"parameter_list -> id : info parameter_list_2\n";
        if ( getIdSymbol( id, __FUNCTION__ )
             && match( LexicalToken( ":", SYMBOL ), __FUNCTION__ ) )
        {
            auto ln = getLineNumber();
            const Type_Ext type_ext = type();

            ext.list.push_back( type_ext.info );
            if(!_scope->addVariable( id, type_ext.info ))
            {
                _output << "Re-declaring parameter "<<_table->get( id).lex<< " at line "<< ln << " in " <<__FUNCTION__<<"\n";
                Par_List_Ext lst_2 = parameter_list_2();
                ext.list.emplace_back( VarInfo(T_ERROR));
                return ext;
            }
            Par_List_Ext lst_2 = parameter_list_2();
            for ( auto& p : lst_2.list )
                ext.list.push_back( p );
            return ext;
        }
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                ID << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
        LexicalToken( ")", SYMBOL ),
    } );
    const ParInfo err = VarInfo( T_ERROR );
    ext.list.push_back( err );
    return ext;
}

PascalParser::Par_List_Ext PascalParser::parameter_list_2()
{
    Par_List_Ext ext;
    //_output<<"parameter_list_2 -> ";
    //; id : info parameter_list_2 | \epsilon
    if ( check( LexicalToken( ";", SYMBOL ) ) )
    {
        unsigned id;
        //_output<<"; id : info parameter_list_2 \n";
        if ( match( LexicalToken( ";", SYMBOL ), __FUNCTION__ )
             && getIdSymbol( id, __FUNCTION__ )
             && match( LexicalToken( ":", SYMBOL ), __FUNCTION__ ) )
        {
            auto ln = getLineNumber();
            const Type_Ext type_ext = type();
            ext.list.push_back( type_ext.info );
            if(!_scope->addVariable( id, type_ext.info ))
            {
                _output << "Re-declaring parameter "<<_table->get( id).lex<< " at line "<< ln << " in " <<__FUNCTION__<<"\n";
                Par_List_Ext lst_2 = parameter_list_2();
                ext.list.emplace_back( VarInfo(T_ERROR));
                return ext;
            }

            Par_List_Ext lst_2 = parameter_list_2();
            for ( auto& p : lst_2.list )
                ext.list.push_back( p );
            return ext;
        }
    }
    else if ( check( LexicalToken( ")", SYMBOL ) ) )
    {
        //_output<<"epsilon\n";
        return ext;
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                LexicalToken( ";", SYMBOL ) << ", " <<
                LexicalToken( ")", SYMBOL ) << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
        LexicalToken( ")", SYMBOL ),
    } );
    const ParInfo err = VarInfo( T_ERROR );
    ext.list.push_back( err );
    return ext;
}



void PascalParser::statement()
{
    //_output<<"statement -> ";
    //variable assignop expression | compound_statement | if expression then statement statement_2 | while expression do statement
    const int ln = getLineNumber();
    if ( check( ID ) )
    {
        //_output<<"variable assignop expression\n";
        const VarInfo var = variable();
        if ( match( ASSIGN_OP, __FUNCTION__ ) )
        {
            const Exp_Ext exp = expression();
            if(holds_alternative<VarInfo>(exp.info))
            {
                const VarInfo info = get<VarInfo>(exp.info);
                if ( var.type != info.type && var.type != T_ERROR && info.type != T_ERROR )
                    _output << "Type Error at Line " << ln << " in " << __FUNCTION__
                            << ": Type Mismatch between " << getString( var.type ) << " and " << getString( info.type )
                            << "\n";
                else if(var.type == info.type && var.type != T_ERROR && info.type != T_ERROR)
                    return;
            }
            else
            {
                const ArrayInfo info = get<ArrayInfo>(exp.info);
                if ( var.type != info.type && var.type != T_ERROR && info.type != T_ERROR )
                    _output << "Type Error at Line " << ln << " in " << __FUNCTION__
                            << ": Type Mismatch between " << getString( var.type ) << " and " << getString( info.type )
                            << "\n";
                else if( var.type == info.type && var.type != T_ERROR && info.type != T_ERROR )
                    return;
            }
        }
    }
    else if ( check( LexicalToken( "begin", RESERVED ) ) )
    {
        //_output<<"compound_statement\n";
        compound_statement();
        return;
    }
    else if ( check( LexicalToken( "if", RESERVED ) ) )
    {
        //_output<<"if expression then statement statement_2\n";
        if ( match( LexicalToken( "if", RESERVED ), __FUNCTION__ ) )
        {
            const Exp_Ext exp = expression();
            if(holds_alternative<VarInfo>(exp.info))
            {
                const VarInfo info = get<VarInfo>(exp.info);
                
                if ( info.type != T_BOOLEAN && info.type != T_ERROR )
                    _output << "Type Error at Line " << ln << " in " << __FUNCTION__
                            << ": If conditions require T_BOOLEAN but found " << getString( info.type ) << "\n";
            }
            else
            {
                const ArrayInfo info = get<ArrayInfo>(exp.info);
                _output << "Type Error at Line " << ln << " in " << __FUNCTION__
                        << ": If conditions require T_BOOLEAN but found an array of " << getString( info.type ) << "\n";
            }

            if ( match( LexicalToken( "then", RESERVED ), __FUNCTION__ ) )
            {
                statement();
                statement_2();
                return;
            }
        }
    }
    else if ( check( LexicalToken( "while", RESERVED ) ) )
    {
        //_output<<"while expression do statement\n";
        if ( match( LexicalToken( "while", RESERVED ), __FUNCTION__ ) )
        {
            const Exp_Ext exp = expression();
            if(holds_alternative<VarInfo>(exp.info))
            {
                const VarInfo info = get<VarInfo>(exp.info);
                
                if ( info.type != T_BOOLEAN && info.type != T_ERROR )
                    _output << "Type Error at Line " << ln << " in " << __FUNCTION__
                            << ": If conditions require T_BOOLEAN but found " << getString( info.type ) << "\n";
            }
            else
            {
                const ArrayInfo info = get<ArrayInfo>(exp.info);
                _output << "Type Error at Line " << ln << " in " << __FUNCTION__
                        << ": If conditions require T_BOOLEAN but found an array of " << getString( info.type ) << "\n";
            }

            if ( match( LexicalToken( "do", RESERVED ), __FUNCTION__ ) )
            {
                statement();
                return;
            }
        }
    }
    else
    {
        _output << "Syntax Error at Line " << ln << " in " << __FUNCTION__ << ": Expected " <<
                ID << ", " <<
                LexicalToken( "begin", RESERVED ) << ", " <<
                LexicalToken( "if", RESERVED ) << ", " <<
                LexicalToken( "while", RESERVED ) << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
        LexicalToken( "end", RESERVED ),
        LexicalToken( ";", SYMBOL),
        LexicalToken( "else", RESERVED ),
    } );
}

void PascalParser::statement_2()
{
    //_output<<"statement_2 -> ";
    //else statement | \epsilon
    if ( check( LexicalToken( "else", RESERVED ) ) )
    {
        //_output<<"else statement\n";
        if ( match( LexicalToken( "else", RESERVED ), __FUNCTION__ ) )
        {
            statement();
            return;
        }
    }
    else if ( check( LexicalToken( ";", SYMBOL ) )
              || check( LexicalToken( "end", RESERVED ) ) )
    {
        //_output<<"epsilon\n";
        return;
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                LexicalToken( "else", RESERVED ) << ", " <<
                LexicalToken( ";", SYMBOL ) << ", " <<
                LexicalToken( "end", RESERVED ) << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
        LexicalToken( "end", RESERVED ),
        LexicalToken( ";", SYMBOL),
        LexicalToken( "else", RESERVED ),
    } );
}

VarInfo PascalParser::variable()
{
    if ( check( ID ) )
    {
        //_output<<"variable -> id variable_2\n";
        unsigned id_idx;
        if ( getIdSymbol( id_idx, __FUNCTION__ ) )
        {
            Info inf;
            try
            {
                inf = _scope->getVariable( id_idx );
            }catch(...)
            {
                _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": ID "
                    << _table->get(id_idx).lex << " is undeclared.\n";
            }
            return variable_2( inf );
        }
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                ID << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
        LexicalToken( ":=", ASSIGN_OP ),
    } );
    return VarInfo();
}

VarInfo PascalParser::variable_2( const Info& intr )
{
    VarInfo ext;
    //_output<<"variable_2 -> ";
    //[ expression ] | \epsilon
    if ( check( LexicalToken( "[", SYMBOL ) ) )
    {
        //_output<<"[ expression ]\n";
        if ( match( LexicalToken( "[", SYMBOL ), __FUNCTION__ ) )
        {
            const Exp_Ext exp = expression();
            if ( const auto arr = get_if< ArrayInfo >( &intr ) )
            {
                if ( const auto info = get_if<VarInfo>(&exp.info) )
                {
                    if(info->type != T_INTEGER && info->type != T_ERROR)
                        _output << "Type Error at Line " << getLineNumber() << " in " << __FUNCTION__
                                << ": Array index must be an integer.  Instead have a " << getString( info->type ) <<
                                "\n";
                }
                ext.type = arr->type;
            }
            else
            {
                if ( const auto var = get_if< VarInfo >( &intr ))
                {
                    if( var->type != T_ERROR)
                        _output << "Type Error at Line " << getLineNumber() << " in " << __FUNCTION__
                                << ": Only arrays have indexes.  Attempting to access a " << getString( var->type ) << "\n";
                }
                else
                    _output << "Type Error at Line " << getLineNumber() << " in " << __FUNCTION__
                            << ": Only arrays have indexes.  Attempting to access a function.\n";
            }

            if ( match( LexicalToken( "]", SYMBOL ), __FUNCTION__ ) )
            {
                return ext;
            }
        }
    }
    else if ( check( LexicalToken( ":=", ASSIGN_OP ) ) )
    {
        if(const auto arr = get_if<ArrayInfo>(&intr))
        {
            ext.type = T_ERROR;
            if(arr->type != T_ERROR)
                _output << "Type Error at Line " << getLineNumber() << " in " << __FUNCTION__
                        << ": Cannot assign a value to an array.\n";
        }
        else if(const auto fun = get_if<FuncInfo>(&intr))
        {
            ext.type = fun->type;
        }
        else if(const auto var = get_if<VarInfo>(&intr))
        {
            ext.type = var->type;
        }
        //_output<<"epsilon\n";
        return ext;
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                LexicalToken( "[", SYMBOL ) << ", " <<
                LexicalToken( ":=", ASSIGN_OP ) << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
        LexicalToken( ":=", ASSIGN_OP ),
    } );
    ext.type = T_ERROR;
    return ext;
}

PascalParser::Exp_Ext PascalParser::expression()
{
    Exp_Ext ext;
    //_output<<"expression -> ";
    //simple_expression expression_2
    if ( check( ID )
         || check( INTEGER )
         || check( REAL )
         || check( LexicalToken( "(", SYMBOL ) )
         || check( LexicalToken( "not", REL_OP ) )
         || check( LexicalToken( "+", ADD_OP ) )
         || check( LexicalToken( "-", ADD_OP ) ) )
    {
        //_output<<"simple_expression expression_2\n";
        Exp_Ext se = simple_expression();
        ext = expression_2( se );
        return ext;
    }

    _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
            ID << ", " <<
            INTEGER << ", " <<
            REAL << ", " <<
            LexicalToken( "(", SYMBOL ) << ", " <<
            LexicalToken( "not", REL_OP ) << ", " <<
            LexicalToken( "+", ADD_OP ) << ", " <<
            LexicalToken( "-", ADD_OP ) << ", " <<
            "but found " << _tok << "\n";

    synch( {
        EOF_TOK,
        LexicalToken( "]", SYMBOL ),
        LexicalToken( ")", SYMBOL ),
        LexicalToken( ",", SYMBOL ),
        LexicalToken( ";", SYMBOL ),
        LexicalToken( "then", RESERVED ),
        LexicalToken( "do", RESERVED ),
        LexicalToken( "end", RESERVED ),
        LexicalToken( "else", RESERVED ),
    } );
    ext.info = VarInfo(T_ERROR);
    return ext;
}

PascalParser::Exp_Ext PascalParser::expression_2( Exp_Ext& intr )
{
    Exp_Ext ext;
    //_output<<"expression_2 -> ";
    //relop  simple_expression | \epsilon
    if ( check( REL_OP ) )
    {
        //_output<<"relop  simple_expression\n";

        if ( match( REL_OP, __FUNCTION__ ) )
        {
            const Exp_Ext rhs = simple_expression();

            const auto intr_info = get_if<VarInfo>(&intr.info);
            const auto rhs_info = get_if<VarInfo>(&rhs.info);

            if(!intr_info || !rhs_info)
            {
                _output << "Type Error at Line " << getLineNumber() << " in " << __FUNCTION__
                        << ": Cannot Compare an array."
                        << "\n";
                ext.info = VarInfo(T_ERROR);
            }
            else if ( intr_info->type != rhs_info->type )
            {
                if ( intr_info->type != T_ERROR && rhs_info->type != T_ERROR )
                    _output << "Type Error at Line " << getLineNumber() << " in " << __FUNCTION__
                            << ": Cannot Compare " << getString( intr_info->type ) << " and " <<
                            getString( rhs_info->type )
                            << "\n";
                ext.info = VarInfo(T_ERROR);
            }
            else if (intr_info->type == T_BOOLEAN)
            {
                _output << "Type Error at Line " << getLineNumber() << " in " << __FUNCTION__
                        << ": Cannot Compare " << getString( intr_info->type )
                        << "\n";
                ext.info = VarInfo(T_ERROR);
            }
            else
            {
                ext.info = VarInfo(T_BOOLEAN);
            }
            return ext;
        }
    }
    else if ( check( LexicalToken( "]", SYMBOL ) )
              || check( LexicalToken( ")", SYMBOL ) )
              || check( LexicalToken( ",", SYMBOL ) )
              || check( LexicalToken( ";", SYMBOL ) )
              || check( LexicalToken( "then", RESERVED ) )
              || check( LexicalToken( "do", RESERVED ) )
              || check( LexicalToken( "end", RESERVED ) )
              || check( LexicalToken( "else", RESERVED ) ) )
    {
        //_output<<"epsilon\n";
        return intr;
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                REL_OP << ", " <<
                LexicalToken( "]", SYMBOL ) << ", " <<
                LexicalToken( ")", SYMBOL ) << ", " <<
                LexicalToken( ",", SYMBOL ) << ", " <<
                LexicalToken( ";", SYMBOL ) << ", " <<
                LexicalToken( "then", RESERVED ) << ", " <<
                LexicalToken( "do", RESERVED ) << ", " <<
                LexicalToken( "end", RESERVED ) << ", " <<
                LexicalToken( "else", RESERVED ) << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
        LexicalToken( "]", SYMBOL ),
        LexicalToken( ")", SYMBOL ),
        LexicalToken( ",", SYMBOL ),
        LexicalToken( ";", SYMBOL ),
        LexicalToken( "then", RESERVED ),
        LexicalToken( "do", RESERVED ),
        LexicalToken( "end", RESERVED ),
        LexicalToken( "else", RESERVED ),
    } );
    ext.info = VarInfo(T_ERROR);
    return ext;
}

PascalParser::Exp_Ext PascalParser::simple_expression()
{
    Exp_Ext ext;
    //_output<<"simple_expression -> ";
    //term simple_expression_2 | sign term simple_expression_2
    if ( check( ID )
         || check( INTEGER )
         || check( REAL )
         || check( LexicalToken( "(", SYMBOL ) )
         || check( LexicalToken( "not", REL_OP ) ) )
    {
        //_output<<"term simple_expression_2\n";
        Exp_Ext term_ext = term();
        ext = simple_expression_2( term_ext );
        return ext;
    }
    if ( check( LexicalToken( "+", ADD_OP ) )
         || check( LexicalToken( "-", ADD_OP ) ) )
    {
        //_output<<"sign term simple_expression_2\n";
        sign();
        Exp_Ext term_ext = term();
        const auto term_info = get_if<VarInfo>(&term_ext.info);

        if(!term_info)
        {
            const int ln = getLineNumber();
            _output << "Type Error at Line " << ln << " in " << __FUNCTION__
                    << ": or operation requires T_INTEGER or T_REAL but found an array.\n";
            term_ext.info = VarInfo(T_ERROR);
        }
        else if(term_info->type != T_INTEGER && term_info->type != T_REAL)
        {
            const int ln = getLineNumber();
            if(term_info->type != T_ERROR)
                _output << "Type Error at Line " << ln << " in " << __FUNCTION__
                        << ": or operation requires T_INTEGER or T_REAL but found " << getString( term_info->type ) << ".\n";
            term_ext.info = VarInfo(T_ERROR);
        }

        ext = simple_expression_2( term_ext );
        return ext;
    }
    _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
            ID << ", " <<
            INTEGER << ", " <<
            REAL << ", " <<
            LexicalToken( "(", SYMBOL ) << ", " <<
            LexicalToken( "not", REL_OP ) << ", " <<
            LexicalToken( "+", ADD_OP ) << ", " <<
            LexicalToken( "-", ADD_OP ) << ", " <<
            "but found " << _tok << "\n";

    synch( {
        EOF_TOK,
        LexicalToken( "#", REL_OP ),
        LexicalToken( "]", SYMBOL ),
        LexicalToken( ")", SYMBOL ),
        LexicalToken( ",", SYMBOL ),
        LexicalToken( ";", SYMBOL ),
        LexicalToken( "then", RESERVED ),
        LexicalToken( "do", RESERVED ),
        LexicalToken( "end", RESERVED ),
        LexicalToken( "else", RESERVED ),
    } );
    ext.info = VarInfo(T_ERROR);
    return ext;
}

PascalParser::Exp_Ext PascalParser::simple_expression_2( Exp_Ext& intr )
{
    Exp_Ext ext;
    //_output<<"simple_expression_2 -> ";
    //addop term simple_expression_2 | \epsilon
    if ( check( ADD_OP ) )
    {
        const int ln = getLineNumber();

        const auto intr_info = get_if<VarInfo>(&intr.info);
        if(!intr_info)
        {
            _output << "Type Error at Line " << ln << " in " << __FUNCTION__
                    << ": Operation requires singular types but found an array on the left side.\n";
            term();
            ext.info = VarInfo(T_ERROR);
            simple_expression_2( ext );
            return ext;
        }
        else if ( check( LexicalToken( "or", ADD_OP ) ))
        {
            if( intr_info->type != T_BOOLEAN)
            {
                if(intr_info->type != T_ERROR)
                    _output << "Type Error at Line " << ln << " in " << __FUNCTION__
                            << ": or operation requires T_BOOL but found " << getString( intr_info->type ) << ".\n";
                match( ADD_OP, __FUNCTION__ );
                term();
                ext.info = VarInfo(T_ERROR);
                simple_expression_2( ext );
            }
            else
            {
                match( ADD_OP, __FUNCTION__ );
                const Exp_Ext term_ext = term();
                const auto t_ext_info = get_if<VarInfo>(&term_ext.info);
                if(!t_ext_info)
                {
                     _output << "Type Error at Line " << ln << " in " << __FUNCTION__
                    << ": or operation requires a singular type but found an array on the right side.\n";
                    ext.info = VarInfo(T_ERROR);
                    simple_expression_2( ext );
                }
                if(t_ext_info->type != T_BOOLEAN)
                {
                    if(t_ext_info->type != T_ERROR)
                        _output << "Type Error at Line " << ln << " in " << __FUNCTION__
                            << ": or operation requires T_BOOL but found " << getString( t_ext_info->type ) << ".\n";
                    
                    ext.info = VarInfo(T_ERROR);
                    simple_expression_2( ext );
                }
                else
                {
                    ext = simple_expression_2( intr );
                }
            }
            return ext;
        }
        else if ( match( ADD_OP, __FUNCTION__ ) ) 
        {
        //_output<<"addop term simple_expression_2\n";
            const Exp_Ext term_ext = term();
            const auto t_ext_info = get_if<VarInfo>(&term_ext.info);
            if(!t_ext_info)
            {
                _output << "Type Error at Line " << ln << " in " << __FUNCTION__
                    << ": add operation requires a singular type but found an array on the right side.\n";
                ext.info = VarInfo(T_ERROR);
                simple_expression_2( ext );
            }
            else if ( intr_info->type != t_ext_info->type || t_ext_info->type == T_ERROR )
            {
                ext.info = VarInfo(T_ERROR);
                simple_expression_2( ext );
                if ( intr_info->type != T_ERROR && t_ext_info->type != T_ERROR )
                    _output << "Type Error at Line " << getLineNumber() << " in " << __FUNCTION__
                            << ": Type Mismatch between " << getString( intr_info->type ) << " and " <<
                            getString( t_ext_info->type ) << "\n";
            }
            else if(t_ext_info->type == T_BOOLEAN)
            {
                ext.info = VarInfo(T_ERROR);
                simple_expression_2( ext );
                if(intr_info->type != T_ERROR)
                    _output << "Type Error at Line " << getLineNumber() << " in " << __FUNCTION__
                            << ": Expected T_INTEGER or T_REAL but found " << getString( intr_info->type ) << " and " <<
                            getString( t_ext_info->type ) << "\n";
            }
            else
            {
                ext = simple_expression_2( intr );
            }

            return ext;
        }
    }
    else if ( check( REL_OP )
              || check( LexicalToken( "]", SYMBOL ) )
              || check( LexicalToken( ")", SYMBOL ) )
              || check( LexicalToken( ",", SYMBOL ) )
              || check( LexicalToken( ";", SYMBOL ) )
              || check( LexicalToken( "then", RESERVED ) )
              || check( LexicalToken( "do", RESERVED ) )
              || check( LexicalToken( "end", RESERVED ) )
              || check( LexicalToken( "else", RESERVED ) ) )
    {
        ext.info = intr.info;
        //_output<<"epsilon\n";
        return ext;
    }
    else
    {
        ext.info = VarInfo(T_ERROR);
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                ADD_OP << ", " <<
                REL_OP << ", " <<
                LexicalToken( "]", SYMBOL ) << ", " <<
                LexicalToken( ")", SYMBOL ) << ", " <<
                LexicalToken( ",", SYMBOL ) << ", " <<
                LexicalToken( ";", SYMBOL ) << ", " <<
                LexicalToken( "then", RESERVED ) << ", " <<
                LexicalToken( "do", RESERVED ) << ", " <<
                LexicalToken( "end", RESERVED ) << ", " <<
                LexicalToken( "else", RESERVED ) << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
        LexicalToken( "#", REL_OP ),
        LexicalToken( "]", SYMBOL ),
        LexicalToken( ")", SYMBOL ),
        LexicalToken( ",", SYMBOL ),
        LexicalToken( ";", SYMBOL ),
        LexicalToken( "then", RESERVED ),
        LexicalToken( "do", RESERVED ),
        LexicalToken( "end", RESERVED ),
        LexicalToken( "else", RESERVED ),
    } );
    return ext;
}

PascalParser::Exp_Ext PascalParser::term()
{
    Exp_Ext ext;
    //_output<<"term -> ";
    //factor term_2
    if ( check( ID )
         || check( INTEGER )
         || check( REAL )
         || check( LexicalToken( "(", SYMBOL ) )
         || check( LexicalToken( "not", REL_OP ) ) )
    {
        //_output<<"factor term_2\n";
        const Exp_Ext fext = factor();
        const Exp_Ext t2_ext = term_2( fext );
        ext.info = t2_ext.info;
        return ext;
    }

    ext.info = VarInfo(T_ERROR);
    _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
            ID << ", " <<
            INTEGER << ", " <<
            REAL << ", " <<
            LexicalToken( "(", SYMBOL ) << ", " <<
            LexicalToken( "not", REL_OP ) << ", " <<
            "but found " << _tok << "\n";

    synch( {
        EOF_TOK,
        LexicalToken( "#", REL_OP ),
        LexicalToken( "#", ADD_OP ),
        LexicalToken( "]", SYMBOL ),
        LexicalToken( ")", SYMBOL ),
        LexicalToken( ",", SYMBOL ),
        LexicalToken( ";", SYMBOL ),
        LexicalToken( "then", RESERVED ),
        LexicalToken( "do", RESERVED ),
        LexicalToken( "end", RESERVED ),
        LexicalToken( "else", RESERVED ),
    } );
    return ext;
}

PascalParser::Exp_Ext PascalParser::term_2( Exp_Ext intr )
{
    Exp_Ext ext;
    
    //_output<<"term_2 -> ";
    //mulop factor term_2 | \epsilon
    if ( check( MUL_OP ) )
    {
        const auto var = get_if<VarInfo>(&intr.info);
        if ( !var )
        {
            _output << "Type Error at Line " << getLineNumber() << " in " << __FUNCTION__
                    << ": Type must resolve to a value.  Instead have a ";
            if ( holds_alternative< ArrayInfo >( intr.info ) )
            {
                _output << "Array.\n";
            }
            else
            {
                _output << "Function.\n";
            }
            ext.info = VarInfo(T_ERROR);
            return ext;
        }

        const int ln = getLineNumber();
        if ( check( LexicalToken( "and", MUL_OP ) ) && var->type != T_BOOLEAN )
        {
            if(var->type != T_ERROR)
                _output << "Type Error at Line " << ln << " in " << __FUNCTION__
                        << ": and operation requires T_BOOL but found " << getString( var->type ) << ".\n";
            match( MUL_OP, __FUNCTION__ );
            factor();
            ext.info = VarInfo(T_ERROR);
            term_2( ext );
            return ext;
        }
        //_output<<"mulop factor term_2\n";
        if ( match( MUL_OP, __FUNCTION__ ) )
        {
            Exp_Ext type_ext = factor();
            const auto type_ext_info = get_if<VarInfo>(&type_ext.info);
            if ( !type_ext_info )
            {
                _output << "Type Error at Line " << getLineNumber() << " in " << __FUNCTION__
                        << ": Type must resolve to a value.  Instead have a ";
                if ( holds_alternative< ArrayInfo >( type_ext.info ) )
                {
                    _output << "Array.\n";
                }
                else
                {
                    _output << "Function.\n";
                }
                type_ext.info = VarInfo(T_ERROR);
            }
            else if(type_ext_info->type != var->type)
            {
                if(var->type != T_ERROR && type_ext_info->type != T_ERROR)
                    _output << "Type Error at Line " << ln << " in " << __FUNCTION__
                            << ": Type Mismatch between " << getString( var->type ) << " and " <<
                            getString( type_ext_info->type ) << "\n";
                type_ext.info = VarInfo(T_ERROR);
            }

            const Exp_Ext t2_ext = term_2( type_ext );
            return t2_ext;
        }
    }
    else if ( check( REL_OP )
              || check( ADD_OP )
              || check( LexicalToken( "]", SYMBOL ) )
              || check( LexicalToken( ")", SYMBOL ) )
              || check( LexicalToken( ",", SYMBOL ) )
              || check( LexicalToken( ";", SYMBOL ) )
              || check( LexicalToken( "then", RESERVED ) )
              || check( LexicalToken( "do", RESERVED ) )
              || check( LexicalToken( "end", RESERVED ) )
              || check( LexicalToken( "else", RESERVED ) ) )
    {
        //_output<<"epsilon\n";
        if( const auto var = get_if<VarInfo>(&intr.info))
            ext.info = *var;
        else if( const auto arr = get_if<ArrayInfo>(&intr.info))
            ext.info = *arr;
        else
            ext.info = VarInfo(T_ERROR);
        return ext;
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                MUL_OP << ", " <<
                ADD_OP << ", " <<
                REL_OP << ", " <<
                LexicalToken( "]", SYMBOL ) << ", " <<
                LexicalToken( ")", SYMBOL ) << ", " <<
                LexicalToken( ",", SYMBOL ) << ", " <<
                LexicalToken( ";", SYMBOL ) << ", " <<
                LexicalToken( "then", RESERVED ) << ", " <<
                LexicalToken( "do", RESERVED ) << ", " <<
                LexicalToken( "end", RESERVED ) << ", " <<
                LexicalToken( "else", RESERVED ) << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
        LexicalToken( ">=", REL_OP ),
        LexicalToken( "<=", REL_OP ),
        LexicalToken( "<>", REL_OP ),
        LexicalToken( "=", REL_OP ),
        LexicalToken( "<", REL_OP ),
        LexicalToken( ">", REL_OP ),
        LexicalToken( "not", REL_OP ),
        LexicalToken( "+", ADD_OP ),
        LexicalToken( "-", ADD_OP ),
        LexicalToken( "or", ADD_OP ),
        LexicalToken( "]", SYMBOL ),
        LexicalToken( ")", SYMBOL ),
        LexicalToken( ",", SYMBOL ),
        LexicalToken( ";", SYMBOL ),
        LexicalToken( "then", RESERVED ),
        LexicalToken( "do", RESERVED ),
        LexicalToken( "end", RESERVED ),
        LexicalToken( "else", RESERVED ),
    } );
    ext.info = VarInfo(T_ERROR);
    return ext;
}

void PascalParser::sign()
{
    //_output<<"sign -> ";
    // + | -
    if ( check( LexicalToken( "+", ADD_OP ) ) )
    {
        //_output<<"+\n";
        if ( match( LexicalToken( "+", ADD_OP ), __FUNCTION__ ) )
        {
            return;// true;
        }
    }
    else if ( check( LexicalToken( "-", ADD_OP ) ) )
    {
        //_output<<"-\n";
        if ( match( LexicalToken( "-", ADD_OP ), __FUNCTION__ ) )
        {
            return;// true;
        }
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                LexicalToken( "+", ADD_OP ) << ", " <<
                LexicalToken( "-", ADD_OP ) << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
        LexicalToken( "#", ID ),
        LexicalToken( "#", INTEGER ),
        LexicalToken( "#", REAL ),
        LexicalToken( "(", SYMBOL ),
        LexicalToken( "not", REL_OP ),
    } );
}

bool PascalParser::checkParameters( vector< ParInfo >& actual, vector< ParInfo >& found ) const
{
    if ( actual.size() != found.size() )
    {
        if(found.size() == 1)
        {
            if( const auto param = get_if<VarInfo>(&found[0]))
            {
                if(param->type == T_ERROR)
                {
                    return false;
                }
            }
        }
        _output << "Type Error at line " << getLineNumber() << " in " << __FUNCTION__ << ": "
                << "Incorrect Number of parameters in function call.  Expected "
                << actual.size() << " but found " << found.size() << ".\n";
        return false;
    }
    for ( int idx = 0; idx < static_cast< int >(actual.size()); ++idx )
    {
        if ( holds_alternative< VarInfo >( actual[idx] ) && holds_alternative< VarInfo >( found[idx] ) )
        {
            const VarInfo a = get< VarInfo >( actual[idx] );
            const VarInfo f = get< VarInfo >( found[idx] );
            if ( a.type != f.type && f.type != T_ERROR && a.type != T_ERROR )
            {
                _output << "Type Error at line " << getLineNumber() << " in " << __FUNCTION__ << ": "
                        "Parameter " << idx << " incorrect.  Expected "
                        << getString( a.type ) << " but found " << getString( f.type ) << ".\n";
                return false;
            }
        }
        else if ( holds_alternative< ArrayInfo >( actual[idx] ) && holds_alternative< ArrayInfo >( found[idx] ) )
        {
            const ArrayInfo a = get< ArrayInfo >( actual[idx] );
            const ArrayInfo f = get< ArrayInfo >( found[idx] );
            if ( a.type != f.type && f.type != T_ERROR && a.type != T_ERROR || a.access_offset != f.access_offset || a.count != f.count )
            {
                _output << "Type Error at line " << getLineNumber() << " in " << __FUNCTION__ << ": "
                        "Parameter " << idx << " incorrect.  Expected "
                        << getString( a.type ) << " array[" << a.access_offset << " .. " << a.count << "]"
                        << " but found "
                        << getString( f.type ) << " array[" << f.access_offset << " .. " << f.count << "].\n";
                return false;
            }
        }
        else
        {
            if ( holds_alternative< VarInfo >( actual[idx] ) )
            {
                const VarInfo a = get< VarInfo >( actual[idx] );
                const ArrayInfo f = get< ArrayInfo >( found[idx] );
                if(a.type != T_ERROR && f.type != T_ERROR)
                    _output << "Type Error at line " << getLineNumber() << " in " << __FUNCTION__ << ": "
                            << "Parameter " << idx << " incorrect.  Expected an " << getString( a.type ) << " but found "
                            << getString( f.type ) << " array[" << f.access_offset << " .. " << f.count << "].\n";
            }
            else
            {
                const ArrayInfo a = get< ArrayInfo >( actual[idx] );
                const VarInfo f = get< VarInfo >( found[idx] );
                if(a.type != T_ERROR && f.type != T_ERROR)
                    _output << "Type Error at line " << getLineNumber() << " in " << __FUNCTION__ << ": "
                            << "Parameter " << idx << " incorrect.  Expected :"
                            << getString( a.type ) << " array[" << a.access_offset << " .. " << a.count << "]"
                            << " but found " << getString( f.type ) << ".\n";
            }
            return false;
        }
    }
    return true;
}

PascalParser::Exp_Ext PascalParser::factor_prod_1( const unsigned id_idx )
{
    //_output<<"id factor_2\n";
    Exp_Ext ext;
    VarInfo var;
    Info variable;
    try
    {
        variable= _scope->getVariable( id_idx );
    }catch(...)
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": ID "
            << _table->get(id_idx).lex << " is undeclared.\n";
    }

    Fac_2_Ext fac_2 = factor_2();
    if ( holds_alternative< monostate >( fac_2 ) ) //if factor_2 is epsilon
    {
        if ( holds_alternative< FuncInfo >( variable ) )
        {
            var.type = T_ERROR;
            _output << "Type Error at line " << getLineNumber() << " in " << __FUNCTION__ << ": "
                    << "Function " << _table->get( id_idx ).lex << " used as a variable.\n";
            ext.info = var;
        }
        else if( const auto arrInfo = get_if<ArrayInfo>(&variable))
        {
            ext.info = *arrInfo;
        }
        else if( const auto varInfo = get_if<VarInfo>(&variable))
        {
            ext.info = *varInfo;
        }
        else
        {
            ext.info = VarInfo(T_ERROR);
        }
    }
    else if ( holds_alternative< Exp_List_Ext >( fac_2 ) ) //if factor_2 is ( parameter_list )
    {
        var.type = T_ERROR;
        if ( auto func = get_if< FuncInfo >( &variable ) )
        {
            auto vec = get< Exp_List_Ext >( fac_2 );
            if ( func->type != T_ERROR && checkParameters( func->parameters, vec ) )
                var.type = func->type;
        }
        ext.info = var;
    }
    else //if factor_2 is [expression] and fac_2 is VarInfo
    {
        var.type = T_ERROR;
        if ( holds_alternative< ArrayInfo >( variable ) )
        {
            const VarInfo& accessor = get< VarInfo >( fac_2 );
            if ( accessor.type == T_ERROR )
            {
                //Error already handled.
            }
            else if ( accessor.type != T_INTEGER )
            {
                _output << "Type Error at line " << getLineNumber() << " in " << __FUNCTION__ << ": "
                        << "Array index info is T_INTEGER, but found " << getString( accessor.type ) << ".\n";
            }
            else
            {
                const ArrayInfo arr = get< ArrayInfo >( variable );
                var.type = arr.type;
            }
        }
        else if ( holds_alternative< FuncInfo >( variable ) )
        {
            _output << "Type Error at line " << getLineNumber() << " in " << __FUNCTION__ << ": "////
                    << "Attempting to access Function as an Array.\n";
        }
        else if(get<VarInfo>(variable).type != T_ERROR)
        {
            _output << "Type Error at line " << getLineNumber() << " in " << __FUNCTION__ << ": "
                    << "Attempting to access Variable as an Array.\n";
        }
        ext.info = var;
    }
    return ext;
}

PascalParser::Exp_Ext PascalParser::factor()
{
    Exp_Ext ext;
    //_output<<"factor -> ";
    // id factor_2 | num | ( expression ) | not factor
    if ( check( ID ) )
    {
        //_output<<"id factor_2\n";
        unsigned id_idx;
        if ( getIdSymbol( id_idx, __FUNCTION__ ) )
        {
            const Exp_Ext facp1 = factor_prod_1( id_idx );
            if(auto var = get_if<VarInfo>(&facp1.info))
            {
                if(var->type != T_ERROR)
                    return facp1;
            }
            else
                return facp1;
        }
    }
    else if ( check( INTEGER ) )
    {
        //_output<<"num\n";
        int value;
        if ( getNum( value, __FUNCTION__ ) )
        {
            VarInfo info( T_INTEGER );
            ext.info = info;
            return ext;
        }
    }
    else if ( check( REAL ) )
    {
        //_output<<"num\n";
        double value;
        if ( getNum( value, __FUNCTION__ ) )
        {
            VarInfo info( T_REAL );
            ext.info = info;
            return ext;
        }
    }
    else if ( check( LexicalToken( "(", SYMBOL ) ) && match( LexicalToken( "(", SYMBOL ), __FUNCTION__ ) )
    {
        //_output<<"( expression )\n";
        const Exp_Ext expr = expression();
        if ( match( LexicalToken( ")", SYMBOL ), __FUNCTION__ ) )
        {
            ext.info = expr.info;
            return ext;
        }
    }
    else if ( check( LexicalToken( "not", REL_OP ) ) && match( LexicalToken( "not", REL_OP ), __FUNCTION__ ) )
    {
        //_output<<"not factor\n";
        const int ln = getLineNumber();
        Exp_Ext fac = factor();
        ext.info = VarInfo( T_ERROR );
        if ( holds_alternative< VarInfo >( fac.info ) )
        {
            const VarInfo v = get< VarInfo >( fac.info );
            if ( v.type == T_BOOLEAN )
            {
                ext.info = fac.info;
            }
            else if(v.type != T_ERROR)
            {
                _output << "Type Error at line " << ln << " in " << __FUNCTION__ << ": "
                        << "REL_OP not requires T_BOOLEAN but found " << getString( v.type ) << ".\n";
            }
        }
        else if ( holds_alternative< ArrayInfo >( fac.info ) )
        {
            _output << "Type Error at line " << ln << " in " << __FUNCTION__ << ": "
                    << "REL_OP not requires T_BOOLEAN but found an array.\n";
        }
        //else if ( holds_alternative< FuncInfo >( fac.info ) )
        //{
        //    _output << "Type Error at line " << ln << " in " << __FUNCTION__ << ": "
        //            << "REL_OP not requires T_BOOLEAN but found a function.\n";
        //}
        return ext;
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                ID << ", " <<
                INTEGER << ", " <<
                REAL << ", " <<
                LexicalToken( "(", SYMBOL ) << ", " <<
                LexicalToken( "not", REL_OP ) << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
        LexicalToken( "#", MUL_OP ),
        LexicalToken( "#", ADD_OP ),
        LexicalToken( "#", REL_OP ),
        LexicalToken( "]", SYMBOL ),
        LexicalToken( ")", SYMBOL ),
        LexicalToken( ",", SYMBOL ),
        LexicalToken( ";", SYMBOL ),
        LexicalToken( "then", RESERVED ),
        LexicalToken( "do", RESERVED ),
        LexicalToken( "end", RESERVED ),
        LexicalToken( "else", RESERVED ),
    } );
    ext.info = VarInfo( T_ERROR );
    return ext;
}

PascalParser::Fac_2_Ext PascalParser::factor_2()
{
    Fac_2_Ext ext;
    //_output<<"factor_2 -> ";
    //(expression_list) | [ expression ] | \epsilon
    if ( check( LexicalToken( "(", SYMBOL ) ) )
    {
        //_output<<"( expression_list )\n";
        if ( match( LexicalToken( "(", SYMBOL ), __FUNCTION__ ) )
        {
            Exp_List_Ext exp_list = expression_list();
            if ( match( LexicalToken( ")", SYMBOL ), __FUNCTION__ ) )
            {
                ext = exp_list;
                return ext;
            }
        }
    }
    else if ( check( LexicalToken( "[", SYMBOL ) ) )
    {
        //_output<<"[ expression ]\n";
        if ( match( LexicalToken( "[", SYMBOL ), __FUNCTION__ ) )
        {
            const int ln = getLineNumber();
            Exp_Ext exp = expression();
            if ( match( LexicalToken( "]", SYMBOL ), __FUNCTION__ ) )
            {
                const auto var = get_if<VarInfo>(&exp.info);
                if(var && var->type == T_INTEGER)
                    ext = *var;
                else if(var)
                {
                    _output << "Type Error at line " << ln << " in " << __FUNCTION__ << ": "
                        << "array access requires T_INTEGER but found "<< getString(var->type) <<".\n";
                    ext = VarInfo( T_ERROR );
                }
                else
                {
                    _output << "Type Error at line " << ln << " in " << __FUNCTION__ << ": "
                        << "array access requires T_INTEGER but found an array.\n";
                    ext = VarInfo( T_ERROR );
                }
                //DEBUGJH
            }
        }
    }
    else if ( check( MUL_OP )
              || check( ADD_OP )
              || check( REL_OP )
              || check( LexicalToken( "]", SYMBOL ) )
              || check( LexicalToken( ")", SYMBOL ) )
              || check( LexicalToken( ",", SYMBOL ) )
              || check( LexicalToken( ";", SYMBOL ) )
              || check( LexicalToken( "then", RESERVED ) )
              || check( LexicalToken( "do", RESERVED ) )
              || check( LexicalToken( "end", RESERVED ) )
              || check( LexicalToken( "else", RESERVED ) ) )
    {
        //_output<<"epsilon\n";
        ext = monostate();
        return ext;
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                LexicalToken( "(", SYMBOL ) << ", " <<
                LexicalToken( "[", SYMBOL ) << ", " <<
                MUL_OP << ", " <<
                ADD_OP << ", " <<
                REL_OP << ", " <<
                LexicalToken( "]", SYMBOL ) << ", " <<
                LexicalToken( ")", SYMBOL ) << ", " <<
                LexicalToken( ",", SYMBOL ) << ", " <<
                LexicalToken( ";", SYMBOL ) << ", " <<
                LexicalToken( "then", RESERVED ) << ", " <<
                LexicalToken( "do", RESERVED ) << ", " <<
                LexicalToken( "end", RESERVED ) << ", " <<
                LexicalToken( "else", RESERVED ) << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
        LexicalToken( "#", MUL_OP ),
        LexicalToken( "#", ADD_OP ),
        LexicalToken( "#", REL_OP ),
        LexicalToken( "]", SYMBOL ),
        LexicalToken( ")", SYMBOL ),
        LexicalToken( ",", SYMBOL ),
        LexicalToken( ";", SYMBOL ),
        LexicalToken( "then", RESERVED ),
        LexicalToken( "do", RESERVED ),
        LexicalToken( "end", RESERVED ),
        LexicalToken( "else", RESERVED ),
    } );
    ext = VarInfo( T_ERROR );
    return ext;
}

PascalParser::Exp_List_Ext PascalParser::expression_list()
{
    Exp_List_Ext ext;
    //_output<<"expression_list -> ";
    //expression expression_list_2
    if ( check( ID )
         || check( INTEGER )
         || check( REAL )
         || check( LexicalToken( "(", SYMBOL ) )
         || check( LexicalToken( "not", REL_OP ) )
         || check( LexicalToken( "+", ADD_OP ) )
         || check( LexicalToken( "-", ADD_OP ) ) )
    {
        //_output<<"expression expression_list_2\n";
        const Exp_Ext exp = expression();
        ext.push_back( exp.info );
        Exp_List_Ext exp_list_2 = expression_list_2();
        for ( auto& item : exp_list_2 )
        {
            ext.push_back( item );
        }
        return ext;
    }
    _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
            ID << ", " <<
            INTEGER << ", " <<
            REAL << ", " <<
            LexicalToken( "(", SYMBOL ) << ", " <<
            LexicalToken( "not", REL_OP ) << ", " <<
            LexicalToken( "+", ADD_OP ) << ", " <<
            LexicalToken( "-", ADD_OP ) << ", " <<
            "but found " << _tok << "\n";

    synch( {
        EOF_TOK,
        LexicalToken( "]", SYMBOL ),
        LexicalToken( ")", SYMBOL ),
        LexicalToken( ",", SYMBOL ),
        LexicalToken( ";", SYMBOL ),
        LexicalToken( "then", RESERVED ),
        LexicalToken( "do", RESERVED ),
        LexicalToken( "end", RESERVED ),
        LexicalToken( "else", RESERVED ),
    } );
    ext.clear();
    ext.push_back( VarInfo( T_ERROR ) );
    return ext;
}

PascalParser::Exp_List_Ext PascalParser::expression_list_2()
{
    Exp_List_Ext ext;
    //_output<<"expression_list_2 -> ";
    //, expression expression_list_2 | \epsilon
    if ( check( LexicalToken( ",", SYMBOL ) ) )
    {
        //_output<<", expression expression_list_2\n";
        if ( match( LexicalToken( ",", SYMBOL ), __FUNCTION__ ) )
        {
            const Exp_Ext exp = expression();
            ext.push_back( exp.info );
            Exp_List_Ext exp_list_2 = expression_list_2();
            for ( auto& item : exp_list_2 )
            {
                ext.push_back( item );
            }
            return ext;
        }
    }
    else if ( check( LexicalToken( "]", SYMBOL ) )
              || check( LexicalToken( ")", SYMBOL ) )
              || check( LexicalToken( ";", SYMBOL ) )
              || check( LexicalToken( "then", RESERVED ) )
              || check( LexicalToken( "do", RESERVED ) )
              || check( LexicalToken( "end", RESERVED ) )
              || check( LexicalToken( "else", RESERVED ) ) )
    {
        //_output<<"epsilon\n";
        return ext;
    }
    else
    {
        _output << "Syntax Error at Line " << getLineNumber() << " in " << __FUNCTION__ << ": Expected " <<
                LexicalToken( "]", SYMBOL ) << ", " <<
                LexicalToken( ")", SYMBOL ) << ", " <<
                LexicalToken( ",", SYMBOL ) << ", " <<
                LexicalToken( ";", SYMBOL ) << ", " <<
                LexicalToken( "then", RESERVED ) << ", " <<
                LexicalToken( "do", RESERVED ) << ", " <<
                LexicalToken( "end", RESERVED ) << ", " <<
                LexicalToken( "else", RESERVED ) << ", " <<
                "but found " << _tok << "\n";
    }

    synch( {
        EOF_TOK,
        LexicalToken( "]", SYMBOL ),
        LexicalToken( ")", SYMBOL ),
        LexicalToken( ",", SYMBOL ),
        LexicalToken( ";", SYMBOL ),
        LexicalToken( "then", RESERVED ),
        LexicalToken( "do", RESERVED ),
        LexicalToken( "end", RESERVED ),
        LexicalToken( "else", RESERVED ),
    } );
    ext.clear();
    ext.push_back( VarInfo( T_ERROR ) );
    return ext;
}


shared_ptr< Parser > Parser::createPascalParser( ifstream& r_words_file, ifstream& file, std::ostream& output )
{
    return std::make_shared< PascalParser >( PascalParser( r_words_file, file, output ) );
}
