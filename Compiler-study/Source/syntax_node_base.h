#ifndef SYNTAX_NODE_BASE
#define SYNTAX_NODE_BASE

#include <iostream>
#include "scanner.h"
#include "generator.h"

class SymType;

class SyntaxNode{
public:
	virtual void Print(std::ostream& o, int offset = 0) const;
	virtual const SymType* GetSymType() const;
	virtual bool IsLValue() const;
	virtual void GenerateLValue(AsmCode& asm_code) const;    
	virtual void GenerateValue(AsmCode& asm_code) const;
};

#endif