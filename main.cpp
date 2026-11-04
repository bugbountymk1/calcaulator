#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <limits>

class Calculator {
public:
    Calculator() : memory(0.0) {}

    void run() {
        int choice = 0;
        while (choice != 9) {
            displayMenu();
            std::cout << "\nEnter choice: ";
            if (!(std::cin >> choice)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input. Please enter a number.\n";
                continue;
            }

            switch (choice) {
                case 1: performAddition(); break;
                case 2: performSubtraction(); break;
                case 3: performMultiplication(); break;
                case 4: performDivision(); break;
                case 5: performPower(); break;
                case 6: performSquareRoot(); break;
                case 7: performModulo(); break;
                case 8: showHistory(); break;
                case 9: std::cout << "Goodbye!\n"; break;
                default: std::cout << "Invalid choice. Try again.\n";
            }
        }
    }

private:
    double memory;
    std::vector<std::string> history;

    void displayMenu() const {
        std::cout << "\n==============================\n";
        std::cout << "      C++ CALCULATOR\n";
        std::cout << "==============================\n";
        std::cout << "1. Addition\n";
        std::cout << "2. Subtraction\n";
        std::cout << "3. Multiplication\n";
        std::cout << "4. Division\n";
        std::cout << "5. Power (x^y)\n";
        std::cout << "6. Square Root\n";
        std::cout << "7. Modulo\n";
        std::cout << "8. Show History\n";
        std::cout << "9. Exit\n";
        std::cout << "------------------------------\n";
        std::cout << "Memory: " << memory << "\n";
    }

    double getNumber(const std::string& prompt) {
        double value;
        while (true) {
            std::cout << prompt;
            if (std::cin >> value) {
                return value;
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a numeric value.\n";
        }
    }

    void record(const std::string& entry) {
        history.push_back(entry);
        if (history.size() > 50) {
            history.erase(history.begin());
        }
    }

    void performAddition() {
        double a = getNumber("Enter first number: ");
        double b = getNumber("Enter second number: ");
        double result = a + b;
        std::cout << a << " + " << b << " = " << result << "\n";
        record(std::to_string(a) + " + " + std::to_string(b) + " = " + std::to_string(result));
    }

    void performSubtraction() {
        double a = getNumber("Enter first number: ");
        double b = getNumber("Enter second number: ");
        double result = a - b;
        std::cout << a << " - " << b << " = " << result << "\n";
        record(std::to_string(a) + " - " + std::to_string(b) + " = " + std::to_string(result));
    }

    void performMultiplication() {
        double a = getNumber("Enter first number: ");
        double b = getNumber("Enter second number: ");
        double result = a * b;
        std::cout << a << " * " << b << " = " << result << "\n";
        record(std::to_string(a) + " * " + std::to_string(b) + " = " + std::to_string(result));
    }

    void performDivision() {
        double a = getNumber("Enter first number: ");
        double b = getNumber("Enter second number: ");
        if (b == 0) {
            std::cout << "Error: Division by zero is not allowed.\n";
            return;
        }
        double result = a / b;
        std::cout << a << " / " << b << " = " << result << "\n";
        record(std::to_string(a) + " / " + std::to_string(b) + " = " + std::to_string(result));
    }

    void performPower() {
        double base = getNumber("Enter base: ");
        double exp = getNumber("Enter exponent: ");
        double result = std::pow(base, exp);
        std::cout << base << " ^ " << exp << " = " << result << "\n";
        record(std::to_string(base) + " ^ " + std::to_string(exp) + " = " + std::to_string(result));
    }

    void performSquareRoot() {
        double a = getNumber("Enter number: ");
        if (a < 0) {
            std::cout << "Error: Cannot compute square root of a negative number.\n";
            return;
        }
        double result = std::sqrt(a);
        std::cout << "sqrt(" << a << ") = " << result << "\n";
        record("sqrt(" + std::to_string(a) + ") = " + std::to_string(result));
    }

    void performModulo() {
        long a = static_cast<long>(getNumber("Enter first number: "));
        long b = static_cast<long>(getNumber("Enter second number: "));
        if (b == 0) {
            std::cout << "Error: Modulo by zero is not allowed.\n";
            return;
        }
        long result = a % b;
        std::cout << a << " % " << b << " = " << result << "\n";
        record(std::to_string(a) + " % " + std::to_string(b) + " = " + std::to_string(result));
    }

    void showHistory() const {
        if (history.empty()) {
            std::cout << "No history yet.\n";
            return;
        }
        std::cout << "\n--- Calculation History ---\n";
        for (size_t i = 0; i < history.size(); ++i) {
            std::cout << i + 1 << ". " << history[i] << "\n";
        }
        std::cout << "---------------------------\n";
    }
};

int main() {
    Calculator calc;
    calc.run();
    return 0;
}
