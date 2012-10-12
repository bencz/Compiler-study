#ifndef PARSER
#define PARSER

#include <vector>
#include <ostream>

#include "scanner.h"
#include <string.h>
#include "exception.h"

enum ExpressionClass
{
	EXPRESSION,
	CONSTANT,
	VARIBLE,
	UN_OPER,
	BIN_OPER,
	RECORD_ACCESS,
	ARRAY_ACCESS,
	FUNCTION_CALL
};

struct Expression
{
	Token token;
	Expression* pred;
	Expression();
	Expression(Token token_, Expression* pred_ = NULL);
	virtual ExpressionClass GetType() { return EXPRESSION; }
};

struct BinOper: public Expression
{
	Expression* left;
	Expression* right;
	BinOper(Token token_, Expression* pred_ = NULL, Expression* left_ = NULL, Expression* right_ = NULL);
	virtual ExpressionClass GetType()  { return BIN_OPER; }
};

struct UnOper: public Expression
{
	Expression* child;
	UnOper(Token token_, Expression* pred_ = NULL, Expression* child_ = NULL);
	virtual ExpressionClass GetType()  { return UN_OPER; }
};

struct Varible: public Expression
{
	Varible(Token token_, Expression* pred_ = NULL);
	virtual ExpressionClass GetType()  { return VARIBLE; }
};

struct Constant: public Expression
{
	Constant(Token token_, Expression* pred_ = NULL);
	virtual ExpressionClass GetType()  { return CONSTANT; }
};

struct RecordAccess: public Expression
{
	Expression* record;
	RecordAccess(Token field, Expression* record_, Expression* pred_ = NULL);
	virtual ExpressionClass GetType() { return RECORD_ACCESS; }
};

struct ArrayAccess: public Expression
{
	Expression* index;
	ArrayAccess(Token& name, Expression* index_, Expression* pred_ = NULL);
	virtual ExpressionClass GetType() { return ARRAY_ACCESS; }
};

struct FunctionCall: public Expression
{
	vector<Expression*> args;
	FunctionCall(Token name, Expression* pred_ = NULL);
	void AddArgument(Expression* arg);
	virtual ExpressionClass GetType() { return FUNCTION_CALL; }
};

class Parser
{
private:
	Scanner& scan;
	void NextToken();
	Expression* GetExpression();
	Expression* GetTerm();
	Expression* GetTermToken();
	bool IsExprOp(const Token& token) const;
	bool IsTermOp(const Token& token) const;
	bool IsConst(const Token& token) const;
	void Error(char* msgn);
	void PrintNode(ostream& o, Expression* e, int margin = 0);

public:
	Parser(Scanner& scanner);
	ostream& operator<<(ostream& o);
};

#endif