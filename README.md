# C++ Calculator

A full-featured console calculator written in C++ with dark/white themes, calculation history, and memory functions.

## Features

- **Basic arithmetic:** Addition, subtraction, multiplication, division
- **Advanced math:** Power (`x^y`), square root, modulo, factorial (`n!`), percentage
- **Trigonometry:** Sine, cosine, tangent (degrees)
- **Logarithms:** Log10 and natural log (ln)
- **Last result (Ans):** Each calculation updates `Ans`, shown on the menu
- **Memory functions:** M+ (add), M- (subtract), MR (recall), MC (clear)
- **History:** Last 100 calculations, with option to clear history
- **Themes:** 14 console color themes (Dark, White, Blue, Green, Red, Matrix, Purple, Cyan, and Glass variants)
- **Clear screen:** Clear the console at any time
- **Input validation:** Rejects invalid numbers, division/modulo by zero, negative square roots, invalid factorials, etc.

## Requirements

- Windows
- **Visual Studio 2022** (with C++ build tools) — provides the Developer Command Prompt and `cl.exe` (MSVC compiler)

## How to Build the `.exe`

1. Open the **Developer Command Prompt for VS 2022**  
   (Start menu → Visual Studio 2022 → Developer Command Prompt for VS 2022)

2. Navigate to this project folder:
   ```cmd
   cd path\to\calcaulator
   ```

3. Compile with MSVC:
   ```cmd
   cl /EHsc /O2 /std:c++17 main.cpp
   ```

   This produces `calculator.exe` in the current folder.

   Alternatively, run the included build script:
   ```cmd
   build.bat
   ```

## How to Run

```cmd
calculator.exe
```

## How to Use the App

When you start the calculator, you will see a menu:

```
==============================
      C++ CALCULATOR
==============================
1. Addition
2. Subtraction
3. Multiplication
4. Division
5. Power (x^y)
6. Square Root
7. Modulo
8. Factorial (n!)
9. Percentage
10. Sine (sin)
11. Cosine (cos)
12. Tangent (tan)
13. Log10 (log)
14. Natural Log (ln)
15. Show History
16. Clear History
17. Change Theme
18. Memory Functions
19. Clear Screen
20. Exit
------------------------------
Theme: Dark
Memory: 0
Ans: --
```

### Menu Options

| Option | Description |
|--------|-------------|
| 1 | Addition — enter two numbers, get the sum |
| 2 | Subtraction — enter two numbers, get the difference |
| 3 | Multiplication — enter two numbers, get the product |
| 4 | Division — enter two numbers, get the quotient (cannot divide by zero) |
| 5 | Power — enter base and exponent, compute `base^exponent` |
| 6 | Square Root — enter a non-negative number |
| 7 | Modulo — enter two integers, get the remainder |
| 8 | Factorial — enter a non-negative integer (max 20) |
| 9 | Percentage — enter a value and a percentage, compute `value × percent%` |
| 10 | Sine — enter angle in degrees, compute sine |
| 11 | Cosine — enter angle in degrees, compute cosine |
| 12 | Tangent — enter angle in degrees, compute tangent |
| 13 | Log10 — compute base-10 logarithm (positive value required) |
| 14 | Natural Log — compute natural logarithm (positive value required) |
| 15 | Show History — view the last 100 calculations |
| 16 | Clear History — erase all stored calculations |
| 17 | Theme — cycle through 14 console color themes |
| 18 | Memory Functions — submenu for M+, M-, MR, MC |
| 19 | Clear Screen — clear the console |
| 20 | Exit — close the calculator |

### Memory Submenu (Option 13)

```
--- Memory Functions ---
1. M+ (Add to memory)
2. M- (Subtract from memory)
3. MR (Recall memory)
4. MC (Clear memory)
5. Back
```

- **M+:** Adds a value to the current memory
- **M-:** Subtracts a value from the current memory
- **MR:** Displays the current memory value
- **MC:** Resets memory to 0

## Files

| File | Purpose |
|------|---------|
| `main.cpp` | C++ source code for the calculator |
| `build.bat` | Windows batch script to compile with `cl.exe` |
| `Makefile` | Build file for MinGW/g++ (alternative compiler) |
| `calculator.exe` | Compiled executable (produced after building) |

## Notes

- The app uses Windows console colors (`system("color ...")`) for theme switching, so it is designed for Windows CMD/PowerShell.
- Trigonometric functions accept angles in degrees.
- `Ans` stores the result of the last successful calculation and is shown on the main menu.
- History is kept in memory only and is cleared when you exit the program.