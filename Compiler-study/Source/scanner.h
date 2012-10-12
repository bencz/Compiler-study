#ifndef SCANNER
#define SCANNER

#include <istream>
#include <stdio.h>
#include <map>
#include <string.h>
#include <sstream>

#include "exception.h"

using namespace std;

enum TokenType
{
	IDENTIFIER,
	RESERVED_WORD,
	HEX_CONST,
	INT_CONST,
	REAL_CONST,
	STR_CONST,
	OPERATION,
	DELIMITER,
	END_OF_FILE
};

class ReservedWords
{
private:
	map<string, TokenType> words;
	void Add(char* value, TokenType type);
public:
	ReservedWords();
	bool Identify(string& str, TokenType& returned_type);
};

class Token
{
private:
	TokenType type;
	int line;
	int pos;
	char* value;

public:
	Token();
	Token(const char* value, TokenType type, int line, int pos);
	Token(const Token& token);
	Token& operator=(const Token& token);
	~Token();
	TokenType GetType() const;
	const char* GetValue() const;
	int GetPos() const;
	int GetLine() const;
};

ostream& operator<<(ostream& out, const Token& token);

class Scanner
{
public:
	static const int MAX_TOKEN_LENGTH = 256;

	enum State
	{
		IDENTIFIER_ST,
		HEX_ST,
		INTEGER_ST,
		OPERATION_ST,
		EOF_ST,
		NONE_ST
	};

private:
	ReservedWords reserved_words;
	Token* currentToken;
	istream& in;
	string buffer;
	string buffer_low;
	int first_pos;
	int first_line;
	Token token;
	int line;
	int pos;
	char c;
	State state;
	void AddToBuffer(char c);
	void ReduceBuffer();
	void MakeToken(TokenType type);
	void IdentifyAndMake();
	bool TryToIdentify();
	void Error(const char* msg) const;
	void ExtractChar();
	void EatLineComment();
	void EatBlockComment();
	void EatRealFractPart();
	void EatStrNum();
	void EatStrConst();
	void EatHex();
	void EatInteger();
	void EatIdentifier();
	void EatOperation();

public:
	Scanner(istream& input);
	Token GetToken();
	Token NextToken();
};

#endif