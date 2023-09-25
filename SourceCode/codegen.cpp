#include "AST.h"
#include "codegen.h"

// --------- define structure for code generator like symbol table ---------

inline AddrDespt& AddrDespt::setReg(AST* ast, int regNo)
{
    addrs[ast].reg = regNo;
    return *this;
}

inline AddrDespt& AddrDespt::setStOffset(AST* ast, int offset)
{
    addrs[ast].reg = offset;
    return *this;
}

inline AddrDespt& AddrDespt::deleteReg(AST* ast)
{
    addrs[ast].reg = -1;
    return *this;
}

inline int AddrDespt::deleteNode(AST* node) {
    int res = -1;
    if (inReg(node)) {
        res = RegPos(node);
    }
    addrs.erase(node);
    return res;
}

int RegDespt::getFree() {
    for (unsigned i = 0; i < REGISTES_SIZE; ++i) {
        if (registers[i] == nullptr) {
            return i + START_OFFSET;
        }
    }
    return -1;
}

int RegDespt::swap(AST *node)
{
    node = registers[swapIdx];
    registers[swapIdx] = nullptr;
    int res = swapIdx;
    swapIdx = (swapIdx + 1) / REGISTES_SIZE;
    return res + START_OFFSET;
}

RegDespt& RegDespt::setReg(AST* node, int reg)
{
    reg -= RegDespt::START_OFFSET;
    registers[reg] = node;
    return *this;
}

unsigned CG::getReg(AST* a1, AST* a2) {
    int res;
    // if one of them already in register, choose one of them
    if (a1 != nullptr && addrDespt.inReg(a1)) {
        return addrDespt.RegPos(a1);
    }
    else if (a2 != nullptr && addrDespt.inReg(a2)) {
        return addrDespt.RegPos(a2);
    }
    // try to get a free register
    else if ((res = regDespt.getFree()) != -1) {
        return res;
    }
    // swap out a filled register
    else {
        AST* node;
        res = regDespt.swap(node);
        addrDespt.deleteReg(node);
        spTop += 4;
        addrDespt.setStOffset(node, spTop);
        // mips code for store the register to stack
        cout << "addi $sp, $sp, -4" << endl;
        cout << "sw $" <<  res << ", ($sp)" << endl;

        return res;
    }
}

CG& CG::bind_RegNode(AST* node, int reg) {
    addrDespt.setReg(node, reg);
    regDespt.setReg(node, reg);
    return *this;
}

CG& CG::freeNode(AST* node) {
    int reg;
    if ((reg = addrDespt.deleteNode(node)) != -1) {
        regDespt.deleteNode(reg);
    }
    return *this;
}

inline int CG::getNodeReg(AST* node) {
    return addrDespt.RegPos(node);
}

// ---------------- define code generator for each type of node -----------------

inline void programAST::codegen(CG& cg)
{
    var_declarations->codegen(cg);
    statements_AST->codegen(cg);
    label_mips(cg.PROGRAMM_END);
    cout << "move $fp, $sp" << endl;
    cout << "lw $fp, " << cg.fpTop - 4 << "($sp)" << endl;
    cout << "addiu $sp, $sp," << cg.fpTop << endl;
}

inline void varDecls_AST::codegen(CG& cg)
{
    // get all the symbols into the cg structure
    for (auto var_decl : var_declarations) {
        var_decl->codegen(cg);
    }

    // now allocate space for variables in frame and stack
    unsigned offset = CG::FP_START_OFFSET;
    for (auto &p: cg.initIDs) {
        cg.symTable.addID(p.first, offset);
        offset += 4;
    }

    for (auto &p: cg.uninitIDs) {
        cg.symTable.addID(p, offset);
        offset += 4;
    }

    for (auto &p: cg.arrayIDs) {
        cg.symTable.addID(p.first, offset);
        offset += p.second * 4;
    }

    cg.fpTop = offset;
    // generate code for fp allocation
    cout << "addiu $sp, $sp," << -cg.fpTop << endl;
    cout << "sw $fp, " << cg.fpTop - 4 <<"($sp)" << endl;
    cout << "move $fp, $sp" << endl;

    for (auto& p : cg.initIDs) {
        cout << "li $24, " << p.second << endl;
        cout << "sw $24, " << cg.symTable.getOffset(p.first) << "($fp)" << endl;
    }
}

inline void varDeclAST::codegen(CG& cg){
    decls_ast->codegen(cg, INT_t);
};

inline void decls_AST::codegen(CG& cg, varDataType type) {
    for (auto decl: declaration_lists) {
        if (decl->decl_type() == declAST::INIT) {
            InitVarDeclAST* a = (InitVarDeclAST*) decl;
            a->codegen(cg, type);
        }
        else if (decl->decl_type() == declAST::UNINIT) {
            unInitVarDeclAST* a = (unInitVarDeclAST*)decl;
            a->codegen(cg, type);
        }
        // Array
        else {
            ArrayDeclAST* a = (ArrayDeclAST*)decl;
            a->codegen(cg, type);
        }
    }
}

inline void InitVarDeclAST::codegen(CG& cg, varDataType type) {
    if (cg.initIDs.count(IDname)) {
        error_redefine(IDname);
    }
    cg.initIDs[IDname] = value;
}

inline void unInitVarDeclAST::codegen(CG& cg, varDataType type)
{
    if (cg.uninitIDs.count(IDname)) {
        error_redefine(IDname);
    }
    cg.uninitIDs.insert(IDname);
}

inline void ArrayDeclAST::codegen(CG& cg, varDataType type) {
    if (cg.arrayIDs.count(IDname)) {
        error_redefine(IDname);
    }
    cg.arrayIDs[IDname] = size;
}


// ----- code gen for statement-----------
inline void stmts_AST::codegen(CG &cg) {
    for (auto p: statments) {
        switch (p->stmt_type())
        {
        case stmtAST::Assign_t:{
            AssignStmt_AST* a = (AssignStmt_AST*)p;
            a->codegen(cg);
        } break;

        case stmtAST::Read_t: {
            readStmt_AST* a = (readStmt_AST*)p;
            a->codegen(cg);
        } break;

        case stmtAST::Write_t: {
            writeStmt_AST* a = (writeStmt_AST*)p;
            a->codegen(cg);
        } break;

        case stmtAST::IF_t: {
            IFstmt_AST* a = (IFstmt_AST*)p;
            a->codegen(cg);
        } break;

        case stmtAST::WHILE_t: {
            WHILEstmt_AST* a = (WHILEstmt_AST*)p;
            a->codegen(cg);
        } break;

        case stmtAST::Return_t: {
            Return_AST* a = (Return_AST*)p;
            a->codegen(cg);
        } break;

        default:
            break;
        }
    }
}

inline void AssignStmt_AST::codegen(CG& cg) {
    int fp_offset = cg.getSymOffset(IDname);
    int idxReg;
    valExp->codegen(cg);
    int valReg = cg.getNodeReg(valExp);
    // array assignment
    if (idxExp != nullptr) {
        idxExp->codegen(cg);
        idxReg = cg.getNodeReg(idxExp);
        Rshift_mips("sll", idxReg, idxReg, 2);
        Rtype_mips("addu", idxReg, "fp", idxReg);
        Iload_mips("sw", valReg, fp_offset, idxReg);

        cg.freeNode(valExp);
        cg.freeNode(idxExp);
    }
    // single ID assignment
    else {
        idxReg = CG::TMP_REG;
        Iload_mips("sw", valReg, fp_offset, "fp");

        cg.freeNode(valExp);
    }
}

inline void readStmt_AST::codegen(CG& cg) {
    int fp_offset = cg.getSymOffset(IDname);
    li_mips("v0", 5);
    syscall_mips();
    Iload_mips("sw", "v0", fp_offset, "fp");
}

inline void writeStmt_AST::codegen(CG &cg) {
    exp->codegen(cg);
    int s = cg.getNodeReg(exp);
    move_mips("a0", s);
    li_mips("v0", 1);
    syscall_mips();
    cg.freeNode(exp);
}

inline void IFstmt_AST::codegen(CG &cg) {
    exp->codegen(cg);
    int reg = cg.getNodeReg(exp);
    int labelNo = cg.currIF_No++;
    if (else_stmts == nullptr) {
        string exitLabel = "IFEXIT" + to_string(labelNo);
        branch_mips("beq", reg, 0, exitLabel);
        if_stmts->codegen(cg);
        label_mips(exitLabel);
    }
    else {
        string elseLabel = "ELSE" + to_string(labelNo);
        string exitLabel = "IFEXIT" + to_string(labelNo);
        branch_mips("beq", reg, 0, elseLabel);
        if_stmts->codegen(cg);
        branch_mips("b", exitLabel);
        label_mips(elseLabel);
        else_stmts->codegen(cg);
        label_mips(exitLabel);
    }
}

inline void WHILEstmt_AST::codegen(CG& cg)
{
    if (isDoWHILE) {
        int labelNo = cg.currDoWhile_NO++;
        string doWhileLabel = "DOWHILE" + to_string(labelNo);
        label_mips(doWhileLabel);
        while_stmts->codegen(cg);
        exp->codegen(cg);
        int reg = cg.getNodeReg(exp);
        branch_mips("bne", reg, 0, doWhileLabel);
    }
    else {
        int labelNo = cg.currWHILE_NO++;
        string whileLabel = "WHILE" + to_string(labelNo);
        string loopLabel = "LOOP" + to_string(labelNo);
        branch_mips("b", whileLabel);
        label_mips(loopLabel);
        while_stmts->codegen(cg);
        label_mips(whileLabel);
        exp->codegen(cg);
        int reg = cg.getNodeReg(exp);
        branch_mips("bne", reg, 0, loopLabel);
    }
}

// ---------  code gen for expression ---------
void expAST::codegen(CG& cg)
{
    switch (expType)
    {
    case INTNUM_t: {
        INTNUMexpAST* a = (INTNUMexpAST*) this;
        a->codegen(cg);
    } break;

    case ID_t: {
        IDexpAST* a = (IDexpAST*)this;
        a->codegen(cg);
    } break;

    case ARRAY_t: {
        ArrayIDexpAST* a = (ArrayIDexpAST*)this;
        a->codegen(cg);
    } break;

    case SINGLE_OP_t: {
        singleOPexpAST* a = (singleOPexpAST*)this;
        a->codegen(cg);
    } break;

    case BIN_OP_t: {
        BinaryOPexpAST* a = (BinaryOPexpAST*)this;
        a->codegen(cg);
    } break;

    default:
        break;
    }
}

inline void INTNUMexpAST::codegen(CG &cg) {
    int reg = cg.getReg(nullptr, nullptr);
    li_mips(reg, num);
    cg.bind_RegNode(this, reg);
}

inline void IDexpAST::codegen(CG &cg) {
    int offset = cg.symTable.getOffset(IDname);
    int reg = cg.getReg(nullptr, nullptr);
    lwfp_mips(reg, offset);
    cg.bind_RegNode(this, reg);
}

inline void singleOPexpAST::codegen(CG &cg) {
    exp->codegen(cg);
    int reg = cg.getReg(exp, nullptr);
    int reg2 = cg.getNodeReg(exp);
    if (reg2 == -1) {
        int offset = cg.getNodeSt(exp);
        lwsp_mips(reg, offset);
    }

    if (op == NOT_OP)
        Itype_mips("sltiu", reg, reg2, 1);
    else if (op == MINUS)
        Rtype_mips("subu", reg, 0, reg2);
    

    cg.freeNode(exp);
    cg.bind_RegNode(this, reg);
}

inline void ArrayIDexpAST::codegen(CG& cg)
{
    exp->codegen(cg);
    int reg = cg.getReg(exp, nullptr);
    int reg2 = cg.getNodeReg(exp);
    if (reg2 == -1) {
        int offset = cg.getNodeSt(exp);
        lwsp_mips(reg, offset);
    }

    int fp_offset = cg.symTable.getOffset(IDname);
    Rshift_mips("sll", reg2, reg2, 2);
    Rtype_mips("addu", reg, "fp", reg2);
    Iload_mips("lw", reg, fp_offset, reg);


    cg.freeNode(exp);
    cg.bind_RegNode(this, reg);
}

void Return_AST::codegen(CG &cg) {
    branch_mips("b", cg.PROGRAMM_END);
}

void BinaryOPexpAST::codegen(CG& cg)
{
    // 尽量优先深度大的递归， 减少寄存器的占用
    if (Rexp->expType == BIN_OP_t) {
        Rexp->codegen(cg);
        Lexp->codegen(cg);
    }
    else {
        Lexp->codegen(cg);
        Rexp->codegen(cg);
    }


    int reg = cg.getReg(Lexp, Rexp);
    int Lreg = cg.getNodeReg(Lexp);
    int Rreg = cg.getNodeReg(Rexp);

    if (Lreg == -1) {
        
    }

    if (Rreg == -1) {

    }
    
    switch (op)
    {
    case OR_OP: {
        Rtype_mips("or", reg, Lreg, Rreg);
    } break;

    case OROR: {
        Rtype_mips("or", reg, Lreg, Rreg);
        Itype_mips("slti", reg, reg, 1);
        Itype_mips("xori", reg, reg, 1);
    } break;

    case AND_OP:{
        Rtype_mips("and", reg, Lreg, Rreg);
    } break;

    case ANDAND: {
        Itype_mips("sltiu", Lreg, Lreg, 1);
        Itype_mips("xori", Lreg, Lreg, 1);
        Itype_mips("sltiu", Rreg, Rreg, 1);
        Itype_mips("xori", Rreg, Rreg, 1);
        Rtype_mips("and", reg, Lreg, Rreg);
    }break;

    case EQ: {
        Rtype_mips("xor", reg, Lreg, Rreg);
        Itype_mips("sltiu", reg, reg, 1);
        Itype_mips("andi", reg, reg, 0x00fff);
    }break;

    case NOTEQ: {
        Rtype_mips("xor", reg, Lreg, Rreg);
        Rtype_mips("sltu", reg, 0, reg);
        Itype_mips("andi", reg, reg, 0x00fff);
    }break;

    case GT:
    case LTEQ: {
        Rtype_mips("sltu", reg, Rreg, Lreg);
        Itype_mips("andi", reg, reg, 0x00fff);
    } break;

    case LT:
    case GTEQ: {
        Rtype_mips("sltu", reg, Lreg, Rreg);
        Itype_mips("andi", reg, reg, 0x00fff);
    } break;

    case SHL_OP: {
        Rtype_mips("sllv", reg, Lreg, Rreg);
    } break;

    case SHR_OP: {
        Rtype_mips("srav", reg, Lreg, Rreg);
    } break;

    case PLUS: {
        Rtype_mips("addu", reg, Lreg, Rreg);
    }break;

    case MINUS: {
        Rtype_mips("subu", reg, Lreg, Rreg);
    }break;

    case MUL_OP: {
        twoReg_mips("mult", Lreg, Rreg);
        oneReg_mips("mflo", reg);
    }break;

    case DIV_OP: {
        twoReg_mips("div", Lreg, Rreg);
        oneReg_mips("mflo", reg);
    } break;

    default:
        break;
    }



    cg.freeNode(Lexp);
    cg.freeNode(Rexp);
    cg.bind_RegNode(this, reg);
}
