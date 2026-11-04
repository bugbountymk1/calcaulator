#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <limits>
#include <windows.h>

class Calculator {
public:
    Calculator() : memory(0.0), themeIndex(0), ans(0.0) { hasAns = false; }

    void run() {
        applyTheme();
        int choice = 0;
        while (choice != 20) {
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
                case 8: performFactorial(); break;
                case 9: performPercentage(); break;
                case 10: performSine(); break;
                case 11: performCosine(); break;
                case 12: performTangent(); break;
                case 13: performLog10(); break;
                case 14: performLn(); break;
                case 15: showHistory(); break;
                case 16: clearHistory(); break;
                case 17: toggleTheme(); break;
                case 18: memoryMenu(); break;
                case 19: system("cls"); break;
                case 20: std::cout << "Goodbye!\n"; break;
                default: std::cout << "Invalid choice. Try again.\n";
            }
        }
    }

private:
    double memory;
    double ans;
    bool hasAns;
    std::vector<std::string> history;
    int themeIndex;

    void applyTheme() const {
        const char* colors[] = {"0F", "F0", "1F", "2F", "4F", "0A", "5F", "3F", "F1", "F2", "F3", "F5", "F4", "F6"};
        std::string cmd = "color ";
        cmd += colors[themeIndex];
        system(cmd.c_str());
    }

    const char* themeName() const {
        const char* names[] = {"Dark", "White", "Blue", "Green", "Red", "Matrix", "Purple", "Cyan", "Glass Blue", "Glass Green", "Glass Cyan", "Glass Purple", "Glass Red", "Glass Gold"};
        return names[themeIndex];
    }

    void toggleTheme() {
        themeIndex = (themeIndex + 1) % 14;
        applyTheme();
        std::cout << "Switched to " << themeName() << " theme.\n";
    }

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
        std::cout << "8. Factorial (n!)\n";
        std::cout << "9. Percentage\n";
        std::cout << "10. Sine (sin)\n";
        std::cout << "11. Cosine (cos)\n";
        std::cout << "12. Tangent (tan)\n";
        std::cout << "13. Log10 (log)\n";
        std::cout << "14. Natural Log (ln)\n";
        std::cout << "15. Show History\n";
        std::cout << "16. Clear History\n";
        std::cout << "17. Change Theme\n";
        std::cout << "18. Memory Functions\n";
        std::cout << "19. Clear Screen\n";
        std::cout << "20. Exit\n";
        std::cout << "------------------------------\n";
        std::cout << "Theme: " << themeName() << "\n";
        std::cout << "Memory: " << formatNumber(memory) << "\n";
        std::cout << "Ans: " << (hasAns ? formatNumber(ans) : "--") << "\n";
    }

    std::string formatNumber(double value) const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6) << value;
        std::string s = oss.str();
        size_t dot = s.find('.');
        if (dot != std::string::npos) {
            s.erase(s.find_last_not_of('0') + 1, std::string::npos);
            if (!s.empty() && s.back() == '.') {
                s.pop_back();
            }
        }
        return s;
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
        if (history.size() > 100) {
            history.erase(history.begin());
        }
    }

    void setAns(double value) {
        ans = value;
        hasAns = true;
    }

    void performAddition() {
        double a = getNumber("Enter first number: ");
        double b = getNumber("Enter second number: ");
        double result = a + b;
        std::cout << formatNumber(a) << " + " << formatNumber(b) << " = " << formatNumber(result) << "\n";
        setAns(result);
        record(formatNumber(a) + " + " + formatNumber(b) + " = " + formatNumber(result));
    }

    void performSubtraction() {
        double a = getNumber("Enter first number: ");
        double b = getNumber("Enter second number: ");
        double result = a - b;
        std::cout << formatNumber(a) << " - " << formatNumber(b) << " = " << formatNumber(result) << "\n";
        setAns(result);
        record(formatNumber(a) + " - " + formatNumber(b) + " = " + formatNumber(result));
    }

    void performMultiplication() {
        double a = getNumber("Enter first number: ");
        double b = getNumber("Enter second number: ");
        double result = a * b;
        std::cout << formatNumber(a) << " * " << formatNumber(b) << " = " << formatNumber(result) << "\n";
        setAns(result);
        record(formatNumber(a) + " * " + formatNumber(b) + " = " + formatNumber(result));
    }

    void performDivision() {
        double a = getNumber("Enter first number: ");
        double b = getNumber("Enter second number: ");
        if (b == 0) {
            std::cout << "Error: Division by zero is not allowed.\n";
            return;
        }
        double result = a / b;
        std::cout << formatNumber(a) << " / " << formatNumber(b) << " = " << formatNumber(result) << "\n";
        setAns(result);
        record(formatNumber(a) + " / " + formatNumber(b) + " = " + formatNumber(result));
    }

    void performPower() {
        double base = getNumber("Enter base: ");
        double exp = getNumber("Enter exponent: ");
        double result = std::pow(base, exp);
        std::cout << formatNumber(base) << " ^ " << formatNumber(exp) << " = " << formatNumber(result) << "\n";
        setAns(result);
        record(formatNumber(base) + " ^ " + formatNumber(exp) + " = " + formatNumber(result));
    }

    void performSquareRoot() {
        double a = getNumber("Enter number: ");
        if (a < 0) {
            std::cout << "Error: Cannot compute square root of a negative number.\n";
            return;
        }
        double result = std::sqrt(a);
        std::cout << "sqrt(" << formatNumber(a) << ") = " << formatNumber(result) << "\n";
        setAns(result);
        record("sqrt(" + formatNumber(a) + ") = " + formatNumber(result));
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
        setAns(static_cast<double>(result));
        record(std::to_string(a) + " % " + std::to_string(b) + " = " + std::to_string(result));
    }

    void performFactorial() {
        double input = getNumber("Enter non-negative integer: ");
        if (input < 0 || input != std::floor(input)) {
            std::cout << "Error: Factorial requires a non-negative integer.\n";
            return;
        }
        if (input > 20) {
            std::cout << "Error: Input too large (max 20).\n";
            return;
        }
        long n = static_cast<long>(input);
        unsigned long long result = 1;
        for (long i = 2; i <= n; ++i) {
            result *= static_cast<unsigned long long>(i);
        }
        std::cout << n << "! = " << result << "\n";
        setAns(static_cast<double>(result));
        record(std::to_string(n) + "! = " + std::to_string(result));
    }

    void performPercentage() {
        double value = getNumber("Enter value: ");
        double percent = getNumber("Enter percentage: ");
        double result = value * (percent / 100.0);
        std::cout << formatNumber(percent) << "% of " << formatNumber(value) << " = " << formatNumber(result) << "\n";
        setAns(result);
        record(formatNumber(percent) + "% of " + formatNumber(value) + " = " + formatNumber(result));
    }

    void performSine() {
        double a = getNumber("Enter angle in degrees: ");
        double result = std::sin(a * 3.14159265358979323846 / 180.0);
        std::cout << "sin(" << formatNumber(a) << "°) = " << formatNumber(result) << "\n";
        setAns(result);
        record("sin(" + formatNumber(a) + "°) = " + formatNumber(result));
    }

    void performCosine() {
        double a = getNumber("Enter angle in degrees: ");
        double result = std::cos(a * 3.14159265358979323846 / 180.0);
        std::cout << "cos(" << formatNumber(a) << "°) = " << formatNumber(result) << "\n";
        setAns(result);
        record("cos(" + formatNumber(a) + "°) = " + formatNumber(result));
    }

    void performTangent() {
        double a = getNumber("Enter angle in degrees: ");
        double result = std::tan(a * 3.14159265358979323846 / 180.0);
        std::cout << "tan(" << formatNumber(a) << "°) = " << formatNumber(result) << "\n";
        setAns(result);
        record("tan(" + formatNumber(a) + "°) = " + formatNumber(result));
    }

    void performLog10() {
        double a = getNumber("Enter value: ");
        if (a <= 0) {
            std::cout << "Error: Log10 requires a positive value.\n";
            return;
        }
        double result = std::log10(a);
        std::cout << "log10(" << formatNumber(a) << ") = " << formatNumber(result) << "\n";
        setAns(result);
        record("log10(" + formatNumber(a) + ") = " + formatNumber(result));
    }

    void performLn() {
        double a = getNumber("Enter value: ");
        if (a <= 0) {
            std::cout << "Error: Natural log requires a positive value.\n";
            return;
        }
        double result = std::log(a);
        std::cout << "ln(" << formatNumber(a) << ") = " << formatNumber(result) << "\n";
        setAns(result);
        record("ln(" + formatNumber(a) + ") = " + formatNumber(result));
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

    void clearHistory() {
        history.clear();
        std::cout << "History cleared.\n";
    }

    void memoryMenu() {
        int choice = 0;
        while (choice != 5) {
            std::cout << "\n--- Memory Functions ---\n";
            std::cout << "1. M+ (Add to memory)\n";
            std::cout << "2. M- (Subtract from memory)\n";
            std::cout << "3. MR (Recall memory)\n";
            std::cout << "4. MC (Clear memory)\n";
            std::cout << "5. Back\n";
            std::cout << "Enter choice: ";
            if (!(std::cin >> choice)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            switch (choice) {
                case 1: {
                    double val = getNumber("Enter value to add: ");
                    memory += val;
                    record("M+ " + formatNumber(val) + " -> Memory: " + formatNumber(memory));
                    break;
                }
                case 2: {
                    double val = getNumber("Enter value to subtract: ");
                    memory -= val;
                    record("M- " + formatNumber(val) + " -> Memory: " + formatNumber(memory));
                    break;
                }
                case 3:
                    std::cout << "Memory: " << formatNumber(memory) << "\n";
                    break;
                case 4:
                    memory = 0.0;
                    std::cout << "Memory cleared.\n";
                    break;
                case 5:
                    break;
                default:
                    std::cout << "Invalid choice.\n";
            }
        }
    }
};

int main() {
    Calculator calc;
    calc.run();
    return 0;
}