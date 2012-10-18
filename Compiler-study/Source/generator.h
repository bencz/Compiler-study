#ifndef GENERATOR
#define GENERATOR

#include <vector>
#include <string>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <sstream>
using namespace std;

class AsmOperand;
class AsmImmidiate;

enum CmdSize{
	SIZE_NONE,
	SIZE_BYTE,
	SIZE_SHORT,
	SIZE_LONG,
	SIZE_QUARD
};

extern const string SIZE_TO_STR[];

enum RegisterName{
	REG_AL,
	REG_BL,
	REG_CL,
	REG_DL,
	REG_DI,
	REG_SI,
	REG_EAX,
	REG_EBX,
	REG_ECX,
	REG_EDX,
	REG_EDI,
	REG_ESI,
	REG_EBP,
	REG_ESP
};

extern const string REG_TO_STR[];

enum AsmCmdName{
	ASM_ADD,
	ASM_AND,
	ASM_CALL,
	ASM_CMP,
	ASM_DIV,
	ASM_FILD,
	ASM_FLD,
	ASM_FSTP,
	ASM_IDIV,
	ASM_IMUL,
	ASM_JMP,
	ASM_JNE,
	ASM_JNG,
	ASM_JNL,
	ASM_JNZ,
	ASM_JZ,
	ASM_LEA,
	ASM_MOV,
	ASM_MOVZB,
	ASM_MUL,
	ASM_NOT,
	ASM_OR,
	ASM_POP,
	ASM_PUSH,
	ASM_RET,
	ASM_SAR,
	ASM_SAL,
	ASM_SETG,
	ASM_SETGE,
	ASM_SETL,
	ASM_SETLE,
	ASM_SETE,
	ASM_SETNE, 
	ASM_SUB,
	ASM_TEST,
	ASM_XOR
};

extern const string ASM_CMD_TO_STR[];

enum AsmDataType{
	DATA_UNTYPED,
	DATA_INT,
	DATA_REAL,
	DATA_STR
};

extern const string ASM_DATA_TYPE_TO_STR[];

class AsmCmd{
public:
	virtual void Print(ostream& o) const;
};

class AsmLabel: public AsmCmd{
	AsmImmidiate* label;
public:
	AsmLabel(AsmImmidiate* label);
	AsmLabel(AsmImmidiate label);
	AsmLabel(string label);
	virtual void Print(ostream& o) const;
};

class AsmRawCmd: public AsmCmd{
private:
	string str;
public:
	AsmRawCmd(string cmd);
	virtual void Print(ostream& o) const;
};

class AsmCmd0: public AsmCmd{
protected:
	AsmCmdName command;
	CmdSize size;
public:
	AsmCmd0(AsmCmdName cmd, CmdSize cmd_size = SIZE_LONG);
	virtual void Print(ostream& o) const;
};

class AsmData{
private:
	string name;
	string value;
	AsmDataType type;
public:
	AsmData(string name_, string value, AsmDataType type = DATA_UNTYPED);
	virtual void Print(ostream& o) const;    
};

class AsmCmd1: public AsmCmd0{
private:
	AsmOperand* oper;
public:
	AsmCmd1(AsmCmdName cmd, AsmOperand* oper_, CmdSize size = SIZE_LONG);
	virtual void Print(ostream& o) const;    
};

class AsmCmd2: public AsmCmd0{
private:
	AsmOperand* src;
	AsmOperand* dest;
public:
	AsmCmd2(AsmCmdName cmd, AsmOperand* src_, AsmOperand* dest_, CmdSize size = SIZE_LONG);
	virtual void Print(ostream& o) const;
};

class AsmOperand{
public:
	virtual void Print(ostream& o) const;    
	virtual void PrintBase(ostream& o) const;
};

class AsmOperandBase: public AsmOperand{
public:
	virtual void Print(ostream& o) const;
	virtual void PrintBase(ostream& o) const;
};

class AsmRegister: public AsmOperandBase{
private:
	RegisterName reg;
public:
	AsmRegister(RegisterName reg_);
	virtual void Print(ostream& o) const;
	virtual void PrintBase(ostream& o) const;
};

class AsmImmidiate: public AsmOperandBase{
private:
	string value;
public:
	AsmImmidiate();
	AsmImmidiate(const string& value_);
	AsmImmidiate(int num);
	AsmImmidiate(const AsmImmidiate& src);
	string GetValue();
	virtual void Print(ostream& o) const;
	virtual void PrintBase(ostream& o) const;
};

class AsmMemory: public AsmOperand{
private:
	AsmOperandBase* base;
	int disp;
	int index;
	unsigned scale;
public:
	AsmMemory(AsmOperandBase* base_, int disp_ = 0, int index_ = 0, unsigned scale_ = 0);
	AsmMemory(AsmImmidiate base, int disp_ = 0, int index_ = 0, unsigned scale_ = 0);
	AsmMemory(RegisterName reg, int disp_ = 0, int index_ = 0, unsigned scale_ = 0);
	virtual void Print(ostream& o) const;
};

class AsmCode{
private:
	AsmImmidiate format_str_real;
	AsmImmidiate format_str_int;
	AsmImmidiate format_str_str;
	AsmMemory funct_write; 
	vector<AsmCmd*> commands;
	vector<AsmData*> data;
	string ChangeName(string str);
	unsigned label_counter;
public:
	AsmCode();
	AsmImmidiate GenLabel();
	string GenStrLabel();
	AsmImmidiate GenLabel(string prefix);
	string GenStrLabel(string prefix);
	AsmImmidiate LabelByStr(string str);
	void AddCmd(AsmCmd* cmd);
	void AddCmd(AsmCmdName cmd);
	void AddCmd(string raw_cmd);
	void AddCmd(AsmCmdName cmd, AsmOperand* oper);
	void AddCmd(AsmCmdName cmd, RegisterName reg, CmdSize size = SIZE_LONG);
	void AddCmd(AsmCmdName cmd, AsmMemory* mem);
	void AddCmd(AsmCmdName cmd, AsmMemory mem, CmdSize size = SIZE_LONG);
	void AddCmd(AsmCmdName cmd, AsmImmidiate* imm);
	void AddCmd(AsmCmdName cmd, AsmImmidiate imm, CmdSize size = SIZE_LONG);
	void AddCmd(AsmCmdName cmd, AsmOperand* oper1, AsmOperand* oper2);
	void AddCmd(AsmCmdName cmd, RegisterName src, RegisterName dest);
	void AddCmd(AsmCmdName cmd, RegisterName reg, AsmImmidiate* dest);
	void AddCmd(AsmCmdName cmd, RegisterName reg, AsmImmidiate dest);
	void AddCmd(AsmCmdName cmd, AsmImmidiate* src, RegisterName oper1);
	void AddCmd(AsmCmdName cmd, AsmImmidiate src, RegisterName oper1);
	void AddCmd(AsmCmdName cmd, AsmMemory* mem, RegisterName reg);
	void AddCmd(AsmCmdName cmd, AsmMemory mem, RegisterName reg);
	void AddCmd(AsmCmdName cmd, RegisterName reg, AsmMemory* mem);
	void AddCmd(AsmCmdName cmd, RegisterName reg, AsmMemory mem);
	void AddCmd(AsmCmdName cmd, AsmImmidiate* src, AsmMemory* mem);
	void AddCmd(AsmCmdName cmd, AsmImmidiate src, AsmMemory mem);    
	void AddData(AsmData* new_data);
	AsmImmidiate AddData(string label, string value, AsmDataType type = DATA_UNTYPED);
	AsmImmidiate AddData(string label, unsigned size);
	AsmImmidiate AddData(string value, AsmDataType type = DATA_UNTYPED);
	AsmImmidiate AddData(unsigned size);
	void AddLabel(AsmImmidiate* label);
	void AddLabel(AsmImmidiate label);
	void AddLabel(string label);
	virtual void Print(ostream& o) const;
	void CallWriteForInt();
	void CallWriteForReal();
	void CallWriteForStr();
	void PushMemory(unsigned size);
	void MoveToMemoryFromStack(unsigned size);
	void MoveMemory(unsigned size);
};

#endif