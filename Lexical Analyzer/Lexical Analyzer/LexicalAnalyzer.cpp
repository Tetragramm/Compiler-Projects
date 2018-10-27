// LexicalAnalyzer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <fstream>
#include <iomanip>
#include <regex>
#include <iostream>

#include "Machines.h"
#include "SymbolTable.h"
#include "TokenAttr.h"

int main()
{
    using namespace std;
    
    vector<string> r_words;
    ifstream r_words_file("ReservedWords.txt");
    while(!r_words_file.eof())
    {
        string word;
        getline(r_words_file, word);
        r_words.push_back( word );
    }
    r_words_file.close();

    vector<string> file_names{"TestPascal", "TestErrors", "TestSample"};

    for(string& file : file_names)
    {
        SymbolTable symbol_table;
        PascalMachine machine(r_words, symbol_table);

        ifstream input_file(file+".txt");
        ofstream listing_file(file+"Listing.txt");
        ofstream token_file(file+"Tokens.txt");
        ofstream symbol_table_file(file+"SymbolTable.txt");
        token_file<<setw(10)<<"Line No."<<"  ";
        token_file<<setw(10)<<"Lexeme"<<"  ";
        token_file<<setw(14)<<"TOKEN_TYPE"<<"  ";
        token_file<<setw(10)<<"ATTRIBUTE"<<"\n";

        int line_num = 1;
        string line;
        while(!input_file.eof())
        {
            getline(input_file, line);

            machine.setLine( line );

            listing_file<<line_num<<"\t"<<line<<"\n";

            LexicalToken tok;
            do
            {
                tok = machine.getToken();
                if(tok.second.token < NONE)
                    listing_file<<GetString(tok.second.token)<<" "<<GetString(tok.second.attribute)<<" "<<tok.first<<"\n";
                
                if(tok.second != EMPTY_TOKEN)
                {
                    token_file<<setw(10)<<line_num<<"  ";
                    token_file<<setw(10)<<tok.first<<"  ";
                    token_file<<setw(14)<<GetString(tok.second.token)<<"  ";
                    token_file<<setw(10)<<GetString(tok.second.attribute)<<"\n";
                }
            } while( tok.second != EMPTY_TOKEN);

            line_num++;
        }

        symbol_table_file << symbol_table;
    }

    return 0;
}
