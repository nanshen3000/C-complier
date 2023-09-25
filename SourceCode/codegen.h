#ifndef CODEGEN_H
#define CODEGEN_H

#include <iostream>
#include <map>
#include <set>
#include <string>
#include "AST.h"

using namespace std;

class AST;

class SymTable {
   public:
    SymTable& addID(const string& s, unsigned offset)
    {
        table[s] = offset;
        return *this;
    };

    unsigned getOffset(const string& s) { return table[s]; };

   protected:
    map<string, unsigned> table;

   private:
    int static_Addr() { return 0; };
    int reg_addr(int reg) { return 0x8000000 & reg; }
    int stack_addr(int offset) { return 0x4000000 | offset; }
};

class Pos_AST {
   public:
    short int reg = -1;
    int stOffset = -1;
};

// record the addr of the variable (stack or register)
class AddrDespt {
   public:
    map<AST*, Pos_AST> addrs;

    AddrDespt& setReg(AST* ast, int regNo);
    AddrDespt& deleteReg(AST* ast);
    AddrDespt& setStOffset(AST* ast, int offset);
    bool inReg(AST* ast)  { return addrs[ast].reg != -1; }
    bool inSt(AST* ast)  { return addrs[ast].stOffset != -1; }
    int RegPos(AST* ast)  { return addrs[ast].reg; }
    int StPos(AST* ast)  { return addrs[ast].stOffset; }
    int deleteNode(AST* node);
};

class RegDespt {
   public:
    static const unsigned REGISTES_SIZE = 20;
    static const unsigned START_OFFSET = 3;
    vector<AST*> registers;
    int swapIdx = 0;

    RegDespt()
    {
        for (unsigned i = 0; i < REGISTES_SIZE; ++i)
            registers.push_back(nullptr);
    }

    int getFree();
    int swap(AST* node);
    RegDespt& setReg(AST *node, int reg);
    RegDespt& deleteNode(int reg) { registers[reg - START_OFFSET] = nullptr;
        return *this;
    };
};

// the structure that is used in codegen()
class CG {
   public:
    static const unsigned FP_START_OFFSET = 8;
    static const unsigned FP_REG_NO = 30;
    static const unsigned TMP_REG = 24;
    const string PROGRAMM_END = "END";
    map<string, int> arrayIDs;
    map<string, int> initIDs;
    set<string> uninitIDs;
    SymTable symTable;
    int fpTop;

    RegDespt regDespt;
    AddrDespt addrDespt;
    int spTop;

    int currIF_No = 1;
    int currDoWhile_NO = 1;
    int currWHILE_NO = 1;
    

    unsigned getReg(AST* a1, AST* a2);
    CG& bind_RegNode(AST* node, int reg);
    CG& freeNode(AST* node);
    int getNodeReg(AST* node);
    int getNodeSt(AST* node) { return addrDespt.StPos(node); };
    int getSymOffset(const string &IDname) {
        return symTable.getOffset(IDname);
    };
};

inline void label_mips(const string &s) {
    cout << s << ":" << endl;
}

inline void branch_mips(const string& s,const string& label)
{
    cout << s << " "  << label << endl;
}

inline void branch_mips(const string& s, int reg1, int reg2, const string &label) {
    cout << s << " $" << reg1 << ", $" << reg2 << ", " << label << endl;
}

inline void twoReg_mips(const string &s, int reg1, int reg2) {
    cout << s << " $"  << reg1 << ", $" << reg2 << endl;
}

inline void twoReg_mips(const string& s, int reg1, const string & reg2)
{
    cout << s << " $" << reg1 << ", $" << reg2 << endl;
}

inline void oneReg_mips(const string &s, int reg)
{
    cout << s << " $" << reg << endl;
}

inline void li_mips(int reg, int num) {
    cout << "li $" << reg << ", " << num << endl;
}

inline void syscall_mips(){
    cout << "syscall"<< endl;
}

inline void move_mips(const string&d, int &s) {
    cout << "move $" << d << ", $" << s << endl;
}

inline void li_mips(const string &s, int num){
    cout << "li $" << s << ", " << num << endl;
}

inline void lwfp_mips(int reg, int offset) {
    cout << "lw $" << reg << ", " << offset << "($fp)" << endl;
}

inline void lwsp_mips(int reg, int offset){
    cout << "lw $" << reg << ", " << offset << "($sp)" << endl;
}

inline void Itype_mips(const string &s, int reg1, int reg2, int imm) {
    cout << s << " $" << reg1 << ", $" << reg2 << ", " << imm << endl;
}

inline void Iload_mips(const string& s,
                       const string& reg1,
                       int imm,
                       const string& reg2)
{
    cout << s << " $" << reg1 << ", " << imm << "($" << reg2 << ')' << endl;
}

inline void Iload_mips(const string& s,
                       int reg1,
                       int imm,
                       const string& reg2)
{
    cout << s << " $" << reg1 << ", " << imm << "($" << reg2 << ')' << endl;
}

inline void Iload_mips(const string& s,
                      int reg1,
                      int imm,
                      int reg2)
{
    cout << s << " $" << reg1 << ", " << imm << "($" << reg2 << ')' << endl;
}

inline void Rtype_mips(const string& s, int reg1, int reg2, int reg3)
{
    cout << s << " $" << reg1 << ", $" << reg2 << ", $" << reg3 << endl;
}

inline void Rtype_mips(const string& s, int reg1, const string& reg2, int reg3)
{
    cout << s << " $" << reg1 << ", $" << reg2 << ", $" << reg3 << endl;
}

inline void Rshift_mips(const string& s, int reg1, int reg2, int shift)
{
    cout << s << " $" << reg1 << ", $" << reg2 << ", " << shift << endl;
}

#endif