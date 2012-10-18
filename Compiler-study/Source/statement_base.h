#ifndef STATEMENT
#define STATEMENT

#include "syntax_node_base.h"
#include "scanner.h"
#include <vector>

class NodeStatement: public SyntaxNode{
public:
	virtual void Print(ostream& o, int offset = 0) const;
	virtual void Generate(AsmCode& asm_code) const;
};

#endif