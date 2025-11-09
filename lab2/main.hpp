#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <memory>
#include <stack>
#include <unordered_map>
#include <functional>

using namespace std;

/// Типы лексем, распознаваемые анализатором.
enum class TokenType
{
    WHILE,
    DONE,
    SEMICOLON,
    LPAREN,
    RPAREN,
    IDENTIFIER,
    ROMAN_NUMERAL,
    ASSIGN,
    LESS,
    GREATER,
    EQUAL,
    END
};

/// Представляет одну лексему (токен).
struct Token
{
    TokenType type;
    std::string value;
    Token(TokenType t, std::string v) : type(t), value(std::move(v)) {}
};

/// Проверяет, является ли символ допустимым в римском числе (I, V, X).
bool isRomanChar(char c)
{
    return c == 'I' || c == 'V' || c == 'X';
}

/// Выполняет лексический анализ: разбивает строку на токены.
std::vector<Token> tokenize(const std::string &input)
{
    std::vector<Token> tokens;
    size_t i = 0;

    while (i < input.length())
    {
        char c = input[i];
        if (std::isspace(c))
        {
            i++;
            continue;
        }

        if (input.substr(i, 5) == "while")
        {
            tokens.emplace_back(TokenType::WHILE, "while");
            i += 5;
        }
        else if (input.substr(i, 4) == "done")
        {
            tokens.emplace_back(TokenType::DONE, "done");
            i += 4;
        }
        else if (c == ';')
        {
            tokens.emplace_back(TokenType::SEMICOLON, ";");
            i++;
        }
        else if (c == '(')
        {
            tokens.emplace_back(TokenType::LPAREN, "(");
            i++;
        }
        else if (c == ')')
        {
            tokens.emplace_back(TokenType::RPAREN, ")");
            i++;
        }
        else if (i + 1 < input.length() && input.substr(i, 2) == ":=")
        {
            tokens.emplace_back(TokenType::ASSIGN, ":=");
            i += 2;
        }
        else if (c == '<')
        {
            tokens.emplace_back(TokenType::LESS, "<");
            i++;
        }
        else if (c == '>')
        {
            tokens.emplace_back(TokenType::GREATER, ">");
            i++;
        }
        else if (c == '=')
        {
            tokens.emplace_back(TokenType::EQUAL, "=");
            i++;
        }
        else if (std::isalpha(c))
        {
            size_t start = i;
            while (i < input.length() && std::isalnum(input[i]))
                i++;
            std::string word = input.substr(start, i - start);

            bool isRoman = !word.empty();
            for (char ch : word)
            {
                if (!isRomanChar(ch))
                {
                    isRoman = false;
                    break;
                }
            }

            tokens.emplace_back(isRoman ? TokenType::ROMAN_NUMERAL : TokenType::IDENTIFIER, word);
        }
        else
        {
            std::cerr << "Ошибка лексики: недопустимый символ '" << c << "'\n";
            return {};
        }
    }

    tokens.emplace_back(TokenType::END, "");
    return tokens;
}

/// Узел дерева абстрактного синтаксического разбора (AST).
struct ASTNode
{
    std::string type;
    std::string value;
    std::vector<std::shared_ptr<ASTNode>> children;
    ASTNode(std::string t, std::string v = "") : type(std::move(t)), value(std::move(v)) {}
};

/// LR-анализатор с полной таблицей разбора
class LRParser
{
private:
    std::vector<Token> tokens;
    size_t pos = 0;

    enum ActionType
    {
        SHIFT,
        REDUCE,
        ACCEPT,
        ERROR
    };

    struct TableEntry
    {
        ActionType action;
        int value;
    };

    std::unordered_map<int, std::unordered_map<TokenType, TableEntry>> actionTable;
    std::unordered_map<int, std::unordered_map<std::string, int>> gotoTable;

    // Правила грамматики
    struct Production
    {
        std::string lhs;
        int rhsSize;
        std::function<std::shared_ptr<ASTNode>(std::vector<std::shared_ptr<ASTNode>> &)> builder;
    };

    std::vector<Production> productions;

    void initializeTables()
    {
        // Инициализация правил грамматики
        productions = {
            {"S'", 1, [](std::vector<std::shared_ptr<ASTNode>> &children)
             { return children[0]; }},
            {"Program", 1, [](std::vector<std::shared_ptr<ASTNode>> &children)
             {
                 auto node = std::make_shared<ASTNode>("Program");
                 node->children.push_back(children[0]);
                 return node;
             }},
            {"StatementList", 1, [](std::vector<std::shared_ptr<ASTNode>> &children)
             {
                 auto node = std::make_shared<ASTNode>("StatementList");
                 node->children.push_back(children[0]);
                 return node;
             }},
            {"StatementList", 3, [](std::vector<std::shared_ptr<ASTNode>> &children)
             {
                 auto node = std::make_shared<ASTNode>("StatementList");
                 node->children.push_back(children[0]);
                 node->children.push_back(children[2]);
                 return node;
             }},
            {"Statement", 7, [](std::vector<std::shared_ptr<ASTNode>> &children)
             {
                 auto node = std::make_shared<ASTNode>("WhileLoop");
                 node->children.push_back(children[2]); // Condition
                 node->children.push_back(children[4]); // Body
                 return node;
             }},
            {"Condition", 3, [](std::vector<std::shared_ptr<ASTNode>> &children)
             {
                 auto node = std::make_shared<ASTNode>("Condition");
                 node->children.push_back(children[0]); // Expression
                 node->children.push_back(children[1]); // RelOp
                 node->children.push_back(children[2]); // Expression
                 return node;
             }},
            {"Body", 1, [](std::vector<std::shared_ptr<ASTNode>> &children)
             { return children[0]; }},
            {"Assignment", 3, [](std::vector<std::shared_ptr<ASTNode>> &children)
             {
                 auto node = std::make_shared<ASTNode>("Assignment");
                 node->children.push_back(std::make_shared<ASTNode>("LValue", children[0]->value));
                 node->children.push_back(children[2]);
                 return node;
             }},
            {"Expression", 1, [](std::vector<std::shared_ptr<ASTNode>> &children)
             {
                 return std::make_shared<ASTNode>("Identifier", children[0]->value);
             }},
            {"Expression", 1, [](std::vector<std::shared_ptr<ASTNode>> &children)
             {
                 return std::make_shared<ASTNode>("RomanNumeral", children[0]->value);
             }},
            {"RelOp", 1, [](std::vector<std::shared_ptr<ASTNode>> &children)
             {
                 return std::make_shared<ASTNode>("RelOp", children[0]->value);
             }},
            {"RelOp", 1, [](std::vector<std::shared_ptr<ASTNode>> &children)
             {
                 return std::make_shared<ASTNode>("RelOp", children[0]->value);
             }},
            {"RelOp", 1, [](std::vector<std::shared_ptr<ASTNode>> &children)
             {
                 return std::make_shared<ASTNode>("RelOp", children[0]->value);
             }}};

        // Полная ACTION таблица
        actionTable = {
            {0, {{TokenType::WHILE, {SHIFT, 2}}, {TokenType::IDENTIFIER, {SHIFT, 3}}}},
            {1, {{TokenType::END, {ACCEPT, 0}}}},
            {2, {{TokenType::LPAREN, {SHIFT, 4}}}},
            {3, {{TokenType::ASSIGN, {SHIFT, 5}}}},
            {4, {{TokenType::IDENTIFIER, {SHIFT, 6}}, {TokenType::ROMAN_NUMERAL, {SHIFT, 7}}}},
            {5, {{TokenType::IDENTIFIER, {SHIFT, 6}}, {TokenType::ROMAN_NUMERAL, {SHIFT, 7}}}},
            {6, {{TokenType::LESS, {REDUCE, 8}}, {TokenType::GREATER, {REDUCE, 8}}, {TokenType::EQUAL, {REDUCE, 8}}, {TokenType::RPAREN, {REDUCE, 8}}, {TokenType::ASSIGN, {REDUCE, 8}}, {TokenType::DONE, {REDUCE, 8}}}},
            {7, {{TokenType::LESS, {REDUCE, 9}}, {TokenType::GREATER, {REDUCE, 9}}, {TokenType::EQUAL, {REDUCE, 9}}, {TokenType::RPAREN, {REDUCE, 9}}, {TokenType::ASSIGN, {REDUCE, 9}}, {TokenType::DONE, {REDUCE, 9}}}},
            {8, {{TokenType::LESS, {SHIFT, 9}}, {TokenType::GREATER, {SHIFT, 10}}, {TokenType::EQUAL, {SHIFT, 11}}}},
            {9, {{TokenType::IDENTIFIER, {SHIFT, 6}}, {TokenType::ROMAN_NUMERAL, {SHIFT, 7}}}},
            {10, {{TokenType::IDENTIFIER, {SHIFT, 6}}, {TokenType::ROMAN_NUMERAL, {SHIFT, 7}}}},
            {11, {{TokenType::IDENTIFIER, {SHIFT, 6}}, {TokenType::ROMAN_NUMERAL, {SHIFT, 7}}}},
            {12, {{TokenType::IDENTIFIER, {SHIFT, 6}}, {TokenType::ROMAN_NUMERAL, {SHIFT, 7}}}},
            {13, {{TokenType::IDENTIFIER, {SHIFT, 6}}, {TokenType::ROMAN_NUMERAL, {SHIFT, 7}}}},
            {14, {{TokenType::IDENTIFIER, {SHIFT, 6}}, {TokenType::ROMAN_NUMERAL, {SHIFT, 7}}}},
            {15, {{TokenType::RPAREN, {SHIFT, 16}}}},
            {16, {{TokenType::IDENTIFIER, {SHIFT, 17}}}},
            {17, {{TokenType::ASSIGN, {SHIFT, 18}}}},
            {18, {{TokenType::IDENTIFIER, {SHIFT, 6}}, {TokenType::ROMAN_NUMERAL, {SHIFT, 7}}}},
            {19, {{TokenType::DONE, {SHIFT, 20}}}},
            {20, {{TokenType::SEMICOLON, {SHIFT, 21}}, {TokenType::END, {REDUCE, 4}}, {TokenType::WHILE, {REDUCE, 4}}}},
            {21, {{TokenType::WHILE, {SHIFT, 2}}, {TokenType::IDENTIFIER, {SHIFT, 3}}}},
            {22, {{TokenType::END, {REDUCE, 1}}, {TokenType::WHILE, {REDUCE, 1}}}},
            {23, {{TokenType::RPAREN, {REDUCE, 5}}}},
            {24, {{TokenType::RPAREN, {REDUCE, 5}}}},
            {25, {{TokenType::RPAREN, {REDUCE, 5}}}},
            {26, {{TokenType::DONE, {REDUCE, 7}}, {TokenType::SEMICOLON, {REDUCE, 7}}, {TokenType::END, {REDUCE, 7}}}},
            {27, {{TokenType::DONE, {REDUCE, 6}}}},
            {28, {{TokenType::END, {REDUCE, 2}}, {TokenType::WHILE, {REDUCE, 2}}, {TokenType::SEMICOLON, {REDUCE, 2}}}},
            {29, {{TokenType::END, {REDUCE, 3}}, {TokenType::WHILE, {REDUCE, 3}}}}};

        // Полная GOTO таблица
        gotoTable = {
            {0, {{"Program", 1}, {"StatementList", 28}, {"Statement", 20}}},
            {4, {{"Condition", 15}, {"Expression", 8}}},
            {5, {{"Expression", 26}}},
            {9, {{"Expression", 23}}},
            {10, {{"Expression", 24}}},
            {11, {{"Expression", 25}}},
            {16, {{"Body", 19}, {"Assignment", 27}}},
            {18, {{"Expression", 26}}},
            {21, {{"StatementList", 29}, {"Statement", 20}}},
            {28, {{"Program", 1}}}};
    }

    TokenType getCurrentTokenType() const
    {
        return pos < tokens.size() ? tokens[pos].type : TokenType::END;
    }

    std::string getTokenValue() const
    {
        return pos < tokens.size() ? tokens[pos].value : "$";
    }

    std::string getTokenTypeName(TokenType type) const
    {
        switch (type)
        {
        case TokenType::WHILE:
            return "WHILE";
        case TokenType::DONE:
            return "DONE";
        case TokenType::SEMICOLON:
            return "SEMICOLON";
        case TokenType::LPAREN:
            return "LPAREN";
        case TokenType::RPAREN:
            return "RPAREN";
        case TokenType::IDENTIFIER:
            return "IDENTIFIER";
        case TokenType::ROMAN_NUMERAL:
            return "ROMAN_NUMERAL";
        case TokenType::ASSIGN:
            return "ASSIGN";
        case TokenType::LESS:
            return "LESS";
        case TokenType::GREATER:
            return "GREATER";
        case TokenType::EQUAL:
            return "EQUAL";
        case TokenType::END:
            return "END";
        default:
            return "UNKNOWN";
        }
    }

public:
    LRParser(std::vector<Token> t) : tokens(std::move(t))
    {
        initializeTables();
    }

    std::shared_ptr<ASTNode> parse()
    {
        std::stack<int> stateStack;                      // Стек состояний автомата
        std::stack<std::shared_ptr<ASTNode>> valueStack; // Стек значений (узлы AST)

        stateStack.push(0); // Начинаем с состояния 0
        pos = 0;            // Начинаем с первого токена

        std::cout << "Начало LR-разбора...\n";

        while (true)
        {
            int currentState = stateStack.top();            // Текущее состояние
            TokenType currentToken = getCurrentTokenType(); // Тип текущего токена
            std::string tokenValue = getTokenValue();       // Значение текущего токена

            // Поиск действия в таблице действий
            auto stateIt = actionTable.find(currentState);
            if (stateIt == actionTable.end())
            {
                std::cerr << "Ошибка: состояние " << currentState << " не найдено\n";
                return nullptr;
            }

            auto actionIt = stateIt->second.find(currentToken);
            if (actionIt == stateIt->second.end())
            {
                std::cerr << "Синтаксическая ошибка: неожиданный токен '"
                          << tokenValue << "' в состоянии " << currentState << "\n";
                return nullptr;
            }

            TableEntry entry = actionIt->second;

            switch (entry.action)
            {
            case SHIFT:
            {
                // Переход в новое состояние
                stateStack.push(entry.value);
                // Кладём токен в стек значений как лист AST
                valueStack.push(std::make_shared<ASTNode>("Token", tokenValue));
                pos++; // Берём следующий токен
                break;
            }
            case REDUCE:
            {
                // Правило свёртки
                Production &prod = productions[entry.value];

                // Извлекаем нужное количество детей для нового узла AST
                std::vector<std::shared_ptr<ASTNode>> children;
                for (int i = 0; i < prod.rhsSize; i++)
                {
                    if (stateStack.empty() || valueStack.empty())
                    {
                        std::cerr << "Ошибка: стек пуст при свёртке\n";
                        return nullptr;
                    }
                    stateStack.pop();
                    children.insert(children.begin(), valueStack.top());
                    valueStack.pop();
                }

                // Строим новый узел AST по правилу
                std::shared_ptr<ASTNode> newNode = prod.builder(children);
                valueStack.push(newNode);

                // Переход по таблице GOTO
                currentState = stateStack.top();
                auto gotoIt = gotoTable.find(currentState);
                if (gotoIt == gotoTable.end())
                {
                    std::cerr << "Ошибка: нет GOTO для состояния " << currentState << "\n";
                    return nullptr;
                }

                auto gotoEntry = gotoIt->second.find(prod.lhs);
                if (gotoEntry == gotoIt->second.end())
                {
                    std::cerr << "Ошибка: нет перехода для " << prod.lhs
                              << " в состоянии " << currentState << "\n";
                    return nullptr;
                }

                stateStack.push(gotoEntry->second);
                break;
            }
            case ACCEPT:
            {
                // Разбор успешно завершён
                std::cout << "  ACCEPT - разбор успешно завершен!\n";
                if (valueStack.size() == 1)
                {
                    return valueStack.top(); // Возвращаем AST
                }
                else
                {
                    std::cerr << "Ошибка: в стеке значений больше одного элемента\n";
                    return nullptr;
                }
            }
            case ERROR:
            {
                std::cerr << "Синтаксическая ошибка\n";
                return nullptr;
            }
            }

            if (entry.action == ACCEPT)
                break;
        }

        return nullptr;
    }
};

/// Рекурсивно выводит AST с отступами для наглядности.
void printAST(const std::shared_ptr<ASTNode> &node, int indent = 0)
{
    if (!node)
        return;

    std::cout << std::string(indent, ' ') << node->type;
    if (!node->value.empty())
        std::cout << " (" << node->value << ")";
    std::cout << "\n";
    for (const auto &child : node->children)
    {
        printAST(child, indent + 2);
    }
}