#include"../main.hpp"

/// Главная функция: запускает тестовые примеры и выводит деревья разбора.
int main() {
    std::vector<std::string> tests = {
        "while (x < V) y := I done",
        "while (a = I) b := X done; while (n > III) m := a done"
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "=== Тест " << (i + 1) << " ===\n";
        std::cout << "Входная строка: " << tests[i] << "\n\n";
        
        auto tokens = tokenize(tests[i]);
        if (tokens.empty()) {
            std::cout << "Лексический анализ не удался.\n\n";
            continue;
        }

        // LR-анализ
        std::cout << "=== LR-анализ ===\n";
        LRParser parser(tokens);
        auto ast = parser.parse();
        
        if (ast) {
            std::cout << "\n=== Результат AST ===\n";
            printAST(ast);
        } else {
            std::cout << "LR-анализ завершился с ошибками\n";
        }
        
        std::cout << "\n" << std::string(40, '=') << "\n\n";
    }

    return 0;
}