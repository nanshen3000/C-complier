#include "scanner.h"
#include "grammar.h"
#include "AST.h"
#include <vector>

using namespace std;

int main(int agrc, char **argv) {
    if (agrc < 2) {
        cout << "please enter a input file" << endl;
    }
    vector<SemanticToken> tokens;
    Scanner(argv[1], tokens);                // scanner
    Grammar grammar("grammar/grammar4.txt"); // initialize the grammar
    grammar.get_LR1table();                 // get the LR1 table 
    grammar.setPriority("grammar/priority.txt");

    programAST* root = (programAST*) grammar.prase(tokens);  // start prasing

    CG* cg = new CG();
    root->codegen(*cg);
}