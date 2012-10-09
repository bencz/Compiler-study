#include <istream>
#include <map>
#include <string>
#include <string.h>
#include <sstream>
#include <ctype.h>
#include <cstdio>

using namespace std;

enum TokenType
{
	IDENTIFIER,
	RESERVED_WORD,
	INT_CONST,
	REAL_CONST,
	STR_CONST,
	STRING_CONST,
	OPERATION,
	DELIMITER,
	END_OF_FILE
};

class ReservedWords
{
private:
	map<string, TokenType> words;
	void Add(char *value, TokenType type);
public:
	ReservedWords();
	bool Identify(char *value, TokenType& returned_type);
};

class Token
{
private:
	TokenType type;
	int line;
	int pos;
	char *value;
public:
	Token();
};

class Scanner
{
public:
	class exception: std::exception
	{
    private:
        char *msg;
    public:
        exception();
        exception(const char* const msg);
        virtual const char* what() const throw();
    };
public:
	Scanner(istream& input);
	Token GetToken();
	Token NextToken();
};