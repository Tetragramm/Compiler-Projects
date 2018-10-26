// SyntaxAnalyzer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <fstream>
#include <regex>

#include "Machines.h"
#include "Parser.h"

int main()
{
    using namespace std;
    

    vector<string> file_names{"TestPascal", "TestErrorsA"};

    for(string& file : file_names)
    {
        ifstream r_words_file("ReservedWords.txt");
        ifstream input_file(file+".txt");
        ofstream output_file(file+"Listing.txt");
        auto parser = Parser::createPascalParser( r_words_file, input_file, output_file );
        parser->setTokenStream( make_shared<ofstream>(file+"Tokens.txt") );
        parser->parse();
    }

    return 0;
}
