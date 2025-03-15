#include <iostream>
#include <string>
#include <stdexcept>
using namespace std;

class Tree {
public:
    class Node {
    public:
        enum class Type { Operator, Number, Variable, Conditional };

        Type type;
        string strValue;
        double numValue;
        Node* left;
        Node* middle;
        Node* right;

        // Конструктор для операторов и условных выражений
        Node(Type t, const string& val, Node* l = nullptr, Node* m = nullptr, Node* r = nullptr)
            : type(t), strValue(val), left(l), middle(m), right(r), numValue(0.0) {}

        // Конструктор для чисел
        Node(double num, Node* l = nullptr, Node* r = nullptr)
            : type(Type::Number), numValue(num), left(l), right(r), middle(nullptr) {}

        // Конструктор для переменных
        Node(const string& var, Node* l = nullptr, Node* r = nullptr)
            : type(Type::Variable), strValue(var), left(l), right(r), middle(nullptr) {}
    };

    explicit Tree(const string& inputStr) : root(nullptr), input(inputStr), pos(0) {
        root = Expression();
        if (pos != input.length()) {
            throw runtime_error("Ошибка: неожиданный символ на позиции " + to_string(pos));
        }
    }

    ~Tree() { deleteTree(root); }

    void printReversedPolishNotation() const {
        printRPN(root);
        cout << endl;
    }

private:
    Node* root;
    string input;
    int pos;

    static bool IsDigit(char c) { return c >= '0' && c <= '9'; }
    static bool IsLetter(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }

    // <EXPRESSION> ::= ( [‘-‘] <ADDEND> {(‘+’ | ‘-‘) <ADDEND>} ) | <IF>
    Node* Expression() {
        if (pos < input.size() && input[pos] == '#') {
            return If();
        }

        Node* left = nullptr;
        if (pos < input.size() && input[pos] == '-') {
            string op(1, input[pos]);
            pos++;
            left = new Node(Node::Type::Operator, op, nullptr, Addend());
        } else {
            left = Addend();
        }

        while (pos < input.size() && (input[pos] == '+' || input[pos] == '-')) {
            string op(1, input[pos]);
            pos++;
            Node* right = Addend();
            left = new Node(Node::Type::Operator, op, left, right);
        }
        return left;
    }

    // <ADDEND> ::= <FACTOR> {(‘*’ | ‘/’ | ‘//’ | ‘%’) <FACTOR>}
    Node* Addend() {
        Node* left = Factor();
        while (pos < input.size()) {
            if (input[pos] == '*') {
                string op(1, input[pos]);
                pos++;
                left = new Node(Node::Type::Operator, op, left, Factor());
            } else if (input[pos] == '/') {
                string op(1, input[pos]);
                pos++;
                if (pos < input.size() && input[pos] == '/') {
                    op = "//";
                    pos++;
                }
                left = new Node(Node::Type::Operator, op, left, Factor());
            } else if (input[pos] == '%') {
                string op(1, input[pos]);
                pos++;
                left = new Node(Node::Type::Operator, op, left, Factor());
            } else break;
        }
        return left;
    }

    // <FACTOR> ::= <NUMBER> | <VARIABLE> | ‘(‘ <EXPRESSION> ‘)’
    Node* Factor() {
        if (pos >= input.size()) {
            throw runtime_error("Ошибка: ожидался множитель на позиции " + to_string(pos));
        }

        if (IsDigit(input[pos]))
            return Number();
        if (IsLetter(input[pos]))
            return Variable();
        if (input[pos] == '(') {
            pos++;
            Node* node = Expression();
            if (pos >= input.size() || input[pos] != ')') {
                throw runtime_error("Ошибка: ожидалась ')' на позиции " + to_string(pos));
            }
            pos++;
            return node;
        }
        throw runtime_error("Ошибка: неверный символ в множителе на позиции " + to_string(pos));
    }

    // <NUMBER> ::= {<DIGIT>} [‘.’ {<DIGIT>}]
    Node* Number() {
        string num;
        while (pos < input.size() && IsDigit(input[pos])) {
            num += input[pos++];
        }

        if (pos < input.size() && input[pos] == '.') {
            num += input[pos++];
            if (!IsDigit(input[pos])) {
                throw runtime_error("Ошибка: ожидалась цифра после точки на позиции " + to_string(pos));
            }
            while (pos < input.size() && IsDigit(input[pos])) {
                num += input[pos++];
            }
        }
        return new Node(stod(num));
    }

    // <VARIABLE> ::= <LETTER> { <LETTER> | <DIGIT> }
    Node* Variable() {
        string var;
        while (pos < input.size() && (IsLetter(input[pos]) || IsDigit(input[pos]))) {
            var += input[pos++];
        }
        return new Node(var);
    }

    // <RELATION> ::= [‘!’] ( <EXPRESSION> (‘<’ | ‘=’ | ‘>’) <EXPRESSION> )
    Node* Relation() {
        bool negation = false;
        if (pos < input.size() && input[pos] == '!') {
            negation = true;
            pos++;
        }

        Node* leftExpr = Expression();
        if (pos >= input.size() || (input[pos] != '<' && input[pos] != '=' && input[pos] != '>')) {
            throw runtime_error("Ошибка: ожидался оператор сравнения на позиции " + to_string(pos));
        }
        string op(1, input[pos++]);
        Node* rightExpr = Expression();

        Node* relNode = new Node(Node::Type::Operator, op, leftExpr, rightExpr);
        return negation ? new Node(Node::Type::Operator, "!", nullptr, relNode) : relNode;
    }

    // <LOGIC_ADDEND> ::= <RELATION> {‘&’ (<RELATION> | ‘(‘ <CONDITION> ‘)’ )}
    Node* LogicAddend() {
        Node* left = Relation();
        while (pos < input.size() && input[pos] == '&') {
            pos++;
            Node* right;
            if (pos < input.size() && input[pos] == '(') {
                pos++;
                right = Condition();
                if (pos >= input.size() || input[pos] != ')') {
                    throw runtime_error("Ошибка: ожидалась ')' в логическом слагаемом на позиции " + to_string(pos));
                }
                pos++;
            } else {
                right = Relation();
            }
            left = new Node(Node::Type::Operator, "&", left, right);
        }
        return left;
    }

    // <CONDITION> ::= <LOGIC_ADDEND> {‘|’ <LOGIC_ADDEND>}
    Node* Condition() {
        Node* left = LogicAddend();
        while (pos < input.size() && input[pos] == '|') {
            pos++;
            Node* right = LogicAddend();
            left = new Node(Node::Type::Operator, "|", left, right);
        }
        return left;
    }

    // <IF> ::= ‘#’ <CONDITION> ‘?’ <EXPRESSION> ‘:’ <EXPRESSION>
    Node* If() {
        pos++;
        Node* cond = Condition();
        if (pos >= input.size() || input[pos] != '?') {
            throw runtime_error("Ошибка: ожидался '?' в IF на позиции " + to_string(pos));
        }
        pos++;
        Node* trueExpr = Expression();
        if (pos >= input.size() || input[pos] != ':') {
            throw runtime_error("Ошибка: ожидался ':' в IF на позиции " + to_string(pos));
        }
        pos++;
        Node* falseExpr = Expression();
        return new Node(Node::Type::Conditional, "IF", cond, trueExpr, falseExpr);
    }

    void deleteTree(Node* node) {
        if (node) {
            deleteTree(node->left);
            deleteTree(node->middle);
            deleteTree(node->right);
            delete node;
        }
    }

    void printRPN(const Node* node) const {
        if (node) {
            printRPN(node->left);
            printRPN(node->middle);
            printRPN(node->right);
            if (node->type == Node::Type::Number) {
                cout << node->numValue << " ";
            } else {
                cout << node->strValue << " ";
            }
        }
    }
};

int main() {
    cout << "Введите выражение: ";
    string input;
    getline(cin, input);

    try {
        Tree tree(input);
        cout << "\nОбратная польская запись: ";
        tree.printReversedPolishNotation();
    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }

    return 0;
}