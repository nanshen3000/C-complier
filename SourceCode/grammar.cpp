#include "grammar.h"



// get the terminals and nonterminals, convert them to numbers
void Grammar::getSymbol()
{
    // get the nonterminals first
    ifstream inFile(fname);
    if (inFile.eof())
        return;
    string tok1, tok2;
    inFile >> tok1;

    SYMBOL i = 0;
    nonterminalStart = i;
    while (!inFile.eof())
    {
        inFile >> tok2;
        if (tok2 == "->" && !nonterminals.count(tok1))
        {
            nonterminals[tok1] = i++;
        }
        tok1 = tok2;
    }

    terminalStart = nonterminal_size = nonterminals.size();

    // get the terminals
    inFile.close();
    inFile.open(fname);

    while (inFile >> tok1)
    {
        if (tok1 != "->" && !nonterminals.count(tok1) && !terminals.count(tok1))
        {
            terminals[tok1] = i++;
        }
    }
    inFile.close();

    terminal_size = terminals.size();
    totalSize = terminal_size + nonterminal_size;

    symbolStrs.resize(totalSize, "");
    for (auto &p : terminals)
    {
        symbolStrs[p.second] = p.first;
    }

    for (auto &p : nonterminals)
    {
        symbolStrs[p.second] = p.first;
    }
}

void Grammar::getProductions()
{
    ifstream inFile(fname);
    string line;

    nonterminals_proIdxs.resize(nonterminal_size, vector<int>{});
    int i = 0;

    while (getline(inFile, line))
    {
        if (line.size() > 0)
        {
            Production *p = new Production(line, terminals, nonterminals);
            productions.push_back(p);
            nonterminals_proIdxs[p->lhs].push_back(i++);
        }
    }

    inFile.close();
}

// suppose the input grammar with no NULLABLE symbol
// so only needs to consider case First(A -> BC) = First(B)
void Grammar::getFirstSet()
{
    firstSet.resize(totalSize, vector<SYMBOL>{});

    // the first set of terminal is theirselves
    for (int i = terminalStart; i < terminalStart + terminal_size; ++i)
    {
        firstSet[i].push_back(i);
    }

    for (SYMBOL i = nonterminalStart; i < nonterminalStart + nonterminal_size; ++i)
    {
        if (firstSet[i].size() == 0)
        {
            filledFirstSet(i);
        }
    }
}

void Grammar::filledFirstSet(SYMBOL nonterminal)
{
    // already filled before
    if (firstSet[nonterminal].size() > 0)
        return;

    set<SYMBOL> res;
    for (auto idx : nonterminals_proIdxs[nonterminal])
    {
        int firstSymbol = productions[idx]->rhs[0];
        if (firstSymbol == nonterminal)
            continue;

        if (isNonterminal(firstSymbol))
        {
            filledFirstSet(firstSymbol);
        }

        for (auto first : firstSet[firstSymbol])
        {
            res.insert(first);
        }
    }

    for (auto first : res)
    {
        firstSet[nonterminal].push_back(first);
    }
}

void Grammar::get_LR1table()
{
    vector<State *> states;
    State *s = new State(0, 1, 0, 0);
    states.push_back(s);
    build_states(states, s);
    build_LR1table(states);
    /*
    for (auto state: states) {
        showState(state);
    }
    */
}

void Grammar::getClosure(State *s)
{
    bool changing = true;

    while (changing)
    {
        changing = false;
        int oldSize = s->closure.size();

        for (int i = 0; i < oldSize; ++i)
        {
            Configuration *c = s->closure[i];
            Production *production = productions[c->proIdx];

            int rhsSize = production->rhs.size();
            if (c->pos == rhsSize)
            {
                c->isReduce = true;
                continue;
            }
            SYMBOL symbol = production->rhs[c->pos];

            if (isTerminal(symbol))
                continue;

            vector<SYMBOL> look_aheads;
            if (c->pos == rhsSize - 1)
                look_aheads = c->look_aheads;
            else
                look_aheads = firstSet[production->rhs[c->pos + 1]];

            for (auto proIdx : nonterminals_proIdxs[symbol])
            {
                if (s->addConfiguration(proIdx, 0, look_aheads))
                    changing = true;
            }
        }
    }

    // find reduces rules
    for (int i = 0; i < s->BasicProSize; ++i)
    {
        Configuration *c = s->closure[i];
        if (c->isReduce)
        {
            s->reduces.push_back(c);
            continue;
        }
    }
}

void Grammar::showState(State *s)
{
    cout << "Basic Size: " << s->BasicProSize << endl;
    cout << "id: " << s->id << endl;
    for (auto config : s->closure)
    {
        Production *p = productions[config->proIdx];
        cout << symbolStrs[p->lhs] << " -> ";
        int size = p->rhs.size();
        for (int i = 0; i < size; ++i)
        {
            if (i == config->pos)
                cout << '.';
            cout << symbolStrs[p->rhs[i]] << " ";
        }
        cout << " {";
        if (config->look_aheads.size() == 0)
            cout << "NULLABLE";
        else
        {
            for (auto symbol : config->look_aheads)
                cout << symbolStrs[symbol] << ',';
        }
        cout << '}';
        cout << endl;
    }
    cout << "-----link state----" << endl;
    for (auto &link : s->links)
    {
        cout << symbolStrs[link.first] << " : " << (link.second)->id << endl;
    }
    cout << "------reduce productions---" << endl;
    for (auto c : s->reduces)
    {
        cout << c->proIdx << " " << endl;
    }
    cout << endl;
}

void Grammar::build_states(vector<State *> &states, State *s)
{
    getClosure(s);
    vector<State *> nxtStates(totalSize, nullptr);
    for (auto config : s->closure)
    {
        if (config->isReduce)
            continue;
        Production *p = productions[config->proIdx];

        SYMBOL nxtSymbol = p->rhs[config->pos];
        if (nxtStates[nxtSymbol] == nullptr)
        {
            nxtStates[nxtSymbol] = new State();
        }

        nxtStates[nxtSymbol]->addBasicConfig(config->proIdx, config->pos + 1, config->look_aheads);
    }

    for (SYMBOL symbol = 0; symbol < totalSize; ++symbol)
    {
        if (nxtStates[symbol] == nullptr)
            continue;

        State *nxtS = nxtStates[symbol];
        bool found = false;
        for (auto S : states)
        {
            if (*S == *nxtS)
            {
                found = true;
                s->addStateLink(symbol, S);
                delete nxtS;
                break;
            }
        }

        if (!found)
        {
            nxtS->setID(states.size());
            s->addStateLink(symbol, nxtS);
            states.push_back(nxtS);
            build_states(states, nxtS);
        }
    }
}

void Grammar::build_LR1table(vector<State *> &states)
{
    LR1table.resize(states.size(), vector<Action>(totalSize, 0));
    for (auto state : states)
    {
        int id = state->id;

        // filled Goto and shift action
        for (auto &stateLink : state->links)
        {
            SYMBOL symbol = stateLink.first;
            // meet '$', it will be the first nonterminal
            if (symbol == terminalStart)
            {
                setA(LR1table[id][symbol]);
            }
            else
            {
                if (isTerminal(symbol))
                    setS((stateLink.second)->id, LR1table[id][symbol]);
                else
                    setG((stateLink.second)->id, LR1table[id][symbol]);
            }
        }

        // filled Accpet and reduce action
        for (auto &c : state->reduces)
        {
            for (auto terminal : c->look_aheads)
            {
                setR(c->proIdx, LR1table[id][terminal]);
            }
        }
    }
}

void Grammar::setPriority(const string &fname) {
    op_Prioritys.resize(totalSize, 0);
    ifstream fin(fname);
    string line;
    int priority = 1;
    while (getline(fin, line))
    {
        stringstream ss(line);
        string tok;
        while (ss >> tok) {
            op_Prioritys[terminals[tok]] = priority;
        }
        priority++;
    }
    fin.close();
}

void Grammar::output_LR1table()
{
    int m = LR1table.size(), n = LR1table[0].size();
    cout << left << setw(8) << "   ";
    for (SYMBOL i = nonterminalStart; i < totalSize; ++i)
    {
        cout << left << setw(8) << symbolStrs[i];
    }
    cout << endl;

    for (int i = 0; i < m; ++i)
    {
        cout << setw(8) << i;
        for (int j = 0; j < n; ++j)
        {
            Action action = LR1table[i][j];
            if (action == 0)
            {
                cout << left << setw(8) << "   ";
                continue;
            }

            switch (action >> 14)
            {
            case 3:
                cout << left << 'A';
                break;

            case 2:
                cout << left << 'S';
                break;

            case 1:
                cout << left << 'G';
                break;

            case 0:
                cout << left << 'R';

            default:
                break;
            }
            cout << left << setw(4) << (LR1table[i][j] & 0b0011111111111111) << " ";
        }
        cout << endl;
    }
}

bool Grammar::isTerminal(SYMBOL symbol)
{
    return symbol >= terminalStart && symbol <= terminalStart + terminal_size;
}

bool Grammar::isNonterminal(SYMBOL symbol)
{
    return symbol >= nonterminalStart && symbol <= nonterminalStart + nonterminal_size;
}

bool Grammar::isOp(SYMBOL terminal) {
    return op_Prioritys[terminal] > 0;
}

void Grammar::showTerminal()
{
    cout << "------Terminals-------" << endl;
    for (auto x : terminals)
    {
        cout << x.first << " " << x.second << endl;
    }
    cout << endl;
}

void Grammar::showNonterminal()
{
    cout << "------Nonterminals-------" << endl;
    for (auto x : nonterminals)
    {
        cout << x.first << " " << x.second << endl;
    }
}

void Grammar::showProduction(int proIdx) {
    Production *p = productions[proIdx];
    cout << symbolStrs[p->lhs] << " -> ";
    for (auto rsymbol : p->rhs)
    {
        cout << symbolStrs[rsymbol] << " ";
    }
}

void Grammar::showProductions()
{
    cout << "------Productions-------" << endl;
    for (int i = 0; i < productions.size(); ++i) {
        cout << i << ": ";
        showProduction(i);
        cout << endl;
    }
}

void Grammar::showFirstSet()
{
    cout << "------First Set-------" << endl;
    int i = 0;
    for (auto &firstset : firstSet)
    {
        cout << symbolStrs[i++] << " : ";
        for (auto &first : firstset)
        {
            cout << symbolStrs[first] << " ";
        }
        cout << endl;
    }
}

void Grammar::showSymbols() {
    for (int i = nonterminalStart; i < totalSize; ++i) {
        cout << i << ' ' << symbolStrs[i] << endl;
    }
}

void Grammar::showSRconflicts() {
    int m = LR1table.size(), n = LR1table[0].size();
    for (int i = 0; i < m; ++i)
    {
        bool found = false;
        for (SYMBOL j = 0; j < n; ++j)
        {
            Action action = LR1table[i][j];
            if (isS(action) && isR(action))
            {
                cout << symbolStrs[j] << " ";
                found = true;
            }
        }
        if (found)
            cout << "state:" << i << " " << endl;
    }
}


string upper(string &s)
{
    string res = "";
    for (auto ch : s)
    {
        if (ch >= 'a' && ch <= 'z')
            res += (ch - 'a' + 'A');
        else
            res += ch;
    }
    return res;
}

string lower(string &s)
{
    string res = "";
    for (auto ch : s)
    {
        if (ch >= 'A' && ch <= 'Z')
            res += (ch - 'A' + 'a');
        else
            res += ch;
    }
    return res;
}

void Grammar::showProductions2()
{
    cout << "------Productions-------" << endl;
    for (auto p : productions)
    {
        cout << upper(symbolStrs[p->lhs]) << " -> ";
        for (auto rsymbol : p->rhs)
        {
            if (isTerminal(rsymbol))
                cout << lower(symbolStrs[rsymbol]) << " ";
            else
                cout << upper(symbolStrs[rsymbol]) << " ";
        }
        cout << "." << endl;
    }
}

SYMBOL Grammar::convertToken(string &s) {
    if (terminals.count(s)) {
        return terminals[s];
    }
    return nonterminals[s];
}


