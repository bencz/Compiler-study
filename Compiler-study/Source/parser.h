#ifndef PARSER
#define PARSER

#include <ostream>
#include <string.h>

#include "scanner.h"
#include "exception.h"

enum ExpressionClass
{
	EXPRESSION,
	CONSTANT,
	VARIABLE,
	UN_OPER,
	BIN_OPER,
};

struct Expression
{
    Token token;
    Expression* pred;
    Expression();
    Expression(Token token_, Expression* pred_ = NULL);
    virtual ExpressionClass GetType() { return EXPRESSION; }
};

struct BinOper : public Expression
{
	Expression *left;
	Expression *right;
	BinOper(Token token_, Expression *pred_ = NULL, Expression *left_ = NULL, Expression *right_ = NULL);
	virtual ExpressionClass GetType() { return BIN_OPER; }
};

struct UnOper : public Expression
{
	Expression *child;
	UnOper(Token token_, Expression *pred_ = NULL, Expression *child_ = NULL);
	virtual ExpressionClass GetType() { return UN_OPER; }
};

struct Variable : public Expression
{
	Variable(Token token_, Expression *pred_ = NULL);
	virtual ExpressionClass GetType() { return VARIABLE; }
};

struct Constant : public Expression
{
	Constant(Token token_, Expression *pred_ = NULL);
	virtual ExpressionClass GetType() { return CONSTANT; }
};

class Parser
{
private:
	Scanner& scan;
	Token token;
	void NextToken();
	Expression *GetExpression();
	Expression *GetTerm();
	Expression *GetTermToken();

	bool IsExprOp(const Token& token) const;
	bool IsTermOp(const Token& token) const;
	bool IsConst(const Token& token) const;

	void Error(char *msg);
	void PrintNode(ostream& o, Expression *e, int margin = 0);
public:
	Parser(Scanner& scanner);
	ostream& operator<<(ostream& o);
};

#endif PARSER