#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include "codegen.h"

using namespace std;

class CG;

// the basic AST node
class AST {
   public:
    virtual void codegen(CG& cg){};

   protected:
    enum ASTtype {
        TokAST_t,

        expAST_t,
        IDexpAST_t,
        ArrayIDexp_t,
        singleOPexpAST_t,
        BinaryOPexpAST_t,
        INTNUMexpAST_t,

        InitVarDeclAST_t,
        unInitVarDeclAST_t,
        ArrayDeclAST_t,

    };

    enum varDataType { INT_t };

    //ASTtype astType;
};

class TokAST : public AST {
   public:
    TokAST() = default;
    TokAST(const string& s) : content(s){};

    string content;
};

// ------------------- the expression AST class -----------------
class expAST : public AST {
   public:
    enum expASTop {
        OROR,
        ANDAND,
        OR_OP,
        AND_OP,
        EQ,
        NOTEQ,
        LT,
        GT,
        LTEQ,
        GTEQ,
        SHL_OP,
        SHR_OP,
        PLUS,
        MINUS,
        MUL_OP,
        DIV_OP,
        NOT_OP,
    };

    enum ExpASTtype {
        ID_t,
        ARRAY_t,
        SINGLE_OP_t,
        BIN_OP_t,
        INTNUM_t,
    };

    ExpASTtype expType;

    void codegen(CG& cg) override;
};

class IDexpAST : public expAST {
   public:
    IDexpAST(const string& s) : IDname(s) { expType = ID_t; };
    void codegen(CG& cg) override;

   protected:
    string IDname;
};

class ArrayIDexpAST : public expAST {
   public:
    ArrayIDexpAST() { expType = ARRAY_t; };
    ArrayIDexpAST(const string& s, expAST* e) : IDname(s), exp(e){
        expType = ARRAY_t;
    };
    void codegen(CG& cg) override;

   protected:
    string IDname;
    expAST* exp;
};

class singleOPexpAST : public expAST {
   public:
    singleOPexpAST() { expType = SINGLE_OP_t; };
    singleOPexpAST(expASTop type, expAST* e) : op(type), exp(e){
        expType = SINGLE_OP_t;
    };
    void codegen(CG& cg) override;

   protected:
    expASTop op;
    expAST* exp;
};

class BinaryOPexpAST : public expAST {
   public:
    BinaryOPexpAST() { expType = BIN_OP_t; };
    BinaryOPexpAST(expAST* e1, expASTop type, expAST* e2)
        : Lexp(e1), op(type), Rexp(e2){
        expType = BIN_OP_t;
    };
    void codegen(CG& cg) override;

   protected:
    expASTop op;
    expAST* Lexp;
    expAST* Rexp;

};

class INTNUMexpAST : public expAST {
   public:
    INTNUMexpAST() { expType = INTNUM_t; };
    INTNUMexpAST(const string& s) {
        num = stoi(s);
        expType = INTNUM_t;
    }
    INTNUMexpAST(int a) : num(a) { expType = INTNUM_t; };
    void codegen(CG& cg) override;

   
    int num;
};

// ------------------- the var declarations AST class -----------------

class declAST : public AST {
   public:
    enum declType { INIT, UNINIT, ARRAY };
    virtual declType decl_type() = 0;
    virtual void codegen(CG& cg, varDataType type) = 0;

   protected:
    declAST& error_redefine(const string& s)
    {
        cout << "error: redfine varaible: " << s << endl;
        exit(0);
        return *this;
    }
};

class InitVarDeclAST : public declAST {
   public:
    InitVarDeclAST() = default;
    void setID(const string& name) { IDname = name; };
    void setVal(const string& s) { value = stoi(s); }
    declType decl_type() { return INIT; };
    void codegen(CG& cg, varDataType type) override;

   protected:
    string IDname;
    int value;
};

class unInitVarDeclAST : public declAST {
   public:
    unInitVarDeclAST() = default;
    void setID(const string& name) { IDname = name; };
    declType decl_type() { return UNINIT; };
    void codegen(CG& cg, varDataType type) override;

   protected:
    string IDname;
};

class ArrayDeclAST : public declAST {
   public:
    ArrayDeclAST() = default;
    void setID(const string& name) { IDname = name; };
    void setSize(const string& s) { size = stoi(s); }
    declType decl_type() { return ARRAY; };
    void codegen(CG& cg, varDataType type) override;

   protected:
    string IDname;
    int size;
};

class decls_AST : public AST {
   public:
    decls_AST() = default;
    decls_AST(declAST* a) { declaration_lists.push_back(a); };
    void addAST(declAST* a) { declaration_lists.push_back(a); };
    void codegen(CG& cg, varDataType type);

   protected:
    vector<declAST*> declaration_lists;
};

class varDeclAST : public AST {
   public:
    varDeclAST() = default;
    varDeclAST(decls_AST* a) : dataType(INT_t), decls_ast(a){};
    void codegen(CG& cg) override;

   protected:
    varDataType dataType;
    decls_AST* decls_ast;
};

class varDecls_AST : public AST {
   public:
    varDecls_AST() = default;
    varDecls_AST(varDeclAST* a) { this->add(a); };
    void add(varDeclAST* a) { var_declarations.push_back(a); }
    void codegen(CG& cg) override;

   protected:
    vector<varDeclAST*> var_declarations;
};

// ------------------- the statements AST class -----------------
class stmtAST : public AST {
   public:
    
    enum Stmt_type {
        Assign_t,
        Read_t,
        Write_t,
        IF_t,
        WHILE_t,
        Return_t,
    };

    virtual Stmt_type stmt_type() = 0;

   protected:
};

// the code block also use this AST node
class stmts_AST : public AST {
   public:
    stmts_AST() = default;
    void codegen(CG& cg) override;
    void addAST(stmtAST* a)
    {
        if (a != nullptr)
            statments.push_back(a);
    };

   protected:
    vector<stmtAST*> statments;
};

class AssignStmt_AST : public stmtAST {
   public:
    void setID(const string& name) { IDname = name; };
    void setIdxExp(expAST* a) { idxExp = a; };
    void setValExp(expAST* a) { valExp = a; };
    Stmt_type stmt_type() override { return Assign_t; }
    void codegen(CG& cg) override;

   protected:
    string IDname;
    expAST* idxExp;
    expAST* valExp;
};

class readStmt_AST : public stmtAST {
   public:
    readStmt_AST() = default;
    readStmt_AST(const string& s) : IDname(s){};
    Stmt_type stmt_type() override { return Read_t; }
    void codegen(CG &cg) override;

   protected:
    string IDname;
};

class writeStmt_AST : public stmtAST {
   public:
    writeStmt_AST() = default;
    writeStmt_AST(expAST* a) : exp(a){};
    Stmt_type stmt_type() override { return Write_t; }
    void codegen(CG& cg) override;

   protected:
    expAST* exp;
};

class IFstmt_AST : public stmtAST {
   public:
    IFstmt_AST() = default;
    IFstmt_AST(expAST* a, stmts_AST* b) : exp(a), if_stmts(b){};
    IFstmt_AST& addElseAST(stmts_AST* a)
    {
        else_stmts = a;
        return *this;
    };
    Stmt_type stmt_type() override { return IF_t; }
    void codegen(CG &cg) override;

   protected:
    expAST* exp;
    stmts_AST* if_stmts;
    stmts_AST* else_stmts;
};

class WHILEstmt_AST : public stmtAST {
   public:
    WHILEstmt_AST() = default;
    // WHILEstmt_AST& setIs_dowhile(bool flag) { isDoWHILE = flag; };
    WHILEstmt_AST(bool flag, expAST* a, stmts_AST* b)
        : isDoWHILE(flag), exp(a), while_stmts(b){};
    Stmt_type stmt_type() override { return WHILE_t; }
    void codegen(CG& cg) override;

   protected:
    bool isDoWHILE;
    expAST* exp;
    stmts_AST* while_stmts;
};

class DOWHILEstmt_AST : public stmtAST {
   protected:
    expAST* exp;
    stmts_AST* dowhile_stmts;
};

class Return_AST : public stmtAST {
   public:
    Stmt_type stmt_type() override { return Return_t; };
    void codegen(CG &cg) override;
};

// ------------------- the program begin AST class -----------------
class programAST : public AST {
   public:
    programAST() = default;
    programAST(stmts_AST* ast2)
        : var_declarations(nullptr), statements_AST(ast2){};
    programAST(varDecls_AST* ast1, stmts_AST* ast2)
        : var_declarations(ast1), statements_AST(ast2){};

    void codegen(CG& cg) override;

   protected:
    varDecls_AST* var_declarations;
    stmts_AST* statements_AST;
};

#endif