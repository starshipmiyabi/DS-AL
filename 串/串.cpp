// ===============================================
// 数据结构与算法分析 —— Chapter 4 串（String）
// 可执行示例整合版：按课件顺序实现所有核心概念、方法与示例
// 文件名：串.cpp
//
// 说明：
// 1) 本文件将课件“Ch04 串.pdf”中的内容，按 4.1 → 4.2 → 4.3 → 4.4 的顺序
//    实现为一个可编译运行的单文件程序，便于初学者逐段学习与动手实验。
// 2) 为了自包含并可直接编译，本文件补齐了课件中引用到的 LinkList 与
//    DblLinkList（简化实现），并完整实现 CharString 类、C 风格字符串
//    函数（CStrCopy/CStrConcat）、暴力匹配与 KMP、以及一个简易行文本编辑器。
// 3) 注释非常详细，适合入门读者逐行阅读。
// 4) 编译方式（任选一种编译器）：
//      g++ -std=c++17 -O2 -o string_demo 串.cpp
//      clang++ -std=c++17 -O2 -o string_demo 串.cpp
//      (Windows/Visual Studio) 新建空项目后添加本文件编译运行。
//
// 参考与页码标注：
// - 4.1 串类型的定义（P4–P9）
// - 4.2 字符串的实现（P10–P42），包括 CharString 类与自定义 C 字符串函数
// - 4.3 字符串模式匹配算法（P43–P60）：暴力匹配与 KMP（含 next 的求法）
// - 4.4* 文本编辑器实例（P61–P88）：主程序、Editor 类、命令处理与插入示例
//
// 提示：
// - 运行程序后，会先自动演示 4.1–4.3 的示例，然后进入 4.4 文本编辑器交互。
// - 文本编辑器默认输入/输出文件为 file_in.txt / file_out.txt（可在启动时输入覆盖）。
// ===============================================

#include <iostream>
#include <iomanip>
#include <cstring> // 为了和课件一致地演示，仍包含 <cstring>（但我们自写 CStrCopy/CStrConcat）
#include <cctype>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
using namespace std;

// =============================================================
// 4.1 串的基本概念与术语（P4–P6）
// 在代码层面体现在 CharString 类的基本属性与行为上。
// =============================================================

// =============================================================
// [辅] 简单单链表 LinkList<T>（仅用于 CharString 的构造演示）
// 课件 P20：CharString(LinkList<char>&) —— 这里给出最小可用实现
// - 采用 1-based 下标（与课件一致）：GetElem(pos, out) 中 pos 从 1 开始。
// =============================================================
template <typename T>
class LinkList
{
    struct Node
    {
        T data;
        Node *next;
        Node(const T &d, Node *n = nullptr) : data(d), next(n) {}
    };
    Node *head; // 头结点（不存数据）
    int sz;

public:
    LinkList() : head(new Node(T())), sz(0) {}
    ~LinkList()
    {
        clear();
        delete head;
    }

    void clear()
    {
        Node *p = head->next;
        while (p)
        {
            Node *q = p->next;
            delete p;
            p = q;
        }
        head->next = nullptr;
        sz = 0;
    }
    int Length() const { return sz; }

    // 在表尾插入（便于构造）
    void PushBack(const T &x)
    {
        Node *p = head;
        while (p->next)
            p = p->next;
        p->next = new Node(x);
        ++sz;
    }

    // 1-based：获取第 pos 个元素到 out
    bool GetElem(int pos, T &out) const
    {
        if (pos < 1 || pos > sz)
            return false;
        Node *p = head->next;
        for (int i = 1; i < pos; ++i)
            p = p->next;
        out = p->data;
        return true;
    }
};

// =============================================================
// [辅] 双向链表 DblLinkList<T>（用于 4.4 文本编辑器保存"行"）
// 课件 P71–P73 中 Editor 依赖 DblLinkList<CharString>：
// 需要的接口：Empty/Length/GetElem/Insert/Delete（均 1-based）。
// =============================================================
template <typename T>
class DblLinkList
{
    struct Node
    {
        T data;
        Node *prev;
        Node *next;
        Node(const T &d = T()) : data(d), prev(nullptr), next(nullptr) {}
    };
    Node *head; // 哨兵头
    Node *tail; // 哨兵尾
    int sz;

public:
    DblLinkList() : sz(0)
    {
        head = new Node();
        tail = new Node();
        head->next = tail;
        tail->prev = head;
    }
    ~DblLinkList()
    {
        clear();
        delete head;
        delete tail;
    }

    void clear()
    {
        Node *p = head->next;
        while (p != tail)
        {
            Node *q = p->next;
            delete p;
            p = q;
        }
        head->next = tail;
        tail->prev = head;
        sz = 0;
    }
    bool Empty() const { return sz == 0; }
    int Length() const { return sz; }

    // 在第 pos 位置插入（1..sz+1），pos==sz+1 表示尾插
    bool Insert(int pos, const T &val)
    {
        if (pos < 1 || pos > sz + 1)
            return false;
        Node *p = head;
        for (int i = 1; i < pos; ++i)
            p = p->next; // 停在 pos-1
        Node *node = new Node(val);
        Node *right = p->next;
        p->next = node;
        node->prev = p;
        node->next = right;
        right->prev = node;
        ++sz;
        return true;
    }
    // 获取第 pos 个元素到 out（1..sz）
    bool GetElem(int pos, T &out) const
    {
        if (pos < 1 || pos > sz)
            return false;
        Node *p = head->next;
        for (int i = 1; i < pos; ++i)
            p = p->next;
        out = p->data;
        return true;
    }
    // 用 val 替换第 pos 个元素（便于“改行”）
    bool Replace(int pos, const T &val)
    {
        if (pos < 1 || pos > sz)
            return false;
        Node *p = head->next;
        for (int i = 1; i < pos; ++i)
            p = p->next;
        p->data = val;
        return true;
    }
    // 删除第 pos 个元素（1..sz）
    bool Delete(int pos)
    {
        if (pos < 1 || pos > sz)
            return false;
        Node *p = head->next;
        for (int i = 1; i < pos; ++i)
            p = p->next;
        Node *L = p->prev;
        Node *R = p->next;
        L->next = R;
        R->prev = L;
        delete p;
        --sz;
        return true;
    }
    // 遍历辅助（只读）
    template <typename Fn>
    void ForEach(Fn f) const
    {
        Node *p = head->next;
        int idx = 1;
        while (p != tail)
        {
            f(idx, p->data);
            p = p->next;
            ++idx;
        }
    }
};

// =============================================================
// 工具函数前置定义（在 CharString 类之前）
// =============================================================

// C 风格：把 source 复制到 target（直到 '\0'，含 '\0'）—— P38
char *CStrCopy(char *target, const char *source)
{
    char *tar = target;
    while ((*tar++ = *source++) != '\0')
        ; // 复制并包含终止符
    return target;
}

// C 风格：把 source 的前 n 个字符复制到 target，并补 '\0' —— P39
char *CStrCopy(char *target, const char *source, int n)
{
    int srcLen = (int)std::strlen(source);
    int len = (srcLen < n) ? srcLen : n;
    for (int i = 0; i < len; ++i)
        target[i] = source[i];
    target[len] = '\0';
    return target;
}

// C 风格：把 source 连接到 target 末尾（覆盖 target 末尾 '\0'）—— P40
char *CStrConcat(char *target, const char *source)
{
    char *tar = target + std::strlen(target);
    while ((*tar++ = *source++) != '\0')
        ;
    return target;
}

// =============================================================
// 4.2 CharString 类与相关操作（P11–P23）
// - 为了贴近课件，我们实现一个安全的小型字符串类。
// - 内部管理堆分配的 char*，保证深拷贝。
// - 与 C 风格字符串互转：ToCStr / 构造函数(const char*)
// - 关系运算符重载与下标运算符等。
// =============================================================
class CharString
{
protected:
    mutable char *strVal; // 串值（以 '\0' 结尾）
    int length;           // 串长（不含 '\0'）

public:
    // --- 构造 / 析构 / 赋值（P13–P14, P19–P20） ---
    CharString() : strVal(new char[1]{'\0'}), length(0) {}

    // 从 C 风格串构造（P19）
    CharString(const char *source)
    {
        if (!source)
        {
            strVal = new char[1]{'\0'};
            length = 0;
            return;
        }
        length = (int)std::strlen(source);
        strVal = new char[length + 1];
        // 这里用我们自写的 CStrCopy（P38）
        // 为简洁先前向声明，函数定义见后文。
        CStrCopy(strVal, source);
    }

    // 从 LinkList<char> 构造（P20）
    CharString(LinkList<char> &source)
    {
        length = source.Length();
        strVal = new char[length + 1];
        for (int i = 0; i < length; ++i)
        {
            char ch;
            bool ok = source.GetElem(i + 1, ch);
            strVal[i] = ok ? ch : '\0';
        }
        strVal[length] = '\0';
    }

    // 拷贝构造
    CharString(const CharString &other)
    {
        length = other.length;
        strVal = new char[length + 1];
        CStrCopy(strVal, other.strVal);
    }

    // 析构
    virtual ~CharString() { delete[] strVal; }

    // 赋值运算符（深拷贝）
    CharString &operator=(const CharString &other)
    {
        if (this == &other)
            return *this;
        char *newBuf = new char[other.length + 1];
        CStrCopy(newBuf, other.strVal);
        delete[] strVal;
        strVal = newBuf;
        length = other.length;
        return *this;
    }

    // --- 基本接口（P14, P21–P23） ---
    int Length() const { return length; }
    bool Empty() const { return length == 0; }

    // 转为 C 风格串（P21）
    const char *ToCStr() const { return (const char *)strVal; }

    // 下标访问（简化：不越界检查，保持与课件风格一致）
    char &operator[](int pos) const { return strVal[pos]; }
};

// =============================================================
// 全局关系运算（P22）：基于 strcmp
// =============================================================
bool operator==(const CharString &a, const CharString &b)
{
    return std::strcmp(a.ToCStr(), b.ToCStr()) == 0;
}
bool operator<(const CharString &a, const CharString &b)
{
    return std::strcmp(a.ToCStr(), b.ToCStr()) < 0;
}
bool operator>(const CharString &a, const CharString &b)
{
    return std::strcmp(a.ToCStr(), b.ToCStr()) > 0;
}
bool operator<=(const CharString &a, const CharString &b) { return !(a > b); }
bool operator>=(const CharString &a, const CharString &b) { return !(a < b); }
bool operator!=(const CharString &a, const CharString &b) { return !(a == b); }

// 前向声明 Read 函数（在 CharString 类定义之后）
CharString Read(std::istream &in);

// =============================================================
// 4.2.1/4.2.3 串相关操作与"自定义 C 字符串函数"
// - CStrCopy / CStrCopy_n / CStrConcat（P38–P42）
// - 包装：Copy / Concat / SubString / Index 等（P15–P17, P23）
// =============================================================

// --- CharString 的"拷贝/连接/子串/查找"等包装函数 ---

// 将 source 复制到 target（等价于赋值）—— 对应 P7(1) Copy
void Copy(CharString &target, const CharString &source)
{
    target = source; // 直接复用赋值运算
}

// 将 source 的前 n 个字符复制给 target —— 对应 P16 的重载
void Copy(CharString &target, const CharString &source, int n)
{
    int take = std::min(n, source.Length());
    vector<char> buf(take + 1, '\0');
    CStrCopy(buf.data(), source.ToCStr(), take);
    target = CharString(buf.data());
}

// 连接：target += source（P23）
void Concat(CharString &target, const CharString &source)
{
    const char *a = target.ToCStr();
    const char *b = source.ToCStr();
    int la = (int)std::strlen(a), lb = (int)std::strlen(b);
    char *buf = new char[la + lb + 1];
    CStrCopy(buf, a);
    CStrConcat(buf, b);
    target = CharString(buf);
    delete[] buf;
}

// 取子串：从 pos 开始、长度 len（P8）
// 注意：与课件一致，我们采用 0-based 的 pos，并截断越界长度。
CharString SubString(const CharString &s, int pos, int len)
{
    if (pos < 0 || pos >= s.Length() || len <= 0)
        return CharString("");
    int take = std::min(len, s.Length() - pos);
    vector<char> buf(take + 1, '\0');
    for (int i = 0; i < take; ++i)
        buf[i] = s[pos + i];
    return CharString(buf.data());
}

// =============================================================
// 4.3 字符串模式匹配 —— 暴力匹配 & KMP（P45–P58）
// =============================================================

// 暴力匹配：返回 P 在 T 中从 pos（0-based）开始首次出现的位置，否则 -1（P47–P48）
int SimpleIndex(const CharString &T, const CharString &P, int pos = 0)
{
    int startPos = pos;
    int i = pos; // T 的游标
    int j = 0;   // P 的游标
    while (i < T.Length() && j < P.Length())
    {
        if (T[i] == P[j])
        {
            ++i;
            ++j;
        } // 逐字匹配
        else
        {
            ++startPos;
            i = startPos;
            j = 0;
        } // 失配：T 回退到起点+1，P 回到 0
    }
    return (j >= P.Length()) ? startPos : -1;
}

// 求 KMP 的 next 数组（课件使用 next[0] = -1 的风格，P57）
vector<int> KMP_GetNext(const CharString &P)
{
    vector<int> next(P.Length() + 1, 0);
    int i = 0, j = -1;
    next[0] = -1;
    while (i < P.Length())
    {
        if (j == -1 || P[i] == P[j])
        {
            ++i;
            ++j;
            next[i] = j; // 推进并记录
        }
        else
        {
            j = next[j]; // 回溯
        }
    }
    return next;
}

// KMP 匹配：返回首次匹配位置，否则 -1（P54）
int Index_KMP(const CharString &T, const CharString &P, int pos = 0)
{
    if (P.Length() == 0)
        return pos;
    vector<int> next = KMP_GetNext(P);
    int i = pos, j = 0;
    while (i < T.Length() && j < P.Length())
    {
        if (j == -1 || T[i] == P[j])
        {
            ++i;
            ++j;
        }
        else
            j = next[j];
    }
    return (j >= P.Length()) ? (i - P.Length()) : -1;
}

// 统计所有出现位置（用于 demo）
vector<int> FindAll(const CharString &T, const CharString &P, bool useKMP = true)
{
    vector<int> pos;
    if (P.Length() == 0)
        return pos;
    int i = 0;
    while (i <= T.Length() - P.Length())
    {
        int p = useKMP ? Index_KMP(T, P, i) : SimpleIndex(T, P, i);
        if (p == -1)
            break;
        pos.push_back(p);
        i = p + 1; // 允许重叠
    }
    return pos;
}

// =============================================================
// 4.4* 文本编辑器（P61–P88）
// - 支持命令：R/W/I/D/F/C/Q/H/N/P/B/E/G/V（见 P63–P65）
// - 结构：main（P67–P69）+ Editor 类（P71–P87）
// - 为完整可运行，这里把未给出的函数逻辑作了合理填充（尽量贴近教材风格）。
// =============================================================

class Editor
{
private:
    DblLinkList<CharString> textBuffer; // 文本缓冲区（双向链表）—— P71
    int curLineNo = 0;                  // 当前行号（1-based；0 表示暂时没有行）—— P71
    ifstream infile;
    ofstream outfile;
    char userCommand = 0;

    // === 辅助函数（P72 的原型列表） ===
    bool UserSaysYes()
    {
        cout << "确认? (y/n): ";
        char c = 'n';
        cin >> c;
        while (cin.get() != '\n')
            ;
        c = (char)tolower((unsigned char)c);
        return c == 'y';
    }
    bool NextLine()
    { // N：下一行
        if (curLineNo == 0)
            return false;
        if (curLineNo >= textBuffer.Length())
            return false;
        ++curLineNo;
        return true;
    }
    bool PreviousLine()
    { // P：上一行
        if (curLineNo <= 1)
            return false;
        --curLineNo;
        return true;
    }
    bool GotoLine()
    { // G：跳转到指定行
        cout << "转到哪一行(1.." << textBuffer.Length() << "): ";
        int ln = 0;
        cin >> ln;
        while (cin.get() != '\n')
            ;
        if (ln < 1 || ln > textBuffer.Length())
            return false;
        curLineNo = ln;
        return true;
    }

    // C：替换当前行中的文本（把 target 全部替换为 repl）
    bool ChangeLine()
    {
        if (curLineNo == 0)
            return false;
        CharString line;
        if (!textBuffer.GetElem(curLineNo, line))
            return false;
        cout << "输入要查找的目标串: ";
        CharString target = Read(cin);
        cout << "替换为: ";
        CharString repl = Read(cin);

        // 简单实现：把 CharString 转成 std::string 做替换
        std::string s = line.ToCStr();
        std::string t = target.ToCStr();
        std::string r = repl.ToCStr();
        if (t.empty())
        {
            cout << "目标串为空，取消。\n";
            return false;
        }

        size_t pos = 0;
        int cnt = 0;
        while ((pos = s.find(t, pos)) != std::string::npos)
        {
            s.replace(pos, t.size(), r);
            pos += r.size();
            ++cnt;
        }
        if (cnt == 0)
        {
            cout << "当前行未找到目标串。\n";
            return false;
        }
        else
        {
            textBuffer.Replace(curLineNo, CharString(s.c_str()));
            cout << "已替换 " << cnt << " 处。\n";
            return true;
        }
    }

    // F：从当前行开始查找包含用户请求目标串的“第一行” —— P64
    void FindString()
    {
        if (curLineNo == 0)
        {
            cout << "文本缓存空。\n";
            return;
        }
        cout << "要查找的模式串: ";
        CharString pat = Read(cin);
        if (pat.Empty())
        {
            cout << "模式串为空，取消。\n";
            return;
        }

        int n = textBuffer.Length();
        for (int i = curLineNo; i <= n; ++i)
        {
            CharString line;
            textBuffer.GetElem(i, line);
            if (Index_KMP(line, pat, 0) != -1)
            {
                curLineNo = i;
                cout << "在第 " << i << " 行首次匹配到: " << line.ToCStr() << "\n";
                return;
            }
        }
        cout << "未匹配到。\n";
    }

    // I：插入一行（P86–P88）
    bool InsertLine()
    {
        int lineNumber;
        cout << "输入指定行号? ";
        cin >> lineNumber;
        while (cin.get() != '\n')
            ; // 跳过其他字符
        cout << "输入新行文本串: ";
        CharString toInsert = Read(cin);

        if (textBuffer.Insert(lineNumber, toInsert))
        {
            curLineNo = lineNumber;
            return true;
        }
        else
        {
            return false;
        }
    }

    // R：读文件到缓冲区（若已有内容将清空，当前行为第一行）—— P64
    void ReadFile()
    {
        cout << "从输入文件读入内容（会覆盖当前缓冲区）——继续吗？\n";
        if (!UserSaysYes())
            return;

        textBuffer.clear();
        infile.clear();
        infile.seekg(0, ios::beg);
        if (!infile.good())
        {
            cout << "输入文件不可读。\n";
            return;
        }

        std::string line;
        int cnt = 0;
        while (std::getline(infile, line))
        {
            // 去掉行末回车（Windows 文本可能有 \r）
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            textBuffer.Insert(textBuffer.Length() + 1, CharString(line.c_str()));
            ++cnt;
        }
        if (cnt > 0)
            curLineNo = 1;
        else
            curLineNo = 0;
        cout << "已读取 " << cnt << " 行。\n";
    }

    // W：把缓冲区写入输出文件 —— P64
    void WriteFile()
    {
        outfile.clear();
        outfile.seekp(0, ios::beg);
        if (!outfile.good())
        {
            cout << "输出文件不可写。\n";
            return;
        }

        int n = textBuffer.Length();
        textBuffer.ForEach([&](int i, const CharString &s)
                           { outfile << s.ToCStr() << "\n"; });
        cout << "已写出 " << n << " 行。\n";
    }

    // V：查看缓冲区 —— P65
    void View()
    {
        if (textBuffer.Empty())
        {
            cout << "[空]\n";
            return;
        }
        textBuffer.ForEach([&](int i, const CharString &s)
                           { cout << setw(4) << i << " : " << s.ToCStr() << "\n"; });
    }

public:
    // === 公有接口（P73） ===
    Editor(char infName[], char outfName[])
    {
        infile.open(infName);
        outfile.open(outfName);
        if (!outfile.good())
        {
            // 某些平台需要显式以 trunc 打开（不存在则创建）
            outfile.close();
            outfile.open(outfName, ios::out | ios::trunc);
        }
    }

    // P75–P77：读取命令（转小写；q 返回 false 以结束主循环）
    bool GetCommand()
    {
        CharString curLine;
        if (curLineNo != 0)
        {
            textBuffer.GetElem(curLineNo, curLine);
            cout << curLineNo << " : " << curLine.ToCStr() << "\n?";
        }
        else
        {
            cout << "文件缓存空\n?";
        }
        cin >> userCommand;
        userCommand = (char)tolower((unsigned char)userCommand);
        while (cin.get() != '\n')
            ; // 丢弃行余下内容
        return userCommand != 'q';
    }

    // P78–P85：执行命令
    void RunCommand()
    {
        switch (userCommand)
        {
        case 'b': // begin: 到第 1 行
            if (textBuffer.Empty())
                cout << "警告: 文本缓存空\n";
            else
                curLineNo = 1;
            break;

        case 'c': // change: 替换当前行中子串
            if (textBuffer.Empty())
                cout << "警告: 文本缓存空\n";
            else if (!ChangeLine())
                cout << "警告: 操作失败\n";
            break;

        case 'd': // delete: 删除当前行
            if (!textBuffer.Delete(curLineNo))
                cout << "错误: 删除失败\n";
            else
            {
                if (textBuffer.Length() == 0)
                    curLineNo = 0;
                else if (curLineNo > textBuffer.Length())
                    curLineNo = textBuffer.Length();
            }
            break;

        case 'e': // end: 到最后一行
            if (textBuffer.Empty())
                cout << "警告: 文本缓存空\n";
            else
                curLineNo = textBuffer.Length();
            break;

        case 'f': // find: 查找包含模式串的第一行（从当前行开始）
            if (textBuffer.Empty())
                cout << "警告: 文本缓存空\n";
            else
                FindString();
            break;

        case 'g': // go: 跳行
            if (!GotoLine())
                cout << "错误: 操作失败\n";
            break;

        case '?': // help
        case 'h':
            cout << "有效命令: b(egin) c(hange) d(el) e(nd) f(ind) g(o) h(elp)\n"
                 << "           i(nsert) n(ext) p(rior) q(uit) r(ead) v(iew) w(rite)\n";
            break;

        case 'i': // insert
            if (!InsertLine())
                cout << "错误: 操作失败\n";
            break;

        case 'n': // next
            if (!NextLine())
                cout << "错误: 操作失败\n";
            break;

        case 'p': // prior
            if (!PreviousLine())
                cout << "错误: 操作失败\n";
            break;

        case 'r': // read
            ReadFile();
            break;

        case 'v': // view
            View();
            break;

        case 'w': // write
            if (textBuffer.Empty())
                cout << "警告: 文本缓存空\n";
            else
                WriteFile();
            break;

        default:
            cout << "输入 h 或 ? 获得帮助；请键入有效命令字符。\n";
        }
    }
};

// =============================================================
// 工具：读取一整行到 CharString（课件 P67/68/86 等多处用到 Read(cin)）
// - 行可能为空：返回长度为 0 的 CharString("")。
// =============================================================
CharString Read(std::istream &in)
{
    std::string line;
    std::getline(in, line);
    if (!line.empty() && line.back() == '\r')
        line.pop_back();
    return CharString(line.c_str());
}

// （可选）另一个重载：读取直到终止符并返回该终止符（示意）
CharString Read(std::istream &in, char &terminalChar)
{
    std::string token;
    terminalChar = '\n';
    char ch;
    while (in.get(ch))
    {
        if (ch == '\n' || ch == ' ' || ch == '\t')
        {
            terminalChar = ch;
            break;
        }
        token.push_back(ch);
    }
    return CharString(token.c_str());
}

// =============================================================
// 4.2.3 的演示程序片段（对应补例 4.5 的输出效果，P41–P42）
// =============================================================
void Demo_CStrFuncs()
{
    cout << "\n===== 4.2.3 自定义 C 字符串函数演示 =====\n";
    char dest[100] = "destination";
    char src[100] = "source";

    CStrCopy(dest, src);
    cout << "CStrCopy(dest, src): dest:" << dest << "\n";

    CStrCopy(dest, src, 4); // 拷贝前 4 个字符
    cout << "CStrCopy(dest, src, 4): dest:" << dest << "\n";

    CStrCopy(dest, "Hello ");
    CStrCopy(src, "world");
    CStrConcat(dest, src);
    cout << "CStrConcat(dest, src): dest:" << dest << "\n";
}

// =============================================================
// 4.3 的演示：暴力匹配与 KMP（含 next 数组打印）
// - 文本：A man with money is no match against a man on a mission
// - 模式：match / man（统计出现次数与位置）
// - 再演示课件 next 表（示例串可自定，这里也演示一下 next）
// =============================================================
void Demo_Matching()
{
    cout << "\n===== 4.3 字符串模式匹配演示 =====\n";
    CharString T("A man with money is no match against a man on a mission");
    CharString P1("match");
    CharString P2("man");

    int p1 = Index_KMP(T, P1, 0);
    cout << "查找 \"match\"：首次出现位置 = " << p1 << "\n";

    vector<int> all2 = FindAll(T, P2, true);
    cout << "查找 \"man\"：出现次数 = " << all2.size() << "，位置 = ";
    for (size_t i = 0; i < all2.size(); ++i)
        cout << (i ? ", " : "") << all2[i];
    cout << "\n";

    // next 数组演示（课件 P50/P57 的风格）
    CharString Pat("ABAABCAC");
    auto nxt = KMP_GetNext(Pat);
    cout << "模式串 \"ABAABCAC\" 的 next 数组：\n";
    for (size_t i = 0; i < nxt.size(); ++i)
        cout << "next[" << i << "]=" << nxt[i] << (i + 1 == nxt.size() ? '\n' : ' ');
}

// =============================================================
// 4.1/4.2 的演示：CharString 构造、拷贝、连接、子串等
// =============================================================
void Demo_CharString_Basics()
{
    cout << "\n===== 4.1/4.2 CharString 基础演示 =====\n";
    CharString s1("Hello");
    CharString s2(", World");
    Concat(s1, s2);
    cout << "Concat 后: " << s1.ToCStr() << "\n";

    CharString sub = SubString(s1, 7, 5);
    cout << "SubString(7,5): " << sub.ToCStr() << "\n";

    // 用 LinkList<char> 构造（P20）
    LinkList<char> L;
    for (char c : string("scu"))
        L.PushBack(c);
    CharString s3(L);
    cout << "用 LinkList<char> 构造得到: " << s3.ToCStr() << "\n";

    // 比较运算（P22）
    cout << boolalpha;
    cout << "\"abc\" < \"abd\" ? " << (CharString("abc") < CharString("abd")) << "\n";
    cout << "\"abc\" == \"abc\" ? " << (CharString("abc") == CharString("abc")) << "\n";
}

// =============================================================
// 主程序（P67–P69）：先做演示，再进入文本编辑器交互
// =============================================================
int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // --- 按课件顺序的演示环节 ---
    Demo_CharString_Basics(); // 4.1/4.2
    Demo_CStrFuncs();         // 4.2.3
    Demo_Matching();          // 4.3

    // --- 进入 4.4 文本编辑器 ---
    char infName[256] = {0}, outfName[256] = {0};
    cout << "\n===== 4.4* 文本编辑器 =====\n";
    cout << "输入文件名(缺省: file_in.txt): ";
    CStrCopy(infName, Read(cin).ToCStr());
    if (std::strlen(infName) == 0)
        CStrCopy(infName, "file_in.txt");

    cout << "输出文件名(缺省: file_out.txt): ";
    CStrCopy(outfName, Read(cin).ToCStr());
    if (std::strlen(outfName) == 0)
        CStrCopy(outfName, "file_out.txt");

    Editor text(infName, outfName);
    cout << "键入 h 或 ? 查看帮助；q 退出。\n";
    while (text.GetCommand())
    {
        text.RunCommand();
    }
    cout << "Bye.\n";
    return 0;
}
