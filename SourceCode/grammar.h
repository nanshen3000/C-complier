#ifndef GRAMMAR_H
#define GRAMMAR_H

#include "scanner.h"
#include "AST.h"
#include <cstdint>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <set>
#include <iomanip>

using namespace std;

// the number to represent terminals and nonterminal
typedef uint16_t TERMINALS;
typedef uint16_t NONTERMINALS;
typedef uint16_t SYMBOL;

class Grammar;
class Production;
class Configuration;
class State;

class Grammar {
public:
    typedef uint32_t Action;

    Grammar() = default;
    Grammar(string Fname) : fname(Fname){
        getSymbol();
        getProductions();
        getFirstSet();
    };
    void get_LR1table();
    void getClosure(State *s); // get all the closure with the basic rules
    void build_states(vector<State *> &states, State *s);
    void build_LR1table(vector<State *> &states);

    void setPriority(const string &fname);

    void prase(const string &fname);
    //void prase(const vector<SemanticToken> &tokens);
    AST* prase(const vector<SemanticToken>& tokens);

    // print information of the grammaer
    void showTerminal();
    void showNonterminal();
    void showSymbols();
    void showProduction(int proIdx);
    void showProductions();
    void showProductions2();
    void showFirstSet();
    void showState(State *s);
    void output_LR1table();
    void showSRconflicts();

protected:
    string fname;
    map<string, TERMINALS> terminals;
    map<string, NONTERMINALS> nonterminals;
    vector<string> symbolStrs;

    vector<Production *> productions;
    vector<vector<int>> nonterminals_proIdxs;
    vector<vector<SYMBOL>> firstSet;
    vector<int> op_Prioritys;

    vector<vector<Action>> LR1table;

    uint nonterminalStart = 0;
    uint nonterminal_size = 0;
    uint terminal_size = 0;
    uint terminalStart = 0;
    uint totalSize = 0;

    void getSymbol();
    void getProductions();
    void getFirstSet();
    void filledFirstSet(SYMBOL nonterminal);

private:
    bool isTerminal(SYMBOL symbol);
    bool isNonterminal(SYMBOL symbol);
    bool isOp(SYMBOL terminal);
    SYMBOL convertToken(string &s);
    void reduceAST(vector<AST*> &ASTst, int proIdx);
    void deleteTokAst(vector<AST*>& ASTst);
    string getContentTokAST(vector<AST*>& ASTst);

    void setA(Action &a)
    {
        a = (1 << 31);
    }

    void setS(Action stateId, Action &a)
    {
        a |= (1 << 30) + ((stateId & 0xfff) << 12);
    }

    void setG(Action stateId, Action &a)
    {
        a |= (1 << 29) + ((stateId & 0xfff) << 12);
    }

    void setR(Action proIdx, Action &a){
        a |= (1 << 28) + (proIdx & 0xfff);
    }

    bool isError(Action x) {
        return x == 0;
    }

    bool isA(Action x){
        return (x >> 31) & 1;
    }

    bool isS(Action x) {
        return (x >> 30) & 1;
    }

    bool isG(Action x){
        return (x >> 29) & 1;
    }

    bool isR(Action x) {
        return (x >> 28) & 1;
    }


    int getR(Action x) {
        return x & 0xfff;
    }

    int getS(Action x) {
        return (x >> 12) & 0xfff;
    }


};


class Production
{
    friend class Grammar;

public:
    Production() = default;
    Production(string line, map<string, TERMINALS> &terminals, map<string, NONTERMINALS> &nonterminals)
    {
        convert(line, terminals, nonterminals);
    }

    Production &convert(string line, map<string, TERMINALS> &terminals, map<string, NONTERMINALS> &nonterminals);

protected:
    NONTERMINALS lhs;
    vector<SYMBOL> rhs;
};


class Configuration
{
    friend Grammar;
    friend State;

public:
    Configuration() = default;
    Configuration(int Pos, int ProIdx);
    Configuration(int Pos, int ProIdx, vector<SYMBOL> &Look_aheads);
    bool operator!=(const Configuration &other) const;

    bool addLook_aheads(vector<SYMBOL> &newLook_aheads);

protected:
    vector<TERMINALS> look_aheads;
    int pos = 0;   // the dot position
    int proIdx = 0;    // which production it belongs to
    bool isReduce = false; // whether this is a reduce configuration
};

class State
{
    friend Grammar;

public:
    State() = default;
    State(int Id, int BasicSize, int proIdx, int pos);

    bool operator==(const State &other) const;

    bool addConfiguration(int proIdx, int pos, vector<SYMBOL> &look_aheads);
    void addBasicConfig(int ProIdx, int Pos, vector<SYMBOL> &Look_aheads);
    void addBasicConfig(Configuration *c);
    void addStateLink(SYMBOL symbol, State* s);
    void setID(int x);

protected:
    typedef pair<SYMBOL, State*> StateLink; 

    vector<Configuration*> closure;
    int id;
    int BasicProSize = 0;
    int totalSize = 0;
    vector<StateLink> links; // the link to other state
    vector<Configuration *> reduces;
};

#endif