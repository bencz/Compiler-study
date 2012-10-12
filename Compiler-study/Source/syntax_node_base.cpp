#include "syntax_node_base.h"

//---SyntaxNode---

void SyntaxNode::Print(ostream& o, int offset) const 
{
}

const SymType* SyntaxNode::GetSymType() const
{
	return NULL;
}

bool SyntaxNode::IsLValue() const
{
	return false;
}
