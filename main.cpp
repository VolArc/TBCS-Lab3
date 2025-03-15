#include <iostream>
#include <stack>
#include <string>
using std::cout, std::cerr, std::cin, std::string, std::endl, std::stack;

/* class Tree {
    class Node {
    public:
        Node* pLeft;
        Node* pRight;
        long long inf;

        explicit Node (const long long inf, Node* pLeft = nullptr, Node* pRight = nullptr) {
            this->inf=inf;
            this->pLeft=pLeft;
            this->pRight=pRight;
        }
    };

    Node* pRoot = nullptr;

    void InsertOperator (const char operand) {
        pRoot = new Node(operand, pRoot);
    }

    void InsertOperand (const char operation) {
        if (pRoot == nullptr)
            pRoot = new Node (operation);
        else
            pRoot->pRight = new Node (operation);
    }

public:
    Tree();
    ~Tree();
};

Tree::Tree() {
    pRoot = nullptr;
}

Tree::~Tree() {
    if (pRoot == nullptr) return;

    std::stack<Node*> nodeStack;
    nodeStack.push(pRoot);

    while (!nodeStack.empty()) {
        const Node* current = nodeStack.top();
        nodeStack.pop();

        if (current->pLeft) nodeStack.push(current->pLeft);
        if (current->pRight) nodeStack.push(current->pRight);

        delete current;
    }

    pRoot = nullptr;
}*/

bool IsDigit (const char symbol) {
    return symbol >= '0' && symbol <= '9';
}

bool IsLetter (const char symbol) {
    return symbol >= 'A' && symbol <= 'Z' || symbol >= 'a' && symbol <= 'z';
}

string input;
int pos = 0;

// <DIGIT> ::= '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'
void Digit() {
    if (pos < input.size() && IsDigit(input[pos]))
        cout << input[pos++];
    else {
        cerr << "Ошибка: ожидалась цифра на позиции " << pos << endl;
        exit(1);
    }
}

// <LETTER> ::= 'A' | 'B' | … | 'Z' | 'a' | 'b' | … | 'z'
void Letter() {
    if (pos < input.size() && IsLetter(input[pos]))
        cout << input[pos++];
    else {
        cerr << "Ошибка: ожидалась буква на позиции " << pos << endl;
        exit(1);
    }
}

// <VARIABLE> ::= <LETTER> { <LETTER> | <DI0> }
void Variable() {
    Letter();
    // После первой буквы могут идти буквы или цифры (DI0)
    while (pos < input.size() && (IsLetter(input[pos]) || IsDigit(input[pos]))) {
        // Для цифры вызываем соответствующий разбор: если это '0', то DI0,
        // если не '0', то DE0 (который сам выведет символ).
        if (IsLetter(input[pos])) Letter();
        else Digit();
    }
}

// <NUMBER> ::= ("0" | <DE0> { <DI0> }) [ '.' {<DI0>} <DE0> ]
void Number() {
    if (pos < input.size() && IsDigit(input[pos])) {
        Digit();
        while (pos < input.size() && IsDigit(input[pos]))
            Digit();
    } else {
        cerr << "Ошибка: ожидалось число на позиции " << pos << endl;
        exit(1);
    }

    // Опциональная дробная часть
    if (pos < input.size() && input[pos] == '.') {
        cout << input[pos++];
        // Должна быть хотя бы одна цифра после точки.
        if (pos < input.size() && IsDigit(input[pos])) {
            // Собираем цифры дробной части.
            Digit();
            while (pos < input.size() && IsDigit(input[pos]))
                Digit();
        } else {
            cerr << "Ошибка: ожидалась цифра после точки в числе на позиции " << pos << endl;
            exit(1);
        }
    }
}

// Предварительные объявления для взаимных вызовов
void If();
void Expression();
void Condition();

// <FACTOR> ::= <NUMBER> | <VARIABLE> | '(' <EXPRESSION> ')'
void Factor() {
    if (pos < input.size()) {
        if (IsDigit(input[pos])) {
            Number();
        } else if (IsLetter(input[pos])) {
            Variable();
        } else if (input[pos] == '(') {
            cout << input[pos++];
            Expression();
            if (pos < input.size() && input[pos] == ')')
                cout << input[pos++];
            else {
                cerr << "Ошибка: ожидалась ')' на позиции " << pos << endl;
                exit(1);
            }
        } else {
            cerr << "Ошибка: неверный символ в множителе на позиции " << pos
                 << " (" << input[pos] << ")" << endl;
            exit(1);
        }
    }

    else {
        cerr << "Ошибка: ожидался множитель на позиции " << pos << endl;
        exit(1);
    }
}

// <ADDEND> ::= <FACTOR> {( '*' | '/' | '//' | '%' ) <FACTOR>}
void Addend() {
    Factor();
    while (pos < input.size()) {
        if (input[pos] == '*') {
            cout << input[pos++];
        } else if (input[pos] == '/') {
            // Проверяем, не начинается ли оператор с двойного слэша.
            cout << input[pos++];
            if (pos < input.size() && input[pos] == '/')
                cout << input[pos++];
        } else if (input[pos] == '%')
            cout << input[pos++];
        else
            break;

        Factor();
    }
}

// <EXPRESSION> ::= ( [ '-' ] <ADDEND> {( '+' | '-' ) <ADDEND>} ) | <IF>
void Expression() {
    // Если начинается с '#' - это конструкция IF.
    if (pos < input.size() && input[pos] == '#') {
        If();
        return;
    }
    // Иначе арифметическое выражение
    if (pos < input.size() && input[pos] == '-')
        cout << input[pos++]; // Унарный минус

    Addend();
    while (pos < input.size() && (input[pos] == '+' || input[pos] == '-')) {
        cout << input[pos++];
        Addend();
    }
}

// <RELATION> ::= [ '!' ] (<EXPRESSION> ('<' | '=' | '>') <EXPRESSION>)
void Relation() {
    if (pos < input.size() && input[pos] == '!')
        cout << input[pos++];

    Expression();

    if (pos < input.size() && (input[pos] == '<' || input[pos] == '=' || input[pos] == '>'))
        cout << input[pos++];
    else {
        cerr << "Ошибка: ожидался оператор сравнения (<, = или >) на позиции " << pos << endl;
        exit(1);
    }
    Expression();
}

// <LOGIC_ADDEND> ::= <RELATION> { '&' (<RELATION> | '(' <Condition> ')' )}
void LogicAddend() {
    Relation();
    while (pos < input.size() && input[pos] == '&') {
        cout << input[pos++];
        if (pos < input.size() && input[pos] == '(') {
            Condition();
            if (pos < input.size() && input[pos] == ')')
                cout << input[pos++];
            else {
                cerr << "Ошибка: ожидалась ')' в логическом слагаемом на позиции " << pos << endl;
                exit(1);
            }
        }
        else Relation();
    }
}

// <CONDITION> ::= <LOGIC_ADDEND> { '|' <LOGIC_ADDEND> }
void Condition() {
    LogicAddend();
    while (pos < input.size() && input[pos] == '|') {
        cout << input[pos++];
        LogicAddend();
    }
}

// <IF> ::= '#' <CONDITION> '?' <EXPRESSION> ':' <EXPRESSION>
void If() {
    if (pos < input.size() && input[pos] == '#')
        cout << input[pos++];
    else {
        cerr << "Ошибка: ожидался символ '#' перед условием " << pos << endl;
        exit(1);
    }
    Condition();
    if (pos < input.size() && input[pos] == '?')
        cout << input[pos++];
    else {
        cerr << "Ошибка: ожидался символ '?' в IF на позиции " << pos << endl;
        exit(1);
    }
    Expression();
    if (pos < input.size() && input[pos] == ':')
        cout << input[pos++];
    else {
        cerr << "Ошибка: ожидался символ ':' в IF на позиции " << pos << endl;
        exit(1);
    }
    if (pos == input.size()) {
        cerr << "Ошибка: нет блока ИНАЧЕ"<< endl;
        exit(1);
    }
    Expression();
}

int main() {
    cout << "Введите выражение: ";
    getline(cin, input);

    Expression();

    if (pos != input.size()) {
        cerr << "\nОшибка: неожиданный символ: : " << input[pos] << endl;
        return 1;
    }
    return 0;
}