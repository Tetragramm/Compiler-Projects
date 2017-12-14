#include "stdafx.h"
#include "Parser.h"
#include "Machines.h"
#include "BaseParser.h"
#include "Scope.h"

using namespace std;

#define EOF_TOK LexicalToken("", TAPair(END_OF_FILE,NONE))

class PascalParser : public BaseParser
{
public:
    PascalParser(ifstream& r_words_file, ifstream& file, std::ostream& output) :
    BaseParser( file, output )
    {
        vector<string> r_words;
        while(!r_words_file.eof())
        {
            string word;
            getline(r_words_file, word);
            r_words.push_back( word );
        }
        r_words_file.close();

        _machine = make_shared<PascalMachine>(r_words, _table);
    }
    void testTable() override
    {
        _table->addOrReturn( "Test" );
    }
    void parse() override;

private:

    bool program();
    bool program_2();
    bool program_3();
    bool program_4();
    bool identifier_list();
    bool identifier_list_2();
    bool declarations();
    bool declarations_2();
    bool subprogram_declarations();
    bool subprogram_declarations_2();
    bool compound_statement();
    bool compound_statement_2();
    struct Type_Ext { Info info; };
    bool type(Type_Ext& ext);
    struct Standard_Type_Ext { E_TYPE type; };
    bool standard_type(Standard_Type_Ext& ext);
    bool subprogram_declaration();
    bool subprogram_declaration_2();
    bool subprogram_declaration_3();
    bool subprogram_head();
    struct SP_Head_Ext{FuncInfo fInfo;};
    bool subprogram_head_2(SP_Head_Ext& ext);
    bool optional_statements();
    typedef variant<VarInfo, ArrayInfo> ParInfo;
    struct Arg_Ext{vector<ParInfo> list;};
    bool arguments(Arg_Ext& ext);
    bool statement_list();
    bool statement_list_2();
    struct Par_List_Ext{vector<ParInfo> list;};
    bool parameter_list(Par_List_Ext& ext);
    bool parameter_list_2(Par_List_Ext& ext);
    bool statement();
    bool statement_2();
    bool variable();
    bool variable_2();
    bool expression();
    bool expression_2();
    bool simple_expression();
    bool simple_expression_2();
    bool term();
    bool term_2();
    bool sign();
    bool checkParameters(vector<ParInfo>& actual, vector<ParInfo>& found) const;
    struct Fac_Ext { Info info; };
    bool factor(Fac_Ext& ext);
    struct Fac_2_Ext{variant<monostate, vector<ParInfo>, VarInfo> type;};
    bool factor_prod_1( unsigned id_idx, Fac_Ext& ext );
    bool factor_2(Fac_2_Ext& ext);
    bool expression_list();
    bool expression_list_2();
};

void PascalParser::parse()
{
    BaseParser::parse();
    //_output<<"Begin Parse\n";
    _tok = getToken();
    program();
    match(END_OF_FILE, __FUNCTION__ );
}

bool PascalParser::program()
{
    _scope = make_shared<Scope>();
    //_output<<"Program -> program id ( identifier_list ) ; program_2\n";
    if(check(LexicalToken("program", RESERVED_WORD)))
    {
        if(match(LexicalToken("program", RESERVED_WORD), __FUNCTION__ )
            && match(ID, __FUNCTION__ )
            && match(LexicalToken("(", SYMBOL), __FUNCTION__ )
            && identifier_list()
            && match(LexicalToken(")", SYMBOL), __FUNCTION__ )
            && match(LexicalToken(";", SYMBOL), __FUNCTION__ )
            && program_2()
            )
        {
            //I want to short-circuit the other symbols if an early one breaks.
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken("program", RESERVED_WORD)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
    });
    return true;
}

bool PascalParser::program_2()
{
    //_output<<"program_2 -> ";
    if(check(LexicalToken("var", RESERVED_WORD)))
    {
        //_output<<"declarations \\n program_3 \n";
        if(declarations()
            && program_3())
        {
            return true;
        }
    }
    else if(check(LexicalToken("function", RESERVED_WORD))
        || check(LexicalToken("begin", RESERVED_WORD)))
    {
        //_output<<"program_3\n";
        if(program_3())
        {
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken("var", RESERVED_WORD)<<", "<<
            LexicalToken("function", RESERVED_WORD)<<", "<<
            LexicalToken("begin", RESERVED_WORD)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
    });
    return true;
}

bool PascalParser::program_3()
{
    //_output<<"program_3 -> ";
    if(check(LexicalToken("function", RESERVED_WORD)))
    {
        //_output<<"subprogram_declarations \\n program_4\n";
        if(subprogram_declarations()
            && program_4())
        {
            return true;
        }
    }
    else if(check(LexicalToken("begin", RESERVED_WORD)))
    {
        //_output<<"program_4\n";
        if(program_4())
        {
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken("function", RESERVED_WORD)<<", "<<
            LexicalToken("begin", RESERVED_WORD)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
    });
    return true;
}

bool PascalParser::program_4()
{
    //_output<<"program_4 -> ";
    if(check(LexicalToken("begin", RESERVED_WORD)))
    {
        //_output<<"compound_statement \\n .\n";
        if(compound_statement()
            && match(LexicalToken(".", SYMBOL), __FUNCTION__ ))
        {
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken("begin", RESERVED_WORD)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
    });
    return true;
}

bool PascalParser::identifier_list()
{
    if(check(ID))
    {
        //_output<<"identifier_list -> id identifier_list_2\n";
        if(match(ID, __FUNCTION__ )
            && identifier_list_2()
            )
        {
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            ID<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken(")", SYMBOL),
    });
    return true;
}

bool PascalParser::identifier_list_2()
{
    //_output<<"identifier_list_2 -> ";
    if(check(LexicalToken(",", SYMBOL)))
    {
        //_output<<", id identifier_list_2\n";
        if(match(LexicalToken(",", SYMBOL), __FUNCTION__ )
            && match(ID, __FUNCTION__ )
            && identifier_list_2()
            )
        {
            return true;
        }
    }
    else if(check(LexicalToken(")", SYMBOL)))
    {
        //_output<<"epsilon\n";
        return true;
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken(",", SYMBOL)<<", "<<
            LexicalToken(")", SYMBOL)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken(")", SYMBOL),
    });
    return true;
}

bool PascalParser::declarations()
{
    if(check(LexicalToken("var", RESERVED_WORD)))
    {
        unsigned id;
        Type_Ext type_ext;
        //_output<<"declarations -> var id : type ; declarations_2\n";
        if(match(LexicalToken("var", RESERVED_WORD), __FUNCTION__ )
            && getIdSymbol( id, __FUNCTION__ )
            && match(LexicalToken(":", SYMBOL), __FUNCTION__ )
            && type(type_ext)
            && match(LexicalToken(";", SYMBOL), __FUNCTION__ )
            && declarations_2())
        {
            _scope->addVariable( id, type_ext.info );
            //var id : type ; declarations_2
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken(",", SYMBOL)<<", "<<
            LexicalToken(")", SYMBOL)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken("function", RESERVED_WORD),
        LexicalToken("begin", RESERVED_WORD),
    });
    return true;
}

bool PascalParser::declarations_2()
{
    //_output<<"declarations_2 -> ";
    if(check(LexicalToken("var", RESERVED_WORD)))
    {
        //_output<<"declarations\n";
        if(declarations())
        {
            return true;
        }
    }
    else if(check(LexicalToken("function", RESERVED_WORD))
        || check(LexicalToken("begin", RESERVED_WORD)))
    {
        //_output<<"epsilon\n";
        return true;
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken("var", RESERVED_WORD)<<", "<<
            LexicalToken("function", RESERVED_WORD)<<", "<<
            LexicalToken("begin", RESERVED_WORD)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken("function", RESERVED_WORD),
        LexicalToken("begin", RESERVED_WORD),
    });
    return true;
}

bool PascalParser::subprogram_declarations()
{
    //_output<<"subprogram_declarations -> subprogram_declaration ; subprogram_declarations_2\n";
    if(check(LexicalToken("function", RESERVED_WORD)))
    {
        //subprogram_declaration ; subprogram_declarations_2
        if(subprogram_declaration()
            && match(LexicalToken(";", SYMBOL), __FUNCTION__ )
            && subprogram_declarations_2())
        {
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken("function", RESERVED_WORD)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken("begin", RESERVED_WORD),
    });
    return true;
}

bool PascalParser::subprogram_declarations_2()
{
    //_output<<"subprogram_declarations_2 -> ";
    if(check(LexicalToken("function", RESERVED_WORD)))
    {
        //_output<<"subprogram_declarations\n";
        if(subprogram_declarations())
        {
            return true;
        }
    }
    else if(check(LexicalToken("begin", RESERVED_WORD)))
    {
        //_output<<"epsilon\n";
        return true;
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken("function", RESERVED_WORD)<<", "<<
            LexicalToken("begin", RESERVED_WORD)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken("begin", RESERVED_WORD),
    });
    return true;
}

bool PascalParser::compound_statement()
{
    //_output<<"compound_statement -> begin \\n compound_statement_2 \n";
    if(check(LexicalToken("begin", RESERVED_WORD)))
    {
        if(match(LexicalToken("begin", RESERVED_WORD), __FUNCTION__ )
            && compound_statement_2())
        {
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken("begin", RESERVED_WORD)<<", "<<
            "but found "<<_tok<<"\n";
    }
    
    synch({EOF_TOK,
        LexicalToken(";", SYMBOL),
        LexicalToken("end", RESERVED_WORD),
        LexicalToken(".", SYMBOL),
        LexicalToken("else", RESERVED_WORD),
    });
    return true;
}

bool PascalParser::compound_statement_2()
{
    //_output<<"compound_statement_2 -> ";
    //optional_statements \n end | \n end
    if(check(ID)
        || check(LexicalToken("begin", RESERVED_WORD))
        || check(LexicalToken("if", RESERVED_WORD))
        || check(LexicalToken("while", RESERVED_WORD)))
    {
        //_output<<"optional_statements \\n end\n";
        if(optional_statements()
            && match(LexicalToken("end", RESERVED_WORD), __FUNCTION__ ))
        {
            return true;
        }
    }
    else if(match(LexicalToken("end", RESERVED_WORD), __FUNCTION__ ))
    {
        //_output<<"\\n end\n";
        return true;
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            ID<<", "<<
            LexicalToken("begin", RESERVED_WORD)<<", "<<
            LexicalToken("if", RESERVED_WORD)<<", "<<
            LexicalToken("while", RESERVED_WORD)<<", "<<
            LexicalToken("end", RESERVED_WORD)<<", "<<
            "but found "<<_tok<<"\n";
    }
    synch({EOF_TOK,
        LexicalToken(";", SYMBOL),
        LexicalToken("end", RESERVED_WORD),
        LexicalToken(".", SYMBOL),
        LexicalToken("else", RESERVED_WORD),
    });
    return true;
}

bool PascalParser::type(Type_Ext& ext)
{
    //_output<<"type -> ";
    //standard_type | array [ num .. num ] of standard_type
    if(check(LexicalToken("integer", TYPE))
        || check(LexicalToken("real", TYPE)))
    {
        //_output<<"standard_type\n";
        Standard_Type_Ext st_ext;
        if(standard_type(st_ext))
        {
            ext.info = VarInfo(st_ext.type);
            return true;
        }
    }
    else if(check(LexicalToken("array", RESERVED_WORD)))
    {
        Standard_Type_Ext st_ext;
        ArrayInfo info;
        int start, stop;
        if(match(LexicalToken("array", RESERVED_WORD), __FUNCTION__ )
            && match(LexicalToken("[", SYMBOL), __FUNCTION__ )
            && getNum( start, __FUNCTION__ )
            && match(LexicalToken(".", SYMBOL), __FUNCTION__ )
            && match(LexicalToken(".", SYMBOL), __FUNCTION__ )
            && getNum( stop, __FUNCTION__ )
            && match(LexicalToken("]", SYMBOL), __FUNCTION__ )
            && match(LexicalToken("of", RESERVED_WORD), __FUNCTION__ )
            && standard_type(st_ext))
        {
            //_output<<"array [ num .. num ] of standard_type\n";
            info.access_offset = start;
            info.count = stop-start;
            ext.info = info;
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken("integer", TYPE)<<", "<<
            LexicalToken("real", TYPE)<<", "<<
            LexicalToken("array", RESERVED_WORD)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken(";", SYMBOL),
        LexicalToken(")", SYMBOL),
    });
    ext.info = VarInfo();
    return true;
}

bool PascalParser::standard_type(Standard_Type_Ext& ext)
{
    //_output<<"standard_type -> ";
    //integer | real
    if(check(LexicalToken("integer", TYPE)))
    {
        //_output<<"integer\n";
        if(match(LexicalToken("integer", TYPE), __FUNCTION__ ))
        {
            ext.type = T_INTEGER;
            return true;
        }
    }
    else if(check(LexicalToken("real", TYPE)))
    {
        //_output<<"real\n";
        if(match(LexicalToken("real", TYPE), __FUNCTION__ ))
        {
            ext.type = T_REAL;
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken("integer", TYPE)<<", "<<
            LexicalToken("real", TYPE)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken(";", SYMBOL),
        LexicalToken(")", SYMBOL),
    });
    ext.type = T_ERROR;
    return true;
}

bool PascalParser::subprogram_declaration()
{
    //_output<<"subprogram_declaration -> ";
    //subprogram_head subprogram_declaration_2
    if(check(LexicalToken("function", RESERVED_WORD)))
    {
        //_output<<"subprogram_head subprogram_declaration_2\n";
        if(subprogram_head()
            && subprogram_declaration_2())
        {
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken("function", RESERVED_WORD)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken(";", SYMBOL),
    });
    return true;
}

bool PascalParser::subprogram_declaration_2()
{
    //_output<<"subprogram_declaration_2 -> ";
    //declarations subprogram_declaration_3 | subprogram_declaration_3
    if(check(LexicalToken("var", RESERVED_WORD)))
    {
        //_output<<"declarations subprogram_declaration_3\n";
        if(declarations()
            && subprogram_declaration_3())
        {
            return true;
        }
    }
    else if(check(LexicalToken("begin", RESERVED_WORD))
        || check(LexicalToken("function", RESERVED_WORD)))
    {
        //_output<<"subprogram_declaration_3\n";
        if(subprogram_declaration_3())
        {
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken("var", RESERVED_WORD)<<", "<<
            LexicalToken("begin", RESERVED_WORD)<<", "<<
            LexicalToken("function", RESERVED_WORD)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken(";", SYMBOL),
    });
    return true;
}

bool PascalParser::subprogram_declaration_3()
{
    //_output<<"subprogram_declaration_3 -> ";
    //subprogram_declarations compound_statement | compound_statement
    if(check(LexicalToken("begin", RESERVED_WORD)))
    {
        //_output<<"compound_statement\n";
        if(compound_statement())
        {
            return true;
        }
    }
    else if(check(LexicalToken("function", RESERVED_WORD)))
    {
        //_output<<"subprogram_declarations compound_statement\n";
        if(subprogram_declarations()
            && compound_statement())
        {
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken("begin", RESERVED_WORD)<<", "<<
            LexicalToken("function", RESERVED_WORD)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken(";", SYMBOL),
    });
    return true;
}

bool PascalParser::subprogram_head()
{
    if(check(LexicalToken("function", RESERVED_WORD)))
    {
        //_output<<"subprogram_head -> function id subprogram_head_2\n";
        SP_Head_Ext sp2_ext;
        unsigned id;
        if(match(LexicalToken("function", RESERVED_WORD), __FUNCTION__ )
            && getIdSymbol( id, __FUNCTION__ )
            && ((_scope = _scope->newScope()))
            && subprogram_head_2(sp2_ext))
        {
            _scope = _scope->getParent();
            Info f_info;
            f_info = sp2_ext.fInfo;
            _scope->addVariable( id, f_info );
            return true;
        }
    }
    else
    {
    _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
        LexicalToken("function", RESERVED_WORD)<<", "<<
        "but found "<<_tok<<"\n";
    }
    
    synch({EOF_TOK,
        LexicalToken("var", RESERVED_WORD),
        LexicalToken("function", RESERVED_WORD),
        LexicalToken("begin", RESERVED_WORD),
    });
    return true;
}

bool PascalParser::subprogram_head_2(SP_Head_Ext& ext)
{
    //_output<<"subprogram_head_2 -> ";
    //arguments : standard_type ; | : standard_type ;
    if(check(LexicalToken("(", SYMBOL)))
    {
        //_output<<"arguments : standard_type ;\n";
        Standard_Type_Ext st_ext;
        Arg_Ext args;
        if(arguments(args)
            && match(LexicalToken(":", SYMBOL), __FUNCTION__ )
            && standard_type(st_ext)
            && match(LexicalToken(";", SYMBOL), __FUNCTION__ ))
        {
            ext.fInfo.type = st_ext.type;
            ext.fInfo.parameters = args.list;
            return true;
        }
    }
    else if(check(LexicalToken(":", SYMBOL)))
    {
        //_output<<": standard_type ;\n";
        Standard_Type_Ext st_ext;
        if(match(LexicalToken(":", SYMBOL), __FUNCTION__ )
            && standard_type(st_ext)
            && match(LexicalToken(";", SYMBOL), __FUNCTION__ ))
        {
            ext.fInfo.type = st_ext.type;
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken("(", SYMBOL)<<", "<<
            LexicalToken(":", SYMBOL)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken("var", RESERVED_WORD),
        LexicalToken("function", RESERVED_WORD),
        LexicalToken("begin", RESERVED_WORD),
    });
    return true;
}

bool PascalParser::optional_statements()
{
    //_output<<"optional_statements -> statement_list\n";
    if(check(ID)
        || check(LexicalToken("begin", RESERVED_WORD))
        || check(LexicalToken("if", RESERVED_WORD))
        || check(LexicalToken("while", RESERVED_WORD)))
    {
        if(statement_list())
        {
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            ID<<", "<<
            LexicalToken("begin", RESERVED_WORD)<<", "<<
            LexicalToken("if", RESERVED_WORD)<<", "<<
            LexicalToken("while", RESERVED_WORD)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken("end", RESERVED_WORD),
    });
    return true;
}

bool PascalParser::arguments(Arg_Ext& ext)
{
    if(check(LexicalToken("(", SYMBOL)))
    {
        Par_List_Ext params;
        //_output<<"arguments -> ( parameter_list ) \n";
        if(match(LexicalToken("(", SYMBOL), __FUNCTION__ )
            && parameter_list(params)
            && match(LexicalToken(")", SYMBOL), __FUNCTION__))
        {
            ext.list = params.list;
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken("(", SYMBOL)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken("end", RESERVED_WORD),
    });
    return true;
}

bool PascalParser::statement_list()
{
    //_output<<"statement_list -> ";
    //statement statement_list_2
    if(check(ID)
        || check(LexicalToken("begin", RESERVED_WORD))
        || check(LexicalToken("if", RESERVED_WORD))
        || check(LexicalToken("while", RESERVED_WORD)))
    {
        //_output<<"statement statement_list_2\n";
        if(statement()
            && statement_list_2())
        {
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            ID<<", "<<
            LexicalToken("begin", RESERVED_WORD)<<", "<<
            LexicalToken("if", RESERVED_WORD)<<", "<<
            LexicalToken("while", RESERVED_WORD)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken("end", RESERVED_WORD),
    });
    return true;
}

bool PascalParser::statement_list_2()
{
    //_output<<"statement_list_2 -> ";
    //; statement statement_list_2 | \epsilon
    if(check(LexicalToken(";", SYMBOL)))
    {
        //_output<<"; statement statement_list_2\n";
        if( match(LexicalToken(";", SYMBOL), __FUNCTION__ )
            && statement()
            && statement_list_2())
        {
            return true;
        }
    }
    else if(check(LexicalToken("end", RESERVED_WORD)))
    {
        //_output<<"epsilon\n";
        return true;
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken(";", SYMBOL)<<", "<<
            LexicalToken("end", RESERVED_WORD)<<", "<<
            "but found "<<_tok<<"\n";
    }
    synch({EOF_TOK,
        LexicalToken("end", RESERVED_WORD),
    });
    return true;
}

bool PascalParser::parameter_list(Par_List_Ext& ext)
{
    if(check(ID))
    {
        Type_Ext type_ext;
        unsigned id;
        //_output<<"parameter_list -> id : type parameter_list_2\n";
        if(getIdSymbol( id, __FUNCTION__ )
            && match(LexicalToken(":", SYMBOL), __FUNCTION__ )
            && type(type_ext))
        {
            ParInfo param;
            if(holds_alternative<VarInfo>(type_ext.info))
            {
                param = get<VarInfo>(type_ext.info);
            }
            else if(holds_alternative<ArrayInfo>(type_ext.info))
            {
                param = get<ArrayInfo>(type_ext.info);
            }
            else
            {
                param = VarInfo(T_ERROR);
                _output<<"Type Error at line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
                    " a Variable or Array, but found a function.\n";
                
            }
            ext.list.push_back( param );
            _scope->addVariable( id, type_ext.info );

            Par_List_Ext lst_2;
            if(parameter_list_2(lst_2))
            {
                for(auto& p : lst_2.list)
                    ext.list.push_back( p );
                return true;
            }
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            ID<<", "<<
            "but found "<<_tok<<"\n";
    }
    
    synch({EOF_TOK,
        LexicalToken(")", SYMBOL),
    });
    return true;
}

bool PascalParser::parameter_list_2(Par_List_Ext& ext)
{
    //_output<<"parameter_list_2 -> ";
    //; id : type parameter_list_2 | \epsilon
    if(check(LexicalToken(";", SYMBOL)))
    {
        Type_Ext type_ext;
        unsigned id;
        //_output<<"; id : type parameter_list_2 \n";
        if(match(LexicalToken(";", SYMBOL), __FUNCTION__ )
            && getIdSymbol( id, __FUNCTION__ )
            && match(LexicalToken(":", SYMBOL), __FUNCTION__ )
            && type(type_ext))
        {
            ParInfo param;
            if(holds_alternative<VarInfo>(type_ext.info))
            {
                param = get<VarInfo>(type_ext.info);
            }
            else if(holds_alternative<ArrayInfo>(type_ext.info))
            {
                param = get<ArrayInfo>(type_ext.info);
            }
            else
            {
                param = VarInfo(T_ERROR);
                _output<<"Type Error at line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
                    " a Variable or Array, but found a function.\n";
                
            }
            ext.list.push_back( param );
            _scope->addVariable( id, type_ext.info );

            Par_List_Ext lst_2;
            if(parameter_list_2(lst_2))
            {
                for(auto& p : lst_2.list)
                    ext.list.push_back( p );
                return true;
            }
        }
    }
    else if(check(LexicalToken(")", SYMBOL)))
    {
        //_output<<"epsilon\n";
        return true;
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken(";", SYMBOL)<<", "<<
            LexicalToken(")", SYMBOL)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken(")", SYMBOL),
    });
    return true;
}

bool PascalParser::statement()
{
    //_output<<"statement -> ";
    //variable assignop expression | compound_statement | if expression then statement statement_2 | while expression do statement
    if(check(ID))
    {
        //_output<<"variable assignop expression\n";
        if(variable()
            && match(ASSIGN_OP, __FUNCTION__ )
            && expression())
        {
            return true;
        }
    }
    else if(check(LexicalToken("begin", RESERVED_WORD)))
    {
        //_output<<"compound_statement\n";
        if(compound_statement())
        {
            return true;
        }
    }
    else if(check(LexicalToken("if", RESERVED_WORD)))
    {
        //_output<<"if expression then statement statement_2\n";
        if(match(LexicalToken("if", RESERVED_WORD), __FUNCTION__ )
            && expression()
            && match(LexicalToken("then", RESERVED_WORD), __FUNCTION__ )
            && statement()
            && statement_2())
        {
            return true;
        }
    }
    else if(check(LexicalToken("while", RESERVED_WORD)))
    {
        //_output<<"while expression do statement\n";
        if(match(LexicalToken("while", RESERVED_WORD), __FUNCTION__ )
            && expression()
            && match(LexicalToken("do", RESERVED_WORD), __FUNCTION__ )
            && statement())
        {
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            ID<<", "<<
            LexicalToken("begin", RESERVED_WORD)<<", "<<
            LexicalToken("if", RESERVED_WORD)<<", "<<
            LexicalToken("while", RESERVED_WORD)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken("end", RESERVED_WORD),
    });
    return true;
}

bool PascalParser::statement_2()
{
    //_output<<"statement_2 -> ";
    //else statement | \epsilon
    if(check(LexicalToken("else", RESERVED_WORD)))
    {
        //_output<<"else statement\n";
        if(match(LexicalToken("else", RESERVED_WORD), __FUNCTION__ )
            && statement())
        {
            return true;
        }
    }
    else if(check(LexicalToken(";", SYMBOL))
        || check(LexicalToken("end", RESERVED_WORD)))
    {
        //_output<<"epsilon\n";
        return true;
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken("else", RESERVED_WORD)<<", "<<
            LexicalToken(";", SYMBOL)<<", "<<
            LexicalToken("end", RESERVED_WORD)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken("end", RESERVED_WORD),
    });
    return true;
}

bool PascalParser::variable()
{
    if(check(ID))
    {
        //_output<<"variable -> id variable_2\n";
        if(match(ID, __FUNCTION__ )
            && variable_2())
        {
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            ID<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken(":=", ASSIGN_OP),
    });
    return true;
}

bool PascalParser::variable_2()
{
    //_output<<"variable_2 -> ";
    //[ expression ] | \epsilon
    if(check(LexicalToken("[", SYMBOL)))
    {
        //_output<<"[ expression ]\n";
        if(match(LexicalToken("[", SYMBOL), __FUNCTION__ )
            && expression()
            && match(LexicalToken("]", SYMBOL), __FUNCTION__ ))
        {
            return true;
        }
    }
    else if(check(LexicalToken(":=", ASSIGN_OP)))
    {
        //_output<<"epsilon\n";
        return true;
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken("[", SYMBOL)<<", "<<
            LexicalToken(":=", ASSIGN_OP)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken(":=", ASSIGN_OP),
    });
    return true;
}

bool PascalParser::expression()
{
    //_output<<"expression -> ";
    //simple_expression expression_2
    if(check(ID)
        || check(INTEGER)
        || check(REAL)
        || check(LexicalToken("(", SYMBOL))
        || check(LexicalToken("not", REL_OP))
        || check(LexicalToken("+", ADD_OP))
        || check(LexicalToken("-", ADD_OP)))
    {
        //_output<<"simple_expression expression_2\n";
        if(simple_expression()
            && expression_2())
        {
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            ID<<", "<<
            INTEGER<<", "<<
            REAL<<", "<<
            LexicalToken("(", SYMBOL)<<", "<<
            LexicalToken("not", REL_OP)<<", "<<
            LexicalToken("+", ADD_OP)<<", "<<
            LexicalToken("-", ADD_OP)<<", "<<
            "but found "<<_tok<<"\n";
    }
    synch({EOF_TOK,
        LexicalToken("]", SYMBOL),
        LexicalToken(")", SYMBOL),
        LexicalToken(",", SYMBOL),
        LexicalToken(";", SYMBOL),
        LexicalToken("then", RESERVED_WORD),
        LexicalToken("do", RESERVED_WORD),
        LexicalToken("end", RESERVED_WORD),
        LexicalToken("else", RESERVED_WORD),
    });
    return true;
}

bool PascalParser::expression_2()
{
    //_output<<"expression_2 -> ";
    //relop  simple_expression | \epsilon
    if(check(REL_OP))
    {
        //_output<<"relop  simple_expression\n";
        if(match(REL_OP, __FUNCTION__ )
            && simple_expression())
        {
            return true;
        }
    }
    else if(check(LexicalToken("]", SYMBOL))
        || check(LexicalToken(")", SYMBOL))
        || check(LexicalToken(",", SYMBOL))
        || check(LexicalToken(";", SYMBOL))
        || check(LexicalToken("then", RESERVED_WORD))
        || check(LexicalToken("do", RESERVED_WORD))
        || check(LexicalToken("end", RESERVED_WORD))
        || check(LexicalToken("else", RESERVED_WORD)))
    {
        //_output<<"epsilon\n";
        return true;
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            REL_OP<<", "<<
            LexicalToken("]", SYMBOL)<<", "<<
            LexicalToken(")", SYMBOL)<<", "<<
            LexicalToken(",", SYMBOL)<<", "<<
            LexicalToken(";", SYMBOL)<<", "<<
            LexicalToken("then", RESERVED_WORD)<<", "<<
            LexicalToken("do", RESERVED_WORD)<<", "<<
            LexicalToken("end", RESERVED_WORD)<<", "<<
            LexicalToken("else", RESERVED_WORD)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken("]", SYMBOL),
        LexicalToken(")", SYMBOL),
        LexicalToken(",", SYMBOL),
        LexicalToken(";", SYMBOL),
        LexicalToken("then", RESERVED_WORD),
        LexicalToken("do", RESERVED_WORD),
        LexicalToken("end", RESERVED_WORD),
        LexicalToken("else", RESERVED_WORD),
    });
    return true;
}

bool PascalParser::simple_expression()
{
    //_output<<"simple_expression -> ";
    //term simple_expression_2 | sign term simple_expression_2
    if(check(ID)
        || check(INTEGER)
        || check(REAL)
        || check(LexicalToken("(", SYMBOL))
        || check(LexicalToken("not", REL_OP)))
    {
        //_output<<"term simple_expression_2\n";
        if(term()
            && simple_expression_2())
        {
            return true;
        }
    }
    else if(check(LexicalToken("+", ADD_OP))
        || check(LexicalToken("-", ADD_OP)))
    {
        //_output<<"sign term simple_expression_2\n";
        if(sign()
            && term()
            && simple_expression_2())
        {
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            ID<<", "<<
            INTEGER<<", "<<
            REAL<<", "<<
            LexicalToken("(", SYMBOL)<<", "<<
            LexicalToken("not", REL_OP)<<", "<<
            LexicalToken("+", ADD_OP)<<", "<<
            LexicalToken("-", ADD_OP)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken("#", REL_OP),
        LexicalToken("]", SYMBOL),
        LexicalToken(")", SYMBOL),
        LexicalToken(",", SYMBOL),
        LexicalToken(";", SYMBOL),
        LexicalToken("then", RESERVED_WORD),
        LexicalToken("do", RESERVED_WORD),
        LexicalToken("end", RESERVED_WORD),
        LexicalToken("else", RESERVED_WORD),
    });
    return true;
}

bool PascalParser::simple_expression_2()
{
    //_output<<"simple_expression_2 -> ";
    //addop term simple_expression_2 | \epsilon
    if(check(ADD_OP))
    {
        //_output<<"addop term simple_expression_2\n";
        if(match(ADD_OP, __FUNCTION__ )
            && term()
            && simple_expression_2())
        {
            return true;
        }
    }
    else if(check(REL_OP)
        || check(LexicalToken("]", SYMBOL))
        || check(LexicalToken(")", SYMBOL))
        || check(LexicalToken(",", SYMBOL))
        || check(LexicalToken(";", SYMBOL))
        || check(LexicalToken("then", RESERVED_WORD))
        || check(LexicalToken("do", RESERVED_WORD))
        || check(LexicalToken("end", RESERVED_WORD))
        || check(LexicalToken("else", RESERVED_WORD)))
    {
        //_output<<"epsilon\n";
        return true;
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            ADD_OP<<", "<<
            REL_OP<<", "<<
            LexicalToken("]", SYMBOL)<<", "<<
            LexicalToken(")", SYMBOL)<<", "<<
            LexicalToken(",", SYMBOL)<<", "<<
            LexicalToken(";", SYMBOL)<<", "<<
            LexicalToken("then", RESERVED_WORD)<<", "<<
            LexicalToken("do", RESERVED_WORD)<<", "<<
            LexicalToken("end", RESERVED_WORD)<<", "<<
            LexicalToken("else", RESERVED_WORD)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken("#", REL_OP),
        LexicalToken("]", SYMBOL),
        LexicalToken(")", SYMBOL),
        LexicalToken(",", SYMBOL),
        LexicalToken(";", SYMBOL),
        LexicalToken("then", RESERVED_WORD),
        LexicalToken("do", RESERVED_WORD),
        LexicalToken("end", RESERVED_WORD),
        LexicalToken("else", RESERVED_WORD),
    });
    return true;
}

bool PascalParser::term()
{
    //_output<<"term -> ";
    //factor term_2
    if(check(ID)
        || check(INTEGER)
        || check(REAL)
        || check(LexicalToken("(", SYMBOL))
        || check(LexicalToken("not", REL_OP)))
    {
        //_output<<"factor term_2\n";
        if(factor()
            && term_2())
        {
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            ID<<", "<<
            INTEGER<<", "<<
            REAL<<", "<<
            LexicalToken("(", SYMBOL)<<", "<<
            LexicalToken("not", REL_OP)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken("#", REL_OP),
        LexicalToken("#", ADD_OP),
        LexicalToken("]", SYMBOL),
        LexicalToken(")", SYMBOL),
        LexicalToken(",", SYMBOL),
        LexicalToken(";", SYMBOL),
        LexicalToken("then", RESERVED_WORD),
        LexicalToken("do", RESERVED_WORD),
        LexicalToken("end", RESERVED_WORD),
        LexicalToken("else", RESERVED_WORD),
    });
    return true;
}

bool PascalParser::term_2()
{
    //_output<<"term_2 -> ";
    //mulop factor term_2 | \epsilon
    if(check(MUL_OP))
    {
        //_output<<"mulop factor term_2\n";
        if(match(MUL_OP, __FUNCTION__ )
            && factor()
            && term_2())
        {
            return true;
        }
    }
    else if(check(REL_OP)
        || check(ADD_OP)
        || check(LexicalToken("]", SYMBOL))
        || check(LexicalToken(")", SYMBOL))
        || check(LexicalToken(",", SYMBOL))
        || check(LexicalToken(";", SYMBOL))
        || check(LexicalToken("then", RESERVED_WORD))
        || check(LexicalToken("do", RESERVED_WORD))
        || check(LexicalToken("end", RESERVED_WORD))
        || check(LexicalToken("else", RESERVED_WORD)))
    {
        //_output<<"epsilon\n";
        return true;
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            MUL_OP<<", "<<
            ADD_OP<<", "<<
            REL_OP<<", "<<
            LexicalToken("]", SYMBOL)<<", "<<
            LexicalToken(")", SYMBOL)<<", "<<
            LexicalToken(",", SYMBOL)<<", "<<
            LexicalToken(";", SYMBOL)<<", "<<
            LexicalToken("then", RESERVED_WORD)<<", "<<
            LexicalToken("do", RESERVED_WORD)<<", "<<
            LexicalToken("end", RESERVED_WORD)<<", "<<
            LexicalToken("else", RESERVED_WORD)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken(">=", REL_OP),
        LexicalToken("<=", REL_OP),
        LexicalToken("<>", REL_OP),
        LexicalToken("=", REL_OP),
        LexicalToken("<", REL_OP),
        LexicalToken(">", REL_OP),
        LexicalToken("not", REL_OP),
        LexicalToken("+", ADD_OP),
        LexicalToken("-", ADD_OP),
        LexicalToken("or", ADD_OP),
        LexicalToken("]", SYMBOL),
        LexicalToken(")", SYMBOL),
        LexicalToken(",", SYMBOL),
        LexicalToken(";", SYMBOL),
        LexicalToken("then", RESERVED_WORD),
        LexicalToken("do", RESERVED_WORD),
        LexicalToken("end", RESERVED_WORD),
        LexicalToken("else", RESERVED_WORD),
    });
    return true;
}

bool PascalParser::sign()
{
    //_output<<"sign -> ";
    // + | -
    if(check(LexicalToken("+", ADD_OP)))
    {
        //_output<<"+\n";
        if(match(LexicalToken("+", ADD_OP), __FUNCTION__ ))
        {
            return true;
        }
    }
    else if(check(LexicalToken("-", ADD_OP)))
    {
        //_output<<"-\n";
        if(match(LexicalToken("-", ADD_OP), __FUNCTION__ ))
        {
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken("+", ADD_OP)<<", "<<
            LexicalToken("-", ADD_OP)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken("#", ID),
        LexicalToken("#", INTEGER),
        LexicalToken("#", REAL),
        LexicalToken("(", SYMBOL),
        LexicalToken("not", REL_OP),
    });
    return true;
}

bool PascalParser::checkParameters(vector<ParInfo>& actual, vector<ParInfo>& found) const
{
    if(actual.size() != found.size())
    {
        _output<<"Type Error at line "<<getLineNumber()<<" in "<<__FUNCTION__<<": "
            <<"Incorrect Number of parameters in function call.  Expected "
            <<actual.size()<<" but found "<<found.size()<<".\n";
        return false;
    }
    for(int idx = 0; idx<actual.size(); ++idx)
    {
        if(holds_alternative<VarInfo>( actual[idx] ) && holds_alternative<VarInfo>( found[idx] ))
        {
            const VarInfo a = get<VarInfo>(actual[idx]);
            const VarInfo f = get<VarInfo>(found[idx]);
            if(a.type != f.type)
            {
                _output<<"Type Error at line "<<getLineNumber()<<" in "<<__FUNCTION__<<": "
                    "Parameter "<<idx<<" incorrect.  Expected "
                    <<getString(a.type)<<" but found "<<getString(f.type)<<".\n";
                return false;
            }
        }
        else if(holds_alternative<ArrayInfo>( actual[idx] ) && holds_alternative<ArrayInfo>( found[idx] ))
        {
            const ArrayInfo a = get<ArrayInfo>(actual[idx]);
            const ArrayInfo f = get<ArrayInfo>(found[idx]);
            if(a.type != f.type || a.access_offset != f.access_offset || a.count != f.count)
            {
                _output<<"Type Error at line "<<getLineNumber()<<" in "<<__FUNCTION__<<": "
                    "Parameter "<<idx<<" incorrect.  Expected "
                    <<getString(a.type)<<" array["<<a.access_offset<<" .. "<<a.count<<"]"
                    <<" but found "
                    <<getString(f.type)<<" array["<<f.access_offset<<" .. "<<f.count<<"].\n";
                return false;
            }
        }
        else
        {
            if(holds_alternative<VarInfo>( actual[idx] ))
            {
                const VarInfo a = get<VarInfo>(actual[idx]);
                const ArrayInfo f = get<ArrayInfo>(found[idx]);
                _output<<"Type Error at line "<<getLineNumber()<<" in "<<__FUNCTION__<<": "
                    <<"Parameter "<<idx<<" incorrect.  Expected an "<<getString(a.type)<<" but found "
                    <<getString(f.type)<<" array["<<f.access_offset<<" .. "<<f.count<<"].\n";
            }
            else
            {
                const ArrayInfo a = get<ArrayInfo>(actual[idx]);
                const VarInfo f = get<VarInfo>(found[idx]);
                _output<<"Type Error at line "<<getLineNumber()<<" in "<<__FUNCTION__<<": "
                    <<"Parameter "<<idx<<" incorrect.  Expected :"
                    <<getString(a.type)<<" array["<<a.access_offset<<" .. "<<a.count<<"]"
                    <<" but found "<<getString(f.type)<<".\n";
            }
            return false;
        }
    }
    return true;
}

bool PascalParser::factor_prod_1( const unsigned id_idx, Fac_Ext& ext )
{
    //_output<<"id factor_2\n";
    VarInfo var;
    ;
    Info variable = _scope->getVariable(id_idx);
    Fac_2_Ext fac_2;
    if(factor_2(fac_2))
    {
        if(holds_alternative<monostate>( fac_2.type )) //if factor_2 is epsilon
        {
            if(holds_alternative<FuncInfo>(variable))
            {
                var.type = T_ERROR;
                _output<<"Type Error at line "<<getLineNumber()<<" in "<<__FUNCTION__<<": "
                    <<"Function "<<_table->get( id_idx ).lex<<" used as a variable.\n";
                ext.info = var;
            }
            else
            {
                ext.info = variable;
            }
        }
        else if(holds_alternative<vector<ParInfo>>( fac_2.type )) //if factor_2 is ( parameter_list )
        {
            if(auto func = get_if<FuncInfo>( &variable ))
            {
                var.type = func->type;
                auto vec = get<vector<ParInfo>>(fac_2.type);
                if(!checkParameters( func->parameters, vec ))
                    var.type = T_ERROR;
            }
            ext.info = var;
        }
        else //if factor_2 is [expression]
        {
            var.type = T_ERROR;
            if(holds_alternative<ArrayInfo>(variable))
            {
                const VarInfo& accessor = get<VarInfo>(fac_2.type);
                if(accessor.type != T_INTEGER)
                {
                    _output<<"Type Error at line "<<getLineNumber()<<" in "<<__FUNCTION__<<": "
                        <<"Array index type is T_INTEGER, but found "<<getString(accessor.type)<<".\n";
                }
                else
                {
                    const ArrayInfo arr = get<ArrayInfo>(variable);
                    var.type = arr.type;
                }
            }
            else if(holds_alternative<FuncInfo>(variable))
            {
                _output<<"Type Error at line "<<getLineNumber()<<" in "<<__FUNCTION__<<": "
                    <<"Attempting to access Function as an Array.\n";
            }
            else
            {
                _output<<"Type Error at line "<<getLineNumber()<<" in "<<__FUNCTION__<<": "
                    <<"Attempting to access Variable as an Array.\n";
            }
            ext.info = var;
        }
        return true;
    }
    return false;
}

bool PascalParser::factor(Fac_Ext& ext)
{
    //_output<<"factor -> ";
    // id factor_2 | num | ( expression ) | not factor
    if(check(ID))
    {
        //_output<<"id factor_2\n";
        unsigned id_idx;
        if(getIdSymbol( id_idx, __FUNCTION__ ))
        {
            factor_prod_1( id_idx, ext );
            return true;
        }
    }
    else if(check(INTEGER))
    {
        //_output<<"num\n";
        int value;
        if(getNum(value, __FUNCTION__))
        {
            VarInfo info(T_INTEGER);
            ext.info = info;
            return true;
        }
    }
    else if(check(REAL))
    {
        //_output<<"num\n";
        double value;
        if(getNum(value, __FUNCTION__))
        {
            VarInfo info(T_REAL);
            ext.info = info;
            return true;
        }
    }
    else if(check(LexicalToken("(", SYMBOL)))
    {
        //_output<<"( expression )\n";
        if(match(LexicalToken("(", SYMBOL), __FUNCTION__ )
            && expression()
            && match(LexicalToken(")", SYMBOL), __FUNCTION__ ))
        {
            //TODO: Expression.info
            return true;
        }
    }
    else if(check(LexicalToken("not", REL_OP)))
    {
        //_output<<"not factor\n";
        Fac_Ext fac;
        if(match(LexicalToken("not", REL_OP), __FUNCTION__ )
            && factor(fac))
        {
            if(holds_alternative<VarInfo>( fac.info ))
            {
                const VarInfo v = get<VarInfo>(fac.info);
                if(v.type == T_BOOLEAN)
                {
                    ext.info = fac.info;
                }
                else
                {
                    _output<<"Type Error at line "<<getLineNumber()<<" in "<<__FUNCTION__<<": "
                        <<"REL_OP not requires T_BOOLEAN but found "<<v.type<<".\n";
                }
            }
            else if(holds_alternative<ArrayInfo>( fac.info ))
            {
                _output<<"Type Error at line "<<getLineNumber()<<" in "<<__FUNCTION__<<": "
                    <<"REL_OP not requires T_BOOLEAN but found an array.\n";
            }
            else if(holds_alternative<FuncInfo>( fac.info ))
            {
                _output<<"Type Error at line "<<getLineNumber()<<" in "<<__FUNCTION__<<": "
                    <<"REL_OP not requires T_BOOLEAN but found a function.\n";
            }
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            ID<<", "<<
            INTEGER<<", "<<
            REAL<<", "<<
            LexicalToken("(", SYMBOL)<<", "<<
            LexicalToken("not", REL_OP)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken("#", MUL_OP),
        LexicalToken("#", ADD_OP),
        LexicalToken("#", REL_OP),
        LexicalToken("]", SYMBOL),
        LexicalToken(")", SYMBOL),
        LexicalToken(",", SYMBOL),
        LexicalToken(";", SYMBOL),
        LexicalToken("then", RESERVED_WORD),
        LexicalToken("do", RESERVED_WORD),
        LexicalToken("end", RESERVED_WORD),
        LexicalToken("else", RESERVED_WORD),
    });
    return true;
}

bool PascalParser::factor_2(Fac_2_Ext& ext)
{
    //_output<<"factor_2 -> ";
    //(expression_list) | [ expression ] | \epsilon
    if(check(LexicalToken("(", SYMBOL)))
    {
        //_output<<"( expression_list )\n";
        if(match(LexicalToken("(", SYMBOL), __FUNCTION__ )
            && expression_list()
            && match(LexicalToken(")", SYMBOL), __FUNCTION__ ))
        {
            ext.type = exp_list.type;
            return true;
        }
    }
    else if(check(LexicalToken("[", SYMBOL)))
    {
        //_output<<"[ expression ]\n";
        if(match(LexicalToken("[", SYMBOL), __FUNCTION__ )
            && expression()
            && match(LexicalToken("]", SYMBOL), __FUNCTION__ ))
        {
            ext.type = exp.type;
            return true;
        }
    }
    else if(check(MUL_OP)
        || check(ADD_OP)
        || check(REL_OP)
        || check(LexicalToken("]", SYMBOL))
        || check(LexicalToken(")", SYMBOL))
        || check(LexicalToken(",", SYMBOL))
        || check(LexicalToken(";", SYMBOL))
        || check(LexicalToken("then", RESERVED_WORD))
        || check(LexicalToken("do", RESERVED_WORD))
        || check(LexicalToken("end", RESERVED_WORD))
        || check(LexicalToken("else", RESERVED_WORD)))
    {
        //_output<<"epsilon\n";
        ext.type = monostate();
        return true;
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken("(", SYMBOL)<<", "<<
            LexicalToken("[", SYMBOL)<<", "<<
            MUL_OP<<", "<<
            ADD_OP<<", "<<
            REL_OP<<", "<<
            LexicalToken("]", SYMBOL)<<", "<<
            LexicalToken(")", SYMBOL)<<", "<<
            LexicalToken(",", SYMBOL)<<", "<<
            LexicalToken(";", SYMBOL)<<", "<<
            LexicalToken("then", RESERVED_WORD)<<", "<<
            LexicalToken("do", RESERVED_WORD)<<", "<<
            LexicalToken("end", RESERVED_WORD)<<", "<<
            LexicalToken("else", RESERVED_WORD)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken("#", MUL_OP),
        LexicalToken("#", ADD_OP),
        LexicalToken("#", REL_OP),
        LexicalToken("]", SYMBOL),
        LexicalToken(")", SYMBOL),
        LexicalToken(",", SYMBOL),
        LexicalToken(";", SYMBOL),
        LexicalToken("then", RESERVED_WORD),
        LexicalToken("do", RESERVED_WORD),
        LexicalToken("end", RESERVED_WORD),
        LexicalToken("else", RESERVED_WORD),
    });
    return true;
}

bool PascalParser::expression_list()
{
    //_output<<"expression_list -> ";
    //exression expression_list_2
    if(check(ID)
        || check(INTEGER)
        || check(REAL)
        || check(LexicalToken("(", SYMBOL))
        || check(LexicalToken("not", REL_OP))
        || check(LexicalToken("+", ADD_OP))
        || check(LexicalToken("-", ADD_OP)))
    {
        //_output<<"expression expression_list_2\n";
        if(expression()
            && expression_list_2())
        {
            return true;
        }
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            ID<<", "<<
            INTEGER<<", "<<
            REAL<<", "<<
            LexicalToken("(", SYMBOL)<<", "<<
            LexicalToken("not", REL_OP)<<", "<<
            LexicalToken("+", ADD_OP)<<", "<<
            LexicalToken("-", ADD_OP)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken("]", SYMBOL),
        LexicalToken(")", SYMBOL),
        LexicalToken(",", SYMBOL),
        LexicalToken(";", SYMBOL),
        LexicalToken("then", RESERVED_WORD),
        LexicalToken("do", RESERVED_WORD),
        LexicalToken("end", RESERVED_WORD),
        LexicalToken("else", RESERVED_WORD),
    });
    return true;
}

bool PascalParser::expression_list_2()
{
    //_output<<"expression_list_2 -> ";
    //, expression expression_list_2 | \epsilon
    if(check(LexicalToken(",", SYMBOL)))
    {
        //_output<<", expression expression_list_2\n";
        if(match(LexicalToken(",", SYMBOL), __FUNCTION__ )
            && expression()
            && expression_list_2())
        {
            return true;
        }
    }
    else if(check(LexicalToken("]", SYMBOL))
        || check(LexicalToken(")", SYMBOL))
        || check(LexicalToken(";", SYMBOL))
        || check(LexicalToken("then", RESERVED_WORD))
        || check(LexicalToken("do", RESERVED_WORD))
        || check(LexicalToken("end", RESERVED_WORD))
        || check(LexicalToken("else", RESERVED_WORD)))
    {
        //_output<<"epsilon\n";
        return true;
    }
    else
    {
        _output<<"Syntax Error at Line "<<getLineNumber()<<" in "<<__FUNCTION__<<": Expected "<<
            LexicalToken("]", SYMBOL)<<", "<<
            LexicalToken(")", SYMBOL)<<", "<<
            LexicalToken(",", SYMBOL)<<", "<<
            LexicalToken(";", SYMBOL)<<", "<<
            LexicalToken("then", RESERVED_WORD)<<", "<<
            LexicalToken("do", RESERVED_WORD)<<", "<<
            LexicalToken("end", RESERVED_WORD)<<", "<<
            LexicalToken("else", RESERVED_WORD)<<", "<<
            "but found "<<_tok<<"\n";
    }

    synch({EOF_TOK,
        LexicalToken("]", SYMBOL),
        LexicalToken(")", SYMBOL),
        LexicalToken(",", SYMBOL),
        LexicalToken(";", SYMBOL),
        LexicalToken("then", RESERVED_WORD),
        LexicalToken("do", RESERVED_WORD),
        LexicalToken("end", RESERVED_WORD),
        LexicalToken("else", RESERVED_WORD),
    });
    return true;
}


shared_ptr< Parser > Parser::createPascalParser(ifstream& r_words_file, ifstream& file, std::ostream& output)
{
    return std::make_shared<PascalParser>( PascalParser(r_words_file, file, output ) );
}
