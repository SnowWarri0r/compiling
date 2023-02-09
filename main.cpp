#include "grammar.h"
#include "lexer.h"

int main() {
    grammar g;
    g.read("g.txt");
    cout << "Before removing left recursion:" << endl;
    g.print_language();
    g.remove_left_recursion();
    cout << "After removing left recursion:" << endl;
    g.print_language();
    g.remove_left_gene();
    cout << "After removing left gene:" << endl;
    g.print_language();
    cout << "First:" << endl;
    g.get_FIRST();
    g.print_FIRST_extend();

    g.get_FOLLOW();
    cout << "Follow:" << endl;
    g.print_FOLLOW();
    bool isLL1 = g.judge_LL1();
    if (isLL1) {
        cout << "It is LL1 grammar." << endl;
    } else {
        cout << "It is not LL1 grammar." << endl;
        exit(0);
    }
    g.create_LL1_table();
    g.print_LL1_table();
    lexer("expression.txt");
//    g.init_action_and_goto();
    string expression;
    for (auto &symbol: symbols) {
        if (symbol.first == "0" || symbol.first == "1" || symbol.first == "2" || symbol.first == "3") expression += "i";
        else if (symbol.first == ";") {
            expression += "#";
            cout << "Expression: " << expression << endl;
//            g.LR_analyze(expression);
            g.LL1_analyze(expression);
            expression = "";
        }
        else expression += symbol.first;
    }
    return 0;
}
