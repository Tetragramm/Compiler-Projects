// LexicalAnalyzer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <fstream>
#include <sstream>

int main()
{
	using namespace std;

	ifstream sourceFile("TestPascal.txt");
	ofstream listingFile("TestListing.txt");
	string line;
	int lineCount = 0;
	bool complete = false;
	while(!complete)
	{
		getline(sourceFile, line);
		listingFile << lineCount << " " << line << "\n";
		lineCount++;
		complete = sourceFile.eof();
	}

    return 0;
}

