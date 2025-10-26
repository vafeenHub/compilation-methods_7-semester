/**
 * @brief Реализация лексического и синтаксического анализаторов
 *        для языка с операторами цикла while (...) ... done.
 *
 * Входной язык:
 * - Операторы цикла: while (условие) тело done
 * - Несколько операторов разделяются точкой с запятой ';'
 * - Условие: выражение оператор_сравнения выражение
 * - Выражение: идентификатор или римское число (I, V, X в верхнем регистре)
 * - Тело цикла: одно присваивание вида "идентификатор := выражение"
 *
 * Программа строит дерево разбора (AST) для корректных входных строк.
 */

#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <memory>

/**
 * @enum TokenType
 * @brief Перечисление типов лексем (токенов), распознаваемых лексическим анализатором.
 */
enum class TokenType {
    WHILE,          ///< Ключевое слово "while"
    DONE,           ///< Ключевое слово "done"
    SEMICOLON,      ///< Символ точки с запятой ';'
    LPAREN,         ///< Левая круглая скобка '('
    RPAREN,         ///< Правая круглая скобка ')'
    IDENTIFIER,     ///< Идентификатор (начинается с буквы, далее буквы/цифры)
    ROMAN_NUMERAL,  ///< Римское число (непустая строка из заглавных I, V, X)
    ASSIGN,         ///< Оператор присваивания ":="
    LESS,           ///< Оператор сравнения '<'
    GREATER,        ///< Оператор сравнения '>'
    EQUAL,          ///< Оператор сравнения '='
    END             ///< Маркер конца входного потока
};

/**
 * @struct Token
 * @brief Представляет одну лексему (токен) после лексического анализа.
 */
struct Token {
    TokenType type;       ///< Тип токена
    std::string value;    ///< Исходная строка токена (для отладки и AST)
    int pos;              ///< Позиция начала токена во входной строке

    /**
     * @brief Конструктор токена.
     * @param t Тип токена
     * @param v Строковое значение токена
     * @param p Позиция в исходной строке
     */
    Token(TokenType t, const std::string &v, int p) : type(t), value(v), pos(p) {}
};

/**
 * @brief Проверяет, является ли символ допустимым в римском числе.
 * @param c Символ для проверки
 * @return true, если c — 'I', 'V' или 'X'; иначе false.
 */
bool isRomanChar(char c) {
    return c == 'I' || c == 'V' || c == 'X';
}

/**
 * @brief Выполняет лексический анализ входной строки.
 *
 * Разбивает строку на последовательность токенов, игнорируя пробелы.
 * Распознаёт ключевые слова, символы, идентификаторы и римские числа.
 *
 * @param input Входная строка программы
 * @return Вектор токенов, завершающийся токеном TokenType::END.
 *         В случае ошибки возвращает вектор с токеном END в позиции ошибки.
 */
std::vector<Token> tokenize(const std::string &input) {
    std::vector<Token> tokens;
    size_t i = 0;

    while (i < input.length()) {
        char c = input[i];

        // Пропускаем пробельные символы
        if (std::isspace(c)) {
            i++;
            continue;
        }

        // Распознавание ключевых слов и многосимвольных операторов
        if (input.substr(i, 5) == "while") {
            tokens.emplace_back(TokenType::WHILE, "while", static_cast<int>(i));
            i += 5;
        }
        else if (input.substr(i, 4) == "done") {
            tokens.emplace_back(TokenType::DONE, "done", static_cast<int>(i));
            i += 4;
        }
        else if (c == ';') {
            tokens.emplace_back(TokenType::SEMICOLON, ";", static_cast<int>(i));
            i++;
        }
        else if (c == '(') {
            tokens.emplace_back(TokenType::LPAREN, "(", static_cast<int>(i));
            i++;
        }
        else if (c == ')') {
            tokens.emplace_back(TokenType::RPAREN, ")", static_cast<int>(i));
            i++;
        }
        else if (i + 1 < input.length() && input.substr(i, 2) == ":=") {
            tokens.emplace_back(TokenType::ASSIGN, ":=", static_cast<int>(i));
            i += 2;
        }
        // Односимвольные операторы сравнения
        else if (c == '<') {
            tokens.emplace_back(TokenType::LESS, "<", static_cast<int>(i));
            i++;
        }
        else if (c == '>') {
            tokens.emplace_back(TokenType::GREATER, ">", static_cast<int>(i));
            i++;
        }
        else if (c == '=') {
            tokens.emplace_back(TokenType::EQUAL, "=", static_cast<int>(i));
            i++;
        }
        // Идентификаторы и римские числа
        else if (std::isalpha(c)) {
            size_t start = i;
            // Собираем максимально длинную альфанумерическую последовательность
            while (i < input.length() && (std::isalnum(input[i]))) {
                i++;
            }
            std::string ident = input.substr(start, i - start);

            // Проверяем: состоит ли строка ТОЛЬКО из I, V, X (заглавных)
            bool isRoman = !ident.empty();
            for (char ch : ident) {
                if (!isRomanChar(ch)) {
                    isRoman = false;
                    break;
                }
            }

            if (isRoman) {
                tokens.emplace_back(TokenType::ROMAN_NUMERAL, ident, static_cast<int>(start));
            } else {
                tokens.emplace_back(TokenType::IDENTIFIER, ident, static_cast<int>(start));
            }
        }
        // Недопустимый символ
        else {
            std::cerr << "Ошибка лексического анализа на позиции " << i << ": неизвестный символ '" << c << "'\n";
            tokens.emplace_back(TokenType::END, "", static_cast<int>(i));
            return tokens;
        }
    }

    // Добавляем маркер конца
    tokens.emplace_back(TokenType::END, "", static_cast<int>(i));
    return tokens;
}

/**
 * @struct ASTNode
 * @brief Узел дерева абстрактного синтаксического разбора (AST).
 *
 * Используется для представления структуры программы в виде дерева.
 */
struct ASTNode {
    std::string type;    ///< Тип узла (например, "WhileLoop", "Assignment")
    std::vector<std::shared_ptr<ASTNode>> children; ///< Дочерние узлы
    std::string value;   ///< Значение узла (для листьев: имя идентификатора, число и т.д.)

    /**
     * @brief Конструктор узла без значения (для внутренних узлов).
     * @param t Тип узла
     */
    ASTNode(const std::string &t) : type(t) {}

    /**
     * @brief Конструктор узла со значением (для листьев).
     * @param t Тип узла
     * @param v Значение узла
     */
    ASTNode(const std::string &t, const std::string &v) : type(t), value(v) {}
};

/**
 * @class Parser
 * @brief Класс синтаксического анализатора, реализующий рекурсивный спуск.
 *
 * Принимает последовательность токенов и строит AST в соответствии с грамматикой:
 * Program      → StatementList
 * StatementList→ Statement (';' Statement)*
 * Statement    → WHILE '(' Condition ')' Body DONE
 * Condition    → Expression RelOp Expression
 * Body         → Assignment
 * Assignment   → IDENTIFIER ':=' Expression
 * Expression   → IDENTIFIER | ROMAN_NUMERAL
 * RelOp        → '<' | '>' | '='
 */
class Parser {
    std::vector<Token> tokens; ///< Последовательность токенов
    size_t pos = 0;            ///< Текущая позиция в потоке токенов

public:
    /**
     * @brief Конструктор парсера.
     * @param t Вектор токенов, полученный от лексера
     */
    Parser(std::vector<Token> t) : tokens(std::move(t)) {}

    /**
     * @brief Запускает синтаксический анализ всей программы.
     * @return Корень дерева разбора (узел типа "Program")
     */
    std::shared_ptr<ASTNode> parse() {
        auto program = std::make_shared<ASTNode>("Program");
        program->children.push_back(parseStatementList());
        return program;
    }

private:
    /**
     * @brief Возвращает текущий токен без продвижения.
     * @return Константная ссылка на текущий токен
     */
    const Token &current() const {
        return tokens[pos];
    }

    /**
     * @brief Потребляет ожидаемый токен и продвигает позицию.
     * @param expected Ожидаемый тип токена
     * @throws Завершает программу с ошибкой, если тип не совпадает
     */
    void consume(TokenType expected) {
        if (current().type != expected) {
            std::cerr << "Синтаксическая ошибка: ожидался токен типа " << static_cast<int>(expected)
                      << ", найден " << static_cast<int>(current().type) << " (\"" << current().value << "\")\n";
            exit(1);
        }
        pos++;
    }

    /**
     * @brief Анализирует список операторов, разделённых ';'.
     * @return Узел "StatementList"
     */
    std::shared_ptr<ASTNode> parseStatementList() {
        auto node = std::make_shared<ASTNode>("StatementList");
        node->children.push_back(parseStatement());

        while (current().type == TokenType::SEMICOLON) {
            consume(TokenType::SEMICOLON);
            node->children.push_back(parseStatement());
        }

        return node;
    }

    /**
     * @brief Анализирует один оператор цикла while.
     * @return Узел "WhileLoop"
     */
    std::shared_ptr<ASTNode> parseStatement() {
        if (current().type != TokenType::WHILE) {
            std::cerr << "Синтаксическая ошибка: ожидался 'while'\n";
            exit(1);
        }
        consume(TokenType::WHILE);

        auto whileNode = std::make_shared<ASTNode>("WhileLoop");

        consume(TokenType::LPAREN);
        whileNode->children.push_back(parseCondition());
        consume(TokenType::RPAREN);

        whileNode->children.push_back(parseBody());

        consume(TokenType::DONE);

        return whileNode;
    }

    /**
     * @brief Анализирует условие цикла (выражение оператор_сравнения выражение).
     * @return Узел "Condition"
     */
    std::shared_ptr<ASTNode> parseCondition() {
        auto cond = std::make_shared<ASTNode>("Condition");
        cond->children.push_back(parseExpression());

        TokenType opType = current().type;
        if (opType == TokenType::LESS || opType == TokenType::GREATER || opType == TokenType::EQUAL) {
            cond->children.push_back(std::make_shared<ASTNode>("RelOp", current().value));
            pos++;
        }
        else {
            std::cerr << "Синтаксическая ошибка: ожидался оператор сравнения (<, >, =)\n";
            exit(1);
        }

        cond->children.push_back(parseExpression());
        return cond;
    }

    /**
     * @brief Анализирует тело цикла (одно присваивание).
     * @return Узел "Assignment"
     */
    std::shared_ptr<ASTNode> parseBody() {
        auto assign = std::make_shared<ASTNode>("Assignment");

        if (current().type != TokenType::IDENTIFIER) {
            std::cerr << "Синтаксическая ошибка: ожидался идентификатор в левой части присваивания\n";
            exit(1);
        }
        assign->children.push_back(std::make_shared<ASTNode>("LValue", current().value));
        consume(TokenType::IDENTIFIER);

        consume(TokenType::ASSIGN);

        assign->children.push_back(parseExpression());

        return assign;
    }

    /**
     * @brief Анализирует выражение: идентификатор или римское число.
     * @return Узел "Identifier" или "RomanNumeral"
     */
    std::shared_ptr<ASTNode> parseExpression() {
        if (current().type == TokenType::IDENTIFIER) {
            auto node = std::make_shared<ASTNode>("Identifier", current().value);
            consume(TokenType::IDENTIFIER);
            return node;
        }
        else if (current().type == TokenType::ROMAN_NUMERAL) {
            auto node = std::make_shared<ASTNode>("RomanNumeral", current().value);
            consume(TokenType::ROMAN_NUMERAL);
            return node;
        }
        else {
            std::cerr << "Синтаксическая ошибка: ожидалось выражение (идентификатор или римское число)\n";
            exit(1);
        }
    }
};

/**
 * @brief Рекурсивно выводит дерево разбора в консоль с отступами.
 * @param node Указатель на корень поддерева
 * @param indent Текущий уровень отступа (в пробелах)
 */
void printAST(const std::shared_ptr<ASTNode> &node, int indent = 0) {
    std::cout << std::string(indent, ' ') << node->type;
    if (!node->value.empty()) {
        std::cout << " (" << node->value << ")";
    }
    std::cout << "\n";

    for (const auto &child : node->children) {
        printAST(child, indent + 2);
    }
}

/**
 * @brief Главная функция программы.
 *
 * Запускает серию тестов: для каждой строки в testInputs
 * выполняет лексический и синтаксический анализ,
 * затем выводит построенное дерево разбора.
 *
 * @return 0 при успешном завершении
 */
int main() {
    // Набор тестовых входных строк
    std::vector<std::string> testInputs = {
        "while (x < V) y := I done",
        "while (a = I) b := X done; while (n > III) m := a done",
        "while (flag < X) counter := V done",
        "while (A > I) B := C done; while (X = V) Y := II done"
    };

    // Обработка каждого теста
    for (size_t idx = 0; idx < testInputs.size(); ++idx) {
        std::cout << "=== Тест " << (idx + 1) << " ===\n";
        std::cout << "Вход: \"" << testInputs[idx] << "\"\n";

        // Лексический анализ
        auto tokens = tokenize(testInputs[idx]);
        if (tokens.empty() || tokens.back().type != TokenType::END) {
            std::cerr << "❌ Лексический анализ завершился с ошибкой.\n\n";
            continue;
        }

        // Синтаксический анализ и построение AST
        try {
            Parser parser(tokens);
            auto ast = parser.parse();
            std::cout << "✅ Успешный разбор:\n";
            printAST(ast);
        }
        catch (...) {
            std::cerr << "❌ Ошибка синтаксического анализа.\n";
        }

        std::cout << "\n";
    }

    return 0;
}