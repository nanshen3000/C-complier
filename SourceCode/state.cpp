#include "grammar.h"

Production &Production::convert(string line, map<string, TERMINALS> &terminals, map<string, NONTERMINALS> &nonterminals)
{
    stringstream ssline(line);
    string tok;
    ssline >> tok;
    if (nonterminals.count(tok))
        lhs = nonterminals[tok];
    else
        return *this;

    ssline >> tok;
    while (ssline >> tok)
    {
        if (terminals.count(tok))
        {
            rhs.push_back(terminals[tok]);
        }
        else if (nonterminals.count(tok))
        {
            rhs.push_back(nonterminals[tok]);
        }
        else
        {
        }
    }

    return *this;
}

inline Configuration::Configuration(int ProIdx, int Pos)
{
    pos = Pos;
    proIdx = ProIdx;
    isReduce = false;
}

Configuration::Configuration(int Pos, int ProIdx, vector<SYMBOL> &Look_aheads)
{
    pos = Pos;
    proIdx = ProIdx;
    isReduce = false;
    addLook_aheads(Look_aheads);
}

bool Configuration::addLook_aheads(vector<SYMBOL> &newLook_aheads)
{
    bool added = false;
    for (auto symbol : newLook_aheads)
    {
        bool found = false;
        for (auto oldSymbol : look_aheads)
        {
            if (symbol == oldSymbol)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            look_aheads.push_back(symbol);
            added = true;
        }
    }
    return added;
}

bool Configuration::operator != (const Configuration &other) const
{
    if (pos != other.pos || proIdx != other.proIdx || look_aheads.size() != other.look_aheads.size())
        return true;
    int size = look_aheads.size();
    for (int i = 0; i < size; ++i)
    {
        if (look_aheads[i] != other.look_aheads[i])
            return true;
    }
    return false;
}

State::State(int Id, int BasicSize, int proIdx, int pos)
{
    id = Id;
    BasicProSize = BasicSize;
    totalSize = BasicSize;
    Configuration *p = new Configuration(proIdx, pos);
    closure.push_back(p);
}

// as long as the basic config are the same, the state will be the same
bool State::operator==(const State &other) const {
    if (BasicProSize != other.BasicProSize)
        return false;
    for (int i = 0; i < BasicProSize; ++i) {
        if (*(closure[i]) != *(other.closure[i]))
            return false;
    }
    return true;
}

void State::setID(int x)
{
    id = x;
}

bool State::addConfiguration(int proIdx, int pos, vector<SYMBOL> &look_aheads)
{
    for (auto config : closure)
    {
        if (proIdx == config->proIdx && pos == config->pos)
        {
            return config->addLook_aheads(look_aheads);
        }
    }

    Configuration *config = new Configuration(pos, proIdx, look_aheads);
    closure.push_back(config);
    totalSize = closure.size();
    return true;
}

void State::addBasicConfig(Configuration *c)
{
    addBasicConfig(c->proIdx, c->pos, c->look_aheads);
}

void State::addBasicConfig(int ProIdx, int Pos, vector<SYMBOL> &Look_aheads) {
    Configuration *config = new Configuration(Pos, ProIdx, Look_aheads);
    closure.push_back(config);
    ++BasicProSize;
}

void State::addStateLink(SYMBOL symbol, State *s) {
    links.push_back(StateLink {symbol, s});
}