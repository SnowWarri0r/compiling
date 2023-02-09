//
// Created by admin on 2023/2/7.
//

#ifndef GRAMMAR_GRAMMAR_H
#define GRAMMAR_GRAMMAR_H

#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <stack>

using namespace std;

class grammar {
    const string upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    unordered_set<char> vn, vt;
    char start{};
    unordered_map<char, unordered_set<string>> p;
    unordered_map<char, unordered_set<string>> first, follow;
    unordered_map<char, unordered_map<string, unordered_set<string>>> first_extend;
    unordered_map<char, unordered_map<string, string>> table;
    vector<unordered_map<char, string>> action;
    vector<unordered_map<char, int>> goto_table;
    vector<string> items;

    bool is_terminal(char c) {
        return vt.find(c) != vt.end();
    }

    bool is_non_terminal(char c) {
        return vn.find(c) != vn.end();
    }

public:
    grammar() = default;

    // S->ab
    // S->Cd
    // C->ef
    // R->a|b|c
    void read(const char *filename) {
        string line;
        ifstream fileStream(filename, ios::in);
        if (fileStream.fail()) exit(-1);
        while (!fileStream.eof()) {
            getline(fileStream, line);
            // non-terminated symbol is upper case
            char nt = line[0];
            vn.insert(nt);
            int i = 3, j = 3;
            while (line[i] != ';') {
                if (isupper(line[i]))
                    vn.insert(line[i]);
                else if (line[i] == '|') {
                    p[nt].insert(line.substr(j, i - j));
                    j = i + 1;
                } else
                    vt.insert(line[i]);

                i++;
            }
            p[nt].insert(line.substr(j, i - j));
        }
        fileStream.close();
        cout << "Please input the start symbol: ";
        cin >> start;
    }

    // G[L]：
    // 0:L->E
    // 1:E->T
    // 2:E->E+T
    // 3:T->F
    // 4:T->T*F
    // 5:F->(E)
    // 6:F->i
    void init_action_and_goto() {
        action = vector<unordered_map<char, string>>(12);
        goto_table = vector<unordered_map<char, int>>(12);
        ifstream fileStream("l.txt", ios::in);
        if (fileStream.fail()) exit(-1);
        string line;
        while (!fileStream.eof()) {
            getline(fileStream, line);
            // L L+R
            // R R*F
            // read right part
            int i = 0;
            while (line[i] != ' ') i++;
            string right = line.substr(i + 1);
            items.emplace_back(right);
        }
        fileStream.close();
        action[0] = {
                {'i', "s5"},
                {'(', "s4"}
        };
        goto_table[0] = {
                {'E', 1},
                {'T', 2},
                {'F', 3}
        };
        action[1] = {
                {'+', "s6"},
                {'#', "acc"}
        };
        action[2] = {
                {'+', "r2"},
                {'*', "s7"},
                {')', "r2"},
                {'#', "r2"}
        };
        action[3] = {
                {'+', "r4"},
                {'*', "r4"},
                {')', "r4"},
                {'#', "r4"}
        };
        action[4] = {
                {'i', "s5"},
                {'(', "s4"}
        };
        goto_table[4] = {
                {'E', 8},
                {'T', 2},
                {'F', 3}
        };
        action[5] = {
                {'+', "r6"},
                {'*', "r6"},
                {')', "r6"},
                {'#', "r6"}
        };
        action[6] = {
                {'i', "s5"},
                {'(', "s4"}
        };
        goto_table[6] = {
                {'T', 9},
                {'F', 3}
        };
        action[7] = {
                {'i', "s5"},
                {'(', "s4"}
        };
        goto_table[7] = {
                {'F', 10}
        };
        action[8] = {
                {'+', "s6"},
                {')', "s11"}
        };
        action[9] = {
                {'+', "r1"},
                {'*', "s7"},
                {')', "r1"},
                {'#', "r1"}
        };
        action[10] = {
                {'+', "r3"},
                {'*', "r3"},
                {')', "r3"},
                {'#', "r3"}
        };
        action[11] = {
                {'+', "r5"},
                {'*', "r5"},
                {')', "r5"},
                {'#', "r5"}
        };
    }

    void LR_analyze(const string &exp) {
        stack<int> state;
        stack<char> symbol;
        symbol.emplace('#');
        state.emplace(0);
        int i = 0;
        while (true) {
            int top = state.top();
            char a = exp[i];
            if (action[top].find(a) == action[top].end()) {
                cout << "error" << endl;
                return;
            }
            string action_str = action[top][a];
            if (action_str[0] == 's') {
                symbol.emplace(a);
                state.push(stoi(action_str.substr(1)));
                i++;
            } else if (action_str[0] == 'r') {
                int rule = stoi(action_str.substr(1));
                string right = items[rule];
                for (int j = 0; j < right.size(); j++) {
                    symbol.pop();
                    state.pop();
                }
                int x = state.top();
                if (rule == 1 || rule == 2) {
                    symbol.emplace('E');
                    state.push(goto_table[x]['E']);
                } else if (rule == 3 || rule == 4) {
                    symbol.emplace('T');
                    state.push(goto_table[x]['T']);
                } else if (rule == 5 || rule == 6) {
                    symbol.emplace('F');
                    state.push(goto_table[x]['F']);
                }
            } else if (action_str == "acc") {
                cout << "accept" << endl;
                return;
            }
        }
    }

    void remove_left_recursion() {
        vector<char> v(vn.begin(), vn.end());
        sort(v.begin(), v.end(), greater<>());
        for (int i = 0; i < v.size(); ++i) {
            char nt1 = v[i];
            for (int j = 0; j < i; ++j) {
                char nt2 = v[j];
                // 将 pj 代入 pi
                vector<string> pi_right(p[nt1].begin(), p[nt1].end());
                for (auto &pi: pi_right) {
                    // 可代入pi的开头如pi->pja的产生式
                    if (pi[0] == nt2) {
                        for (auto &pj: p[nt2]) {
                            // 代入
                            p[nt1].insert(pj + pi.substr(1));
                        }
                        // 删除原来的产生式
                        p[nt1].erase(pi);
                    }
                }
            }
            // 删除直接左递归
            // A→Aα1| Aα2 |… |Aαm|β1|β2 |…|βn
            // A→ β1A'| β2 A' |…| βnA'
            // A'→ α1A' | α2A' |…| αmA' |ε
            // p1 对应 alpha
            // p2 对应 beta
            unordered_set<string> p1, p2;
            for (auto &pi: p[nt1]) {
                if (pi[0] == nt1) p1.insert(pi.substr(1));
                else p2.insert(pi);
            }
            // 没有直接左递归，跳过
            if (p1.empty()) continue;
            // 有直接左递归
            // 生成新的非终结符
            int index = nt1 + 1 - 'A';
            char nt3 = upper[index];
            while (is_non_terminal(nt3)) {
                index = (index + 1) % 26;
                nt3 = upper[index];
            }

            vn.insert(nt3);
            // p3 对应 A'
            // p4 对应 A
            unordered_set<string> p3, p4;
            for (auto &pi: p1) p3.insert(pi + nt3);
            for (auto &pi: p2) {
                if (pi != "ε") p4.insert(pi + nt3);
                else p4.insert(string(1, nt3));
            }
            p3.insert("ε");
            p[nt1] = p4;
            p[nt3] = p3;
        }
    }

    void remove_left_gene() {
        for (const auto &pa: p) {
            char nt = pa.first;
            // A —＞ δβ1|δβ2|…|δβn|γ1|γ2|…|γm
            // 匹配最大前缀，只要有一个符合就可以
            vector<string> p_right(pa.second.begin(), pa.second.end());
            // 从大到小排序
            sort(p_right.begin(), p_right.end(), [](const string &a, const string &b) {
                return a.size() > b.size();
            });
            int max_public_len = 0;
            string max_public_str;
            for (int i = 0; i < p_right.size(); ++i) {
                for (int j = i + 1; j < p_right.size(); ++j) {
                    string &a = p_right[i], &b = p_right[j];
                    int len = 0;
                    while (len < a.size() && len < b.size() && a[len] == b[len]) len++;
                    if (len > max_public_len) {
                        max_public_len = len;
                        max_public_str = a.substr(0, len);
                    }
                }
            }
            if (max_public_len == 0) continue;
            // 有最大公共前缀
            // 生成新的非终结符
            int index = nt + 1 - 'A';
            char nt2 = upper[index];
            while (is_non_terminal(nt2)) {
                index = (index + 1) % 26;
                nt2 = upper[index];
            }
            vn.insert(nt2);
            // A —＞ δβ1|δβ2|…|δβn|γ1|γ2|…|γm
            // A -> δA' | γ1 | γ2 | … | γm
            // A' -> β1 | β2 | … | βn
            unordered_set<string> p1, p2;
            for (auto &pi: p_right) {
                if (pi.length() >= max_public_len && pi.substr(0, max_public_len) == max_public_str) {
                    if (pi.length() > max_public_len) p1.insert(pi.substr(max_public_len));
                    else p1.insert("ε");
                } else {
                    p2.insert(pi);
                }
            }
            p2.insert(max_public_str + nt2);
            p[nt] = p2;
            p[nt2] = p1;
        }
    }

    // 求解FIRST集
    void get_FIRST() {
        for (const auto &t: vt) first[t].insert(string(1, t));
        unordered_map<char, bool> visited;
        for (const auto &t: vn) {
            visited[t] = false;
            first_extend[t] = unordered_map<string, unordered_set<string>>();
        }
        for (const auto &pa: p) {
            char nt = pa.first;
            get_FIRST_next(visited, nt);
        }
    }

    // 求解FIRST集合的递归函数
    // visited 判断是否已经求解过 FIRST 集合
    // nt 当前非终结符
    unordered_set<string> &get_FIRST_next(unordered_map<char, bool> &visited, char nt) {
        if (visited[nt]) return first[nt];
        visited[nt] = true;
        vector<string> p_right(p[nt].begin(), p[nt].end());
        // 从大到小排序
        sort(p_right.begin(), p_right.end(), [](const string &a, const string &b) {
            return a.size() > b.size();
        });
        for (const auto &pi: p_right) {
            if (is_terminal(pi[0])) {
                first_extend[nt][string(1, pi[0])].insert(pi);
                first[nt].insert(string(1, pi[0]));
            } else if (pi == "ε") {
                first_extend[nt][pi].insert(pi);
                first[nt].insert(pi);
            } else {
                // 递归求解
                for (auto &symbol: pi) {
                    if (is_terminal(symbol)) {
                        first_extend[nt][string(1, symbol)].insert(pi);
                        first[nt].insert(string(1, symbol));
                        break;
                    } else {
                        unordered_set<string> &f = get_FIRST_next(visited, symbol);
                        first[nt].insert(f.begin(), f.end());
                        for (const auto &e: f) if (e != "ε") first_extend[nt][e].insert(pi);
                        first[nt].erase("ε");
                        if (f.find("ε") == f.end()) break;
                        else if (symbol == pi[pi.size() - 1]) {
                            first_extend[nt]["ε"].insert(pi);
                            first[nt].insert("ε");
                        }
                    }
                }
            }
        }
        return first[nt];
    }

    // 求解FOLLOW集
    void get_FOLLOW() {
        follow[start].insert("#");
        unordered_map<char, bool> visited;
        for (const auto &t: vn) {
            visited[t] = false;
        }
        for (const auto &pa: p) {
            char nt = pa.first;
            get_FOLLOW_next(visited, nt);
        }
    }

    // 求解FOLLOW集合的递归函数
    // visited 判断是否已经求解过 FOLLOW 集合
    // nt 当前非终结符
    unordered_set<string> &get_FOLLOW_next(unordered_map<char, bool> &visited, char nt) {
        if (visited[nt]) return follow[nt];
        visited[nt] = true;
        for (const auto &pa: p) {
            char nt2 = pa.first;
            vector<string> p_right(p[nt2].begin(), p[nt2].end());
            // 从大到小排序
            sort(p_right.begin(), p_right.end(), [](const string &a, const string &b) {
                return a.size() > b.size();
            });
            for (const auto &pi: p_right) {
                if (pi.find(nt) != string::npos) {
                    int index = pi.find(nt);
                    if (index == pi.size() - 1) {
                        unordered_set<string> &f = get_FOLLOW_next(visited, nt2);
                        follow[nt].insert(f.begin(), f.end());
                    } else {
                        if (is_terminal(pi[index + 1])) {
                            follow[nt].insert(string(1, pi[index + 1]));
                        } else {
                            unordered_set<string> &f = first[pi[index + 1]];
                            follow[nt].insert(f.begin(), f.end());
                            follow[nt].erase("ε");
                            if (f.find("ε") != f.end()) {
                                unordered_set<string> &f2 = get_FOLLOW_next(visited, nt2);
                                follow[nt].insert(f2.begin(), f2.end());
                            }
                        }
                    }
                }
            }
        }
        return follow[nt];
    }

    bool judge_LL1() {
        for (const auto &i: first_extend) {
            for (const auto &j: i.second) {
                if (j.second.size() > 1) {
                    cout << "非 LL(1) 文法" << endl;
                    return false;
                }
            }
        }
        for (auto &f: first) {
            if (f.second.find("ε") != f.second.end()) {
                auto &first_set = f.second, &follow_set = follow[f.first];
                // intersection
                unordered_set<string> tmp(first_set);
                for (const auto &e: follow_set) {
                    if (first_set.find(e) != first_set.end()) {
                        cout << "非 LL(1) 文法" << endl;
                        return false;
                    }
                }
            }
        }
        cout << "是LL(1) 文法" << endl;
        return true;
    }

    // 构造 LL(1) 分析表
    void create_LL1_table() {

        for (const auto &i: vn) {
            for (const auto &j: vt) {
                table[i][string(1, j)] = "ERROR";
            }
        }
        for (const auto &i: first_extend) {
            char nt = i.first;
            for (const auto &j: i.second) {
                string t = j.first;
                string right = *j.second.begin();
                if (t != "ε") table[nt][t] = right;
                else {
                    for (const auto &e: follow[nt]) {
                        table[nt][e] = right;
                    }
                }
            }
        }
    }

    // expression 结尾必须是 #
    void LL1_analyze(const string &expression) {
        cout << "LL(1) 分析 " << expression << ": ";
        stack<char> s;
        s.push('#');
        s.push(start);
        int i = 0;
        while (!s.empty()) {
            char top = s.top();
            s.pop();
            if (is_terminal(top)) {
                if (top == expression[i]) {
                    i++;
                } else {
                    cout << "分析失败" << endl;
                    return;
                }
            } else if (top == '#') {
                if (expression[i] == top) {
                    break;
                } else {
                    cout << "分析失败" << endl;
                    return;
                }
            } else {
                string right = table[top][string(1, expression[i])];
                if (right == "ERROR") {
                    cout << "分析失败" << endl;
                    return;
                }
                if (right != "ε") {
                    for (int j = right.size() - 1; j >= 0; j--) {
                        s.push(right[j]);
                    }
                }
            }
        }
        cout << "分析成功" << endl;
    }

    void print_FIRST() {
        for (const auto &i: first) {
            cout << i.first << ": ";
            vector<string> v(i.second.begin(), i.second.end());
            sort(v.begin(), v.end());
            auto n = i.second.size();
            for (auto &j: v) {
                cout << j;
                if (--n) cout << ", ";
            }
            cout << endl;
        }
    }

    void print_FIRST_extend() {
        for (const auto &i: first_extend) {
            cout << i.first << ": " << endl;
            for (const auto &j: i.second) {
                cout << "\t" << j.first << " <- ";
                vector<string> v(j.second.begin(), j.second.end());
                sort(v.begin(), v.end());
                auto n = j.second.size();
                for (auto &k: v) {
                    cout << k;
                    if (--n) cout << ", ";
                }
                cout << endl;
            }
        }
    }

    void print_FOLLOW() {
        for (const auto &i: follow) {
            cout << i.first << ": ";
            vector<string> v(i.second.begin(), i.second.end());
            sort(v.begin(), v.end());
            auto n = i.second.size();
            for (auto &j: v) {
                cout << j;
                if (--n) cout << ", ";
            }
            cout << endl;
        }
    }

    void print_language() {
        for (const auto &i: p) {
            cout << i.first << "->";
            vector<string> v(i.second.begin(), i.second.end());
            sort(v.begin(), v.end());
            auto n = i.second.size();
            for (auto &j: v) {
                cout << j;
                if (--n) cout << "|";
            }
            cout << endl;
        }
    }

    void print_LL1_table() {
        cout << "LL(1) 分析表" << endl;
        cout << left << setw(15) << "\\";
        for (const auto &i: vt) cout << "\t" << left << setw(15) << i;
        cout << endl;
        for (const auto &i: vn) {
            cout << left << setw(15) << string(1, i) + ": ";
            for (const auto &j: vt) {
                cout << "\t" << left << setw(15) << table[i][string(1, j)];
            }
            cout << endl;
        }
    }

};

#endif //GRAMMAR_GRAMMAR_H
