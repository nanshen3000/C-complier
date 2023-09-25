#include "grammar.h"
#include "scanner.h"

AST* Grammar::prase(const vector<SemanticToken>& tokens)
{
    string token;
    string tokContent;
    vector<int> stateSt;
    vector<SYMBOL> currSt;
    vector<SYMBOL> praseSt;
    vector<AST*> ASTst;
    stateSt.push_back(0);  // initial state 0
    int tokensIdx = 0;

    while (1) {
        int state = stateSt.back();
        //cout << "state: " << state << "   ";

        if (praseSt.empty()) {
            token = tokens[tokensIdx].first;
            tokContent = tokens[tokensIdx].second;
            tokensIdx++;
            praseSt.push_back(convertToken(token));
        }
        SYMBOL symbol = praseSt.back();

        //cout << "next type:" << symbolStrs[symbol] << "   ";

        Action action = LR1table[state][symbol];

        // decided which action to take
        enum Todo { SHIFT, GOTO, REDUCE, ACCEPT, ERROR };

        Todo todo = ERROR;

        if (isS(action) && isR(action)) {
            if (isOp(symbol) && currSt.size() > 1) {
                int prevOp = currSt[currSt.size() - 2];
                if (op_Prioritys[prevOp] >= op_Prioritys[symbol]) {
                    todo = REDUCE;
                }
                else {
                    todo = SHIFT;
                }
            }
            else
                todo = SHIFT;
        }
        else if (isS(action)) {
            todo = SHIFT;
        }
        else if (isG(action)) {
            todo = GOTO;
        }
        else if (isR(action)) {
            todo = REDUCE;
        }
        else if (isA(action)) {
            todo = ACCEPT;
        }

        // now take the action

        if (todo == SHIFT) {
            int nxtState = getS(action);
            //cout << "shift to state  " << nxtState << endl;
            stateSt.push_back(nxtState);
            praseSt.pop_back();
            currSt.push_back(symbol);

            // add a new AST node to AST stack
            TokAST* tokAST = new TokAST(tokContent);
            ASTst.push_back(tokAST);
        }
        else if (todo == GOTO) {
            int nxtState = getS(action);
            //cout << "go to state  " << nxtState << endl;
            stateSt.push_back(nxtState);
            praseSt.pop_back();
        }
        else if (todo == REDUCE) {
            int proIdx = getR(action);
            Production* p = productions[proIdx];
            /*
            cout << "reduce by grammar " << proIdx  << ": ";
            showProduction(proIdx);
            cout << endl;
            */
            int rhsSize = p->rhs.size();
            while (rhsSize--) {
                stateSt.pop_back();
                currSt.pop_back();
            }
            int nxtSymbol = p->lhs;
            currSt.push_back(nxtSymbol);
            praseSt.push_back(nxtSymbol);

            // reduce the node in the AST stack
            reduceAST(ASTst, proIdx);
        }
        else if (todo == ACCEPT) {
            //cout << endl;
            //cout << "Accept!" << endl;
            return ASTst.front();
        }
        else {
            cout << "The file is not Accept!" << endl;
            exit(0);
            return nullptr;
        }
        /*
        cout << "current situation: ";
        for (auto currS : currSt) {
            cout << symbolStrs[currS] << " ";
        }
        cout << '|' << endl;
        cout << "size: " << ASTst.size() << endl;
        cout << endl;

        if (ASTst.size() != currSt.size())
            return nullptr;
        */
    }
}

// unuseful AST like "SEMI" is delete
inline void Grammar::deleteTokAst(vector<AST*>& ASTst)
{
    TokAST* tokast = (TokAST*)ASTst.back();
    delete tokast;
    ASTst.pop_back();
}

// delete tokAST and get its content
inline string Grammar::getContentTokAST(vector<AST*>& ASTst)
{
    TokAST* tokast = (TokAST*)ASTst.back();
    string res = tokast->content;
    delete tokast;
    ASTst.pop_back();
    return res;
}



void Grammar::reduceAST(vector<AST*>& ASTst, int proIdx)
{
    switch (proIdx) {
        case 1: {
            stmts_AST* stmts_ast = (stmts_AST*)ASTst.back();
            ASTst.pop_back();

            varDecls_AST* varDecls_ast = (varDecls_AST*)ASTst.back();
            ASTst.pop_back();

            programAST* a = new programAST(varDecls_ast, stmts_ast);
            ASTst.push_back(a);
        } break;

        case 2: {
            stmts_AST* stmts_ast = (stmts_AST*)ASTst.back();
            ASTst.pop_back();

            programAST* a = new programAST(stmts_ast);
            ASTst.push_back(a);
        } break;

        case 3: {
            varDeclAST* varDecl_ast = (varDeclAST*)ASTst.back();
            ASTst.pop_back();

            varDecls_AST* varDecls_ast = (varDecls_AST*)ASTst.back();
            varDecls_ast->add(varDecl_ast);
        } break;

        case 4: {
            varDeclAST* varDecl_ast = (varDeclAST*)ASTst.back();
            ASTst.pop_back();

            varDecls_AST* varDecls_ast = new varDecls_AST(varDecl_ast);
            ASTst.push_back(varDecls_ast);
        } break;

        case 5: {
            deleteTokAst(ASTst);

            decls_AST* decls_ast = (decls_AST*)ASTst.back();
            ASTst.pop_back();

            deleteTokAst(ASTst);

            varDeclAST* a = new varDeclAST(decls_ast);
            ASTst.push_back(a);
        } break;

        case 6: {
            declAST* a = (declAST*)ASTst.back();
            ASTst.pop_back();

            deleteTokAst(ASTst);

            decls_AST* decls_ast = (decls_AST*)ASTst.back();
            decls_ast->addAST(a);
        } break;

        case 7: {
            declAST* a = (declAST*)ASTst.back();
            ASTst.pop_back();

            decls_AST* decls_ast = new decls_AST(a);
            ASTst.push_back(decls_ast);
        } break;

        case 8: {
            InitVarDeclAST* a = new InitVarDeclAST();
            a->setVal(getContentTokAST(ASTst));
            deleteTokAst(ASTst);
            a->setID(getContentTokAST(ASTst));
            ASTst.push_back(a);
        } break;

        case 9: {
            ArrayDeclAST* a = new ArrayDeclAST();
            deleteTokAst(ASTst);
            a->setSize(getContentTokAST(ASTst));
            deleteTokAst(ASTst);
            a->setID(getContentTokAST(ASTst));
            ASTst.push_back(a);
        } break;

        case 10: {
            unInitVarDeclAST* a = new unInitVarDeclAST();
            a->setID(getContentTokAST(ASTst));
            ASTst.push_back(a);
        } break;

        case 12: {
            deleteTokAst(ASTst);

            stmts_AST* a = (stmts_AST*) ASTst.back();
            ASTst.pop_back();

            deleteTokAst(ASTst);

            ASTst.push_back(a);
        } break;

        case 13: {
            stmtAST* stmt_ast = (stmtAST*)ASTst.back();
            ASTst.pop_back();

            stmts_AST* stmts_ast = (stmts_AST*)ASTst.back();
            stmts_ast->addAST(stmt_ast);
        } break;

        case 11: case 14: {
            stmtAST* stmt_ast = (stmtAST*)ASTst.back();
            ASTst.pop_back();
            stmts_AST* a = new stmts_AST();
            a->addAST(stmt_ast);
            ASTst.push_back(a);
        } break;

        case 15:
        case 17:
        case 21:
        case 22: {
            deleteTokAst(ASTst);

        } break;

        case 16:
        case 19:
        case 20:
        case 23:
        case 24: {
        } break;

        case 18: {
            ASTst.push_back(nullptr);
        } break;

        case 25: {
            AssignStmt_AST* a = new AssignStmt_AST();

            expAST* valExp = (expAST*)ASTst.back();
            ASTst.pop_back();
            a->setValExp(valExp);

            deleteTokAst(ASTst);
            deleteTokAst(ASTst);

            expAST* idxExp = (expAST*)ASTst.back();
            ASTst.pop_back();
            a->setIdxExp(idxExp);

            deleteTokAst(ASTst);

            a->setID(getContentTokAST(ASTst));
            ASTst.push_back(a);
        } break;

        case 26: {
            AssignStmt_AST* a = new AssignStmt_AST();

            a->setIdxExp(nullptr);

            expAST* valExp = (expAST*)ASTst.back();
            ASTst.pop_back();
            a->setValExp(valExp);

            deleteTokAst(ASTst);

            a->setID(getContentTokAST(ASTst));
            ASTst.push_back(a);
        } break;

        case 27: {
            IFstmt_AST* a = (IFstmt_AST*)ASTst.back();
            a->addElseAST(nullptr);
        } break;

        case 28: {
            stmts_AST* elseAST = (stmts_AST*)ASTst.back();
            ASTst.pop_back();

            deleteTokAst(ASTst);

            IFstmt_AST* a = (IFstmt_AST*)ASTst.back();
            a->addElseAST(elseAST);
        } break;

        case 29: {
            stmts_AST* code_block = (stmts_AST*)ASTst.back();
            ASTst.pop_back();

            deleteTokAst(ASTst);

            expAST* exp = (expAST*)ASTst.back();
            ASTst.pop_back();

            deleteTokAst(ASTst);
            deleteTokAst(ASTst);

            IFstmt_AST* a = new IFstmt_AST(exp, code_block);
            ASTst.push_back(a);
        } break;

        case 30: {
            stmts_AST* code_block = (stmts_AST*)ASTst.back();
            ASTst.pop_back();

            deleteTokAst(ASTst);

            expAST* exp = (expAST*)ASTst.back();
            ASTst.pop_back();

            deleteTokAst(ASTst);
            deleteTokAst(ASTst);

            WHILEstmt_AST* a = new WHILEstmt_AST(false, exp, code_block);
            ASTst.push_back(a);
        } break;

        case 31: {
            deleteTokAst(ASTst);

            expAST* exp = (expAST*)ASTst.back();
            ASTst.pop_back();

            deleteTokAst(ASTst);
            deleteTokAst(ASTst);

            stmts_AST* code_block = (stmts_AST*)ASTst.back();
            ASTst.pop_back();

            deleteTokAst(ASTst);

            WHILEstmt_AST* a = new WHILEstmt_AST(true, exp, code_block);
            ASTst.push_back(a);
        } break;

        case 32: {
            ASTst.pop_back();
            ASTst.push_back(new Return_AST());
        } break;

        case 33: {
            deleteTokAst(ASTst);

            readStmt_AST* a = new readStmt_AST(getContentTokAST(ASTst));

            deleteTokAst(ASTst);
            deleteTokAst(ASTst);

            ASTst.push_back(a);
        } break;

        case 34: {
            deleteTokAst(ASTst);

            expAST* exp = (expAST*)ASTst.back();
            ASTst.pop_back();

            deleteTokAst(ASTst);
            deleteTokAst(ASTst);

            ASTst.push_back(new writeStmt_AST(exp));
        } break;

        case 35: {
            deleteTokAst(ASTst);

            expAST* exp = (expAST*)ASTst.back();
            ASTst.pop_back();

            deleteTokAst(ASTst);

            ASTst.push_back(exp);
        } break;

        case 36: {
            INTNUMexpAST* a = new INTNUMexpAST(getContentTokAST(ASTst));
            ASTst.push_back(a);
        } break;

        case 37: {
            IDexpAST* a = new IDexpAST(getContentTokAST(ASTst));
            ASTst.push_back(a);
        } break;

        case 38: {
            deleteTokAst(ASTst);

            expAST* exp = (expAST*)ASTst.back();
            ASTst.pop_back();

            deleteTokAst(ASTst);

            string s = getContentTokAST(ASTst);
            ASTst.push_back(new ArrayIDexpAST(s, exp));
        } break;

        case 39: {
            expAST* exp = (expAST*)ASTst.back();
            // constant reduce
            if (exp->expType == expAST::INTNUM_t) {
                INTNUMexpAST* a = (INTNUMexpAST*)ASTst.back();
                int val = !(a->num);
                ASTst.pop_back();
                deleteTokAst(ASTst);
                ASTst.push_back(new INTNUMexpAST(val));
            }
            else {
                ASTst.pop_back();
                deleteTokAst(ASTst);
                ASTst.push_back(new singleOPexpAST(expAST::NOT_OP, exp));
            }
        } break;

        case 40: {
            expAST* exp = (expAST*)ASTst.back();
            // constant reduce
            if (exp->expType == expAST::INTNUM_t) {
                INTNUMexpAST* a = (INTNUMexpAST*)ASTst.back();
                int val = -a->num;
                ASTst.pop_back();
                deleteTokAst(ASTst);
                ASTst.push_back(new INTNUMexpAST(val));
            }
            else {
                ASTst.pop_back();
                deleteTokAst(ASTst);
                ASTst.push_back(new singleOPexpAST(expAST::MINUS, exp));
            }

        } break;

        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
        case 50:
        case 51:
        case 52:
        case 53:
        case 54:
        case 55:
        case 56: {
            expAST* Rexp = (expAST*)ASTst.back();
            ASTst.pop_back();

            deleteTokAst(ASTst);

            expAST* Lexp = (expAST*)ASTst.back();
            ASTst.pop_back();

            // reduce to a INTNUM node if both child are constant
            if (Lexp->expType == expAST::INTNUM_t &&
                Rexp->expType == expAST::INTNUM_t) {
                INTNUMexpAST* constant1 = (INTNUMexpAST*)Lexp;
                INTNUMexpAST* constant2 = (INTNUMexpAST*)Rexp;
                int val1 = constant1->num;
                int val2 = constant2->num;
                int val;

                switch (proIdx) {
                    case 41:
                        val = val1 * val2;
                        break;
                    case 42:
                        val = val1 / val2;
                        break;
                    case 43:
                        val = val1 + val2;
                        break;
                    case 44:
                        val = val1 - val2;
                        break;
                    case 45:
                        val = val1 << val2;
                        break;
                    case 46:
                        val = val1 >> val2;
                        break;
                    case 47:
                        val = val1 < val2;
                        break;
                    case 48:
                        val = val1 > val2;
                        break;
                    case 49:
                        val = val1 <= val2;
                        break;
                    case 50:
                        val = val1 >= val2;
                        break;
                    case 51:
                        val = val1 == val2;
                        break;
                    case 52:
                        val = val1 != val2;
                        break;
                    case 53:
                        val = val1 & val2;
                        break;
                    case 54:
                        val = val1 | val2;
                        break;
                    case 55:
                        val = val1 && val2;
                        break;
                    case 56:
                        val = val1 || val2;
                        break;

                    default:
                        break;
                }

                ASTst.push_back(new INTNUMexpAST(val));
            }
            else {
                expAST::expASTop opType;

                switch (proIdx) {
                    case 41:
                        opType = expAST::MUL_OP;
                        break;
                    case 42:
                        opType = expAST::DIV_OP;
                        break;
                    case 43:
                        opType = expAST::PLUS;
                        break;
                    case 44:
                        opType = expAST::MINUS;
                        break;
                    case 45:
                        opType = expAST::SHL_OP;
                        break;
                    case 46:
                        opType = expAST::SHR_OP;
                        break;
                    case 47:
                        opType = expAST::LT;
                        break;
                    case 48:
                        opType = expAST::GT;
                        break;
                    case 49:
                        opType = expAST::LTEQ;
                        break;
                    case 50:
                        opType = expAST::GTEQ;
                        break;
                    case 51:
                        opType = expAST::EQ;
                        break;
                    case 52:
                        opType = expAST::NOTEQ;
                        break;
                    case 53:
                        opType = expAST::AND_OP;
                        break;
                    case 54:
                        opType = expAST::OR_OP;
                        break;
                    case 55:
                        opType = expAST::ANDAND;
                        break;
                    case 56:
                        opType = expAST::OROR;
                        break;

                    default:
                        break;
                }

                ASTst.push_back(new BinaryOPexpAST(Lexp, opType, Rexp));
            }
        } break;



        default:
            break;
    }
}
