#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <io.h>

using namespace std;
int token;                    // current token
int token_val;                // value of current token (mainly for number)
int line;                     // line number
int poolsize;
vector<pair<string, string>> symbols;

char *src;          // pointer to source code string;
const static char *keywords[] = {"if", "else", "then", "while", "do"};

enum {
    Num = 128, Id,
    Assign, Lt, Gt, Add, Sub, Mul, Div, If, Then, Else, While, Do
};

void next() {
    char *last_pos;
    char s[128];
    int hash;

    while (token = *src) {
        ++src;

        // parse token here
        if (token == '\n') {
            ++line;
        } else if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || (token == '_')) {
            // parse identifier
            last_pos = src - 1;
            hash = token;

            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') ||
                   (*src == '_')) {
                hash = hash * 147 + *src;
                src++;
            }
            int i = 0;
            // 判断是否为关键字，否则为标识符
            while (i < 5) {
                if (strlen(keywords[i]) == src - last_pos && !memcmp(keywords[i], last_pos, src - last_pos)) {
                    switch (i) {
                        case 0: {
                            token = If;
                            symbols.emplace_back("if", "-");
                            return;
                        }
                        case 1: {
                            token = Else;
                            symbols.emplace_back("else", "-");
                            return;
                        }
                        case 2: {
                            token = Then;
                            symbols.emplace_back("then", "-");
                            return;
                        }
                        case 3: {
                            token = While;
                            symbols.emplace_back("while", "-");
                            return;
                        }
                        case 4: {
                            token = Do;
                            symbols.emplace_back("do", "-");
                            return;
                        }
                        default:
                            break;
                    }
                }
                i++;
            }
            token = Id;
            memset(s, 0, 128);
            strncpy(s, last_pos, src - last_pos);
            symbols.emplace_back("0", s);
            return;
        } else if (token >= '0' && token <= '9') {
            // parse number, three kinds: dec(123) hex(0x123) oct(017)
            token_val = token - '0';
            if (token_val > 0) {
                // dec, starts with [1-9]
                while (*src >= '0' && *src <= '9') {
                    token_val = token_val * 10 + *src++ - '0';
                }
                symbols.emplace_back("1", to_string(token_val));
            } else {
                // starts with 0
                if (*src == 'x' || *src == 'X') {
                    //hex
                    token = *++src;
                    last_pos = src;
                    while ((token >= '0' && token <= '9') || (token >= 'a' && token <= 'f') ||
                           (token >= 'A' && token <= 'F')) {
                        token_val = token_val * 16 + (token & 15) + (token >= 'A' ? 9 : 0);
                        token = *++src;
                    }
                    memset(s, 0, 128);
                    strncpy(s, last_pos, src - last_pos);
                    symbols.emplace_back("3", s);
                } else if (*src >= '0' && *src <= '7') {
                    // oct
                    last_pos = src;
                    while (*src >= '0' && *src <= '7') {
                        token_val = token_val * 8 + *src++ - '0';
                    }
                    memset(s, 0, 128);
                    strncpy(s, last_pos, src - last_pos);
                    symbols.emplace_back("2", s);
                } else {
                    // zero, belongs to dec
                    symbols.emplace_back("1", "0");
                }
            }

            token = Num;
            return;
        } else if (token == '/') {
            // divide operator
            token = Div;
            symbols.emplace_back("/", "-");
            return;
        } else if (token == '=') {
            // parse '='
            token = Assign;
            symbols.emplace_back("=", "-");
            return;
        } else if (token == '+') {
            // parse '+'
            token = Add;
            symbols.emplace_back("+", "-");
            return;
        } else if (token == '-') {
            // parse '-'
            token = Sub;
            symbols.emplace_back("-", "-");
            return;
        } else if (token == '<') {
            // parse '<'
            token = Lt;
            symbols.emplace_back("<", "-");
            return;
        } else if (token == '>') {
            // parse '>'
            token = Gt;
            symbols.emplace_back(">", "-");
            return;
        } else if (token == '*') {
            token = Mul;
            symbols.emplace_back("*", "-");
            return;
        } else if (token == '(' || token == ')' || token == ';') {
            // directly return the character as token;
            symbols.emplace_back(string(1, token), "-");
            return;
        }
    }
}

void scanner() {
    next();
    while (token > 0) next();
}


int lexer(const char *path) {
    int fd, i;
    line = 1;
    poolsize = 256 * 1024; // arbitrary size

    // read the source file
    if ((fd = open(path, 0)) < 0) {
        printf("could not open(%s)\n", path);
        return -1;
    }
    if (!(src = new char[poolsize])) {
        printf("could not malloc(%d) for source area\n", poolsize);
        return -1;
    }
    // read the source file
    if ((i = read(fd, src, poolsize - 1)) <= 0) {
        printf("read() returned %d\n", i);
        return -1;
    }
    src[i] = 0; // add EOF character
    close(fd);

    scanner();
    return 0;
}

#endif
