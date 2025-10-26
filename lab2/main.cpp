#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <memory>

/// Типы лексем, распознаваемые анализатором.
enum class TokenType {
    WHILE, DONE, SEMICOLON, LPAREN, RPAREN,
    IDENTIFIER, ROMAN_NUMERAL,
    ASSIGN, LESS, GREATER, EQUAL, END
};

/// Представляет одну лексему (токен).
struct Token {
    TokenType type;      ///< Тип токена (например, IDENTIFIER, WHILE).
    std::string value;   ///< Строковое содержимое токена.
    Token(TokenType t, std::string v) : type(t), value(std::move(v)) {}
};

/// Проверяет, является ли символ допустимым в римском числе (I, V, X).
bool isRomanChar(char c) {
    return c == 'I' || c == 'V' || c == 'X';
}

/// Выполняет лексический анализ: разбивает строку на токены.
std::vector<Token> tokenize(const std::string& input) {
    std::vector<Token> tokens;
    size_t i = 0;

    while (i < input.length()) {
        char c = input[i];
        if (std::isspace(c)) { i++; continue; }

        if (input.substr(i, 5) == "while") {
            tokens.emplace_back(TokenType::WHILE, "while");
            i += 5;
        } else if (input.substr(i, 4) == "done") {
            tokens.emplace_back(TokenType::DONE, "done");
            i += 4;
        } else if (c == ';') {
            tokens.emplace_back(TokenType::SEMICOLON, ";");
            i++;
        } else if (c == '(') {
            tokens.emplace_back(TokenType::LPAREN, "(");
            i++;
        } else if (c == ')') {
            tokens.emplace_back(TokenType::RPAREN, ")");
            i++;
        } else if (i + 1 < input.length() && input.substr(i, 2) == ":=") {
            tokens.emplace_back(TokenType::ASSIGN, ":=");
            i += 2;
        } else if (c == '<') {
            tokens.emplace_back(TokenType::LESS, "<");
            i++;
        } else if (c == '>') {
            tokens.emplace_back(TokenType::GREATER, ">");
            i++;
        } else if (c == '=') {
            tokens.emplace_back(TokenType::EQUAL, "=");
            i++;
        } else if (std::isalpha(c)) {
            size_t start = i;
            while (i < input.length() && std::isalnum(input[i])) i++;
            std::string word = input.substr(start, i - start);

            bool isRoman = !word.empty();
            for (char ch : word) {
                if (!isRomanChar(ch)) {
                    isRoman = false;
                    break;
                }
            }

            tokens.emplace_back(isRoman ? TokenType::ROMAN_NUMERAL : TokenType::IDENTIFIER, word);
        } else {
            std::cerr << "Ошибка лексики: недопустимый символ '" << c << "'\n";
            return {};
        }
    }

    tokens.emplace_back(TokenType::END, "");
    return tokens;
}

/// Узел дерева абстрактного синтаксического разбора (AST).
struct ASTNode {
    std::string type;    ///< Тип узла (например, "WhileLoop", "Assignment").
    std::string value;   ///< Значение узла (для листьев: имя или число).
    std::vector<std::shared_ptr<ASTNode>> children; ///< Дочерние узлы.
    ASTNode(std::string t, std::string v = "") : type(std::move(t)), value(std::move(v)) {}
};

/// Синтаксический анализатор, строящий AST по потоку токенов.
class Parser {
    std::vector<Token> tokens; ///< Список токенов после лексического анализа.
    size_t pos = 0;            ///< Текущая позиция в списке токенов.

    /// Возвращает текущий токен без продвижения.
    const Token& current() const { return tokens[pos]; }

    /// Потребляет ожидаемый токен; завершает программу при несоответствии.
    void consume(TokenType expected) {
        if (current().type != expected) {
            std::cerr << "Синтаксическая ошибка\n";
            exit(1);
        }
        pos++;
    }

    /// Анализирует список операторов, разделённых ';'.
    std::shared_ptr<ASTNode> parseStatementList() {
        auto node = std::make_shared<ASTNode>("StatementList");
        node->children.push_back(parseStatement());
        while (current().type == TokenType::SEMICOLON) {
            consume(TokenType::SEMICOLON);
            node->children.push_back(parseStatement());
        }
        return node;
    }

    /// Анализирует один оператор цикла: while (...) ... done.
    std::shared_ptr<ASTNode> parseStatement() {
        consume(TokenType::WHILE);
        auto loop = std::make_shared<ASTNode>("WhileLoop");
        consume(TokenType::LPAREN);
        loop->children.push_back(parseCondition());
        consume(TokenType::RPAREN);
        loop->children.push_back(parseBody());
        consume(TokenType::DONE);
        return loop;
    }

    /// Анализирует условие цикла: выражение оператор_сравнения выражение.
    std::shared_ptr<ASTNode> parseCondition() {
        auto cond = std::make_shared<ASTNode>("Condition");
        cond->children.push_back(parseExpression());

        if (current().type == TokenType::LESS || current().type == TokenType::GREATER || current().type == TokenType::EQUAL) {
            cond->children.push_back(std::make_shared<ASTNode>("RelOp", current().value));
            pos++;
        } else {
            std::cerr << "Ожидался оператор сравнения\n";
            exit(1);
        }

        cond->children.push_back(parseExpression());
        return cond;
    }

    /// Анализирует тело цикла — одно присваивание: id := выражение.
    std::shared_ptr<ASTNode> parseBody() {
        auto assign = std::make_shared<ASTNode>("Assignment");
        assign->children.push_back(std::make_shared<ASTNode>("LValue", current().value));
        consume(TokenType::IDENTIFIER);
        consume(TokenType::ASSIGN);
        assign->children.push_back(parseExpression());
        return assign;
    }

    /// Анализирует выражение: идентификатор или римское число.
    std::shared_ptr<ASTNode> parseExpression() {
        if (current().type == TokenType::IDENTIFIER) {
            auto node = std::make_shared<ASTNode>("Identifier", current().value);
            consume(TokenType::IDENTIFIER);
            return node;
        } else if (current().type == TokenType::ROMAN_NUMERAL) {
            auto node = std::make_shared<ASTNode>("RomanNumeral", current().value);
            consume(TokenType::ROMAN_NUMERAL);
            return node;
        } else {
            std::cerr << "Ожидалось выражение\n";
            exit(1);
        }
    }

public:
    /// Конструктор: принимает токены от лексера.
    Parser(std::vector<Token> t) : tokens(std::move(t)) {}

    /// Запускает разбор всей программы и возвращает корень AST.
    std::shared_ptr<ASTNode> parse() {
        auto root = std::make_shared<ASTNode>("Program");
        root->children.push_back(parseStatementList());
        return root;
    }
};

/// Рекурсивно выводит AST с отступами для наглядности.
void printAST(const std::shared_ptr<ASTNode>& node, int indent = 0) {
    std::cout << std::string(indent, ' ') << node->type;
    if (!node->value.empty()) std::cout << " (" << node->value << ")";
    std::cout << "\n";
    for (const auto& child : node->children) {
        printAST(child, indent + 2);
    }
}

/// Главная функция: запускает тестовые примеры и выводит деревья разбора.
int main() {
    std::vector<std::string> tests = {
        "while (x < V) y := I done",
        "while (a = I) b := X done; while (n > III) m := a done"
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "=== Тест " << (i + 1) << " ===\n";
        auto tokens = tokenize(tests[i]);
        if (tokens.empty()) {
            std::cout << "Лексический анализ не удался.\n\n";
            continue;
        }

        Parser parser(tokens);
        auto ast = parser.parse();
        printAST(ast);
        std::cout << "\n";
    }

    return 0;
}