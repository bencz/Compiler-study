#ifndef COMPILER_EXCEPTION
#define COMPILER_EXCEPTION

#include <exception>
#include <string.h>

class CompilerException : public std::exception
{
private:
	char *err;

public:
	CompilerException();
	CompilerException(const char* const msg);
	~CompilerException() throw();
	virtual const char* what() const throw();
};

#endif