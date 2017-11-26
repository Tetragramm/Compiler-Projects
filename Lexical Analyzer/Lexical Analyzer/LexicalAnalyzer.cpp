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
    
    vector<string> rWords;
    ifstream rWordsFile("ReservedWords.txt");
    while(!rWordsFile.eof())
    {
        string word;
        getline(rWordsFile, word);
        rWords.push_back( word );
    }
    rWordsFile.close();

    SymbolTable symbol_table;

    PascalMachine machine(rWords, symbol_table);

    vector<string> fileNames{"TestPascal", "TestErrors"};

    for(string& file : fileNames)
    {
        ifstream inputFile(file+".txt");
        ofstream listingFile(file+"Listing.txt");
        ofstream tokenFile(file+"Tokens.txt");
        tokenFile<<setw(10)<<"Line No."<<"  ";
        tokenFile<<setw(10)<<"Lexeme"<<"  ";
        tokenFile<<setw(14)<<"TOKEN_TYPE"<<"  ";
        tokenFile<<setw(10)<<"ATTRIBUTE"<<"\n";

        int lineNum = 1;
        string line;
        while(!inputFile.eof())
        {
            getline(inputFile, line);

            machine.setLine( line );

            listingFile<<lineNum<<"\t"<<line<<"\n";

            pair<string, TAPair> tok;
            do
            {
                tok = machine.getToken();
                if(tok.second.token < NONE)
                    listingFile<<GetString(tok.second.token)<<" "<<GetString(tok.second.attribute)<<" "<<tok.first<<"\n";
                if(tok.second != TAPair())
                {
                    tokenFile<<setw(10)<<lineNum<<"  ";
                    tokenFile<<setw(10)<<tok.first<<"  ";
                    tokenFile<<setw(14)<<GetString(tok.second.token)<<"  ";
                    tokenFile<<setw(10)<<GetString(tok.second.attribute)<<"\n";
                }
            } while( tok.second != TAPair());

            lineNum++;
        }
    }

    return 0;
}
