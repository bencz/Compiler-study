#include "syntax_node.h"

static void Error(string msg, Token token)
{
	stringstream s;
	s << token.GetLine() << ':' << token.GetPos() << " ERROR at '" << token.GetName() << "': " << msg;
	throw( CompilerException(s.str()) );
}

//---NodeCall---

NodeCall::NodeCall(const SymProc* funct_):
	funct(funct_)
{
}

void NodeCall::AddArg(SyntaxNode* arg)
{
	args.push_back(arg);
}

const SymType* NodeCall::GetCurrentArgType() const
{
	if (args.size() >= funct->GetArgsCount()) return NULL;
	return funct->GetArg(args.size())->GetVarType();
}

bool NodeCall::IsCurrentArfByRef() const
{
	if (args.size() >= funct->GetArgsCount()) return NULL;
	return funct->GetArg(args.size())->IsByRef();
}

void NodeCall::Print(ostream& o, int offset) const 
{
	PrintSpaces(o, offset) << "() " << funct->GetName() << " [";
	GetSymType()->Print(o , offset);
	o << "]\n";
	for (std::vector<SyntaxNode*>::const_iterator it = args.begin(); it != args.end(); ++it)
	{
		(*it)->Print(o, offset + 1);
	}
}

const SymType* NodeCall::GetSymType() const
{
	return funct->GetResultType();
}

void NodeCall::GenerateValue(AsmCode& asm_code) const
{
	if (funct->GetResultType()->GetSize()) asm_code.AddCmd(ASM_SUB, AsmImmidiate(funct->GetResultType()->GetSize()), REG_ESP);
	for (int i = 0; i < args.size(); ++i)
		if (funct->GetArg(i)->IsByRef())
			args[i]->GenerateLValue(asm_code);
		else
			args[i]->GenerateValue(asm_code);
	asm_code.AddCmd(ASM_CALL, AsmMemory(funct->GetLabel()));
}

//---NodeWriteCall---

void NodeWriteCall::AddArg(SyntaxNode* arg)
{
	args.push_back(arg);
}

void NodeWriteCall::GenerateValue(AsmCode& asm_code) const
{
	for (std::vector<SyntaxNode*>::const_iterator it = args.begin(); it != args.end(); ++it)
	{
		(*it)->GenerateValue(asm_code);
		const SymType* type = (*it)->GetSymType();
		if (type == top_type_int)
			asm_code.CallWriteForInt();
		else if (type == top_type_real)
			asm_code.CallWriteForReal();
		else
			asm_code.CallWriteForStr();
	}
}

const SymType* NodeWriteCall::GetSymType() const
{
	return top_type_untyped;
}

//---NodeBinaryOp---

void NodeBinaryOp::FinGenForRelationalOp(AsmCode& asm_code) const
{
	/*
	movl    b, %edx
	movl    c, %eax
	cmpl    %eax, %edx
	setg    %al
	movzbl  %al, %eax
	*/
	asm_code.AddCmd(ASM_CMP, REG_EBX, REG_EAX); 
	switch (token.GetValue())
	{
	case TOK_GREATER:
		asm_code.AddCmd(ASM_SETG, REG_AL, SIZE_NONE);
		break;
	case TOK_GREATER_OR_EQUAL:
		asm_code.AddCmd(ASM_SETGE, REG_AL, SIZE_NONE);
		break;
	case TOK_LESS:
		asm_code.AddCmd(ASM_SETL, REG_AL, SIZE_NONE);        
		break;
	case TOK_LESS_OR_EQUAL:
		asm_code.AddCmd(ASM_SETLE, REG_AL, SIZE_NONE);
		break;
	case TOK_EQUAL:
		asm_code.AddCmd(ASM_SETE, REG_AL, SIZE_NONE);
		break;
	case TOK_NOT_EQUAL:
		asm_code.AddCmd(ASM_SETNE, REG_AL, SIZE_NONE);            
	}
	asm_code.AddCmd(ASM_MOVZB, REG_AL, REG_EAX);
	asm_code.AddCmd(ASM_PUSH, REG_EAX);
}

void NodeBinaryOp::GenerateForInt(AsmCode& asm_code) const
{
	left->GenerateValue(asm_code);
	right->GenerateValue(asm_code);
	asm_code.AddCmd(ASM_POP, REG_EBX);
	asm_code.AddCmd(ASM_POP, REG_EAX);
	switch (token.GetValue())
	{
	case TOK_PLUS:
		asm_code.AddCmd(ASM_ADD, REG_EBX, REG_EAX);
		break;
	case TOK_MINUS:
		asm_code.AddCmd(ASM_SUB, REG_EBX, REG_EAX);
		break;
	case TOK_MULT:
		asm_code.AddCmd(ASM_XOR, REG_EDX, REG_EDX);
		asm_code.AddCmd(ASM_IMUL, REG_EBX);
		break;
	case TOK_DIV:
		asm_code.AddCmd(ASM_XOR, REG_EDX, REG_EDX);
		asm_code.AddCmd(ASM_IDIV, REG_EBX);            
		break;
	case TOK_MOD:
		asm_code.AddCmd(ASM_XOR, REG_EDX, REG_EDX);
		asm_code.AddCmd(ASM_IDIV, REG_EBX);
		asm_code.AddCmd(ASM_PUSH, REG_EDX);
		return;
	case TOK_OR:
		asm_code.AddCmd(ASM_OR, REG_EBX, REG_EAX);
		break;
	case TOK_XOR:
		asm_code.AddCmd(ASM_XOR, REG_EBX, REG_EAX);
		break;
	case TOK_SHR:
		asm_code.AddCmd(ASM_SAR, REG_EAX, REG_EBX);
		asm_code.AddCmd(ASM_PUSH, REG_EBX);
		return;
		break;
	case TOK_SHL:
		asm_code.AddCmd(ASM_SAL, REG_EAX, REG_EBX);
		asm_code.AddCmd(ASM_PUSH, REG_EBX);
		return;
		break;
	case TOK_NOT:
		asm_code.AddCmd(ASM_NOT, REG_EBX, REG_EAX);
		break;
	default:
		FinGenForRelationalOp(asm_code);
		return;
	}
	asm_code.AddCmd(ASM_PUSH, REG_EAX);
}

void NodeBinaryOp::GenerateForReal(AsmCode& asm_code) const
{
	//TODO
}

NodeBinaryOp::NodeBinaryOp(const Token& name, SyntaxNode* left_, SyntaxNode* right_):
	token(name),
	left(left_),
	right(right_)
{
}

void NodeBinaryOp::Print(ostream& o, int offset) const 
{
	PrintSpaces(o, offset) << token.GetName() << " [";
	GetSymType()->Print(o , offset);
	o << "]\n";
	left->Print(o, offset + 1);
	right->Print(o, offset + 1);
}

const SymType* NodeBinaryOp::GetSymType() const
{
	return left->GetSymType();
}

void NodeBinaryOp::GenerateValue(AsmCode& asm_code) const
{
	if (GetSymType() == top_type_int) GenerateForInt(asm_code);
	else GenerateForReal(asm_code);
}

//---NodeUnaryOp---

NodeUnaryOp::NodeUnaryOp(const Token& name, SyntaxNode* child_):
	token(name),
	child(child_)
{
}

void NodeUnaryOp::Print(ostream& o, int offset) const 
{
	PrintSpaces(o, offset) << token.GetName() << " [";
	GetSymType()->Print(o , offset);
	o << "]\n";
	child->Print(o, offset + 1);
}

const SymType* NodeUnaryOp::GetSymType() const
{
	return child->GetSymType();
}

// NodeIntToRealConv

NodeIntToRealConv::NodeIntToRealConv(SyntaxNode* child_, SymType* real_type_):
	NodeUnaryOp(Token(), child_),
	real_type(real_type_)     
{
}

void NodeIntToRealConv::Print(ostream& o, int offset) const 
{
	PrintSpaces(o, offset) << "IntToReal [";
	real_type->Print(o, 0);
	o << "]\n";
	child->Print(o, offset + 1);
}

const SymType* NodeIntToRealConv::GetSymType() const
{
	return real_type;
}

void NodeIntToRealConv::GenerateValue(AsmCode& asm_code) const
{
	child->GenerateValue(asm_code);
	asm_code.AddCmd(ASM_FILD, AsmMemory(REG_ESP), SIZE_SHORT);
	asm_code.AddCmd(ASM_FSTP, AsmMemory(REG_ESP), SIZE_SHORT);
}

//---NodeVar---

NodeVar::NodeVar(const SymVar* var_):
	var(var_)
{
}

const SymVar* NodeVar::GetVar()
{
	return var;
}

const SymType* NodeVar::GetSymType() const
{
	return var->GetVarType()->GetActualType();
}

void NodeVar::Print(ostream& o, int offset) const 
{
	var->PrintAsNode(o, offset);
}

bool NodeVar::IsLValue() const
{
	return !(var->GetClassName() & SYM_VAR_CONST);
}

void NodeVar::GenerateLValue(AsmCode& asm_code) const
{
	var->GenerateLValue(asm_code);
}

void NodeVar::GenerateValue(AsmCode& asm_code) const
{
	var->GenerateValue(asm_code);
}

//---NodeArrayAccess----

NodeArrayAccess::NodeArrayAccess(SyntaxNode* arr_, SyntaxNode* index_):
	arr(arr_),
	index(index_)
{
}

void NodeArrayAccess::Print(ostream& o, int offset) const 
{
	PrintSpaces(o, offset) << "[] [";
	GetSymType()->Print(o, offset);
	o << "]\n";
	arr->Print(o, offset+1);
	index->Print(o, offset+1);
}

const SymType* NodeArrayAccess::GetSymType() const
{
	return ((SymTypeArray*)arr->GetSymType())->GetElemType();
}

bool NodeArrayAccess::IsLValue() const
{
	return true;
}

void NodeArrayAccess::ComputeIndexToEax(AsmCode& asm_code) const
{
	arr->GenerateLValue(asm_code);
	index->GenerateValue(asm_code);
	asm_code.AddCmd(ASM_MOV, GetSymType()->GetSize(), REG_EBX);
	asm_code.AddCmd(ASM_POP, REG_EAX);
	asm_code.AddCmd(ASM_SUB, ((SymTypeArray*)arr->GetSymType())->GetLow(), REG_EAX);
	asm_code.AddCmd(ASM_XOR, REG_EDX, REG_EDX);
	asm_code.AddCmd(ASM_MUL, REG_EBX);
	asm_code.AddCmd(ASM_POP, REG_EBX);
	asm_code.AddCmd(ASM_ADD, REG_EBX, REG_EAX);
}

void NodeArrayAccess::GenerateLValue(AsmCode& asm_code) const
{
	ComputeIndexToEax(asm_code);
	asm_code.AddCmd(ASM_PUSH, REG_EAX);
}

void NodeArrayAccess::GenerateValue(AsmCode& asm_code) const
{
	ComputeIndexToEax(asm_code);
	if (GetSymType()->GetSize() == 4) 
		asm_code.AddCmd(ASM_PUSH, AsmMemory(REG_EAX));
	else
	{
		asm_code.AddCmd(ASM_PUSH, REG_EAX);
		asm_code.PushMemory(GetSymType()->GetSize());
	}
}

//---NodeRecordAccess---

NodeRecordAccess::NodeRecordAccess(SyntaxNode* record_, Token field_):
	record(record_)
{
	const SymVarLocal* var = ((SymTypeRecord*)record_->GetSymType())->FindField(field_);
	if (var == NULL) Error("unknown record field identifier", field_);
	field = var;
}

void NodeRecordAccess::Print(ostream& o, int offset) const
{
	PrintSpaces(o, offset) << ". [";
	GetSymType()->Print(o , offset);
	o << "]\n";
	record->Print(o, offset + 1);
	field->PrintAsNode(o, offset + 1);
}

const SymType* NodeRecordAccess::GetSymType() const
{
	return field->GetVarType();
}

bool NodeRecordAccess::IsLValue() const
{
	return true;
}

void NodeRecordAccess::GenerateLValue(AsmCode& asm_code) const
{
	record->GenerateLValue(asm_code);
	asm_code.AddCmd(ASM_POP, REG_EAX);
	asm_code.AddCmd(ASM_LEA, AsmMemory(REG_EAX, field->GetOffset()), REG_EAX);
	asm_code.AddCmd(ASM_PUSH, REG_EAX);
}

void NodeRecordAccess::GenerateValue(AsmCode& asm_code) const
{
	GenerateLValue(asm_code);
	asm_code.PushMemory(field->GetVarType()->GetSize());
}

//---StmtAssgn---

StmtAssign::StmtAssign(SyntaxNode* left_, SyntaxNode* right_):
	left(left_),
	right(right_)
{
}

const SyntaxNode* StmtAssign::GetLeft() const
{
	return left;
}

const SyntaxNode* StmtAssign::GetRight() const
{
	return right;
}

void StmtAssign::Print(ostream& o, int offset) const 
{
	PrintSpaces(o, offset) << ":= \n";
	left->Print(o, offset + 1);
	right->Print(o, offset + 1);
}

void StmtAssign::Generate(AsmCode& asm_code) const
{
	right->GenerateValue(asm_code);
	left->GenerateLValue(asm_code);
	asm_code.MoveToMemoryFromStack(left->GetSymType()->GetSize());
}

//---StmtBlock---

void StmtBlock::AddStatement(NodeStatement* new_stmt)
{
	statements.push_back(new_stmt);
}

void StmtBlock::Print(ostream& o, int offset) const
{
	PrintSpaces(o, offset) << "begin\n";
	for (vector<NodeStatement*>::const_iterator it = statements.begin(); it != statements.end(); ++it)
		(*it)->Print(o, offset + 1);
	PrintSpaces(o, offset) << "end\n";
}

void StmtBlock::Generate(AsmCode& asm_code) const
{
	for (vector<NodeStatement*>::const_iterator it = statements.begin(); it != statements.end(); ++it)
		(*it)->Generate(asm_code);
}


//---StmtExpression---

StmtExpression::StmtExpression(SyntaxNode* expression):
	expr(expression)
{
}

void StmtExpression::Print(ostream& o, int offset) const
{
	expr->Print(o, offset);
}

void StmtExpression::Generate(AsmCode& asm_code) const
{
	expr->GenerateValue(asm_code);
	asm_code.AddCmd(ASM_ADD, expr->GetSymType()->GetSize(), REG_ESP);
}

//---StmtFor---

StmtFor::StmtFor(const SymVar* index_, SyntaxNode* init_value, SyntaxNode* last_value, bool is_inc, NodeStatement* body_):
	index(index_),
	init_val(init_value),
	last_val(last_value),
	inc(is_inc),
	body(body_)
{
}

void StmtFor::Print(ostream& o, int offset) const
{
	PrintSpaces(o, offset) << "for " << (inc ? "to \n" : "downto \n");
	index->PrintAsNode(o, offset + 1);
	init_val->Print(o, offset + 1);
	last_val->Print(o, offset + 1);
	body->Print(o, offset);
}

void StmtFor::Generate(AsmCode& asm_code) const
{
	StmtAssign(new NodeVar(index), init_val).Generate(asm_code);
	AsmImmidiate start_label(asm_code.GenLabel("for_check"));
	AsmImmidiate check_label(asm_code.GenLabel("for_check"));
	AsmImmidiate fin_label(asm_code.GenLabel("for_fin"));
	last_val->GenerateValue(asm_code);
	asm_code.AddCmd(ASM_JMP, check_label, SIZE_NONE);
	asm_code.AddLabel(start_label);
	body->Generate(asm_code);
	index->GenerateLValue(asm_code);
	asm_code.AddCmd(ASM_POP, REG_EAX);
	if (inc) asm_code.AddCmd(ASM_ADD, 1, AsmMemory(REG_EAX));
	else asm_code.AddCmd(ASM_SUB, 1, AsmMemory(REG_EAX));
	asm_code.AddLabel(check_label);
	index->GenerateValue(asm_code);
	asm_code.AddCmd(ASM_POP, REG_EAX);
	asm_code.AddCmd(ASM_CMP, REG_EAX, AsmMemory(REG_ESP));
	if (inc) asm_code.AddCmd(ASM_JNL, start_label, SIZE_NONE);
	else asm_code.AddCmd(ASM_JNG, start_label, SIZE_NONE);
	asm_code.AddLabel(fin_label);
	asm_code.AddCmd(ASM_ADD, 4, REG_ESP);
}

//---StmtWhile---

StmtWhile::StmtWhile(SyntaxNode* condition_, NodeStatement* body_):
	condition(condition_),
	body(body_)
{
}

void StmtWhile::Print(ostream& o, int offset) const
{
	PrintSpaces(o, offset) << "while\n";
	condition->Print(o, offset + 1);
	body->Print(o, offset + 1);
}

void StmtWhile::Generate(AsmCode& asm_code) const
{
	AsmImmidiate label_check(asm_code.GenLabel("while_check"));
	AsmImmidiate label_fin(asm_code.GenLabel("while_fin"));
	asm_code.AddLabel(label_check);
	condition->GenerateValue(asm_code);
	asm_code.AddCmd(ASM_POP, REG_EAX);
	asm_code.AddCmd(ASM_TEST, REG_EAX, REG_EAX);
	asm_code.AddCmd(ASM_JZ, label_fin, SIZE_NONE);
	body->Generate(asm_code);
	asm_code.AddCmd(ASM_JMP, label_check, SIZE_NONE);
	asm_code.AddLabel(label_fin);
}

//---StmtUntil---

StmtUntil::StmtUntil(SyntaxNode* condition_, NodeStatement* body_):
	condition(condition_),
	body(body_)
{
}

void StmtUntil::Print(ostream& o, int offset) const
{
	PrintSpaces(o, offset) << "until\n";
	condition->Print(o, offset + 1);
	body->Print(o, offset + 1);
}

void StmtUntil::Generate(AsmCode& asm_code) const
{
	AsmImmidiate label_start(asm_code.GenLabel("until_check"));
	asm_code.AddLabel(label_start);
	body->Generate(asm_code);
	condition->GenerateValue(asm_code);
	asm_code.AddCmd(ASM_POP, REG_EAX);
	asm_code.AddCmd(ASM_TEST, REG_EAX, REG_EAX);
	asm_code.AddCmd(ASM_JZ, label_start, SIZE_NONE);
}

//---StmtIf---

StmtIf::StmtIf(SyntaxNode* condition_, NodeStatement* then_branch_, NodeStatement* else_branch_):
	condition(condition_),
	then_branch(then_branch_),
	else_branch(else_branch_)
{
}

void StmtIf::Print(ostream& o, int offset) const
{
	PrintSpaces(o, offset) << "if\n";
	condition->Print(o, offset + 1);
	PrintSpaces(o, offset) << "then\n";
	then_branch->Print(o, offset + 1);
	if (else_branch != NULL)
	{
		PrintSpaces(o, offset) << "else\n";
		else_branch->Print(o, offset + 1);
	}
}

void StmtIf::Generate(AsmCode& asm_code) const
{
	AsmImmidiate label_else(asm_code.GenLabel("else"));
	AsmImmidiate label_fin(asm_code.GenLabel("fin"));
	condition->GenerateValue(asm_code);
	asm_code.AddCmd(ASM_POP, REG_EAX);
	asm_code.AddCmd(ASM_TEST, REG_EAX, REG_EAX);
	asm_code.AddCmd(ASM_JZ, label_else, SIZE_NONE);
	then_branch->Generate(asm_code);
	asm_code.AddCmd(ASM_JMP, label_fin, SIZE_NONE);
	asm_code.AddLabel(label_else);
	if (else_branch != NULL) else_branch->Generate(asm_code);
	asm_code.AddCmd(ASM_JMP, label_fin, SIZE_NONE);
	asm_code.AddLabel(label_fin);    
}