#include "genlang/complier/lexer.h"

using namespace GenLang;
namespace GenLang {
    static const char *KEYWORDS[] = {
            "let", "as", "class", "struct", "if", "else", "for", "while", "return", "continue", "native", nullptr
    };
    static const char *TYPENAMES[] = {
            "void", "string", "object", "list", "int", "char", "double", "long", "float", nullptr
    };
    static const char *OPERATORS[] = {
            ":", ";", "+", "-", "*", "/", "%",
            "**", "<<", ">>", "&&", "&", "||",
            "|", "!", "~", "^", "(", ")", "[", "]",
            "==", "=", "<", ">", "!=", "<=", ">=",
            "+=", "-=", "*=", "/=", "**=", "<<=",
            ">>=", "|=", "&=", "^=", "\"", "\'",
            ".", ",", "?", "//", "/*", "{", "}", nullptr
    };
    static const char *SPCEIAL_CHARS = R"(~!@#$%^&*()_+-={}[]|\:;"',.<>?/)";

    bool is_keyword(const string &s) {
        const char **p = KEYWORDS;
        while (*p)
            if (strcmp(*p++, s.get_val().c_str()) == 0)
                return true;
        return false;
    }

    bool is_operator(const string &s) {
        const char **p = OPERATORS;
        while (*p)
            if (strcmp(*p++, s.get_val().c_str()) == 0)
                return true;
        return false;
    }

    std::set<string> makeset(const char *ks[]) {
        std::set<string> set;
        const char **p = ks;
        while (*p)
            set.insert(*p++);
        return set;
    }

    scanner::scanner(FILE *fin) : fin(fin) {
        keywords = makeset(KEYWORDS);
        typenames = makeset(TYPENAMES);
        operators = makeset(OPERATORS);
    }

    int scanner::getc(FILE *f) {
        if (index >= len)
            return EOF;
        int ch = file[index];
        col = cols[index];
        row = rows[index];
        index += 1;
        return ch;
    }

    void scanner::ungetc(int ch, FILE *f) {
        if (ch == EOF) return;
        index -= 1;
        col = cols[index];
        row = rows[index];
    }

    root_ptr<token> scanner::get_token() {
        bool rep;
        do {
            rep = false;

            string str;
            int ch;
            do ch = getc(fin);
            while (isspace(ch));
            int col = this->col, row = this->row;
            if (ch == EOF)
                return NULL;

            if (isdigit(ch)) {
                do {
                    str += ch;
                    ch = getc(fin);
                } while (isdigit(ch));
                if (isalpha(ch) || ch == '_')
                    throw string("unexpected char");
                if (ch != EOF)
                    ungetc(ch, fin);
                return new_object<token>("token", alloc(String, "CONSTANT"),
                                         alloc(Long, atol(str.get_val().c_str())))->set(col, row);
            } else if (isalpha(ch) || ch == '_') {
                do {
                    str += ch;
                    ch = getc(fin);
                } while (isalpha(ch) || isdigit(ch) || ch == '_');
                ungetc(ch, fin);
                if (str == "native") {
                    ch = getc(fin);
                    while (isspace(ch)) ch = getc(fin);
                    if (ch == '{') {
                        int braces = 1;
                        str = "";
                        while ((ch = getc(fin)) && ch != EOF) {
                            if (ch == '{')
                                braces += 1;
                            else if (ch == '}')
                                braces -= 1;
                            if (braces == 0)
                                break;
                            str += ch;
                        }
                    } else {
                        str = "";
                        str += ch;
                        while (ch = getc(fin), ch != EOF && ch != ';')
                            str += ch;
                    }

                    return new_object<token>("token", alloc(String, "NATIVE"), alloc(String, str))->set(col, row);


                } else if (keywords.count(str))
                    return new_object<token>("token", alloc(String, str), alloc(String, str))->set(col, row);
                else if (typenames.count(str))
                    return new_object<token>("token", alloc(String, "TYPENAME"), alloc(String, str))->set(col, row);
                else
                    return new_object<token>("token", alloc(String, "IDENTIFIER"), alloc(String, str))->set(col, row);
            } else if (ch == '"') {
                int last = 0;
                do {
                    ch = getc(fin);
                    if (ch == EOF)
                        throw "Unfinished string";
                    if (ch == '"') {
                        if (last != '\\')
                            break;
                    }
                    str += ch;
                    last = ch;
                } while (true);
                return new_object<token>("token", alloc(String, "STRING"), alloc(String, str))->set(col, row);
            } else if (strchr(SPCEIAL_CHARS, ch)) {
                while (operators.count(str + (string) ch)) {
                    str += ch;
                    ch = getc(fin);
                }
                if (str == "//") {
                    while (ch = getc(fin), ch != EOF && ch != '\n');
                    rep = true;
                    continue;
                }

                if (str == "/*") {
                    int lst = 0;
                    while (ch = getc(fin), ch != EOF) {
                        if (lst == '*' && ch == '/')
                            break;
                        lst = ch;
                    }
                    rep = true;
                    continue;
                }

                if (str != "") {
                    ungetc(ch, fin);
                    return new_object<token>("token", alloc(String, str), alloc(String, str))->set(col, row);
                }
            }
        } while (rep);
        throw string("Unknown token");
    }

    void scanner::readfile() {
        fseek(fin, 0, SEEK_END);
        len = (int) ftell(fin);
        file = new char[len + 1];
        cols = new int[len + 1];
        rows = new int[len + 1];
        index = 0;
        fseek(fin, 0, SEEK_SET);
        fread(file, 1, (size_t) len, fin);
        row = col = 0;
        while (index < len) {
            int ch = file[index];
            rows[index] = row;
            cols[index] = col;
            if (ch == '\n') {
                col = 0, row += 1;
            } else {
                col += 1;
            }
            ++index;
        }
        file[len] = 0;
        index = 0;
    }

    scanner::scanner() : scanner(stdin) {
    }

    scanner::~scanner() {
        delete[] file;
        delete[] rows;
        delete[] cols;
    }
}
