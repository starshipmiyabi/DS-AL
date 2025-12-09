/******************************************************
 * 广义表.cpp —— 参照《Ch05 数组和广义表》
 * 目标：
 *   1) 采用“引用数法”实现广义表的链式存储（见 P69–P72）。
 *   2) 按课件接口实现 RefGenList 的所有方法（P74–P76）：
 *      First / Next / Empty / Push(atom) / Push(subList)
 *      Depth / Input / Show / 构造、复制构造、赋值、析构
 *   3) 提供可运行示例：构造课件中的 A,B,C,D 列表与深度计算（P73, P79–P81）。
 *
 * 关键概念回顾（见 P63–P67）：
 *   • 广义表是元素可为原子或子表的线性序列；有“表头/head、表尾/tail、
 *     深度/Depth”等概念（P63–P64）。
 *   • 本实现中的“引用数 ref”记录某子表（其头结点）的被引用次数（P69）。
 ******************************************************/

#include <iostream>
#include <sstream>
#include <string>
#include <cctype>
#include <stdexcept>
#include <utility>

//========================= 结点与枚举（P69–P72） =========================//

#ifndef __REF_GEN_LIST_NODE_TYPE__
#define __REF_GEN_LIST_NODE_TYPE__
enum RefGenListNodeType { HEAD, ATOM, LIST };  // 头结点/原子结点/表结点（P69–P70）
#endif

// 引用数法广义表结点（见 P71–P72）
template <class ElemType>
struct RefGenListNode {
    RefGenListNodeType tag;                    // 结点类型：HEAD/ATOM/LIST
    RefGenListNode<ElemType>* nextLink;        // 同层后继指针

    union {
        int ref;                               // tag = HEAD：引用数（P69）
        ElemType atom;                         // tag = ATOM：原子值
        RefGenListNode<ElemType>* subLink;     // tag = LIST：指向子表头结点
    };

    // 构造（P72）
    explicit RefGenListNode(RefGenListNodeType tg = HEAD,
                            RefGenListNode<ElemType>* next = nullptr)
        : tag(tg), nextLink(next) {
        // 其余 union 成员按实际使用场景赋值
    }
};

//============================ 广义表类（P74–P76） ============================//

template <class ElemType>
class RefGenList {
protected:
    RefGenListNode<ElemType>* head;  // 头指针（指向 HEAD 头结点，见 P74）

    // —— 辅助：显示（递归）（P74）
    void ShowHelp(const RefGenListNode<ElemType>* hd) const {
        std::cout << "(";
        const RefGenListNode<ElemType>* p = hd->nextLink; // 第一个元素（P78：First/Next）
        bool first = true;
        while (p) {
            if (!first) std::cout << ", ";
            if (p->tag == ATOM) {
                std::cout << p->atom;
            } else if (p->tag == LIST) {
                ShowHelp(p->subLink); // 递归显示子表
            }
            first = false;
            p = p->nextLink;
        }
        std::cout << ")";
    }

    // —— 辅助：深度计算（递归）（见 P79–P81）
    int DepthHelp(const RefGenListNode<ElemType>* hd) const {
        if (hd->nextLink == nullptr) return 1;   // 空表深度为 1（P80）
        int subMax = 0;
        for (const RefGenListNode<ElemType>* p = hd->nextLink; p; p = p->nextLink) {
            if (p->tag == LIST) {
                int d = DepthHelp(p->subLink);   // 子表深度
                if (d > subMax) subMax = d;
            }
        }
        return subMax + 1;  // 最大子表深度 + 1（P81）
    }

    // —— 辅助：释放（带引用计数，重要！）
    static void ClearHelp(RefGenListNode<ElemType>* hd) {
        if (!hd) return;
        // 释放同层元素链
        RefGenListNode<ElemType>* p = hd->nextLink;
        while (p) {
            RefGenListNode<ElemType>* nxt = p->nextLink;
            if (p->tag == LIST) {
                // 引用计数 -1，若归零，递归释放子表（P69 引用数语义）
                if (p->subLink) {
                    p->subLink->ref--;
                    if (p->subLink->ref <= 0) {
                        ClearHelp(p->subLink);
                        delete p->subLink;
                    }
                }
            }
            delete p;
            p = nxt;
        }
        // 最后释放自身 HEAD
        // 注意：此处不处理 hd->ref（由父调用/持有者控制）
    }

    // —— 辅助：深拷贝（将 sourceHead 拷贝成一个全新结构，P75）
    static void CopyHelp(const RefGenListNode<ElemType>* sourceHead,
                         RefGenListNode<ElemType>*& destHead) {
        destHead = new RefGenListNode<ElemType>(HEAD);
        destHead->ref = 1;               // 新建头结点引用数初始化为 1（P77）
        destHead->nextLink = nullptr;

        RefGenListNode<ElemType>* tail = nullptr; // 构造同层链的尾指针
        for (const RefGenListNode<ElemType>* p = sourceHead->nextLink; p; p = p->nextLink) {
            RefGenListNode<ElemType>* node = nullptr;
            if (p->tag == ATOM) {
                node = new RefGenListNode<ElemType>(ATOM);
                node->atom = p->atom;
            } else if (p->tag == LIST) {
                node = new RefGenListNode<ElemType>(LIST);
                // 递归复制子表（深拷贝）
                RefGenListNode<ElemType>* newSubHead = nullptr;
                CopyHelp(p->subLink, newSubHead);
                node->subLink = newSubHead;
                // 新子表被当前 LIST 结点引用，引用数+1（其自身构造时为1，当前引用再+1）
                // 说明：我们采用“谁持有谁加 1”的策略，保证共享安全。
                node->subLink->ref++;
            }
            node->nextLink = nullptr;

            if (!tail) {
                destHead->nextLink = node;
                tail = node;
            } else {
                tail->nextLink = node;
                tail = node;
            }
        }
    }

    //================= 解析器：从流创建广义表（实现 CreateHelp，P75） =================//

    // 跳过空白
    static void skipSpaces(std::istream& is) {
        while (std::isspace(is.peek())) is.get();
    }

    // 解析原子：默认读取一个非分隔符字符作为 ElemType（面向 char，见课件示例）
    static ElemType parseAtom(std::istream& is) {
        skipSpaces(is);
        int ch = is.get();
        if (ch == EOF) throw std::runtime_error("解析原子失败：遇到 EOF。");
        char c = static_cast<char>(ch);
        if (c == '(' || c == ')' || c == ',' )
            throw std::runtime_error(std::string("解析原子失败：意外的分隔符 '") + c + "'.");
        // 若 ElemType 不是 char，可在此扩展为读取标识符/数字串等
        return static_cast<ElemType>(c);
    }

    // 解析一个“元素”：ATOM 或 LIST
    static RefGenListNode<ElemType>* parseElement(std::istream& is);

    // 解析一个“列表”：形如 ( ... )
    static RefGenListNode<ElemType>* parseList(std::istream& is) {
        skipSpaces(is);
        if (is.get() != '(') throw std::runtime_error("缺少 '(' 开始列表。");

        auto* hd = new RefGenListNode<ElemType>(HEAD);
        hd->ref = 1;            // 新建子表头结点，引用数从 1 开始（P77）
        hd->nextLink = nullptr;

        skipSpaces(is);
        if (is.peek() == ')') { // 空表（P63 定义）
            is.get();           // 吃掉 ')'
            return hd;
        }

        RefGenListNode<ElemType>* tail = nullptr;
        while (true) {
            RefGenListNode<ElemType>* elem = parseElement(is);
            if (!tail) { hd->nextLink = elem; tail = elem; }
            else { tail->nextLink = elem; tail = elem; }

            skipSpaces(is);
            int ch = is.peek();
            if (ch == ',') { is.get(); continue; }   // 还有后续元素
            else if (ch == ')') { is.get(); break; } // 列表结束
            else throw std::runtime_error("列表元素之间缺少 ',' 或 右括号 ')'");
        }
        return hd;
    }

    // parseElement 需要 parseList 的声明后才能定义
    // 放在类外 inline 定义（见后）

public:
    //================== 构造 / 析构 / 复制 / 赋值（P75–P77） ==================//

    // 无参构造：创建空表（只有 HEAD，nextLink=null）（P77）
    RefGenList() {
        head = new RefGenListNode<ElemType>(HEAD);
        head->ref = 1;
        head->nextLink = nullptr;
    }

    // 由头结点构造：共享已有结构（引用数+1）
    explicit RefGenList(RefGenListNode<ElemType>* hd) : head(hd) {
        if (!head) {
            head = new RefGenListNode<ElemType>(HEAD);
            head->ref = 1;
            head->nextLink = nullptr;
        } else {
            head->ref++;  // 新增一个拥有者（P69）
        }
    }

    // 复制构造：深拷贝（不与源共享内部结构，便于入门者理解）
    RefGenList(const RefGenList& src) : head(nullptr) {
        CopyHelp(src.head, head);
    }

    // 析构：释放一切拥有的引用
    ~RefGenList() {
        if (head) {
            head->ref--;
            if (head->ref <= 0) {
                ClearHelp(head);
                delete head;
            }
            head = nullptr;
        }
    }

    // 赋值：强异常安全的“拷贝-再交换”
    RefGenList& operator=(const RefGenList& src) {
        if (this == &src) return *this;
        RefGenListNode<ElemType>* newHead = nullptr;
        CopyHelp(src.head, newHead);
        // 释放旧的
        if (head) {
            head->ref--;
            if (head->ref <= 0) { ClearHelp(head); delete head; }
        }
        head = newHead;
        return *this;
    }

    //========================== 基本操作（P76–P78） ==========================//

    // (1) 返回第一个元素结点指针（P78）
    RefGenListNode<ElemType>* First() const { return head->nextLink; }

    // (2) 返回某元素的后继（P78）
    RefGenListNode<ElemType>* Next(RefGenListNode<ElemType>* elemPtr) const {
        return elemPtr ? elemPtr->nextLink : nullptr;
    }

    // (3) 判空（P76）
    bool Empty() const { return head->nextLink == nullptr; }

    // (4) 头插一个原子元素 e（P67、P76）
    void Push(const ElemType& e) {
        auto* node = new RefGenListNode<ElemType>(ATOM);
        node->atom = e;
        node->nextLink = head->nextLink;
        head->nextLink = node;
    }

    // (5) 头插一个子表 subList（共享其头结点，引用数 +1）（P67、P76）
    void Push(RefGenList& subList) {
        auto* node = new RefGenListNode<ElemType>(LIST);
        node->subLink = subList.head;
        node->subLink->ref++;               // 增加对子表的一个引用（P69）
        node->nextLink = head->nextLink;
        head->nextLink = node;
    }

    // (6) 深度（P79–P81）
    int Depth() { return DepthHelp(head); }

    // （扩展）显示：形如 (a, (b, c), d)（P74）
    void Show() const { ShowHelp(head); }

    // 输入：从一行字符串解析创建广义表（P75 “CreateHelp”思想）
    // 说明：为教学简化，默认按 char 原子解析：遇到 '(', ')' 和 ',' 以外的单个字符视为原子。
    void Input() {
        std::string line;
        std::getline(std::cin, line);
        std::istringstream iss(line);
        RefGenListNode<ElemType>* newHead = parseList(iss);

        // 释放旧的
        if (head) {
            head->ref--;
            if (head->ref <= 0) { ClearHelp(head); delete head; }
        }
        head = newHead;
    }

    // 工厂：从字符串直接生成（便于示例/单元测试）
    static RefGenList FromString(const std::string& s) {
        std::istringstream iss(s);
        RefGenList gl;
        // 先释放默认空表
        gl.head->ref--;
        if (gl.head->ref <= 0) { ClearHelp(gl.head); delete gl.head; }
        gl.head = parseList(iss); // 接管新表
        return gl;
    }
};

// —— 解析元素：要么原子，要么子表
template <class ElemType>
RefGenListNode<ElemType>* RefGenList<ElemType>::parseElement(std::istream& is) {
    skipSpaces(is);
    int ch = is.peek();
    if (ch == '(') {
        // 子表
        auto* subHead = parseList(is);
        auto* node = new RefGenListNode<ElemType>(LIST);
        node->subLink = subHead;         // 当前 LIST 结点持有子表
        // 当前 LIST 作为一个“新引用”，将子表 head 的 ref +1（P69）
        node->subLink->ref++;
        node->nextLink = nullptr;
        return node;
    } else {
        // 原子
        ElemType v = parseAtom(is);
        auto* node = new RefGenListNode<ElemType>(ATOM);
        node->atom = v;
        node->nextLink = nullptr;
        return node;
    }
}

//============================== 演示（P73、P79） ==============================//

/*
 * 我们对课件图示（P73）中的若干表进行构造与验证：
 *   A = ()                     // 空表
 *   B = (x, y, z)              // 三个原子
 *   C = (B, y, z)              // 共享子表 B（体现“元素共享性”，见 P65）
 *   D = (x, (y, z))            // 子表嵌套
 * 并输出它们：
 *   1) 表达式（Show）；
 *   2) 深度（Depth，见 P79–P81）。
 */
int main() {
    using GL = RefGenList<char>;

    // A: 空表
    GL A;
    std::cout << "A = "; A.Show(); std::cout << ", Depth = " << A.Depth() << "\n";

    // B: (x, y, z)
    GL B = GL::FromString("(x, y, z)");
    std::cout << "B = "; B.Show(); std::cout << ", Depth = " << B.Depth() << "\n";

    // C: (B, y, z) —— 通过 Push(subList) 共享 B
    GL C;             // 先空表
    C.Push('z');      // 头插是栈式，注意逆序插入以得到期望显示顺序
    C.Push('y');
    C.Push(B);        // 共享子表 B（B.head->ref++）
    std::cout << "C = "; C.Show(); std::cout << ", Depth = " << C.Depth() << "\n";

    // D: (x, (y, z))
    GL D = GL::FromString("(x, (y, z))");
    std::cout << "D = "; D.Show(); std::cout << ", Depth = " << D.Depth() << "\n";

    // First / Next（P78）
    auto* firstOfB = B.First();
    std::cout << "B.First()->tag = " << (firstOfB ? (firstOfB->tag==ATOM?'A':'L') : 'N')
              << ", value = " << (firstOfB && firstOfB->tag==ATOM ? firstOfB->atom : '#') << "\n";
    auto* secondOfB = B.Next(firstOfB);
    std::cout << "B.Next(First)->value = "
              << (secondOfB && secondOfB->tag==ATOM ? secondOfB->atom : '#') << "\n";

    // Input（从标准输入读取一行进行解析，示例： (a, (b, c), d) ）
    std::cout << "\n请输入一个广义表（示例：(a,(b,c),d)）：\n> ";
    GL userGL;
    userGL.Input();
    std::cout << "你输入的表："; userGL.Show();
    std::cout << ", Depth = " << userGL.Depth() << "\n";

    return 0;
}
