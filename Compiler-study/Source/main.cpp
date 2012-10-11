#include <iostream>
#include <fstream>
#include <list>
#include <sstream>
#include <string.h>

#include "scanner.h"
#include "exception.h"
//#include "parser.h"

void PrintHelp()
{
	cout << "Usage: compiler [options] source.pas\n\
	-h  show this\n\
	-s  simple parser\n\
    -l  show lexems stream\
	";
}

int main(int argc, char **argv)
{
	if(argc == 1)
	{
		PrintHelp();
		return 0;
	}
	try
	{
		if(argc > 3)
			throw CompilerException("too many arguments");
		if(argc == 2)
		{
			if(argv[1][1] == 'h')
			{
				PrintHelp();
				return 0;
			}
			if(argv[1][1] == 'l' || argv[1][1] == 's')
				throw CompilerException("no files specified");
			else
				throw CompilerException("unknow option");
		}
		ifstream in;
		in.open(argv[2], ios::in);
		if(!in.good())
			throw CompilerException("can't open file");
		if(argv[1][0] != '-')
			throw CompilerException("invalid option");
		else
		{
			if(!argv[1][1] || argv[1][2])
				throw CompilerException("invalid option");
			switch (argv[1][1])
			{
			case 's':
				{
				Scanner scan(in);
				//Parser parser(scan);
				//parser << (cout);
				}
				break;
			case 'l':
				{
					Scanner scan(in);
					for(Token t;t.GetType() != END_OF_FILE;)
						cout << ( t = scan.NextToken() );
				}
				break;
			}
		}
	}
	catch(CompilerException& e)
	{
		cout << e.what() << endl;
	}
	catch(exception& e)
	{
		cout << e.what() << endl;
	}
	getchar();
	return 0;
}