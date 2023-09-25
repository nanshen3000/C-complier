#include <stdio.h>
#include <string>
#include <vector>
#include <map>

using namespace std;

// first: Token, second: the content (like number for NUM)
typedef pair<string, string> SemanticToken;

void Scanner(const char *fname, vector<SemanticToken> &tokens);