#include "parser.h"

//---Parser---

SyntaxNode* Parser::ConvertType(SyntaxNode* node, const SymType* type)
{
	if (node->GetSymType() == type) return node;
	if (node->GetSymType() == top_type_int && type == top_type_real)
		return new NodeIntToRealConv(node, top_type_real);
	return NULL;
}

void Parser::TryToConvertType(SyntaxNode*& first, SyntaxNode*& second)
{
	if (first->GetSymType() == second->GetSymType()) return;
	SyntaxNode* tmp = ConvertType(first, second->GetSymType());
	if (tmp != NULL)
	{
		first = tmp;
		return;
	}
	tmp = ConvertType(second, first->GetSymType());
	if (tmp != NULL) second = tmp;
}

void Parser::TryToConvertType(SyntaxNode*& expr, const SymType* type)
{
	if (expr->GetSymType() == type) return;
	SyntaxNode* tmp = ConvertType(expr, type);
	if (tmp == NULL) return;
	expr = tmp;
	return;
}

void Parser::TryToConvertTypeOrDie(SyntaxNode*& first, SyntaxNode*& second, Token tok_err)
{
	TryToConvertType(first, second);
	if (first->GetSymType() != second->GetSymType()) 
	{
		std::stringstream s;
		s << "incompatible types: ";
		first->GetSymType()->Print(s, 0);
		s << " and ";
		second->GetSymType()->Print(s, 0);
		Error(s.str(), tok_err);
	}
}

void Parser::TryToConvertTypeOrDie(SyntaxNode*& expr, const SymType* type, Token tok_err)
{
	TryToConvertType(expr, type);
	if (expr->GetSymType() != type) 
	{
		std::stringstream s;
		s << "incompatible types: ";
		expr->GetSymType()->Print(s, 0);
		s << " and ";
		type->Print(s, 0);
		Error(s.str(), tok_err);
	}
}

void Parser::PrintSyntaxTree(ostream& o)
{
	if (syntax_tree != NULL) syntax_tree->Print(o, 0);
}

void Parser::PrintSymTable(ostream& o)
{
	//    for (std::vector<SynTable*>::const_iterator it = sym_table_stack.begin(); it != sym_table_stack.end(); ++it)
	//        (*it)->Print(o);
	sym_table_stack.back()->Print(o, 0);
}

Parser::Parser(Scanner& scanner):
	syntax_tree(NULL),
	scan(scanner)
{
	scan.NextToken();
	top_sym_table.Add(top_type_int);
	top_sym_table.Add(top_type_real);
	sym_table_stack.push_back(&top_sym_table);
	sym_table_stack.push_back(new SynTable());
	Parse();
}

SymType* Parser::ParseArrayType()
{
	std::vector<std::pair<Token, Token> > bounds;
	CheckTokOrDie(TOK_ARRAY);
	CheckTokOrDie(TOK_BRACKETS_SQUARE_LEFT);
	bool was_comma = true;
	while (was_comma)
	{
		Token low = scan.GetToken();
		CheckNextTokOrDie(TOK_DOUBLE_DOT);
		Token high = scan.GetToken();
		if (low.GetType() != INT_CONST || high.GetType() != INT_CONST)
			Error("non-integer array bounds is not implemented");
		bounds.push_back(std::pair<Token, Token>(low, high));
		if (was_comma = (scan.NextToken().GetValue() == TOK_COMMA)) scan.NextToken();
	}
	CheckTokOrDie(TOK_BRACKETS_SQUARE_RIGHT);
	CheckTokOrDie(TOK_OF);
	SymType* res = ParseType();
	for (std::vector<std::pair<Token, Token> >::reverse_iterator it = bounds.rbegin(); it != bounds.rend(); ++it)
		res = new SymTypeArray(res, it->first.GetIntValue(), it->second.GetIntValue());
	return res;
}

SymType* Parser::ParseRecordType()
{
	CheckTokOrDie(TOK_RECORD);
	sym_table_stack.push_back(new SynTable);
	ParseVarDeclarations(false);
	SymType* res = new SymTypeRecord(sym_table_stack.back());
	sym_table_stack.pop_back();
	CheckTokOrDie(TOK_END);
	return res;
}

SymType* Parser::ParsePointerType()
{
	Error("pointers was not implemented");
	CheckTokOrDie(TOK_CAP);
	if (!scan.GetToken().IsVar()) Error("identifier expected");
	const Symbol* ref_type = FindSymbolOrDie(scan.GetToken(), SYM_TYPE, "type identifier expected");
	scan.NextToken();
	return new SymTypePointer((SymType*)ref_type);    
}

SymType* Parser::ParseType()
{
	Token name = scan.GetToken();
	switch (name.GetValue()) {
	case TOK_ARRAY: {
		return ParseArrayType();
					} break;
	case TOK_RECORD: {
		return ParseRecordType();
					 } break;
	case TOK_CAP: {
		return ParsePointerType(); 
				  } break;
	default: {            
		const Symbol* res = FindSymbolOrDie(scan.GetToken(), SYM_TYPE, "type identifier expected");
		scan.NextToken();
		return (SymType*)res;
			 }
	}
}

void Parser::ParseVarDeclarations(bool is_global)
{
	while (scan.GetToken().IsVar())
	{
		std::vector<Token> vars;
		bool was_comma = true;
		while (was_comma)
		{
			Token name = scan.GetToken();
			if (FindSymbol(name) != NULL) Error("duplicate declaration");
			vars.push_back(name);
			if (was_comma = (scan.NextToken().GetValue() == TOK_COMMA))
				scan.NextToken();
		}
		CheckTokOrDie(TOK_COLON);
		SymType* type = ParseType();
		for (std::vector<Token>::iterator it = vars.begin(); it != vars.end(); ++it)
			if (is_global)
				sym_table_stack.back()->Add(new SymVarGlobal(*it, type));
			else
				sym_table_stack.back()->Add(new SymVarLocal(*it, type));
		CheckTokOrDie(TOK_SEMICOLON);
	}
}

void Parser::ParseTypeDeclarations()
{
	while (scan.GetToken().IsVar())
	{
		Token name = scan.GetToken();
		if (FindSymbol(name) != NULL) Error("duplicate declaration");
		CheckNextTokOrDie(TOK_EQUAL);
		SymType* type = ParseType();
		sym_table_stack.back()->Add(new SymTypeAlias(name, type));
		CheckTokOrDie(TOK_SEMICOLON);
	}
}

void Parser::ParseDeclarations(bool is_global)
{
	bool loop = true;
	while (loop)
	{
		switch (scan.GetToken().GetValue()) {
		case TOK_PROCEDURE:
		case TOK_FUNCTION:
			ParseFunctionDefinition();
			break;
		case TOK_VAR:
			scan.NextToken();
			if (!scan.GetToken().IsVar()) Error("identifier expected");
			ParseVarDeclarations(is_global);
			break;
		case TOK_TYPE:
			scan.NextToken();
			if (!scan.GetToken().IsVar()) Error("identifier expected");
			ParseTypeDeclarations();
			break;
		default:
			loop = false;
		}      
	}
}

void Parser::ParseFunctionParameters(SymProc* funct)
{
	CheckTokOrDie(TOK_BRACKETS_LEFT);
	if (scan.GetToken().GetValue() == TOK_BRACKETS_RIGHT) Error("empty formal parameters list");
	bool was_semicolon = true;
	while (was_semicolon)
	{
		bool by_ref = false;
		if (by_ref = (scan.GetToken().GetValue() == TOK_VAR)) scan.NextToken();
		vector<Token> v;
		bool was_comma = true;
		while (was_comma)
		{
			if (!scan.GetToken().IsVar()) Error("identifier expected");
			v.push_back(scan.GetToken());
			if (was_comma = (scan.NextToken().GetValue() == TOK_COMMA)) scan.NextToken();
		}
		CheckTokOrDie(TOK_COLON);
		const SymType* type = (SymType*)FindSymbolOrDie(scan.GetToken(), SYM_TYPE, "type identifier expected");
		for (vector<Token>::iterator it = v.begin(); it != v.end(); ++it)
		{
			SymVarParam* param = new SymVarParam(*it, type, by_ref);
			sym_table_stack.back()->Add(param);
			funct->AddParam(param);
		}
		if (was_semicolon = (scan.NextToken().GetValue() == TOK_SEMICOLON)) scan.NextToken();
	}
	CheckTokOrDie(TOK_BRACKETS_RIGHT);
}

void Parser::ParseFunctionDefinition()
{
	SymProc* res = NULL;
	TokenValue op = scan.GetToken().GetValue();
	if (op != TOK_PROCEDURE && op != TOK_FUNCTION) return;
	Token name = scan.NextToken();
	if (FindSymbol(name) != NULL) Error("duplicate identifier");
	if (op == TOK_PROCEDURE)
		res = new SymProc(name);
	else
		res = new SymFunct(name);
	sym_table_stack.back()->Add(res);
	sym_table_stack.push_back(new SynTable);
	res->AddSymTable(sym_table_stack.back());
	scan.NextToken();
	if (scan.GetToken().GetValue() == TOK_BRACKETS_LEFT) ParseFunctionParameters(res);
	if (op == TOK_FUNCTION)
	{
		CheckTokOrDie(TOK_COLON);
		const SymType* type = (SymType*)FindSymbolOrDie(scan.GetToken(), SYM_TYPE, "type identifier expected");
		scan.NextToken();
		((SymFunct*)res)->AddResultType(type);
		sym_table_stack.back()->Add(new SymVar(Token("Result", IDENTIFIER, TOK_UNRESERVED, -1, -1), type));
	}
	CheckTokOrDie(TOK_SEMICOLON);
	ParseDeclarations(false);
	if (scan.GetToken().GetValue() != TOK_BEGIN) Error("'begin' expected");
	res->AddBody(ParseStatement());
	CheckTokOrDie(TOK_SEMICOLON);
	sym_table_stack.pop_back();
}

NodeStatement* Parser::ParseStatement()
{
	switch (scan.GetToken().GetValue())
	{
	case TOK_BEGIN:
		return ParseBlockStatement();
	case TOK_FOR:
		return ParseForStatement();
	case TOK_WHILE:
		return ParseWhileStatement();
	case TOK_REPEAT:
		return ParseUntilStatement();
	case TOK_IF:
		return ParseIfStatement();
	default:
		return ParseAssignStatement();
	}
}

NodeStatement* Parser::ParseBlockStatement()
{
	StmtBlock* block = new StmtBlock();
	CheckTokOrDie(TOK_BEGIN);
	while (scan.GetToken().GetValue() != TOK_END)
	{
		block->AddStatement(ParseStatement());
		CheckTokOrDie(TOK_SEMICOLON);
	}
	scan.NextToken();
	return block;
}

NodeStatement* Parser::ParseForStatement()
{
	CheckTokOrDie(TOK_FOR);
	if (!scan.GetToken().IsVar()) Error("identifier expected");
	const SymVar* index = (SymVar*)FindSymbolOrDie(scan.GetToken(), SYM_VAR, "identifier not found");
	if (index->GetVarType() != top_type_int) Error("integer varible expected");
	CheckNextTokOrDie(TOK_ASSIGN);
	SyntaxNode* first = GetIntExprOrDie();
	bool is_inc = (scan.GetToken().GetValue() == TOK_TO);
	if (!is_inc && scan.GetToken().GetValue() != TOK_DOWNTO) Error("'to' or 'downto' expected");
	scan.NextToken();
	SyntaxNode* second = GetIntExprOrDie();
	CheckTokOrDie(TOK_DO);
	NodeStatement* body = ParseStatement();
	if (body == NULL) Error("statement expected");
	return new StmtFor(index, first, second, is_inc, body);
}

NodeStatement* Parser::ParseWhileStatement()
{
	CheckTokOrDie(TOK_WHILE);
	SyntaxNode* cond = GetIntExprOrDie();
	CheckTokOrDie(TOK_DO);
	NodeStatement* body = ParseStatement();
	if (body == NULL) Error("statement expected");
	return new StmtWhile(cond, body); 
}

NodeStatement* Parser::ParseUntilStatement()
{
	CheckTokOrDie(TOK_REPEAT);
	StmtBlock* body = new StmtBlock();
	while (scan.GetToken().GetValue() != TOK_UNTIL)
	{
		body->AddStatement(ParseStatement());
		if (scan.GetToken().GetValue() != TOK_SEMICOLON) Error("';' expected");
		scan.NextToken();
	}
	scan.NextToken();
	SyntaxNode* cond = GetIntExprOrDie();
	return new StmtUntil(cond, body);
}

NodeStatement* Parser::ParseIfStatement()
{
	CheckTokOrDie(TOK_IF);
	SyntaxNode* cond = GetIntExprOrDie();
	CheckTokOrDie(TOK_THEN);
	NodeStatement* then_branch = ParseStatement();
	NodeStatement* else_branch = NULL;
	if (then_branch == NULL) Error("statement expected");
	if (scan.GetToken().GetValue() == TOK_ELSE)
	{
		scan.NextToken();
		else_branch = ParseStatement();
		if (else_branch == NULL) Error("statement expected");
	}
	return new StmtIf(cond, then_branch, else_branch);
}

NodeStatement* Parser::ParseAssignStatement()
{
	SyntaxNode* left = ParseRelationalExpr();
	if (left == NULL) return NULL;
	if (scan.GetToken().GetValue() != TOK_ASSIGN)
		return new StmtExpression(left);
	Token op = scan.GetToken();
	scan.NextToken();
	SyntaxNode* right = ParseRelationalExpr();
	if (right == NULL) Error("expression expected");
	TryToConvertTypeOrDie(right, left->GetSymType(), op);
	if (!(left->IsLValue())) Error("l-value expected", op);
	return new StmtAssign(left, right);    
}

const Symbol* Parser::FindSymbol(Symbol* sym)
{
	const Symbol* res = NULL;
	for (std::vector<SynTable*>::const_reverse_iterator it = sym_table_stack.rbegin();
		it != sym_table_stack.rend() && res == NULL; ++it)
	{
		res = (*it)->Find(sym);
	}
	return res;
}

const Symbol* Parser::FindSymbolOrDie(Symbol* sym, SymbolClass type, string msg)
{
	const Symbol* res = FindSymbol(sym);
	if (res == NULL) Error("identifier not found");
	if (!(res->GetClassName() & type)) Error(msg);
	return res;
}

const Symbol* Parser::FindSymbolOrDie(Token tok, SymbolClass type, string msg)
{
	Symbol sym(tok);
	return FindSymbolOrDie(&sym, type, msg);
}

const Symbol* Parser::FindSymbol(const Token& tok)
{
	Symbol sym(tok);
	return FindSymbol(&sym);
}

void Parser::Parse()
{
	ParseDeclarations(true);
	if (scan.GetToken().GetValue() != TOK_BEGIN) Error("'begin' expected");
	syntax_tree = ParseStatement();
	if (scan.GetToken().GetValue() != TOK_DOT) Error("'.' expected");
}

void Parser::CheckTokOrDie(TokenValue tok_val)
{
	if (scan.GetToken().GetValue() != tok_val)
	{
		stringstream str;
		str << "'" <<  TOKEN_TO_STR[tok_val] << "' expected";
		Error(str.str());
	}        
	scan.NextToken();
}

void Parser::CheckNextTokOrDie(TokenValue tok_val)
{
	scan.NextToken();
	CheckTokOrDie(tok_val);
}

SyntaxNode* Parser::GetIntExprOrDie()
{
	Token err_tok = scan.GetToken();
	SyntaxNode* res = ParseRelationalExpr();
	if (res == NULL) Error("expression expected");
	if (res->GetSymType() != top_type_int) Error("integer expression expected", err_tok);
	return res;
}


SyntaxNode* Parser::ParseFunctionCall(SymProc* funct_name)
{
	NodeCall* funct = new NodeCall(funct_name);
	if (scan.NextToken().GetValue() == TOK_BRACKETS_LEFT)
	{
		scan.NextToken();
		while (scan.GetToken().GetValue() != TOK_BRACKETS_RIGHT)
		{
			Token err_pos_tok = scan.GetToken();
			SyntaxNode* arg = ParseRelationalExpr();
			if (arg == NULL) Error("illegal expression");
			if (scan.GetToken().GetValue() == TOK_COMMA)
				scan.NextToken();
			else if (scan.GetToken().GetValue() != TOK_BRACKETS_RIGHT)
				Error(", expected");
			if (funct->GetCurrentArgType() == NULL) Error("too many actual parameters", err_pos_tok);
			TryToConvertTypeOrDie(arg, funct->GetCurrentArgType(), err_pos_tok);
			if (funct->IsCurrentArfByRef() && !arg->IsLValue()) Error("lvalue expected", err_pos_tok);
			funct->AddArg(arg);
		}
		scan.NextToken();
	}
	if (funct->GetCurrentArgType() != NULL) Error("not enough actual parameters");
	return funct;
}

SyntaxNode* Parser::ParseConstants()
{
	Token token = scan.GetToken();
	SymType* type = NULL;
	if (token.GetType() == INT_CONST) type = top_type_int;
	else if (token.GetType() == REAL_CONST) type = top_type_real;
	else Error("operations on string const not implemented");
	scan.NextToken();
	return new NodeVar(new SymVarConst(token, type));
}

SyntaxNode* Parser::ParseRecordAccess(SyntaxNode* record)
{
	CheckTokOrDie(TOK_DOT);
	Token field = scan.GetToken();
	if (field.GetType() != IDENTIFIER) Error("identifier expected after '.'");
	scan.NextToken();
	if (!(record->GetSymType()->GetClassName() & SYM_TYPE_RECORD)) Error("illegal qualifier", field);
	return new NodeRecordAccess(record, field);
}

SyntaxNode* Parser::ParseArrayAccess(SyntaxNode* array)
{
	Token err_tok = scan.GetToken();
	CheckTokOrDie(TOK_BRACKETS_SQUARE_LEFT);
	while (true)
	{
		SyntaxNode* index = GetIntExprOrDie();
		if (!(array->GetSymType()->GetClassName() & SYM_TYPE_ARRAY)) Error("array expected before '[' token", err_tok);
		array = new NodeArrayAccess(array, index);
		if (scan.GetToken().GetValue() == TOK_COMMA) scan.NextToken();
		else if (scan.GetToken().GetValue() == TOK_BRACKETS_SQUARE_RIGHT)
		{
			if (scan.NextToken().GetValue() == TOK_BRACKETS_SQUARE_LEFT)
				scan.NextToken();
			else
				break;
		}
		else Error("illegal expression");
	}
	return array;
}

SyntaxNode* Parser::ParseFactor()
{
	SyntaxNode* left = NULL;
	if (scan.GetToken().IsConst())
	{
		return ParseConstants();
	}
	else if (scan.GetToken().GetValue() == TOK_BRACKETS_LEFT)
	{
		scan.NextToken();
		left = ParseRelationalExpr();
		if (left == NULL) Error("illegal expression");
		CheckTokOrDie(TOK_BRACKETS_RIGHT);
	}
	else
	{
		if (scan.GetToken().GetType() != IDENTIFIER) return NULL;
		const Symbol* sym = FindSymbolOrDie(scan.GetToken(), SymbolClass(SYM_VAR | SYM_PROC), "identifier not found");
		if (sym->GetClassName() & SYM_VAR)
		{
			left = new NodeVar((SymVar*)sym);
			scan.NextToken();
		}
		else if (sym->GetClassName() & SYM_PROC)
		{
			left = ParseFunctionCall((SymProc*)sym);
		}
		else Error("identifier expected");
	}
	while (scan.GetToken().IsFactorOp())
	{
		if (scan.GetToken().GetValue() == TOK_DOT)
			left = ParseRecordAccess(left);
		else
			left = ParseArrayAccess(left);
	}
	return left;
}

SyntaxNode* Parser::ParseUnaryExpr()
{
	std::vector<Token> un;
	while (scan.GetToken().IsUnaryOp())
	{
		un.push_back(scan.GetToken());
		scan.NextToken();
	}
	SyntaxNode* res = ParseFactor();
	if (un.size() != 0 && res == NULL) Error("illegal expression");
	if (res != NULL)
	{
		for (std::vector<Token>::reverse_iterator it = un.rbegin(); it != un.rend(); ++it)
			res = new NodeUnaryOp(*it, res);
	}
	return res;
}

SyntaxNode* Parser::ParseMultiplyingExpr()
{
	SyntaxNode* left = ParseUnaryExpr();
	if (left == NULL) return NULL;
	Token op = scan.GetToken();
	while (op.IsMultOp())
	{
		scan.NextToken();
		SyntaxNode* right = ParseUnaryExpr();
		if (right == NULL) Error("illegal expression");
		TryToConvertTypeOrDie(left, right, op);
		left = new NodeBinaryOp(op, left, right);
		op = scan.GetToken();
	}
	return left;
}

SyntaxNode* Parser::ParseAddingExpr()
{
	SyntaxNode* left = ParseMultiplyingExpr();
	if (left == NULL) return NULL;
	Token op = scan.GetToken();
	while (op.IsAddingOp())
	{
		scan.NextToken();
		SyntaxNode* right = ParseMultiplyingExpr();
		if (right == NULL) Error("expression expected");
		TryToConvertTypeOrDie(left, right, op);
		left = new NodeBinaryOp(op, left, right);
		op = scan.GetToken();
	}
	return left;
}

SyntaxNode* Parser::ParseRelationalExpr()
{
	SyntaxNode* left = ParseAddingExpr();
	if (left == NULL) return NULL;
	Token op = scan.GetToken();
	while (op.IsRelationalOp())
	{
		scan.NextToken();
		SyntaxNode* right = ParseAddingExpr();
		if (right == NULL) Error("expression expected");
		TryToConvertTypeOrDie(left, right, op);
		left = new NodeBinaryOp(op, left, right);
		op = scan.GetToken();
	}
	return left;
}

void Parser::Error(string msg)
{
	Error(msg, scan.GetToken());
}

void Parser::Error(string msg, Token err_pos_tok)
{
	stringstream s;
	s << err_pos_tok.GetLine() << ':' << err_pos_tok.GetPos() << " ERROR at '" << err_pos_tok.GetName() << "': " << msg;
	throw( CompilerException(s.str()) );
}