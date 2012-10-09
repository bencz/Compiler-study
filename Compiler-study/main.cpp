#include <iostream>
#include <fstream>
#include <list>
#include <sstream>
#include <exception>

using namespace std;

class CompilerException : std::exception
{
private:
	char *msg;

public:
	CompilerException();
	CompilerException(const char* const msg);
	virtual const char* what() const throw();
};

CompilerException::CompilerException() : msg(NULL)
{
}

CompilerException::CompilerException(const char* const msg) :
	msg(strcpy(new char[strlen(msg) + 1], msg))
{
}

const char* CompilerException::what() const throw()
{
	return msg;
}

struct ConsoleKeys
{
	bool PrintLexems;
};

ConsoleKeys ParseParameters(int argc, char* argv[], list<string> & returned_files)
{
	ConsoleKeys keys;
	memset(&keys, 0, sizeof(keys));
	bool was_file =  false;

	for(int i=1;i<argc;++i)
	{
		if(argv[i][0] == '-')
		{
		}
	}
	return keys;
}

int main(int argc, char **argv)
{
}