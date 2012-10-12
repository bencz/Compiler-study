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
SymType* top_type_real = new SymTypeReal(Token("Real", RESERVED_WORD, TOK_REAL, -1, -1));;
SymType* top_type_untyped = new SymType(Token("untyped", RESERVED_WORD, TOK_UNRESERVED, -1, -1));

//---Symbol---

Symbol::Symbol(Token token_)
{
	token_.NameToLowerCase();
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

void SymProc::AddSymTable(SynTable* syn_table_)
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

SymProc::SymProc(Token token, SynTable* syn_table_):
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

//---SymFunct---

SymFunct::SymFunct(Token token_, SynTable* syn_table, const SymType* result_type_):
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

//---SymTypeRecord---

SymTypeRecord::SymTypeRecord(SynTable* syn_table_):
	syn_table(syn_table_)
{
}

const SymVar* SymTypeRecord::FindField(Token& field_name)
{
	const Symbol* res = syn_table->Find(field_name);
	return (const SymVar*)res;
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
	syn_table->Print(o, offset);
	PrintSpaces(o, offset - 1) << "end";    
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

//---SymVarParam---

SymVarParam::SymVarParam(Token name, const SymType* type, bool by_ref_):
	SymVar(name, type),
	by_ref(by_ref_)
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

//---SymVarGlobal---

SymVarGlobal::SymVarGlobal(Token name, const SymType* type):
	SymVar(name, type)
{
}

SymbolClass SymVarGlobal::GetClassName() const
{
	return SymbolClass(SYM | SYM_VAR | SYM_VAR_GLOBAL);
}

//---SymVarLocal---

SymVarLocal::SymVarLocal(Token name, const SymType* type):
	SymVar(name, type)
{
}

SymbolClass SymVarLocal::GetClassName() const
{    
	return SymbolClass(SYM | SYM_VAR | SYM_VAR_GLOBAL);
}

//---SynTable---

void SynTable::Add(Symbol* sym)
{
	table.insert(sym);
}

#include <iostream>

const Symbol* SynTable::Find(Symbol* sym) const
{
	return (table.find(sym) != table.end()) ? *table.find(sym) : NULL;
}

const Symbol* SynTable::Find(const Token& tok) const
{
	Symbol sym(tok);
	Symbol* res = table.find(&sym) != table.end() ? *table.find(&sym) : NULL; // fix
	return res;
}

void SynTable::Print(ostream& o, int offset) const
{
	std::vector<Symbol*> v;    
	for (std::set<Symbol*, SymbLessComp>::iterator it = table.begin(); it != table.end(); ++it)
		v.push_back((*it));
	SymbLessComp comp;
	sort(v.begin(), v.end(), comp);
	for (std::vector<Symbol*>::iterator it = v.begin(); it != v.end(); ++it)
		(*it)->PrintVerbose(o, offset);
}

bool SynTable::IsEmpty() const
{
	return table.empty();
}