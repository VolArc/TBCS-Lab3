#include <cmath>
#include <iostream>
#include <string>
#include <map>
#include <stdexcept>
#include <limits>
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

        Node(Type t, const string& val, Node* l = nullptr, Node* m = nullptr, Node* r = nullptr)
            : type(t), strValue(val), left(l), middle(m), right(r), numValue(0.0) {}

        Node(double num, Node* l = nullptr, Node* r = nullptr)
            : type(Type::Number), numValue(num), left(l), right(r), middle(nullptr) {}

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

    double evaluate() {
        variables.clear();
        return evaluateNode(root);
    }

    void clearVariables() {
        variables.clear();
    }

private:
    Node* root;
    string input;
    int pos;
    map<string, double> variables;

    static bool IsDigit(char c) { return c >= '0' && c <= '9'; }
    static bool IsLetter(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }

    Node* Expression() {
        if (pos < input.size() && input[pos] == '#') {
            return If();
        }

        Node* left = nullptr;
        if (pos < input.size() && input[pos] == '-') {
            string op(1, input[pos]);
            pos++;
            left = new Node(Node::Type::Operator, op, nullptr, nullptr, Addend());
        } else {
            left = Addend();
        }

        while (pos < input.size() && (input[pos] == '+' || input[pos] == '-')) {
            string op(1, input[pos]);
            pos++;
            Node* right = Addend();
            left = new Node(Node::Type::Operator, op, left, nullptr, right);
        }
        return left;
    }

    Node* Addend() {
        Node* left = Factor();
        while (pos < input.size()) {
            if (input[pos] == '*') {
                string op(1, input[pos]);
                pos++;
                left = new Node(Node::Type::Operator, op, left, nullptr, Factor());
            } else if (input[pos] == '/') {
                string op(1, input[pos]);
                pos++;
                if (pos < input.size() && input[pos] == '/') {
                    op = "//";
                    pos++;
                }
                left = new Node(Node::Type::Operator, op, left, nullptr, Factor());
            } else if (input[pos] == '%') {
                string op(1, input[pos]);
                pos++;
                left = new Node(Node::Type::Operator, op, left, nullptr, Factor());
            } else break;
        }
        return left;
    }

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

    Node* Variable() {
        string var;
        while (pos < input.size() && (IsLetter(input[pos]) || IsDigit(input[pos]))) {
            var += input[pos++];
        }
        return new Node(var);
    }

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

        Node* relNode = new Node(Node::Type::Operator, op, leftExpr, nullptr, rightExpr);
        return negation ? new Node(Node::Type::Operator, "!", nullptr, relNode) : relNode;
    }

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
            left = new Node(Node::Type::Operator, "&", left, nullptr, right);
        }
        return left;
    }

    Node* Condition() {
        Node* left = LogicAddend();
        while (pos < input.size() && input[pos] == '|') {
            pos++;
            Node* right = LogicAddend();
            left = new Node(Node::Type::Operator, "|", left, nullptr, right);
        }
        return left;
    }

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

    double evaluateNode(Node* node) {
        if (!node) {
            throw runtime_error("Попытка вычислить пустой узел");
        }

        switch (node->type) {
            case Node::Type::Number:
                return node->numValue;
            case Node::Type::Variable: {
                auto it = variables.find(node->strValue);
                if (it != variables.end()) {
                    return it->second;
                } else {
                    double value;
                    while (true) {
                        cout << "Введите значение для переменной " << node->strValue << ": ";
                        if (cin >> value) {
                            variables[node->strValue] = value;
                            break;
                        } else {
                            cout << "Ошибка ввода. Пожалуйста, введите число." << endl;
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        }
                    }
                    return value;
                }
            }
            case Node::Type::Operator: {
                double left = node->left ? evaluateNode(node->left) : 0.0;
                double right = node->right ? evaluateNode(node->right) : 0.0;
                string op = node->strValue;

                if (op == "+") {
                    double res = left + right;
                    checkOverflow(res, "сложении");
                    cout << "Выполняется сложение: " << left << " + " << right << " = " << res << endl;
                    return res;
                } else if (op == "-") {
                    if (!node->left) {
                        double res = -right;
                        cout << "Выполняется унарный минус: -" << right << " = " << res << endl;
                        return res;
                    } else {
                        double res = left - right;
                        checkOverflow(res, "вычитании");
                        cout << "Выполняется вычитание: " << left << " - " << right << " = " << res << endl;
                        return res;
                    }
                } else if (op == "*") {
                    double res = left * right;
                    checkOverflow(res, "умножении");
                    cout << "Выполняется умножение: " << left << " * " << right << " = " << res << endl;
                    return res;
                } else if (op == "/") {
                    if (right == 0) throw runtime_error("Деление на ноль");
                    double res = left / right;
                    cout << "Выполняется деление: " << left << " / " << right << " = " << res << endl;
                    return res;
                } else if (op == "//") {
                    if (right == 0) throw runtime_error("Деление на ноль при целочисленном делении");
                    double res = floor(left / right);
                    cout << "Выполняется целочисленное деление: " << left << " // " << right << " = " << res << endl;
                    return res;
                } else if (op == "%") {
                    if (right == 0) throw runtime_error("Деление на ноль при вычислении остатка");
                    double res = fmod(left, right);
                    cout << "Выполняется остаток от деления: " << left << " % " << right << " = " << res << endl;
                    return res;
                } else if (op == "!") {
                    double value = evaluateNode(node->middle);
                    double res = (value == 0) ? 1.0 : 0.0;
                    cout << "Выполняется логическое отрицание: !" << value << " = " << res << endl;
                    return res;
                } else if (op == "&") {
                    double res = (left != 0 && right != 0) ? 1.0 : 0.0;
                    cout << "Выполняется логическое И: " << left << " & " << right << " = " << res << endl;
                    return res;
                } else if (op == "|") {
                    double res = (left != 0 || right != 0) ? 1.0 : 0.0;
                    cout << "Выполняется логическое ИЛИ: " << left << " | " << right << " = " << res << endl;
                    return res;
                } else if (op == "<") {
                    double res = (left < right) ? 1.0 : 0.0;
                    cout << "Сравнение: " << left << " < " << right << " = " << res << endl;
                    return res;
                } else if (op == ">") {
                    double res = (left > right) ? 1.0 : 0.0;
                    cout << "Сравнение: " << left << " > " << right << " = " << res << endl;
                    return res;
                } else if (op == "=") {
                    double res = (left == right) ? 1.0 : 0.0;
                    cout << "Сравнение: " << left << " == " << right << " = " << res << endl;
                    return res;
                } else {
                    throw runtime_error("Неизвестный оператор: " + op);
                }
            }
            case Node::Type::Conditional: {
                double condition = evaluateNode(node->middle);
                cout << "Условие IF: " << condition << endl;
                if (condition != 0) {
                    cout << "Выполняется ветка THEN" << endl;
                    return evaluateNode(node->left);
                } else {
                    cout << "Выполняется ветка ELSE" << endl;
                    return evaluateNode(node->right);
                }
            }
            default:
                throw runtime_error("Неизвестный тип узла");
        }
    }

    void checkOverflow(double value, const string& operation) {
        if (isinf(value)) {
            throw runtime_error("Переполнение при " + operation);
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

        char choice;
        do {
            try {
                double result = tree.evaluate();
                cout << "Итоговый результат: " << result << endl;
            } catch (const exception& e) {
                cerr << "Ошибка: " << e.what() << endl;
            }

            cout << "Хотите пересчитать с новыми значениями переменных? (y/n): ";
            cin >> choice;
            cin.ignore();
            if (choice == 'y' || choice == 'Y') {
                tree.clearVariables();
            }
        } while (choice == 'y' || choice == 'Y');

    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }

    return 0;
}