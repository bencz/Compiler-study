#ifndef PARSER
#define PARSER

#include <stdlib.h>
#include "scanner.h"
#include "sym_table.h"
#include "syntax_node.h"
#include "generator.h"
#include "exception.h"
#include <string.h>
#include <vector>
#include <utility>
#include <stack>
#include <ostream>

class Parser{
private:
	NodeStatement* body;
	Scanner& scan;
	SymTable top_sym_table;
	SymType* top_type_bool;
	std::vector<SymTable*> sym_table_stack;
	AsmCode asm_code;
	SyntaxNode* ConvertType(SyntaxNode* node, const SymType* type);
	void TryToConvertType(SyntaxNode*& first, SyntaxNode*& second);
	void TryToConvertType(SyntaxNode*& expr, const SymType* type);
	void TryToConvertTypeOrDie(SyntaxNode*& first, SyntaxNode*& second, Token tok_err);
	void TryToConvertTypeOrDie(SyntaxNode*& expr, const SymType* type, Token tok_err);
	void CheckTokOrDie(TokenValue tok_val);
	void CheckNextTokOrDie(TokenValue tok_val);    
	SyntaxNode* GetIntExprOrDie();
	SyntaxNode* ParseConstants();
	SyntaxNode* ParseFunctionCall(SymProc* funct_if);
	SyntaxNode* ParseWriteFunctCall();
	SyntaxNode* ParseRecordAccess(SyntaxNode* record);
	SyntaxNode* ParseArrayAccess(SyntaxNode* array);
	SyntaxNode* ParseFactor();
	SyntaxNode* ParseAddingExpr();
	SyntaxNode* ParseMultiplyingExpr();
	SyntaxNode* ParseUnaryExpr();
	SyntaxNode* ParseRelationalExpr();
	void Error(string msg, Token err_pos_tok);
	void Error(string msg);
	SymType* ParseArrayType();
	SymType* ParseRecordType();
	SymType* ParsePointerType();
	SymType* ParseType();
	void ParseVarDeclarations(bool is_global = true);
	void ParseTypeDeclarations();
	void ParseDeclarations(bool is_global = true);
	void ParseFunctionParameters(SymProc* funct);
	void ParseFunctionDefinition();    
	NodeStatement* ParseBlockStatement();    
	NodeStatement* ParseStatement();
	NodeStatement* ParseForStatement();
	NodeStatement* ParseWhileStatement();
	NodeStatement* ParseUntilStatement();
	NodeStatement* ParseIfStatement();
	NodeStatement* ParseAssignStatement();
	const Symbol* FindSymbol(Symbol* sym);
	const Symbol* FindSymbolOrDie(Symbol* sym, SymbolClass type, string msg);
	const Symbol* FindSymbolOrDie(Token tok, SymbolClass type, string msg); 
	const Symbol* FindSymbol(const Token& tok);
	void Parse();
public:
	Parser(Scanner& scanner);
	void PrintSyntaxTree(ostream& o);
	void PrintSymTable(ostream& o);
	void Generate(ostream& o);
};

#endif