#include <iostream>
#include <string>

#include "scanner.h"

const char* const TOKEN_DESCRIPTION[] =
{
	"IDENTIFIER",
	"RESERVED_WORD",
	"INT_CONST",
	"REAL_CONST",
	"STR_CONST",
	"STRING_CONST",
	"OPERATION",
	"DELIMITER",
	"END_OF_FILE"
};

// ************************
// Reserved Words
// ************************

void ReservedWords::Add(char *value, TokenType type)
{
	words.insert(pair<char*, TokenType>(value, type));
}

ReservedWords::ReservedWords()
{
	Add("absolute", RESERVED_WORD);
	Add("absolute", RESERVED_WORD);
    Add("and", OPERATION);
    Add("array", RESERVED_WORD);
    Add("asm", RESERVED_WORD);
    Add("begin", RESERVED_WORD);
    Add("case", RESERVED_WORD);
    Add("const", RESERVED_WORD);
    Add("constructor", RESERVED_WORD);
    Add("destructor", RESERVED_WORD);
    Add("div", OPERATION);
    Add("do", RESERVED_WORD);
    Add("downto", RESERVED_WORD);
    Add("else", RESERVED_WORD);
    Add("end", RESERVED_WORD);
    Add("file", RESERVED_WORD);
    Add("for", RESERVED_WORD);
    Add("function", RESERVED_WORD);
    Add("goto", RESERVED_WORD);
    Add("if", RESERVED_WORD);
    Add("implementation", RESERVED_WORD);
    Add("in", RESERVED_WORD);
    Add("inherited", RESERVED_WORD);
    Add("inline", RESERVED_WORD);
    Add("interface", RESERVED_WORD);
    Add("label", RESERVED_WORD);
    Add("mod", OPERATION);
    Add("nil", RESERVED_WORD);
    Add("not", OPERATION);
    Add("object", OPERATION);
    Add("of", RESERVED_WORD);
    Add("or", OPERATION);
    Add("on", RESERVED_WORD);
    Add("operator", RESERVED_WORD);
    Add("packed", RESERVED_WORD);
    Add("procedure", RESERVED_WORD);
    Add("program", RESERVED_WORD);
    Add("record", RESERVED_WORD);
    Add("repeat", RESERVED_WORD);
    Add("reintroduce", RESERVED_WORD);
    Add("set", RESERVED_WORD);
    Add("self", RESERVED_WORD);
    Add("shl", OPERATION);
    Add("shr", OPERATION);
    Add("string", RESERVED_WORD);
    Add("then", RESERVED_WORD);
    Add("to", RESERVED_WORD);
    Add("type", RESERVED_WORD);
    Add("unit", RESERVED_WORD);
    Add("until", RESERVED_WORD);
    Add("uses", RESERVED_WORD);
    Add("var", RESERVED_WORD);
    Add("while", RESERVED_WORD);
    Add("with", RESERVED_WORD);
    Add("xor", OPERATION);
    Add("..", RESERVED_WORD);
    Add(":=", OPERATION);
    Add("-", OPERATION);
    Add("+", OPERATION);
    Add("*", OPERATION);
    Add("/", OPERATION);
    Add("[", OPERATION);
    Add("]", OPERATION);
    Add(";", DELIMITER);
    Add(":", DELIMITER);
    Add(",", DELIMITER);
    Add(".", OPERATION);
    Add("^", OPERATION);
    Add("@", OPERATION);
    Add("(", OPERATION);
    Add(")", OPERATION);
    Add(">", OPERATION);
    Add("<", OPERATION);
    Add("=", OPERATION);
    Add(">=", OPERATION);
    Add("<=", OPERATION);
    Add("<>", OPERATION);
}

bool ReservedWords::Identify(char *value, TokenType& returned_type)
{
	map<string, TokenType>::iterator i = words.find(value);
	if(i == words.end())
		return false;
	returned_type = i->second;
	return true;
}

// ************************
// Token
// ************************

ostream& operator<<(ostream& out, const Token& token)
{
	out << token.GetLine() << ':' << token.GetPos() << ' ' << TOKEN_DESCRIPTION[token.GetType()]
		<< ' ' << token.GetValue() << endl;
	return out;
}

Token::Token() : value(NULL) { }

Token::Token(char *value, TokenType type, int line, int pos)
{
	this->value = strcpy(new char[strlen(value)+1], value);
	this->type = type;
	this->line = line;
	this->pos = pos;
}

Token::Token(const Token& token)
{
	value = strcpy(new char[strlen(token.value)+1], token.value);
	type = token.type;
	line = token.line;
	pos = token.pos;
}

Token& Token::operator=(const Token& token)
{
	if(value != NULL)
		delete(value);
	value = strcpy(new char[strlen(token.value)+1], token.value);
	type = token.type;
	line = token.line;
	pos = token.pos;
	return *this;
}

Token::~Token()
{
	delete(value);
}

TokenType Token::GetType() const
{
	return type;
}

const char* Token::GetValue() const
{
	return value;
}

int Token::GetPos() const
{
	return pos;
}

int Token::GetLine() const
{
	return line;
}

// ************************
// Scanner
// ************************

Scanner::exception::exception() : msg(NULL) { }

Scanner::exception::exception(const char* const msg) :
	msg(strcpy(new char[strlen(msg) + 1], msg))
{
}

const char* Scanner::exception::what() const throw()
{
	return msg;
}

// ************************
// Scanner
// ************************

void Scanner::AddToBuffer(char c)
{
	buffer[bp] = c;
	buffer_low[bp++] = tolower(c);
}

void Scanner::MakeToken(TokenType type)
{
	buffer[bp] = '\0';
	token = Token(buffer, type, first_line, first_pos);
	bp = 0;
	state = NONE_ST;		 
}

void Scanner::IdentifyAndMake()
{
	buffer_low[bp] = '\0';
	TokenType t;
	if(!reserved_words.Identify(buffer_low, t))
		t = IDENTIFIER;
	MakeToken(t);
}

bool Scanner::TryToIdentify()
{
	buffer_low[bp] = '\0';
	TokenType t;

	if(reserved_words.Identify(buffer_low, t))
	{
		MakeToken(t);
		return true;
	}
	else
		return false;
}

void Scanner::Error(const char *msg) const
{
	stringstream s;
	s << line << ':' << pos << " ERROR " << msg;
	throw(Scanner::exception(s.str().c_str() ));
}

Scanner::Scanner(istream& input) : 
	in(input),
	pos(0),
	line(1),
	bp(0),
	state(NONE_ST)
{
}

Token Scanner::GetToken()
{
	return token;
}

Token Scanner::NextToken()
{
	while(ProcessNextCharacter());
	return token;
}

void Scanner::EatLineComment(istream& in, char& curr_char)
{
	char& c = curr_char;
	if(c == '/' && (char)in.peek() == '/')
	{
		do
		{
			c = ExtractChar();
		} while (c != '\n' && !in.eof());
	}
}

void Scanner::EatBlockComment(istream& in, char& curr_char)
{
	char& c = curr_char;
	if(c == '{')
	{
		do
		{
			c = in.get();
			++pos;
			if(c == '\n')
			{
				++line;
				pos = 0;
			}
			if(in.eof())
				Error("end of file in comment");
		} while (c != '}');
		c = ExtractChar();
	}
}

void Scanner::EatRealFractPart(istream& in, char& curr_char)
{
	char& c = curr_char;
	while(isdigit(c))
	{
		AddToBuffer(c);
		c = ExtractChar();
	}
	if(c == 'e' || c == 'E')
	{
		AddToBuffer(c);
		c = ExtractChar();
		if(c == '+' || c == '-')
		{
			AddToBuffer(c);
			c = ExtractChar();
		}
		if(!isdigit(c))
			Error("Illegal character, should be number");
		else
		{
			do
			{
				AddToBuffer(c);
				c = ExtractChar();
			} while (isdigit(c));
		}
	}
	MakeToken(REAL_CONST);
}

void Scanner::EatStrConst(istream& in, char& curr_char)
{
	char& c = curr_char;
	--bp;
	while(c != '\'')
	{
		if(in.eof())
			Error("end of file in string");
		if(c != '\n')
			AddToBuffer(c);
		else if(buffer[bp - 1] != '\\')
			Error("end of line in string");
		else
		{
			++line;
			pos = 0;
			--bp;
		}
		int count = 0;
		c = in.get();
		++pos;
		while(c == '\'' && !in.eof())
		{
			++count;
			if(count % 2 == 0)
				AddToBuffer(c);
			c = in.get();
			++pos;
		}
		if(count % 2)
			break;
	}
	if(!in.eof())
	{
		c = ExtractChar();
	}
	MakeToken(STR_CONST);
}

char Scanner::ExtractChar()
{
	++pos;
	return in.get();
}

bool Scanner::ProcessNextCharacter()
{
	bool matched = false;

	char c;
	if(!in.eof())
	{
		c = ExtractChar();
		EatLineComment(in, c);
		EatBlockComment(in, c);
	}
	switch (state)
	{
	case Scanner::IDENTIFIER_ST:
		if(isalnum(c) || c == '_')
		{
			AddToBuffer(c);
		}
		else
		{
			IdentifyAndMake();
			matched = true;
		}
		break;

	case Scanner::INTEGER_ST:
		if(isdigit(c))
		{
			AddToBuffer(c);
		}
		else if(c == '.')
		{
			AddToBuffer(c);
			state = REAL_FRACT_PART_ST;
		}
		else
		{
			MakeToken(INT_CONST);
			matched = true;
		}
		break;

	case Scanner::REAL_FRACT_PART_ST:
		EatRealFractPart(in, c);
		matched = true;
		break;

	case Scanner::OPERATION_ST:
		if(buffer[0] == '\'')
		{
			EatStrConst(in, c);
			matched = true;
		}
		else
		{
			if(!in.eof() && !isalnum(c) & c != '_' && !isspace(c))
			{
				AddToBuffer(c);
				if(TryToIdentify())
				{
					matched = true;
					c = ExtractChar();
				}
				else
				{
					--bp;
				}
			}
			if(!matched)
				if(TryToIdentify())
					matched = true;
				else
					Error("illegal expression");
		}
		break;

	case Scanner::EOF_ST:
		MakeToken(END_OF_FILE);
		matched = true;
		break;
	}
	if(state == NONE_ST)
	{
		if(c == '\n')
		{
			++line;
			pos = 0;
		}
		else
		{
			first_pos = pos;
			first_line = line;
			if(in.eof())
			{
				state = EOF_ST;
			}
			else if(!isspace(c))
			{
				if(isspace(c) || c == '_')
				{
					state = IDENTIFIER_ST;
				}
				else if(isdigit(c))
				{
					state = INTEGER_ST;
				}
				else
				{
					state = OPERATION_ST;
				}
				AddToBuffer(c);
			}
		}
	}
	return !matched;
}