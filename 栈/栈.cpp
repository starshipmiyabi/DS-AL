// SPDX-License-Identifier: CC-BY-NC-SA-4.0
/*
 * 文件名: 栈.cpp
 * 目标: 针对“栈 Stack”知识点做深度学习式实现与注释，形成可编译、可复用的示例代码。
 * 主要参考（页码来自讲义页脚编号）：
 *   - 栈的定义/特性与基本操作：P6、P7–P11 —— 只允许在一端进行插入/删除，后进先出 LIFO。
 *   - 顺序栈（数组实现）接口与示意：P13–P19；溢出及“双栈共享空间”思路：P20。
 *   - 链式栈：概念与实现片段：P22–P29（栈顶在链头；Push/Pop/Top/Traverse 等）。
 *   - 应用1：括号匹配（例3.2）：P30–P31。
 *   - 应用2：中缀表达式求值（两个栈 + Isp/Icp 优先级表）：P60–P64，优先级表见 P62。
 * 说明：在不改变教材语义的前提下做工程化补充（如顺序栈可选自动扩容），并在注释中标注出处页码。
 * 教材来源：四川大学计算机学院 孙奕髦《Chapter 3：Stack & Queue》— “栈 Stack”部分。
 */

#include <iostream>
#include <vector>
#include <functional>
#include <stdexcept>
#include <string>
#include <cctype>
#include <limits>
#include <utility>

namespace ds {

// ============================== 顺序栈（数组实现） ==============================
// 设计依据：P13–P19；count 记录元素个数，栈顶在数组尾部；基本操作同 P7–P11。
// 工程改进：当容量满时可选择“自动扩容”（教材 P20 讨论了溢出、双栈共享空间；此处增加动态扩容选项）。

template <typename T>
class ArrayStack {
public:
    explicit ArrayStack(int init_capacity = 16, bool auto_expand = true)
        : _cap(init_capacity > 0 ? init_capacity : 16),
          _auto_expand(auto_expand),
          _count(0),
          _data(_cap) // 预分配
    {
        // P6：栈只在一端操作（本实现：尾端为栈顶 top）
    }

    // 拷贝/赋值遵循值语义
    ArrayStack(const ArrayStack& other)
        : _cap(other._cap), _auto_expand(other._auto_expand), _count(other._count), _data(other._data) {}

    ArrayStack& operator=(const ArrayStack& other) {
        if (this != &other) {
            _cap = other._cap;
            _auto_expand = other._auto_expand;
            _count = other._count;
            _data = other._data;
        }
        return *this;
    }

    // 基本操作接口 —— 对应教材 P7–P11 的语义
    int Length() const { return _count; }                 // P7 (1)
    bool Empty() const { return _count == 0; }            // P7 (2)
    void Clear() { _count = 0; }                          // P7 (3)

    // 遍历：从栈底到栈顶（P8 (4)）
    void Traverse(const std::function<void(const T&)>& visit) const {
        for (int i = 0; i < _count; ++i) visit(_data[i]);
    }

    // 入栈：在栈顶压入新元素（P8 (5)）
    bool Push(const T& e) {
        if (_count == _cap) {
            if (_auto_expand) {
                grow();
            } else {
                // 不自动扩容：模拟“顺序栈满”的情况（对应 P18/P20）
                return false;
            }
        }
        _data[_count++] = e;
        return true;
    }

    // 取栈顶：只读（P9 (6)）
    bool Top(T& e) const {
        if (Empty()) return false;
        e = _data[_count - 1];
        return true;
    }

    // 出栈（带返回值版本，P10 (7)）
    bool Pop(T& e) {
        if (Empty()) return false;
        e = _data[--_count];
        return true;
    }

    // 出栈（不返回值版本，P11 (8)）
    bool Pop() {
        if (Empty()) return false;
        --_count;
        return true;
    }

private:
    int _cap;
    bool _auto_expand;
    int _count;
    std::vector<T> _data;

    void grow() {
        // 工程化扩容策略：2 倍扩容
        int new_cap = _cap * 2;
        std::vector<T> nd(new_cap);
        for (int i = 0; i < _count; ++i) nd[i] = std::move(_data[i]);
        _data.swap(nd);
        _cap = new_cap;
    }
};

// ============================== 链式栈（单链表实现） ==============================
// 设计依据：P22–P29；“链式栈无栈满问题，栈顶在链头，插删仅在栈顶进行”（P22）。
template <typename T>
class LinkedStack {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& d, Node* n) : data(d), next(n) {}
    };

    Node* _top = nullptr; // 栈顶指针（P23）
    int _count = 0;       // 元素个数（P23）

public:
    LinkedStack() = default;              // P25 构造：空栈
    ~LinkedStack() { Clear(); }           // P25 析构：清空

    // 拷贝/赋值：深拷贝
    LinkedStack(const LinkedStack& other) {
        if (other._top == nullptr) return;
        // 为保持从栈底到栈顶相对顺序，先反向收集再逐个 Push
        std::vector<T> buf;
        for (Node* p = other._top; p; p = p->next) buf.push_back(p->data);
        for (auto it = buf.rbegin(); it != buf.rend(); ++it) Push(*it);
    }

    LinkedStack& operator=(const LinkedStack& other) {
        if (this != &other) {
            Clear();
            LinkedStack tmp(other);
            // 复制-交换
            std::swap(_top, tmp._top);
            std::swap(_count, tmp._count);
        }
        return *this;
    }

    int Length() const { return _count; }          // P23/P24
    bool Empty()  const { return _top == nullptr; } // P24
    void Clear() {                                  // P24
        while (_top) {
            Node* o = _top; _top = _top->next; delete o;
        }
        _count = 0;
    }

    // P26 Push
    bool Push(const T& e) {
        Node* nt = new (std::nothrow) Node(e, _top);
        if (!nt) return false; // 动态内存耗尽（P26）
        _top = nt; ++_count;
        return true;
    }

    // P28 Top
    bool Top(T& e) const {
        if (Empty()) return false;
        e = _top->data;
        return true;
    }

    // P27 Pop（带返回值）
    bool Pop(T& e) {
        if (Empty()) return false;
        Node* ot = _top;
        e = ot->data;
        _top = ot->next;
        delete ot; --_count;
        return true;
    }

    // Pop（不返回值）
    bool Pop() {
        if (Empty()) return false;
        Node* ot = _top;
        _top = ot->next;
        delete ot; --_count;
        return true;
    }

    // P29 Traverse 思想：从栈底到栈顶遍历（这里用 O(n) 额外空间简化实现）
    void Traverse(const std::function<void(const T&)>& visit) const {
        std::vector<T> rev;
        rev.reserve(_count);
        for (Node* p = _top; p; p = p->next) rev.push_back(p->data);
        for (auto it = rev.rbegin(); it != rev.rend(); ++it) visit(*it);
    }
};

// ============================== 双栈共享一个顺序存储空间（扩展演示） ==============================
// 教材 P20 的思路：两个栈自数组两端向中间生长，最大化共享空间，缓解“单栈溢出”。

template <typename T>
class DualStack {
public:
    explicit DualStack(int capacity = 16)
        : _cap(capacity > 0 ? capacity : 16), _data(_cap), _ltop(-1), _rtop(_cap) {}

    bool pushLeft (const T& e)  { if (full()) return false; _data[++_ltop] = e; return true; }
    bool pushRight(const T& e)  { if (full()) return false; _data[--_rtop] = e; return true; }

    bool popLeft (T& e)  { if (_ltop < 0) return false; e = _data[_ltop--]; return true; }
    bool popRight(T& e)  { if (_rtop >= _cap) return false; e = _data[_rtop++]; return true; }

    bool topLeft (T& e) const  { if (_ltop < 0) return false; e = _data[_ltop]; return true; }
    bool topRight(T& e) const  { if (_rtop >= _cap) return false; e = _data[_rtop]; return true; }

    bool emptyLeft () const { return _ltop < 0; }
    bool emptyRight() const { return _rtop >= _cap; }

private:
    int _cap;
    std::vector<T> _data;
    int _ltop; // 左栈顶（从 -1 向右增长）
    int _rtop; // 右栈顶（从 cap 向左增长）

    bool full() const { return _ltop + 1 == _rtop; } // 两顶指针相邻即满（见 P20 示意）
};

// ============================== 应用 1：括号匹配（例 3.2） ==============================
// 依据 P30–P31 的构思：遇左括号入栈，遇右括号检查类型并弹出。
inline bool is_left_bracket(char c)  { return c=='(' || c=='[' || c=='{'; }
inline bool is_right_bracket(char c) { return c==')' || c==']' || c=='}'; }
inline bool match_pair(char l, char r){
    return (l=='(' && r==')') || (l=='[' && r==']') || (l=='{' && r=='}');
}

// 返回：true 表示匹配；false 表示失配；若失配，reason 描述失败原因/位置。
bool bracket_match(const std::string& s, std::string& reason) {
    ArrayStack<char> st(64, true); // 自动扩容，不需担心“栈满”
    int pos = 0;
    for (char ch : s) {
        ++pos;
        if (is_left_bracket(ch)) {
            st.Push(ch);
        } else if (is_right_bracket(ch)) {
            char top;
            if (!st.Pop(top)) { // 右括号多
                reason = "右括号多，位置 " + std::to_string(pos);
                return false;
            }
            if (!match_pair(top, ch)) {
                reason = std::string("括号不匹配：期望 '") +
                         (top=='(' ? ')' : top=='[' ? ']' : '}') + "'，但遇到 '" + ch +
                         "'（位置 " + std::to_string(pos) + ")";
                return false;
            }
        }
    }
    if (!st.Empty()) { // 左括号多
        reason = "左括号多（有未配对的左括号）。";
        return false;
    }
    return true;
}

// ============================== 应用 2：中缀表达式求值 ==============================
// 依据 P60–P64：两个栈（操作数 opnd / 操作符 optr）；Isp/Icp 优先级表（P62）。
// 简化假设：整数四则运算，支持 + - * / % ()，忽略一元负号；除零报错。

inline bool is_op(char ch) {
    return ch=='+' || ch=='-' || ch=='*' || ch=='/' || ch=='%' || ch=='(' || ch==')' || ch=='=';
}

// Isp: in-stack priority（栈内）；Icp: in-coming priority（栈外）。参见 P62。
inline int Isp(char op) {
    switch (op) {
        case '=': return 0;
        case '(': return 1;
        case '*': case '/': case '%': return 5;
        case '+': case '-': return 3;
        case ')': return 6;
        default : return -1;
    }
}

inline int Icp(char op) {
    switch (op) {
        case '=': return 0;
        case '(': return 6;
        case '*': case '/': case '%': return 4;
        case '+': case '-': return 2;
        case ')': return 1;
        default : return -1;
    }
}

long long apply_op(long long a1, long long a2, char op, bool& ok) {
    switch (op) {
        case '+': return a1 + a2;
        case '-': return a1 - a2;
        case '*': return a1 * a2;
        case '/': if (a2 == 0) { ok = false; return 0; } return a1 / a2;
        case '%': if (a2 == 0) { ok = false; return 0; } return a1 % a2;
        default : ok = false; return 0;
    }
}

// 返回 true 表示求值成功；out 写入结果。失败时返回 false。
bool eval_infix(const std::string& expr, long long& out, std::string& err) {
    ArrayStack<char> optr(32, true);
    ArrayStack<long long> opnd(64, true);
    optr.Push('='); // P60 step 1
    std::string s = expr;
    s.push_back('='); // 末尾补 '='（P60）

    auto read_number = [&](size_t& i) -> bool {
        // 读一个非负整数
        long long val = 0;
        bool has_digit = false;
        while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
        while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) {
            has_digit = true;
            int d = s[i] - '0';
            // 简单溢出保护（不做严格检查）
            if (val > (std::numeric_limits<long long>::max() - d) / 10) {
                err = "数字过大";
                return false;
            }
            val = val * 10 + d;
            ++i;
        }
        if (!has_digit) return false;
        opnd.Push(val);
        return true;
    };

    size_t i = 0;
    char ch = 0, top = 0;
    while (true) {
        // 读下一个 token
        while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
        if (i < s.size()) ch = s[i];
        else ch = '='; // 容错

        // 步骤 P61：非操作符则回退一个字符，读操作数并压栈
        if (!is_op(ch)) {
            if (!read_number(i)) { err = "语法错误：期望数字"; return false; }
            continue; // 继续读取下一个 token
        }

        // 这里 ch 是操作符
        // 获取当前栈顶操作符
        if (!optr.Top(top)) { err = "内部错误：操作符栈空"; return false; }

        // 循环直到结束条件（P60 step 4）
        if (top == '=' && ch == '=') break;

        // 比较 Isp 与 Icp（P64 版本的写法）
        int in_stack = Isp(top);
        int in_coming = Icp(ch);

        if (in_stack < in_coming) {
            // ch 进 optr（P64：Isp(top) < Icp(ch)）
            optr.Push(ch);
            ++i; // 消费 ch
        } else if (in_stack > in_coming) {
            // 计算一次：从 opnd 退 a2、a1；从 optr 退 θ；(a1)θ(a2) 入栈（P64）
            char theta;
            if (!optr.Pop(theta)) { err = "内部错误：弹出操作符失败"; return false; }
            long long a2, a1;
            if (!opnd.Pop(a2) || !opnd.Pop(a1)) { err = "语法错误：操作数不足"; return false; }
            bool ok = true;
            long long r = apply_op(a1, a2, theta, ok);
            if (!ok) { err = "非法运算（如除以零）"; return false; }
            opnd.Push(r);
        } else {
            // in_stack == in_coming
            if (ch == ')') {
                // 对消括号（P61/P64）
                char left;
                if (!optr.Pop(left) || left != '(') { err = "括号不匹配"; return false; }
                ++i; // 消费 ')'
            } else if (ch == '=') {
                // 匹配到结尾 '='：弹出 '='（P62 注释）
                optr.Pop(); // 弹出栈底 '='
                // 注意：此分支只在 top == '=' && ch == '=' 时发生，但上面 break 已处理
            } else {
                // 其它“相等”情况不应出现
                err = "语法错误（优先级相等的非法组合）";
                return false;
            }
        }
    }

    // 结果在 opnd 栈顶（P60）
    if (!opnd.Top(out) || opnd.Length() != 1) {
        err = "语法错误：表达式不完整";
        return false;
    }
    return true;
}

} // namespace ds

// ============================== Demo & 自测 ==============================
// 说明：仅用于演示 API 用法，可据需要删除 main。
int main() {
    using namespace ds;

    // --- 顺序栈演示（P13–P19） ---
    ArrayStack<int> s(2, true); // 小容量 + 自动扩容
    s.Push(1); s.Push(2); s.Push(3);
    std::cout << "[ArrayStack] Length=" << s.Length() << " Top=";
    int tv; s.Top(tv); std::cout << tv << "\n";

    // --- 链式栈演示（P22–P29） ---
    LinkedStack<std::string> ls;
    ls.Push("a"); ls.Push("b"); ls.Push("c");
    std::cout << "[LinkedStack] Traverse from bottom to top: ";
    ls.Traverse([](const std::string& e){ std::cout << e << ' '; });
    std::cout << "\n";

    // --- 双栈共享空间（P20） ---
    DualStack<int> ds2(5);
    ds2.pushLeft(1); ds2.pushRight(9);
    int x,y; ds2.topLeft(x); ds2.topRight(y);
    std::cout << "[DualStack] leftTop=" << x << " rightTop=" << y << "\n";

    // --- 括号匹配（例 3.2，P30–P31） ---
    std::string reason;
    std::string ok1 = "([{}])";
    std::string bad1 = "([)]";
    bool m1 = bracket_match(ok1, reason);
    std::cout << "[Bracket] \"" << ok1 << "\" -> " << (m1 ? "OK" : ("FAIL: " + reason)) << "\n";
    bool m2 = bracket_match(bad1, reason);
    std::cout << "[Bracket] \"" << bad1 << "\" -> " << (m2 ? "OK" : ("FAIL: " + reason)) << "\n";

    // --- 中缀表达式求值（P60–P64；P57 示例 4 + 2 × 3 − 10 / 5 = 8） ---
    long long res; std::string err;
    bool ok = eval_infix("4 + 2 * 3 - 10 / 5", res, err);
    std::cout << "[Eval] 4 + 2 * 3 - 10 / 5 = " << (ok ? std::to_string(res) : ("ERROR: " + err)) << "\n";

    return 0;
}

/*
 * 编译方法（示例）：
 *   g++ -std=gnu++17 -O2 -Wall -Wextra -pedantic 栈.cpp -o stack_demo
 * 运行：
 *   ./stack_demo
 *
 * 设计要点回顾（含页码）：
 *   - LIFO/栈顶/栈底/单口操作：P6。
 *   - 基本操作接口（Length/Empty/Clear/Traverse/Push/Top/Pop）：P7–P11。
 *   - 顺序栈：P13–P19；溢出与双栈共享空间：P20。
 *   - 链式栈：P22–P29（Push/Pop/Top/Traverse 过程）。
 *   - 例 3.2 括号匹配思路：P30–P31。
 *   - 中缀表达式求值流程与 Isp/Icp 表：P60–P64（表见 P62）。
 */
