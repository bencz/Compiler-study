#ifndef GENERATOR
#define GENERATOR

#include <vector>
#include <string>
#include <string.h>
#include <iostream>
using namespace std;

class AsmOperand;

enum RegisterName{
	EAX,
	EBX,
	ECX,
	EDX,
	EDI,
	ESI,
	EBP,
	ESP
};

extern const string REG_TO_STR[];

enum AsmCmdName{
	ASM_ADD,
	ASM_DIV,
	ASM_LEA,
	ASM_MOV,
	ASM_MUL,
	ASM_POP,
	ASM_PUSH,
	ASM_SUB
};

extern const string ASM_CMD_TO_STR[];

class AsmCmd{
protected:
	AsmCmdName command;
public:
	AsmCmd(AsmCmdName cmd);
	virtual void Print(ostream& o) const;
};

class AsmData{
private:
	char* name;
	unsigned size;
public:
	AsmData(string name_, unsigned size_);
	virtual void Print(ostream& o) const;    
};

class AsmCmd1: public AsmCmd{
private:
	AsmOperand* oper;
public:
	AsmCmd1(AsmCmdName cmd, AsmOperand* oper_);
	virtual void Print(ostream& o) const;    
};

class AsmCmd2: public AsmCmd{
private:
	AsmOperand* src;
	AsmOperand* dest;
public:
	AsmCmd2(AsmCmdName cmd, AsmOperand* src_, AsmOperand* dest_);
	virtual void Print(ostream& o) const;
};

class AsmOperand{
public:
	virtual void Print(ostream& o) const;    
};

class AsmOperandBase{
public:
	virtual void Print(ostream& o) const;
};

class AsmRegister: public AsmOperandBase{
private:
	RegisterName reg;
public:
	AsmRegister(RegisterName reg_);
	virtual void Print(ostream& o) const;
};

class AsmImmidiate: public AsmOperandBase{
private:
	char* value;
public:
	AsmImmidiate(string value_);
	virtual void Print(ostream& o) const;
};

class AsmMemory: public AsmOperand{
private:
	AsmOperandBase* base;
	unsigned disp;
	unsigned index;
	unsigned scale;
public:
	AsmMemory(AsmOperandBase* base, unsigned disp_ = 0, unsigned index_ = 0, unsigned scale_ = 0);
	virtual void Print(ostream& o) const;
};

class AsmCode{
private:
	vector<AsmCmd*> commands;
	vector<AsmData*> data;
public:
	void AddCmd(AsmCmd* cmd);
	void AddData(AsmData* new_data);
	virtual void Print(ostream& o) const;
};

#endif