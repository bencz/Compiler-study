#include "sym_table.h"

const string SymbolClassDescription[] = {
	"SYM",
	"SYM_FUNCT",
	"SYM_PROC",
	"SYM_TYPE",
	"SYM_TYPE_SCALAR",
	"SYM_TYPE_INTEGER",
	"SYM_TYPE_REAL",
	"SYM_TYPE_ARRAY",
	"SYM_VAR",
};

SymType* top_type_int = new SymTypeInteger(Token("Integer", RESERVED_WORD, TOK_INTEGER, -1, -1));
SymType* top_type_real = new SymTypeReal(Token("Real", RESERVED_WORD, TOK_REAL, -1, -1));
SymType* top_type_untyped = new SymTypeUntyped();
SymType* top_type_str = new SymType(Token("String", RESERVED_WORD, TOK_STRING, -1, -1));

//---Symbol---

Symbol::Symbol(Token token_)
{
	if (token_.GetType() != STR_CONST) token_.NameToLowerCase();
	token = token_;
}

Symbol::Symbol(const Symbol& sym):
	token(sym.token)
{
}

const char* Symbol::GetName() const
{
	return token.GetName();
}

SymbolClass Symbol::GetClassName() const
{
	return SYM;
}

void Symbol::Print(ostream& o, int offset) const
{
	PrintSpaces(o, offset) << token.GetName();
}

void Symbol::PrintVerbose(ostream& o, int offset) const
{
	Print(o, offset);
}

//---SymType---

SymType::SymType():
	Symbol(Token())
{
}

SymType::SymType(Token name):
	Symbol(name)
{
}

SymbolClass SymType::GetClassName() const
{
	return SymbolClass(SYM | SYM_TYPE);
}

void SymType::Print(ostream& o, int offset) const
{
	o << token.GetName();
}

const SymType* SymType::GetActualType() const
{
	return this;
}

unsigned SymType::GetSize() const
{
	return 4;
}

//---SymProc---

void SymProc::PrintPrototype(ostream& o, int offset) const
{
	o << token.GetName();
	if (params.size() > 0)
	{
		o << "(";
		params.front()->Print(o, 0);
		for (vector<SymVarParam*>::const_iterator it = ++params.begin(); it != params.end(); ++it)
		{
			o << "; ";
			if ((*it)->IsByRef()) o << "var ";
			(*it)->Print(o, 0);
		}
		o << ")";
	}
}

SymProc::SymProc(Token name):
	Symbol(name),
	sym_table(NULL)
{
}

void SymProc::AddSymTable(SymTable* syn_table_)
{
	sym_table = syn_table_;
}

void SymProc::AddParam(SymVarParam* param)
{
	params.push_back(param);
}

int SymProc::GetArgsCount() const
{
	return params.size();
}

const SymVarParam* SymProc::GetArg(int arg_num) const
{
	return params[arg_num];
}

void SymProc::AddBody(NodeStatement* body_)
{
	body = body_;
}

SymProc::SymProc(Token token, SymTable* syn_table_):
	Symbol(token),
	sym_table(syn_table_)
{
}

SymbolClass SymProc::GetClassName() const
{
	return SymbolClass(SYM | SYM_PROC);
}

const SymType* SymProc::GetResultType() const
{
	return top_type_untyped;
}

void SymProc::PrintVerbose(ostream& o, int offset) const
{
	Print(o, offset);
	o << ";\n";
	if (!sym_table->IsEmpty())
	{
		o << "var\n";
		sym_table->Print(o, offset + 1);
	}
	body->Print(o, offset);
}

void SymProc::Print(ostream& o, int offset) const
{
	o << "procedure ";
	PrintPrototype(o, offset);
}

void SymProc::GenerateDeclaration(AsmCode& asm_code)
{
	label = asm_code.LabelByStr(token.GetName());
	asm_code.AddLabel(label);
	asm_code.AddCmd(ASM_PUSH, REG_EBP);
	asm_code.AddCmd(ASM_MOV, REG_ESP, REG_EBP);
	if (sym_table->GetLocalsSize()) asm_code.AddCmd(ASM_SUB, AsmImmidiate(sym_table->GetLocalsSize()), REG_ESP);
	body->Generate(asm_code);    
	asm_code.AddCmd(ASM_MOV, REG_EBP, REG_ESP);
	asm_code.AddCmd(ASM_POP, REG_EBP);
	asm_code.AddCmd(ASM_RET, AsmImmidiate(sym_table->GetParamsSize() - GetResultType()->GetSize()));
}

AsmImmidiate SymProc::GetLabel() const
{
	return label;
}

//---SymFunct---

SymFunct::SymFunct(Token token_, SymTable* syn_table, const SymType* result_type_):
	SymProc(token, syn_table),
	result_type(result_type_)
{
}

SymFunct::SymFunct(Token name):
	SymProc(name),
	result_type(NULL)
{
}

void SymFunct::AddResultType(const SymType* result_type_)
{
	result_type = result_type_;
	Token tok("Result", IDENTIFIER, TOK_UNRESERVED, -1, -1);
	SymVarParam* param = new SymVarParam(tok, result_type, false, sym_table->GetParamsSize() + 8);
	sym_table->Add(param);
}

SymbolClass SymFunct::GetClassName() const
{
	return SymbolClass(SYM | SYM_TYPE | SYM_PROC | SYM_FUNCT);
}

const SymType* SymFunct::GetResultType() const
{
	return result_type->GetActualType();
}

void SymFunct::Print(ostream& o, int offset) const
{
	o << "function ";
	PrintPrototype(o, offset);
	o << ": ";
	result_type->Print(o, 0);
}

//---SymVar---

SymVar::SymVar(Token token, const SymType* type_):
	Symbol(token),
	type(type_)
{
}

SymbolClass SymVar::GetClassName() const
{
	return SymbolClass(SYM | SYM_VAR);
}

void SymVar::Print(ostream& o, int offset) const
{
	o << token.GetName() << ": ";
	type->Print(o, 0);
}

void SymVar::PrintVerbose(ostream& o, int offset) const
{
	PrintSpaces(o, offset) << token.GetName() << ": ";
	if (type->GetClassName() & SYM_TYPE_ALIAS)
		type->Print(o, offset + 1);
	else
		type->PrintVerbose(o, offset + 1);
	o << "\n";
}

const SymType* SymVar::GetVarType() const
{
	return type->GetActualType();
}

void SymVar::PrintAsNode(ostream& o, int offset) const
{
	PrintSpaces(o, offset) << token.GetName() << " [";
	type->Print(o, 0);
	o << "]\n";
}

void SymVar::GenerateLValue(AsmCode& asm_code) const
{
}

void SymVar::GenerateValue(AsmCode& asm_code) const
{
}

//---SymTypeScalar---

SymTypeScalar::SymTypeScalar(Token name):
	SymType(name)
{
}

SymbolClass SymTypeScalar::GetClassName() const
{
	return SymbolClass(SYM | SYM_TYPE | SYM_TYPE_SCALAR);
}

//---SymTypeInteger---

SymTypeInteger::SymTypeInteger(Token name):
	SymTypeScalar(name)
{
}

SymbolClass SymTypeInteger::GetClassName() const
{
	return SymbolClass(SYM | SYM_TYPE | SYM_TYPE_SCALAR | SYM_TYPE_INTEGER);
}

//---SymTypeFloat---

SymTypeReal::SymTypeReal(Token name):
	SymTypeScalar(name)
{
}

SymbolClass SymTypeReal::GetClassName() const
{
	return SymbolClass(SYM | SYM_TYPE | SYM_TYPE_SCALAR | SYM_TYPE_REAL);
}

//---SymTypeUntyped

SymTypeUntyped::SymTypeUntyped():
	SymTypeScalar(Token("untyped", RESERVED_WORD, TOK_UNRESERVED, -1, -1))
{
}

SymbolClass SymTypeUntyped::GetClassName() const
{
	return SymbolClass(SYM | SYM_TYPE | SYM_TYPE_SCALAR | SYM_TYPE_UNTYPED);
}

unsigned SymTypeUntyped::GetSize() const
{
	return 0;
}


//---SymTypeArray---

SymTypeArray::SymTypeArray(SymType* elem_type_, int low_, int high_):
	elem_type(elem_type_),
	low(low_),
	high(high_)
{
}

int SymTypeArray::GetLow()
{
	return low;
}

int SymTypeArray::GetHigh()
{
	return high;
}

const SymType* SymTypeArray::GetElemType()
{
	return elem_type->GetActualType();
}

SymbolClass SymTypeArray::GetClassName() const
{
	return SymbolClass(SYM | SYM_TYPE | SYM_TYPE_ARRAY);
}

void SymTypeArray::Print(ostream& o, int offset) const
{
	o << "array";
}

void SymTypeArray::PrintVerbose(ostream& o, int offset) const
{
	o << "array [" << low << ".." << high << "] of ";
	elem_type->PrintVerbose(o, offset);
}

unsigned SymTypeArray::GetSize() const
{
	return elem_type->GetSize() * (high - low + 1);
}

//---SymTypeRecord---

SymTypeRecord::SymTypeRecord(SymTable* sym_table_):
	sym_table(sym_table_)
{
}

const SymVarLocal* SymTypeRecord::FindField(Token& field_name)
{
	const Symbol* res = sym_table->Find(field_name);
	return (const SymVarLocal*)res;
}

SymbolClass SymTypeRecord::GetClassName() const
{
	return SymbolClass(SYM | SYM_TYPE | SYM_TYPE_RECORD);
}

void SymTypeRecord::Print(ostream& o, int offset) const
{
	o << "record";
}

void SymTypeRecord::PrintVerbose(ostream& o, int offset) const
{
	o << "record\n";
	sym_table->Print(o, offset);
	PrintSpaces(o, offset - 1) << "end";    
}

unsigned SymTypeRecord::GetSize() const
{
	return sym_table->GetSize();
}

//---SymTypeAlias---

SymTypeAlias::SymTypeAlias(Token name, SymType* target_):
	SymType(name),
	target(target_)
{    
}

void SymTypeAlias::Print(ostream& o, int offset) const
{
	o << token.GetName();
}


void SymTypeAlias::PrintVerbose(ostream& o, int offset) const
{
	PrintSpaces(o, offset) << token.GetName() << " = ";
	target->PrintVerbose(o, offset + 1);
	o << "\n";
}

SymbolClass SymTypeAlias::GetClassName() const
{
	return SymbolClass(SYM | SYM_TYPE | SYM_TYPE_ALIAS);
}

const SymType* SymTypeAlias::GetActualType() const
{
	return target->GetActualType();
}

unsigned SymTypeAlias::GetSize() const
{
	return target->GetSize();
}

//---SymTypePointer---

SymTypePointer::SymTypePointer(SymType* ref_type_):
	ref_type(ref_type_)
{  
}

void SymTypePointer::Print(ostream& o, int offset) const
{
	o << '^';
	ref_type->Print(o, offset);
}

SymbolClass  SymTypePointer::GetClassName() const
{
	return SymbolClass(SYM | SYM_TYPE | SYM_TYPE_POINTER);
}

//---SymVarConst---


SymVarConst::SymVarConst(Token name, const SymType* type):
	SymVar(name, type)
{
}

SymbolClass SymVarConst::GetClassName() const
{
	return SymbolClass(SYM | SYM_VAR | SYM_VAR_CONST);
}

void SymVarConst::GenerateLValue(AsmCode& asm_code) const
{
	throw("cant get const l-value");
}

void SymVarConst::GenerateValue(AsmCode& asm_code) const
{
	if (token.GetType() == INT_CONST)
	{
		asm_code.AddCmd(ASM_PUSH, AsmImmidiate(token.GetName()));
	}
	else if (token.GetType() == REAL_CONST)
	{
		AsmImmidiate label = asm_code.AddData(asm_code.GenStrLabel("float"), token.GetName(), DATA_REAL);
		asm_code.AddCmd(ASM_PUSH, AsmMemory(label));
	}
	else
	{
		AsmImmidiate label = asm_code.AddData(asm_code.GenStrLabel("str"), token.GetName(), DATA_STR);
		asm_code.AddCmd(ASM_PUSH, label);
	}

}

//---SymVarParam---

void SymVarParam::GenAdrInStack(AsmCode& asm_code) const
{
	asm_code.AddCmd(ASM_LEA, AsmMemory(REG_EBP, offset), REG_EAX);
	asm_code.AddCmd(ASM_PUSH, REG_EAX);
}

void SymVarParam::GenValueInStack(AsmCode& asm_code) const
{
	if (by_ref || type->GetSize() == 4)
		asm_code.AddCmd(ASM_PUSH, AsmMemory(REG_EBP, offset));
	else
	{
		GenAdrInStack(asm_code);        
		asm_code.PushMemory(type->GetSize());
	}
}

void SymVarParam::GenValueByRef(AsmCode& asm_code) const
{
	asm_code.AddCmd(ASM_PUSH, AsmMemory(REG_EBP, offset));
	asm_code.PushMemory(type->GetSize());
}

SymVarParam::SymVarParam(Token name, const SymType* type, bool by_ref_, int offset_):
	SymVar(name, type),
	by_ref(by_ref_),
	offset(offset_)
{
}

bool SymVarParam::IsByRef() const
{
	return by_ref;
}

SymbolClass SymVarParam::GetClassName() const
{
	return SymbolClass(SYM | SYM_VAR | SYM_VAR_PARAM);
}

void SymVarParam::GenerateLValue(AsmCode& asm_code) const
{
	if (by_ref) GenValueInStack(asm_code);
	else GenAdrInStack(asm_code);
}

void SymVarParam::GenerateValue(AsmCode& asm_code) const
{
	if (by_ref) GenValueByRef(asm_code);
	else GenValueInStack(asm_code);
}

//---SymVarGlobal---

SymVarGlobal::SymVarGlobal(Token name, const SymType* type):
	SymVar(name, type)
{
}

void SymVarGlobal::SetLabel(AsmImmidiate& new_label)
{
	label = new_label;
}

AsmImmidiate SymVarGlobal::GetLabel() const
{
	return label;
}

SymbolClass SymVarGlobal::GetClassName() const
{
	return SymbolClass(SYM | SYM_VAR | SYM_VAR_GLOBAL);
}

void SymVarGlobal::GenerateDeclaration(AsmCode& asm_code)
{
	label = asm_code.AddData(token.GetName(), type->GetSize());
}

void SymVarGlobal::GenerateLValue(AsmCode& asm_code) const
{
	asm_code.AddCmd(ASM_PUSH, label);
}

void SymVarGlobal::GenerateValue(AsmCode& asm_code) const
{
	if (type->GetSize() > 4)
	{
		GenerateLValue(asm_code);
		asm_code.PushMemory(type->GetSize());
	}
	else
		asm_code.AddCmd(ASM_PUSH, AsmMemory(label));
}

//---SymVarLocal---

SymVarLocal::SymVarLocal(Token name, const SymType* type, unsigned offset_):
	SymVar(name, type),
	offset(offset_)
{
}

SymbolClass SymVarLocal::GetClassName() const
{    
	return SymbolClass(SYM | SYM_VAR | SYM_VAR_LOCAL);
}

void SymVarLocal::GenerateLValue(AsmCode& asm_code) const
{
	asm_code.AddCmd(ASM_LEA, AsmMemory(REG_EBP, -offset - 4), REG_EAX);
	asm_code.AddCmd(ASM_PUSH, REG_EAX);
}

void SymVarLocal::GenerateValue(AsmCode& asm_code) const
{
	asm_code.AddCmd(ASM_PUSH, AsmMemory(REG_EBP, -offset - 4));
}

unsigned SymVarLocal::GetOffset() const
{
	return offset;
}

void SymVarLocal::SetOffset(unsigned offset_)
{
	offset = offset_;
}

//---SymTable---

SymTable::SymTable():
	params_size(0),
	locals_size(0)
{
}

void SymTable::Add(Symbol* sym)
{
	table.insert(sym);
	if (sym->GetClassName() & SYM_VAR)
	{
		unsigned sym_size = ((SymVar*)sym)->GetVarType()->GetSize();
		if (sym->GetClassName() & SYM_VAR_PARAM)
			params_size += ((SymVarParam*)sym)->IsByRef() ? 4 : sym_size;
		else if (sym->GetClassName() & SYM_VAR_LOCAL)
			locals_size += sym_size;
	}
	else if (sym->GetClassName() & SYM_PROC)
	{
		proc_decl_order.push_back((SymProc*)sym);
	}
}

const Symbol* SymTable::Find(Symbol* sym) const
{
	return (table.find(sym) != table.end()) ? *table.find(sym) : NULL;
}

const Symbol* SymTable::Find(const Token& tok) const
{
	Symbol sym(tok);
	std::set<Symbol*, SymbLessComp>::const_iterator it = table.find(&sym);
	if (it == table.end()) return NULL;
	return *it;
}

void SymTable::Print(ostream& o, int offset) const
{
	std::vector<Symbol*> v;    
	for (std::set<Symbol*, SymbLessComp>::iterator it = table.begin(); it != table.end(); ++it)
		v.push_back((*it));
	SymbLessComp comp;
	sort(v.begin(), v.end(), comp);
	for (std::vector<Symbol*>::iterator it = v.begin(); it != v.end(); ++it)
		(*it)->PrintVerbose(o, offset);
}

bool SymTable::IsEmpty() const
{
	return table.empty();
}

unsigned SymTable::GetSize() const
{
	return locals_size + params_size;
}

unsigned SymTable::GetLocalsSize() const
{
	return locals_size;
}

unsigned SymTable::GetParamsSize() const
{
	return params_size;
}

void SymTable::GenerateDeclarations(AsmCode& asm_code) const
{
	for (std::set<Symbol*, SymbLessComp>::const_iterator it = table.begin(); it != table.end(); ++it)
		if ((*it)->GetClassName() & SYM_VAR_GLOBAL)
		{
			SymVarGlobal* tmp = (SymVarGlobal*)*it;
			tmp->GenerateDeclaration(asm_code);
		}
		for (std::vector<SymProc*>::const_iterator it = proc_decl_order.begin(); it != proc_decl_order.end(); ++it)
			(*it)->GenerateDeclaration(asm_code);
}