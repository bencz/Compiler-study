#include "generator.h"

const string REG_TO_STR[] =
{
	"%eax",
	"%ebx",
	"%ecx",
	"%edx",
	"%edi",
	"%esi",
	"%ebp",
	"%esp"
};

const string ASM_CMD_TO_STR[] =
{
	"add",
	"div",
	"lea",
	"mov",
	"mul",
	"pop",
	"push",
	"sub"
};


//---AsmCmd---

AsmCmd::AsmCmd(AsmCmdName cmd):
	command(cmd)
{
}

void AsmCmd::Print(ostream& o) const
{
	o << ASM_CMD_TO_STR[command] << ' ';
}

//---AsmData---

AsmData::AsmData(string name_, unsigned size_):
	name(strcpy(new char[name_.size() + 1], name_.c_str())),
	size(size_)
{

}

void AsmData::Print(ostream& o) const
{
	o << name << ": .space " << size;
}

//---AsmCmd1---

AsmCmd1::AsmCmd1(AsmCmdName cmd, AsmOperand* oper_):
	AsmCmd(cmd),
	oper(oper_)
{
}

void AsmCmd1::Print(ostream& o) const
{
	o << ASM_CMD_TO_STR[command] << ' ';
	oper->Print(o);
}

//---AsmCmd2---

AsmCmd2::AsmCmd2(AsmCmdName cmd, AsmOperand* src_, AsmOperand* dest_):
	AsmCmd(cmd),
	src(src_),
	dest(dest_)
{
}

void AsmCmd2::Print(ostream& o) const
{
	o << ASM_CMD_TO_STR[command] << ' ';
	src->Print(o);
	dest->Print(o);
}

//---AsmOperand---

void AsmOperand::Print(ostream& o) const
{
}

//---AsmOperandBase---

void AsmOperandBase::Print(ostream& o) const
{
}

//---AsmRegister---

AsmRegister::AsmRegister(RegisterName reg_):
	reg(reg_)
{
}

void AsmRegister::Print(ostream& o) const
{
	o << REG_TO_STR[reg] << ' ';
}

//---AsmImmidiate---

AsmImmidiate::AsmImmidiate(string value_):
	value(strcpy(new char[value_.size() + 1], value_.c_str()))
{
}

void AsmImmidiate::Print(ostream& o) const
{
	o << value;
}

//---AsmMemory---

AsmMemory::AsmMemory(AsmOperandBase* base_, unsigned disp_, unsigned index_, unsigned scale_):
	base(base_),
	disp(disp_),
	index(index_),
	scale(scale_)
{
}

void AsmMemory::Print(ostream& o) const
{
	if (disp) o << disp;
	o << '(';
	base->Print(o);
	if (index) o << ", " << index;
	if (scale) o << ", " << scale;
	o << ')';
}    

//---AsmCode---

void AsmCode::AddCmd(AsmCmd* cmd)
{
	commands.push_back(cmd);
}

void AsmCode::AddData(AsmData* new_data)
{
	data.push_back(new_data);
}

void AsmCode::Print(ostream& o) const
{
	o << ".data\n";
	for (vector<AsmCmd*>::const_iterator it = commands.begin(); it != commands.end(); ++it)
		(*it)->Print(o);
	o << ".text\n";
	for (vector<AsmData*>::const_iterator it = data.begin(); it != data.end(); ++it)
		(*it)->Print(o);
}