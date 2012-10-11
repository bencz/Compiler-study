#include "exception.h"

CompilerException::CompilerException() : err(NULL) { }

CompilerException::CompilerException(const char* const msg) :
	err(strcpy(new char[strlen(msg) + 1], msg)) { }

const char* CompilerException::what() const throw()
{
	return err;
}

CompilerException::~CompilerException() throw()
{
	if(err != NULL)
		delete err;
}