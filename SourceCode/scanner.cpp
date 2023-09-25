#include "scanner.h"

// the tokens the scanner may gennerate
enum TOKEN {
    LBRACE,
    RBRACE,
    LSQUARE,
    RSQUARE,
    LPAR,
    RPAR,
    SEMI,
    PLUS,
    MINUS,
    MUL_OP,
    DIV_OP,
    AND_OP,
    OR_OP,
    NOT_OP,
    ASSIGN,
    LT,
    GT,
    SHL_OP,
    SHR_OP,
    EQ,
    NOTEQ,
    LTEQ,
    GTEQ,
    ANDAND,
    OROR,
    COMMA,
    INT_NUM,
    ID,
    INT,
    MAIN,
    VOID,
    BREAK,
    DO,
    ELSE,
    IF,
    WHILE,
    RETRUN,
    READ,
    WRITE,
    TOKEN_MAX  // used to get the number of tokens, not a real token
};

vector<string> token_table(TOKEN_MAX);

// init the token_table, convert a token into string that can be printed
void init_token_table() {
    token_table[LBRACE] = "LBRACE";
    token_table[RBRACE] = "RBRACE";
    token_table[LSQUARE] = "LSQUARE";
    token_table[RSQUARE] = "RSQUARE";
    token_table[LPAR] = "LPAR";
    token_table[RPAR] = "RPAR";
    token_table[SEMI] = "SEMI";
    token_table[PLUS] = "PLUS";
    token_table[MINUS] = "MINUS";
    token_table[DIV_OP] = "DIV_OP";
    token_table[MUL_OP] = "MUL_OP";
    token_table[AND_OP] = "AND_OP";
    token_table[OR_OP] = "OR_OP";
    token_table[NOT_OP] = "NOT_OP";
    token_table[ASSIGN] = "ASSIGN";
    token_table[LT] = "LT";
    token_table[GT] = "GT";
    token_table[SHL_OP] = "SHL_OP";
    token_table[SHR_OP] = "SHR_OP";
    token_table[EQ] = "EQ";
    token_table[NOTEQ] = "NOTEQ";
    token_table[LTEQ] = "LTEQ";
    token_table[GTEQ] = "GTEQ";
    token_table[ANDAND] = "ANDAND";
    token_table[OROR] = "OROR";
    token_table[COMMA] = "COMMA";
    token_table[INT_NUM] = "INT_NUM";
    token_table[ID] = "ID";
    token_table[INT] = "INT";
    token_table[MAIN] = "MAIN";
    token_table[VOID] = "VOID";
    token_table[BREAK] = "BREAK";
    token_table[DO] = "DO";
    token_table[ELSE] = "ELSE";
    token_table[IF] = "IF";
    token_table[WHILE] = "WHILE";
    token_table[RETRUN] = "RETURN";
    token_table[READ] = "READ";
    token_table[WRITE] = "WRITE";
}

// use to specify the reserved_words from ID
map<string, enum TOKEN> reserved_words;

void init_reserved_words() {
    reserved_words["int"] = INT;
    reserved_words["main"] = MAIN;
    reserved_words["void"] = VOID;
    reserved_words["break"] = BREAK;
    reserved_words["do"] = DO;
    reserved_words["else"] = ELSE;
    reserved_words["if"] = IF;
    reserved_words["while"] = WHILE;
    reserved_words["return"] = RETRUN;
    reserved_words["scanf"] = READ;
    reserved_words["printf"] = WRITE;
}

// output the token
inline void output_token(int token) {
    printf("%s\n", token_table[token].c_str());
}

inline void output_token_vec(int token,const string &content, vector<SemanticToken>& tokens) {
    tokens.push_back(SemanticToken(token_table[token], content));
}

char curr_char;
int goback = 0;  // goback = 1 means the program get the next character but not use it
int times = 0;
char getchar(FILE* fp) {
    // already get the next character from the file, but go back before

    if (goback) {
        goback = 0;
        return curr_char;
    }
    // get the next character from the file
    else {
        curr_char = fgetc(fp);
        return curr_char;
    }
}

// remove all the following digit [0-9]
// used for case '+' '-' and [0-9]
int getNum(FILE* fp, string &tok) {
    goback = 1;
    if ((curr_char = getchar(fp)) < '0' || curr_char > '9') {
        return 0;
    }
    while ((curr_char = getchar(fp)) >= '0' && curr_char <= '9') {
        tok += curr_char;
    }
    goback = 1;
    return 1;
}

// the main scanning process
void scan(FILE* fp, vector<SemanticToken>& tokens) {
    char c;
    while ((c = getchar(fp)) != EOF) {
        string tok = "";
        switch (c) {
            // the following tokens has only 1 char
            // no look ahead
            case '(':
                output_token_vec(LPAR, "(", tokens);
                break;

            case ')':
                output_token_vec(RPAR, ")", tokens);
                break;

            case '{':
                output_token_vec(LBRACE, "{", tokens);
                break;

            case '}':
                output_token_vec(RBRACE, "}", tokens);
                break;

            case ',':
                output_token_vec(COMMA, ",", tokens);
                break;

            case ';':
                output_token_vec(SEMI, ";", tokens);
                break;

            case '[':
                output_token_vec(LSQUARE, "[", tokens);
                break;

            case ']':
                output_token_vec(RSQUARE, "]", tokens);
                break;

            case '*':
                output_token_vec(MUL_OP, "*", tokens);
                break;

            case '/':
                output_token_vec(DIV_OP, "/", tokens);
                break;

            // the following needs 1 look ahead
            case '&':
                if ((c = getchar(fp)) == '&') {
                    output_token_vec(ANDAND, "&&", tokens);
                } else {
                    goback = 1;
                    output_token_vec(AND_OP, "&", tokens);
                }
                break;

            case '|':
                if ((c = getchar(fp)) == '|') {
                    output_token_vec(OROR, "||", tokens);
                } else {
                    goback = 1;
                    output_token_vec(OR_OP, "|", tokens);
                }
                break;

            case '=':
                if ((c = getchar(fp)) == '=') {
                    output_token_vec(EQ, "==", tokens);
                } else {
                    goback = 1;
                    output_token_vec(ASSIGN, "=", tokens);
                }
                break;

            case '!':
                if ((c = getchar(fp)) == '=') {
                    output_token_vec(NOTEQ, "!=", tokens);
                } else {
                    goback = 1;
                    output_token_vec(NOT_OP, "!", tokens);
                }
                break;

            case '>':
                if ((c = getchar(fp)) == '=') {
                    output_token_vec(GTEQ, ">=", tokens);
                } else if (c == '>') {
                    output_token_vec(SHR_OP, ">>", tokens);
                } else {
                    goback = 1;
                    output_token_vec(GT, ">", tokens);
                }
                break;

            case '<':
                if ((c = getchar(fp)) == '=') {
                    output_token_vec(LTEQ, "<=", tokens);
                } else if (c == '<') {
                    output_token_vec(SHL_OP, "<<", tokens);
                } else {
                    goback = 1;
                    output_token_vec(LT, "<", tokens);
                }
                break;

            // handle '+', '-' and number
            case '+':
                if (getNum(fp, tok) == 1) {
                    output_token_vec(INT_NUM, tok, tokens);
                } else {
                    output_token_vec(PLUS, "+", tokens);
                }
                break;

            case '-':
                if (getNum(fp, tok) == 1) {
                    output_token_vec(INT_NUM, tok, tokens);
                } else {
                    output_token_vec(MINUS, "-", tokens);
                }
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                tok += curr_char;
                getNum(fp, tok);
                output_token_vec(INT_NUM, tok, tokens);
                break;

            default:
                // handle reserved words and ID
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                    tok.push_back(c);
                    while (c = getchar(fp)) {
                        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                            (c >= '0' && c <= '9') || c == '_') {
                            tok.push_back(c);
                        } else {
                            goback = 1;
                            break;
                        }
                    }

                    goback = 1;

                    if (reserved_words.count(tok) > 0) {
                        output_token_vec(reserved_words[tok], tok, tokens);
                    } else {
                        output_token_vec(ID, tok, tokens);
                    }
                }

                // end scanning
                if (c == EOF) {
                    return;
                }

                else {
                    // ignore other character
                }

                break;
        }
    }
}

void Scanner(const char* fname, vector<SemanticToken>& tokens) {
    FILE* fp = fopen(fname, "r");
    init_token_table();
    init_reserved_words();
    scan(fp, tokens);
    tokens.push_back(SemanticToken ("$", ""));
    fclose(fp);
}