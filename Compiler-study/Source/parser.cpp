#include <vector>
#include "parser.h"

void PrintSpaces(ostream& o, int count)
{
	for (int i = 0; i < count; ++i) o << ' ';
}

void Parser::PrintNode(ostream& o, Expression* e, int margin)
{
	if (e == NULL) return;
	PrintSpaces(o, margin);
	switch (e -> GetType())
	{
	case CONSTANT:
		o << e -> token.GetValue() << "\n";
		break;
	case VARIBLE:
		o << e -> token.GetValue() << "\n";
		break;
	case UN_OPER:
		o << e -> token.GetValue() << "\n";
		PrintNode(o, ((UnOper*)e) -> child, margin + 2);
		break;
	case BIN_OPER:
		o << e -> token.GetValue() << "\n";
		PrintNode(o, ((BinOper*)e) -> left, margin + 2);
		PrintNode(o, ((BinOper*)e) -> right, margin + 2);
		break;
	case FUNCTION_CALL:
		{
			o << e -> token.GetValue() << "()" << "\n";
			FunctionCall* f = (FunctionCall*)e;
			for(vector<Expression*>::iterator it = f->args.begin(); it != f->args.end(); ++it)
				PrintNode(o, *it, margin + 2);
		}
		break;
	case RECORD_ACCESS:
		o << ".\n";
		PrintNode(o, ((RecordAccess*)e) -> record, margin + 2);
		PrintSpaces(o, margin);
		o << "  " << e -> token.GetValue() << "\n";
		break;
	case ARRAY_ACCESS:
		o << "[]\n";
		PrintSpaces(o, margin + 2);
		o << e -> token.GetValue() << "\n";
		PrintNode(o, ((ArrayAccess*)e) -> index, margin + 2);
	}

}

ostream& Parser::operator<<(ostream& o)
{
	Expression* root = GetExpression();
	if (scan.GetToken().GetType() != END_OF_FILE)
		Error("end of file expected");
	PrintNode(o, root);
	return o;
}


//---Expression---

Expression::Expression() : pred(NULL)
{
}

Expression::Expression(Token token_, Expression* pred_):
	token(token_),
	pred(pred_)
{
}

//---BinOper---

BinOper::BinOper(Token token_, Expression* pred_, Expression* left_, Expression* right_):
	Expression(token_, pred_),
	left(left_),
	right(right_)
{
}

//---UnOper---

UnOper::UnOper(Token token_, Expression* pred_, Expression* child_):
	Expression(token_, pred_),
	child(child_)
{
}

//---Varible---

Varible::Varible(Token token_, Expression* pred_):
	Expression(token_, pred_)
{
}

//---Constant---

Constant::Constant(Token token_, Expression* pred_):
	Expression(token_, pred_)
{
}

//---RecordAccess

RecordAccess::RecordAccess(Token field, Expression* record_, Expression* pred_):
	Expression(field, pred_),
	record(record_)
{
}

//---ArrayAccess

ArrayAccess::ArrayAccess(Token& name, Expression* index_, Expression* pred_):
	Expression(name, pred_),
	index(index_)
{
	index -> pred = this;
}

//---FunctionCall---

FunctionCall::FunctionCall(Token name, Expression* pred_):
	Expression(name, pred_)
{
}

void FunctionCall::AddArgument(Expression* arg)
{
	args.push_back(arg);
}

//---Parser---

Parser::Parser(Scanner& scanner):
	scan(scanner)
{
	scan.NextToken();
}

Expression* Parser::GetExpression()
{
	Expression* left = GetTerm();
	if (left == NULL)
		return NULL;
	Token op = scan.GetToken();
	while (IsExprOp(op))
	{
		scan.NextToken();
		Expression* right = GetTerm();
		if (right == NULL)
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

Expression* Parser::GetTermToken()
{
	Expression* res = NULL;
	Token token = scan.GetToken();
	if (token.GetType() == IDENTIFIER)
	{
		scan.NextToken();
		if (!strcmp(scan.GetToken().GetValue(), "("))
		{
			scan.NextToken();
			FunctionCall* funct = new FunctionCall(token);
			while (strcmp(scan.GetToken().GetValue(), ")"))
			{
				Expression* arg = GetExpression();
				if (!strcmp(scan.GetToken().GetValue(), ","))
				{
					scan.NextToken();
				}
				else if (strcmp(scan.GetToken().GetValue(), ")"))
				{
					Error(", expected");
				}
				if (arg == NULL) Error(") expected");
				funct->AddArgument(arg);
			}
			scan.NextToken();
			res = funct;
		}
		else if (!strcmp(scan.GetToken().GetValue(), "."))
		{
			Expression* record = new Varible(token);
			while (!strcmp(scan.GetToken().GetValue(), "."))
			{
				Expression* newrec = new RecordAccess(token, record); //token - to tmp put smth value
				record -> pred = newrec;
				record = newrec;
				token = scan.NextToken();
				if (token.GetType() != IDENTIFIER) Error("identifier after . expected");
				record -> token = token;
				scan.NextToken();
			}
			res = record;
		}
		else if (!strcmp(scan.GetToken().GetValue(), "["))
		{
			scan.NextToken();
			Expression* index = GetExpression();
			if (index == NULL) Error("illegal expression");
			ArrayAccess* acc = new ArrayAccess(token, index);
			//BinOper* acc = new BinOper(token, NULL, index, NULL);
			res = acc;
			scan.NextToken();
		}
		else
		{
			res = new Varible(token);
		}
	}
	else if (IsConst(token))
	{
		res = new Constant(token);
		scan.NextToken();
	}
	else if (!strcmp(token.GetValue(), "("))
	{
		scan.NextToken();
		res = GetExpression();
		if (res == NULL) Error("illegal expression");
		token = scan.GetToken();
		if (strcmp(token.GetValue(), ")"))
			Error("expected )");
		scan.NextToken();
	}
	else
	{
		return NULL;
	}
	return res;
}

Expression* Parser::GetTerm()
{
	Expression* left = NULL;
	UnOper* root = NULL;
	UnOper* first_root = NULL;
	Token token = scan.GetToken();
	while (IsExprOp(token))
	{
		root = new UnOper(token, NULL, root);
		if (first_root == NULL) first_root = root;
		token = scan.NextToken();
	}
	left = GetTermToken();
	if (left == NULL)
		return NULL;
	token = scan.GetToken();
	while (IsTermOp(token))
	{
		scan.NextToken();
		Expression* right = GetTermToken();
		if (right == NULL) Error("illegal expression");
		left = new BinOper(token, NULL, left, right);
		token = scan.GetToken();
	}
	if (root != NULL)
	{
		left -> pred = first_root;
		first_root -> child = left;
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

void Parser::Error(char* msg)
{
	stringstream s;
	Token token = scan.GetToken();
	s << token.GetLine() << ':' << token.GetPos() << " ERROR at '" << token.GetValue() << "': " << msg;
	throw( CompilerException( s.str().c_str() ) ) ;
}