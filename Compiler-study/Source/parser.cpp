#include <vector>
#include "parser.h"

void Parser::PrintNode(ostream& o, Expression *e, int margin)
{
	if(e == NULL)
		return;
	for(int i=0;i<margin;++i)
		o << ' ';
	o << e->token.GetValue() << "\n";

	switch (e->GetType())
	{
	case CONSTANT:
		break;
	case VARIABLE:
		break;
	case UN_OPER:
		PrintNode(o, ((UnOper*)e)->child, margin+2);
		break;
	case BIN_OPER:
		PrintNode(o, ((BinOper*)e)->left, margin+2);
		PrintNode(o, ((BinOper*)e)->right, margin+2);
		break;
	}
}

/*ostream& Parser::operator<<(ostream& o)
{
	Expression *root = GetExpression();
	if(scan.GetToken().GetType() != END_OF_FILE)
		Error("end of file exprected");
	PrintNode(o, root);
} */

// ************************
// Expression
// ************************

Expression::Expression() : pred(NULL) { }

Expression::Expression(Token token_, Expression *pred_):
	token(token_),
	pred(pred_)
{
}

// ************************
// BinOper
// ************************

BinOper::BinOper(Token token_, Expression *pred_, Expression *left_, Expression *right_):
	Expression(token_, pred_),
	left(left_),
	right(right_)
{
}

// ************************
// UnOper
// ************************

UnOper::UnOper(Token token_, Expression *pred_, Expression *child_):
	Expression(token_, pred_),
	child(child_)
{
}

// ************************
// Variable
// ************************

Variable::Variable(Token token_, Expression *pred_):
	Expression(token_, pred_)
{
}

// ************************
// Constant
// ************************

Constant::Constant(Token token_, Expression *pred_) :
	Expression(token_, pred_)
{
}

// ************************
// Expression
// ************************

void Parser::NextToken()
{
	token = scan.NextToken();
}

Parser::Parser(Scanner& scanner) : scan(scanner)
{
	scan.NextToken();
}

Expression *Parser::GetExpression()
{
	Expression *left = GetTerm();
	Token op = scan.GetToken();

	while(IsExprOp(op))
	{
		scan.NextToken();
		Expression *right = GetTerm();
		if(right == NULL)
			Error("expression expected");
		left = new BinOper(op, NULL, left, right);
		op = scan.GetToken();
	}
	return left;
}

bool Parser::IsConst(const Token& token) const
{
	TokenType type = token.GetType();
	return type == HEX_CONST || type == INT_CONST || type == REAL_CONST;
}

Expression *Parser::GetTermToken()
{
	Expression *res = NULL;
	Token token = scan.GetToken();

	if(token.GetType() == IDENTIFIER)
	{
		res = new Variable(token);
		scan.NextToken();
	}
	else if(IsConst(token))
	{
		res = new Constant(token);
		scan.NextToken();
	}
	else if(!strcmp(token.GetValue(), "("))
	{
		scan.NextToken();
		res = GetExpression();
		if(res == NULL)
			Error("illegal expression");
		if(strcmp(token.GetValue(), ")"))
			Error("expected )");
		scan.NextToken();
	}
	else
	{
		Error("illegal expression");
	}
	return res;
}

Expression *Parser::GetTerm()
{
	Expression *left = NULL;
	UnOper *root = NULL;
	UnOper *first_root = NULL;
	Token token = scan.GetToken();
	while(IsExprOp(token))
	{
		root = new UnOper(token, NULL, root);
		if(first_root == NULL)
			first_root = root;
		token = scan.NextToken();
	}
	left = GetTermToken();
	token = scan.GetToken();
	while(IsTermOp(token))
	{
		if(!IsTermOp(token))
			break;
		NextToken();
		Expression *right = GetTermToken();
		left = new BinOper(token, NULL, left, right);
		token = scan.GetToken();
	}
	if(root != NULL)
	{
		left->pred = first_root;
		first_root->child = left;
		return root;
	}
	return left;
}

bool Parser::IsExprOp(const Token& token) const
{
	return !strcmp(token.GetValue(), "+") || !strcmp(token.GetValue(), "-");
}

bool Parser::IsTermOp(const Token& token) const
{
	return !strcmp(token.GetValue(), "*") || !strcmp(token.GetValue(), "/") ||
		   !strcmp(token.GetValue(), "div") || !strcmp(token.GetValue(), "mod");
}

void Parser::Error(char *msg)
{
	stringstream s;
	Token token = scan.GetToken();
	s << token.GetLine() << ':' << token.GetPos() << " ERROR at '" << token.GetValue() << "': " << msg;
	throw(CompilerException(s.str().c_str()));
}