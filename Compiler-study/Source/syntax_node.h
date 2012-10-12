#ifndef SYNTAX_NODE
#define SYNTAX_NODE
#include "syntax_node_base.h"
#include "statement_base.h"
#include "scanner.h"
#include "sym_table.h"
#include "exception.h"
#include <vector>

class NodeCall: public SyntaxNode{
private:
	std::vector<SyntaxNode*> args;
	const SymProc* funct;
public: 
	NodeCall(const SymProc* funct_);
	void AddArg(SyntaxNode* arg);
	const SymType* GetCurrentArgType() const;
	bool IsCurrentArfByRef() const;
	virtual void Print(ostream& o, int offset = 0) const;
	virtual const SymType* GetSymType() const;    
};

class NodeBinaryOp: public SyntaxNode{
private:
	Token token;
	SyntaxNode* left;
	SyntaxNode* right;
public:
	NodeBinaryOp(const Token& name, SyntaxNode* left_, SyntaxNode* right_);
	virtual void Print(ostream& o, int offset = 0) const;
	virtual const SymType* GetSymType() const;    
};

class NodeUnaryOp: public SyntaxNode{
protected:
	Token token;
	SyntaxNode* child;
public:
	NodeUnaryOp(const Token& name, SyntaxNode* child_);
	virtual void Print(ostream& o, int offset = 0) const;
	virtual const SymType* GetSymType() const;
};

class NodeIntToRealConv: public NodeUnaryOp{
private:
	SymType* real_type;
public:
	NodeIntToRealConv(SyntaxNode* child_, SymType* real_type_);
	virtual void Print(ostream& o, int offset = 0) const;
	virtual const SymType* GetSymType() const;    
};

class NodeVar: public SyntaxNode{
private:
	SymVar* var;
public:
	NodeVar(Symbol* var_);
	NodeVar(SymVar* var_);
	const SymVar* GetVar();
	virtual const SymType* GetSymType() const;
	virtual void Print(ostream& o, int offset = 0) const;
	virtual bool IsLValue() const;
};

class NodeArrayAccess: public SyntaxNode{
private:
	SyntaxNode* arr;
	SyntaxNode* index;
public:
	NodeArrayAccess(SyntaxNode* arr_, SyntaxNode* index_);
	virtual void Print(ostream& o, int offset = 0) const;
	virtual const SymType* GetSymType() const;    
	virtual bool IsLValue() const;
};

class NodeRecordAccess: public SyntaxNode{
private:
	const SyntaxNode* record;
	const SymVar* field;
public:
	NodeRecordAccess(SyntaxNode* record_, Token field_);
	virtual void Print(ostream& o, int offset = 0) const;
	virtual const SymType* GetSymType() const;    
	virtual bool IsLValue() const;
};

//---Statements---

class StmtAssign: public NodeStatement{
private:
	SyntaxNode* left;
	SyntaxNode* right;
public:
	StmtAssign(SyntaxNode* left_, SyntaxNode* right_);
	const SyntaxNode* GetLeft() const;
	const SyntaxNode* GetRight() const;
	virtual void Print(ostream& o, int offset = 0) const;
};

class StmtBlock: public NodeStatement{
private:
	std::vector<SyntaxNode*> statements;
public:
	void AddStatement(SyntaxNode* new_stmt);
	virtual void Print(ostream& o, int offset = 0) const;    
};

class StmtExpression: public NodeStatement{
private:
	SyntaxNode* expr;
public:
	StmtExpression(SyntaxNode* expression);
	virtual void Print(ostream& o, int offset = 0) const;
};

class StmtFor: public NodeStatement{
private:
	const SymVar* index;
	SyntaxNode* init_val;
	SyntaxNode* last_val;
	bool inc;
	NodeStatement* body;
public:
	StmtFor(const SymVar* index_, SyntaxNode* init_value, SyntaxNode* last_value, bool is_inc, NodeStatement* body_);
	virtual void Print(ostream& o, int offset = 0) const;
};

class StmtWhile: public NodeStatement{
private:
	SyntaxNode* condition;
	NodeStatement* body;
public:
	StmtWhile(SyntaxNode* condition_, NodeStatement* body_);
	virtual void Print(ostream& o, int offset = 0) const;
};

class StmtUntil: public NodeStatement{
private:
	SyntaxNode* condition;
	NodeStatement* body;
public:
	StmtUntil(SyntaxNode* condition_, NodeStatement* body);
	virtual void Print(ostream& o, int offset = 0) const;
};

class StmtIf: public NodeStatement{
private:
	SyntaxNode* condition;
	NodeStatement* then_branch;
	NodeStatement* else_branch;
public:
	StmtIf(SyntaxNode* condition_, NodeStatement* then_branch_, NodeStatement* else_branch_ = NULL);
	virtual void Print(ostream& o, int offset = 0) const;
};


#endif