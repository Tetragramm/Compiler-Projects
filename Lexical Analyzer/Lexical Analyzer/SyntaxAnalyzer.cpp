// LexicalAnalyzer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <fstream>
#include <regex>
#include <experimental/filesystem>

#include "Parser.h"
#include <iostream>

int main()
{
    using namespace std;
    using namespace string_literals;
    namespace fs = experimental::filesystem;

    fs::directory_iterator dir_iter(".\\Productions");

    for(auto& file : dir_iter)
    {
        if(!is_directory(file))
        {
            cout<<file.path().string()<<"\n";

            fs::path listing = file;
            listing.replace_filename( "Listing\\"s + listing.filename().string() );
            fs::path token = file;
            token.replace_filename( "Token\\"s + listing.filename().string() );
            fs::path memory = file;
            memory.replace_filename( "Memory\\"s + listing.filename().string() );

            ifstream r_words_file(".\\ReservedWords.txt");
            ifstream input_file(file.path().string());
            ofstream output_file(listing.string());
            auto parser = Parser::createPascalParser( r_words_file, input_file, output_file );
            parser->setTokenStream( make_shared<ofstream>(token.string()) );
            parser->setMemoryStream( make_shared<ofstream>(memory.string()));
            parser->parse();
        }
    }

    return 0;
}
