#include <windows.h>
#undef max
#undef min

#include <string>
#include <vector>
#include <cmath>
#include <commctrl.h>
#include <sstream>
#include <iomanip>
#include <algorithm>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")

// ---------------------------------------------------------------------------
// Control / menu IDs
// ---------------------------------------------------------------------------
#define IDC_DISPLAY   100

#define IDM_EXIT      1001
#define IDM_HISTORY   1002
#define IDM_THEME     1003
#define IDM_ABOUT     1004

#define BTN_MC        1101
#define BTN_MMINUS    1102
#define BTN_MPLUS     1103
#define BTN_MR        1104
#define BTN_CE        1105
#define BTN_C         1106
#define BTN_BACK      1107
#define BTN_7         1108
#define BTN_8         1109
#define BTN_9         1110
#define BTN_DIV       1111
#define BTN_SQRT      1112
#define BTN_X2        1113
#define BTN_POW       1114
#define BTN_4         1115
#define BTN_5         1116
#define BTN_6         1117
#define BTN_MUL       1118
#define BTN_MOD       1119
#define BTN_INV       1120
#define BTN_SIGN      1121
#define BTN_1         1122
#define BTN_2         1123
#define BTN_3         1124
#define BTN_SUB       1125
#define BTN_SIN       1126
#define BTN_COS       1127
#define BTN_TAN       1128
#define BTN_0         1129
#define BTN_DOT       1130
#define BTN_EQ        1131
#define BTN_ADD       1132
#define BTN_LOG       1133
#define BTN_LN        1134
#define BTN_FACT      1135
#define BTN_ANS       1136
#define BTN_HIST      1137
#define BTN_THEME     1138
#define BTN_ABOUT     1139
#define BTN_EXIT      1140

#define IDC_HIST_LIST 1201
#define IDC_HIST_CLOSE 1202

// ---------------------------------------------------------------------------
// Themes
// ---------------------------------------------------------------------------
struct Theme { COLORREF windowBg, displayBg, displayText, btnBg, btnText; };
static Theme themes[] = {
    { RGB(240,240,240), RGB(255,255,255), RGB(0,0,0),     RGB(225,225,225), RGB(0,0,0) },     // Light
    { RGB(45,45,45),    RGB(30,30,30),    RGB(240,240,240),RGB(60,60,60),   RGB(240,240,240) }, // Dark
    { RGB(235,245,255), RGB(255,255,255), RGB(0,0,120),   RGB(200,225,250), RGB(0,0,120) },    // Blue
    { RGB(235,255,235), RGB(255,255,255), RGB(0,100,0),   RGB(200,250,200), RGB(0,100,0) },    // Green
    { RGB(255,235,235), RGB(255,255,255), RGB(120,0,0),   RGB(250,200,200), RGB(120,0,0) },    // Red
};
static const char* themeNames[] = { "Light", "Dark", "Blue", "Green", "Red" };
static const int NUM_THEMES = 5;

// ---------------------------------------------------------------------------
// Calculator state
// ---------------------------------------------------------------------------
struct Calc {
    HWND hWnd = nullptr;
    HWND hDisplay = nullptr;
    HWND hHistoryWnd = nullptr;
    HFONT hFont = nullptr;
    HFONT hBigFont = nullptr;
    int themeIndex = 0;

    std::string display;
    double storedValue = 0.0;
    char pendingOp = 0;
    bool waitingForOperand = false;
    bool justCalculated = false;

    double memory = 0.0;
    double ans = 0.0;
    bool hasAns = false;

    std::vector<std::string> history;
};

static Calc calc;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static std::string formatNumber(double value) {
    if (std::isnan(value) || std::isinf(value)) return "Error";
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6) << value;
    std::string s = oss.str();
    size_t dot = s.find('.');
    if (dot != std::string::npos) {
        s.erase(s.find_last_not_of('0') + 1, std::string::npos);
        if (!s.empty() && s.back() == '.') s.pop_back();
    }
    return s;
}

static double currentValue() {
    if (calc.display.empty() || calc.display == "Error") return 0.0;
    try { return std::stod(calc.display); } catch (...) { return 0.0; }
}

static void setDisplay(const std::string& text) {
    calc.display = text;
    SetWindowTextA(calc.hDisplay, text.c_str());
}

static void appendDisplay(const std::string& text) {
    calc.display += text;
    SetWindowTextA(calc.hDisplay, calc.display.c_str());
}

static void recordHistory(const std::string& entry) {
    calc.history.push_back(entry);
    if (calc.history.size() > 100) calc.history.erase(calc.history.begin());
    if (calc.hHistoryWnd && IsWindow(calc.hHistoryWnd)) {
        HWND hList = GetDlgItem(calc.hHistoryWnd, IDC_HIST_LIST);
        SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)entry.c_str());
        SendMessage(hList, LB_SETTOPINDEX, (WPARAM)(SendMessage(hList, LB_GETCOUNT, 0, 0) - 1), 0);
    }
}

static void setAns(double value) {
    calc.ans = value;
    calc.hasAns = true;
}

// ---------------------------------------------------------------------------
// Button actions
// ---------------------------------------------------------------------------
static void onDigit(char d) {
    if (calc.justCalculated) { setDisplay(""); calc.justCalculated = false; }
    if (calc.waitingForOperand) { setDisplay(""); calc.waitingForOperand = false; }
    if (calc.display == "Error") setDisplay("");
    if (calc.display.empty()) calc.display = "0";
    appendDisplay(std::string(1, d));
}

static void onDecimal() {
    if (calc.justCalculated) { setDisplay("0"); calc.justCalculated = false; }
    if (calc.waitingForOperand) { setDisplay("0"); calc.waitingForOperand = false; }
    if (calc.display == "Error") setDisplay("0");
    if (calc.display.empty()) calc.display = "0";
    if (calc.display.find('.') == std::string::npos) appendDisplay(".");
}

static void onClearEntry() { setDisplay("0"); }

static void onClearAll() {
    setDisplay("0");
    calc.storedValue = 0.0;
    calc.pendingOp = 0;
    calc.waitingForOperand = false;
    calc.justCalculated = false;
}

static void onBackspace() {
    if (calc.justCalculated || calc.waitingForOperand) return;
    if (!calc.display.empty()) calc.display.pop_back();
    if (calc.display.empty()) calc.display = "0";
    SetWindowTextA(calc.hDisplay, calc.display.c_str());
}

static void performPending() {
    if (calc.pendingOp == 0) return;
    double a = calc.storedValue;
    double b = currentValue();
    double result = 0.0;
    bool ok = true;
    switch (calc.pendingOp) {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': result = a * b; break;
        case '/': if (b == 0.0) ok = false; else result = a / b; break;
        case '^': result = std::pow(a, b); break;
        case '%': if (b == 0.0) ok = false; else result = std::fmod(a, b); break;
        default: ok = false; break;
    }
    if (!ok) { setDisplay("Error"); calc.pendingOp = 0; calc.waitingForOperand = false; return; }
    setAns(result);
    recordHistory(formatNumber(a) + " " + calc.pendingOp + " " + formatNumber(b) + " = " + formatNumber(result));
    setDisplay(formatNumber(result));
    calc.storedValue = result;
    calc.pendingOp = 0;
    calc.waitingForOperand = false;
    calc.justCalculated = true;
}

static void onOp(char op) {
    if (!calc.waitingForOperand) {
        performPending();
        calc.storedValue = currentValue();
    }
    calc.pendingOp = op;
    calc.waitingForOperand = true;
    calc.justCalculated = false;
}

static void onEquals() {
    performPending();
    calc.waitingForOperand = false;
    calc.justCalculated = true;
}

static void onUnary(double (*func)(double), const char* prefix) {
    double v = currentValue();
    double r = func(v);
    if (std::isnan(r) || std::isinf(r)) { setDisplay("Error"); return; }
    setAns(r);
    recordHistory(std::string(prefix) + formatNumber(v) + ") = " + formatNumber(r));
    setDisplay(formatNumber(r));
    calc.justCalculated = true;
}

static void onSqrt()  { onUnary(std::sqrt,  "sqrt("); }
static void onX2()    { double v = currentValue(); double r = v * v; setAns(r); recordHistory(formatNumber(v) + "² = " + formatNumber(r)); setDisplay(formatNumber(r)); calc.justCalculated = true; }
static void onPow()   { onOp('^'); }
static void onMod()   { onOp('%'); }
static void onInv()   { double v = currentValue(); if (v == 0.0) { setDisplay("Error"); return; } double r = 1.0 / v; setAns(r); recordHistory("1/" + formatNumber(v) + " = " + formatNumber(r)); setDisplay(formatNumber(r)); calc.justCalculated = true; }
static void onSign()  {
    if (calc.display == "Error") return;
    if (!calc.display.empty() && calc.display[0] == '-') calc.display.erase(0, 1);
    else if (!calc.display.empty() && calc.display != "0") calc.display = "-" + calc.display;
    SetWindowTextA(calc.hDisplay, calc.display.c_str());
}
static void onSin()   { onUnary([](double d){ return std::sin(d * 3.14159265358979323846 / 180.0); }, "sin("); }
static void onCos()   { onUnary([](double d){ return std::cos(d * 3.14159265358979323846 / 180.0); }, "cos("); }
static void onTan()   { onUnary([](double d){ return std::tan(d * 3.14159265358979323846 / 180.0); }, "tan("); }
static void onLog10() { onUnary(std::log10, "log10("); }
static void onLn()    { onUnary(std::log,   "ln("); }
static void onFact()  {
    double input = currentValue();
    if (input < 0 || input != std::floor(input) || input > 20) { setDisplay("Error"); return; }
    long n = (long)input;
    unsigned long long result = 1;
    for (long i = 2; i <= n; ++i) result *= (unsigned long long)i;
    setAns((double)result);
    recordHistory(std::to_string(n) + "! = " + formatNumber((double)result));
    setDisplay(formatNumber((double)result));
    calc.justCalculated = true;
}

// Memory
static void onMC()  { calc.memory = 0.0; }
static void onMSub() { calc.memory -= currentValue(); }
static void onMAdd() { calc.memory += currentValue(); }
static void onMR()  { setDisplay(formatNumber(calc.memory)); calc.justCalculated = true; }

// Ans
static void onAns() {
    if (!calc.hasAns) return;
    if (calc.justCalculated || calc.waitingForOperand) { setDisplay(""); calc.justCalculated = false; calc.waitingForOperand = false; }
    appendDisplay(formatNumber(calc.ans));
}

// Theme
static void applyTheme() {
    InvalidateRect(calc.hWnd, nullptr, TRUE);
    if (calc.hDisplay) {
        InvalidateRect(calc.hDisplay, nullptr, TRUE);
    }
}

static void onThemeNext() {
    calc.themeIndex = (calc.themeIndex + 1) % NUM_THEMES;
    applyTheme();
}

// History window
static void showHistory() {
    if (calc.hHistoryWnd && IsWindow(calc.hHistoryWnd)) {
        SetForegroundWindow(calc.hHistoryWnd);
        return;
    }
    calc.hHistoryWnd = CreateWindowA("STATIC", "Calculation History", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 420, 380, calc.hWnd, nullptr, GetModuleHandle(nullptr), nullptr);
    HWND hList = CreateWindowA("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
        10, 10, 380, 270, calc.hHistoryWnd, (HMENU)(INT_PTR)IDC_HIST_LIST, GetModuleHandle(nullptr), nullptr);
    for (auto& e : calc.history) SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)e.c_str());
    HWND hClose = CreateWindowA("BUTTON", "Close", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        160, 290, 100, 32, calc.hHistoryWnd, (HMENU)(INT_PTR)IDC_HIST_CLOSE, GetModuleHandle(nullptr), nullptr);
}

static void showAbout() {
    MessageBoxA(calc.hWnd, "C++ Calculator (GUI)\n\nBasic, scientific, memory, history, themes.\nBuilt with the Win32 API.", "About Calculator", MB_OK | MB_ICONINFORMATION);
}

// ---------------------------------------------------------------------------
// UI creation
// ---------------------------------------------------------------------------
static HWND createButton(HWND hParent, int id, const char* text, int x, int y, int w, int h) {
    return CreateWindowA("BUTTON", text, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        x, y, w, h, hParent, (HMENU)(INT_PTR)id, GetModuleHandle(nullptr), nullptr);
}

static void createControls(HWND hWnd) {
    calc.hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    calc.hBigFont = CreateFontA(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

    // Display
    calc.hDisplay = CreateWindowA("EDIT", "0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT | ES_READONLY | ES_AUTOHSCROLL,
        10, 10, 560, 48, hWnd, (HMENU)(INT_PTR)IDC_DISPLAY, GetModuleHandle(nullptr), nullptr);
    SendMessageA(calc.hDisplay, WM_SETFONT, (WPARAM)calc.hBigFont, TRUE);

    int bw = 72, bh = 48, gap = 8, x0 = 10, y0 = 70;

    // Row 1: memory + clear
    createButton(hWnd, BTN_MC,   "MC",   x0, y0, bw, bh);
    createButton(hWnd, BTN_MMINUS,"M-",  x0 + (bw+gap), y0, bw, bh);
    createButton(hWnd, BTN_MPLUS,"M+",  x0 + 2*(bw+gap), y0, bw, bh);
    createButton(hWnd, BTN_MR,   "MR",  x0 + 3*(bw+gap), y0, bw, bh);
    createButton(hWnd, BTN_CE,   "CE",  x0 + 4*(bw+gap), y0, bw, bh);
    createButton(hWnd, BTN_C,    "C",   x0 + 5*(bw+gap), y0, bw, bh);
    createButton(hWnd, BTN_BACK, "←",   x0 + 6*(bw+gap), y0, bw, bh);

    // Row 2
    createButton(hWnd, BTN_7,   "7",  x0, y0 + (bh+gap), bw, bh);
    createButton(hWnd, BTN_8,   "8",  x0 + (bw+gap), y0 + (bh+gap), bw, bh);
    createButton(hWnd, BTN_9,   "9",  x0 + 2*(bw+gap), y0 + (bh+gap), bw, bh);
    createButton(hWnd, BTN_DIV, "/",  x0 + 3*(bw+gap), y0 + (bh+gap), bw, bh);
    createButton(hWnd, BTN_SQRT,"√",  x0 + 4*(bw+gap), y0 + (bh+gap), bw, bh);
    createButton(hWnd, BTN_X2,  "x²", x0 + 5*(bw+gap), y0 + (bh+gap), bw, bh);
    createButton(hWnd, BTN_POW, "x^y",x0 + 6*(bw+gap), y0 + (bh+gap), bw, bh);

    // Row 3
    createButton(hWnd, BTN_4,   "4",  x0, y0 + 2*(bh+gap), bw, bh);
    createButton(hWnd, BTN_5,   "5",  x0 + (bw+gap), y0 + 2*(bh+gap), bw, bh);
    createButton(hWnd, BTN_6,   "6",  x0 + 2*(bw+gap), y0 + 2*(bh+gap), bw, bh);
    createButton(hWnd, BTN_MUL, "*",  x0 + 3*(bw+gap), y0 + 2*(bh+gap), bw, bh);
    createButton(hWnd, BTN_MOD, "%",  x0 + 4*(bw+gap), y0 + 2*(bh+gap), bw, bh);
    createButton(hWnd, BTN_INV, "1/x",x0 + 5*(bw+gap), y0 + 2*(bh+gap), bw, bh);
    createButton(hWnd, BTN_SIGN,"+/-",x0 + 6*(bw+gap), y0 + 2*(bh+gap), bw, bh);

    // Row 4
    createButton(hWnd, BTN_1,   "1",  x0, y0 + 3*(bh+gap), bw, bh);
    createButton(hWnd, BTN_2,   "2",  x0 + (bw+gap), y0 + 3*(bh+gap), bw, bh);
    createButton(hWnd, BTN_3,   "3",  x0 + 2*(bw+gap), y0 + 3*(bh+gap), bw, bh);
    createButton(hWnd, BTN_SUB, "-",  x0 + 3*(bw+gap), y0 + 3*(bh+gap), bw, bh);
    createButton(hWnd, BTN_SIN, "sin",x0 + 4*(bw+gap), y0 + 3*(bh+gap), bw, bh);
    createButton(hWnd, BTN_COS, "cos",x0 + 5*(bw+gap), y0 + 3*(bh+gap), bw, bh);
    createButton(hWnd, BTN_TAN, "tan",x0 + 6*(bw+gap), y0 + 3*(bh+gap), bw, bh);

    // Row 5
    createButton(hWnd, BTN_0,   "0",  x0, y0 + 4*(bh+gap), bw, bh);
    createButton(hWnd, BTN_DOT, ".",  x0 + (bw+gap), y0 + 4*(bh+gap), bw, bh);
    createButton(hWnd, BTN_EQ,  "=",  x0 + 2*(bw+gap), y0 + 4*(bh+gap), bw, bh);
    createButton(hWnd, BTN_ADD, "+",  x0 + 3*(bw+gap), y0 + 4*(bh+gap), bw, bh);
    createButton(hWnd, BTN_LOG, "log",x0 + 4*(bw+gap), y0 + 4*(bh+gap), bw, bh);
    createButton(hWnd, BTN_LN,  "ln", x0 + 5*(bw+gap), y0 + 4*(bh+gap), bw, bh);
    createButton(hWnd, BTN_FACT,"n!", x0 + 6*(bw+gap), y0 + 4*(bh+gap), bw, bh);

    // Row 6
    createButton(hWnd, BTN_ANS, "Ans", x0, y0 + 5*(bh+gap), bw, bh);
    createButton(hWnd, BTN_HIST,"Hist",x0 + (bw+gap), y0 + 5*(bh+gap), bw, bh);
    createButton(hWnd, BTN_THEME,"Theme",x0 + 2*(bw+gap), y0 + 5*(bh+gap), bw, bh);
    createButton(hWnd, BTN_ABOUT,"About",x0 + 3*(bw+gap), y0 + 5*(bh+gap), bw, bh);
    createButton(hWnd, BTN_EXIT,"Exit",x0 + 4*(bw+gap), y0 + 5*(bh+gap), bw, bh);

    // Menu
    HMENU hMenu = CreateMenu();
    HMENU hFile = CreateMenu();
    AppendMenuA(hFile, MF_STRING, IDM_EXIT, "E&xit");
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hFile, "&File");

    HMENU hView = CreateMenu();
    AppendMenuA(hView, MF_STRING, IDM_HISTORY, "&History");
    AppendMenuA(hView, MF_STRING, IDM_THEME, "&Theme");
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hView, "&View");

    HMENU hHelp = CreateMenu();
    AppendMenuA(hHelp, MF_STRING, IDM_ABOUT, "&About");
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hHelp, "&Help");

    SetMenu(hWnd, hMenu);
}

// ---------------------------------------------------------------------------
// Window procedure
// ---------------------------------------------------------------------------
static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            calc.hWnd = hWnd;
            createControls(hWnd);
            setDisplay("0");
            break;

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            switch (id) {
                case BTN_0: onDigit('0'); break;
                case BTN_1: onDigit('1'); break;
                case BTN_2: onDigit('2'); break;
                case BTN_3: onDigit('3'); break;
                case BTN_4: onDigit('4'); break;
                case BTN_5: onDigit('5'); break;
                case BTN_6: onDigit('6'); break;
                case BTN_7: onDigit('7'); break;
                case BTN_8: onDigit('8'); break;
                case BTN_9: onDigit('9'); break;
                case BTN_DOT: onDecimal(); break;
                case BTN_ADD: onOp('+'); break;
                case BTN_SUB: onOp('-'); break;
                case BTN_MUL: onOp('*'); break;
                case BTN_DIV: onOp('/'); break;
                case BTN_POW: onPow(); break;
                case BTN_MOD: onMod(); break;
                case BTN_EQ: onEquals(); break;
                case BTN_CE: onClearEntry(); break;
                case BTN_C: onClearAll(); break;
                case BTN_BACK: onBackspace(); break;
                case BTN_SQRT: onSqrt(); break;
                case BTN_X2: onX2(); break;
                case BTN_INV: onInv(); break;
                case BTN_SIGN: onSign(); break;
                case BTN_SIN: onSin(); break;
                case BTN_COS: onCos(); break;
                case BTN_TAN: onTan(); break;
                case BTN_LOG: onLog10(); break;
                case BTN_LN: onLn(); break;
                case BTN_FACT: onFact(); break;
                case BTN_MC: onMC(); break;
                case BTN_MMINUS: onMSub(); break;
                case BTN_MPLUS: onMAdd(); break;
                case BTN_MR: onMR(); break;
                case BTN_ANS: onAns(); break;
                case BTN_HIST: showHistory(); break;
                case BTN_THEME: onThemeNext(); break;
                case BTN_ABOUT: showAbout(); break;
                case BTN_EXIT: PostMessage(hWnd, WM_CLOSE, 0, 0); break;
                case IDM_EXIT: PostMessage(hWnd, WM_CLOSE, 0, 0); break;
                case IDM_HISTORY: showHistory(); break;
                case IDM_THEME: onThemeNext(); break;
                case IDM_ABOUT: showAbout(); break;
            }
            break;
        }

        case WM_CTLCOLOREDIT: {
            if ((HWND)lParam == calc.hDisplay) {
                HDC hdc = (HDC)wParam;
                Theme& t = themes[calc.themeIndex];
                SetTextColor(hdc, t.displayText);
                SetBkColor(hdc, t.displayBg);
                return (LRESULT)CreateSolidBrush(t.displayBg);
            }
            break;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

// ---------------------------------------------------------------------------
// History window procedure
// ---------------------------------------------------------------------------
static LRESULT CALLBACK HistoryWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == IDC_HIST_CLOSE) {
                calc.hHistoryWnd = nullptr;
                DestroyWindow(hWnd);
            }
            break;
        case WM_CLOSE:
            calc.hHistoryWnd = nullptr;
            DestroyWindow(hWnd);
            break;
        case WM_DESTROY:
            calc.hHistoryWnd = nullptr;
            break;
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    InitCommonControls();

    const char* CLASS_NAME = "CalculatorWindowClass";

    WNDCLASSA wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassA(&wc);

    HWND hWnd = CreateWindowA(
        CLASS_NAME, "C++ Calculator",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 540,
        nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) return 0;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}