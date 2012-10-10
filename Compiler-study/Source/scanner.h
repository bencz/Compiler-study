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
	HEX_CONST,
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
	Token(char *value, TokenType type, int line, int pos);
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
        REAL_FRACT_PART_ST,
        OPERATION_ST,
        EOF_ST,
        NONE_ST
    };

    class exception : std::exception
	{
    private:
        char *msg;
    public:
        exception();
        exception(const char* const msg);
        virtual const char* what() const throw();
    };

private:
    ReservedWords reserved_words;
	Token* currentToken;
	istream& in;
	char buffer[MAX_TOKEN_LENGTH];
	char buffer_low[MAX_TOKEN_LENGTH];
	int bp;
	int first_pos;
	int first_line;
	Token token;
	int line;
	int pos;
	State state;
	void AddToBuffer(char c);
    void MakeToken(TokenType type);
    void IdentifyAndMake();
    bool TryToIdentify();
    void Error(const char* msg) const;
    char ExtractChar();
    bool ProcessNextCharacter();
    void EatLineComment(istream& in,char& curr_char);
    void EatBlockComment(istream& in, char& curr_char);
    void EatRealFractPart(istream& in, char& curr_char);
    void EatStrConst(istream& in, char& curr_char);
	void EatHex(istream& in, char& curr_char);

public:
	Scanner(istream& input);
	Token GetToken();
	Token NextToken();
};