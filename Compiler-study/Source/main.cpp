#include <iostream>
#include <fstream>
#include <list>
#include "scanner.h"
#include "parser.h"
#include <sstream>
#include <string.h>
#include "exception.h"

void PrintHelp()
{
	cout << "Usage: compiler [option] source.pas\n\
			\n\
			avaible options are:\n\
			\t-b\tprint Both syntax tree and symTable\n\
			\t-h\tshow this message\n\
			\t-l\tshow Lexems stream\n\
			\t-s\tprint Syntax tree\n\
			\t-t\tprint symTable\n";    
}

string my_argv[] = {"compiler.exe", "-l", "test/16.in" };

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		PrintHelp();
		return 0;
	}
	try
	{
		if (argc > 3) throw CompilerException("too many parametrs");
		if (argc == 2)
		{
			if (argv[1][1] == 'h')
			{
				PrintHelp();
				return 0;
			}
			if (argv[1][1] == 'l' || argv[1][1] == 's')
				throw CompilerException("no files specified");
			else
				throw CompilerException("uncknown option");
		}
		ifstream in;
		in.open(argv[2], ios::in);
		if (!in.good()) throw CompilerException("can't open file");
		if (argv[1][0] != '-')
			throw CompilerException("invalid option");
		else
		{
			if (!argv[1][1] || argv[1][2]) throw CompilerException("invalid option");
			switch (argv[1][1])
			{
			case 'b':
				{
					Scanner scan(in);
					Parser parser(scan);
					parser.PrintSymTable(std::cout);
					parser.PrintSyntaxTree(std::cout);
				}
				break;
			case 's':
				{
					Scanner scan(in);
					Parser parser(scan);
					parser.PrintSyntaxTree(std::cout);
				}
				break;
			case 't':
				{
					Scanner scan(in);
					Parser parser(scan);                        
					parser.PrintSymTable(std::cout);
				}
				break;
			case 'l':
				{
					Scanner scan(in);
					for (Token t; t.GetType() != END_OF_FILE;)
					{
						cout << ( t = scan.NextToken() );
					}
				}
				break;
			}
		}
	}
	catch (CompilerException& e)
	{
		cout << e.what() << endl;
	}
	catch (exception& e)
	{
		cout << e.what() << endl;
	}
	return 0;
}