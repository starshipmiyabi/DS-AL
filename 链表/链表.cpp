/************************************************************
 * 线性表的链式存储结构（单链表 / 循环单链表 / 带缓存链表）
 * 完整实现 + 演示主程序
 *
 * 课程课件映射（页码）：
 *  - P45：结点 Node 定义（data + next）
 *  - P46–P48：SimpleLinkList 接口
 *  - P49–P55：单链表插入/删除的指针操作顺序
 *  - P57–P61：循环单链表接口及实现要点
 *  - P62–P66：循环链表应用——约瑟夫问题示例
 *  - P72–P74：LinkList（缓存当前位置与元素个数）
 *
 * 统一约定：
 *  - “位序”为 1-based（与课件一致）。
 *  - 采用“头结点/哨兵”（sentinel）简化边界处理（P77）。
 *  - Insert/ Delete 的返回值：成功 true，越界/非法 false。
 ************************************************************/
#include <iostream>
#include <iomanip>

//======================== P45: 结点类模板 ========================
template <class ElemType>
struct Node {
    ElemType data;              // 数据域
    Node<ElemType>* next;       // 指针域（后继）

    // 无参构造：用于头结点
    Node() : data(), next(nullptr) {}

    // 带参构造：按“已知数据 + 指向的后继结点”建立结点（P45）
    Node(const ElemType& e, Node<ElemType>* link = nullptr)
        : data(e), next(link) {}
};


//==================================================================
//  P46–P55: 简单单链表 SimpleLinkList
//  - 仅维护头结点 head；长度通过遍历计算（与课件接口一致）
//  - 插入/删除严格按 P49–P55 的指针变更顺序实现
//==================================================================
template <class ElemType>
class SimpleLinkList {
protected:
    Node<ElemType>* head;                       // 头结点指针（哨兵，位序0）

    // P46: 辅助函数——返回指向第 position 个“结点”的指针
    // position=0 返回 head；position>=1 返回数据结点；越界返回 nullptr
    Node<ElemType>* GetElemPtr(int position) const {
        if (position < 0) return nullptr;
        Node<ElemType>* p = head;               // 从头结点（位序0）出发
        int j = 0;
        while (p && j < position) {             // 走 position 步
            p = p->next;
            ++j;
        }
        return p;                                // 可能为 nullptr（越界）
    }

public:
    // P47: 无参构造
    SimpleLinkList() : head(new Node<ElemType>()) {
        head->next = nullptr;
    }

    // P47: 析构——释放整条链
    virtual ~SimpleLinkList() {
        Clear();
        delete head;
        head = nullptr;
    }

    // P47/P11: 判空
    bool Empty() const { return head->next == nullptr; }

    // P47/P12: 求长度——遍历计数
    int Length() const {
        int len = 0;
        for (Node<ElemType>* p = head->next; p; p = p->next) ++len;
        return len;
    }

    // P47/P16: 置空——删除全部数据结点，保留头结点与结构
    void Clear() {
        Node<ElemType>* p = head->next;
        while (p) {
            Node<ElemType>* q = p->next;
            delete p;
            p = q;
        }
        head->next = nullptr;
    }

    // P47/P14: 遍历（访问函数签名与课件保持一致）
    void Traverse(void (*Visit)(ElemType&)) const {
        for (Node<ElemType>* p = head->next; p; p = p->next) {
            Visit(const_cast<ElemType&>(p->data));
        }
    }

    // P47/P13: 按位序取值
    bool GetElem(int position, ElemType& e) const {
        if (position < 1) return false;
        Node<ElemType>* p = GetElemPtr(position);
        if (!p) return false;
        e = p->data;
        return true;
    }

    // P47/P17: 按位序赋值
    bool SetElem(int position, const ElemType& e) {
        if (position < 1) return false;
        Node<ElemType>* p = GetElemPtr(position);
        if (!p) return false;
        p->data = e;
        return true;
    }

    // P49–P51: 插入 —— 在第 position 个位置“前”插入 e
    bool Insert(int position, const ElemType& e) {
        // 合法范围：1 ≤ position ≤ Length()+1（P50）
        if (position < 1 || position > Length() + 1) return false;

        // 取到“前驱”结点（position-1），按 P49 的顺序换链
        Node<ElemType>* prev = GetElemPtr(position - 1);
        if (!prev) return false;

        Node<ElemType>* newPtr = new Node<ElemType>(e, prev->next); // new->next = prev->next
        prev->next = newPtr;                                         // prev->next = new
        return true;
    }

    // P52–P55: 删除（带出参）
    bool Delete(int position, ElemType& e) {
        // 合法范围：1 ≤ position ≤ Length()（P53）
        if (position < 1 || position > Length()) return false;

        Node<ElemType>* prev    = GetElemPtr(position - 1); // 前驱
        Node<ElemType>* target  = prev->next;               // 待删
        e = target->data;                                    // 先取值（P55）
        prev->next = target->next;                           // 脱链（P54）
        delete target;                                       // 释放（P55）
        return true;
    }

    // 重载：删除（不关心被删值）
    bool Delete(int position) {
        ElemType tmp;
        return Delete(position, tmp);
    }

    // 复制构造（深拷贝）
    SimpleLinkList(const SimpleLinkList& other) : SimpleLinkList() {
        for (Node<ElemType>* p = other.head->next; p; p = p->next) {
            Insert(Length() + 1, p->data);
        }
    }

    // 赋值重载（强异常安全：先构造临时，再交换头指针）
    SimpleLinkList& operator=(const SimpleLinkList& other) {
        if (this == &other) return *this;
        SimpleLinkList temp;
        for (Node<ElemType>* p = other.head->next; p; p = p->next) {
            temp.Insert(temp.Length() + 1, p->data);
        }
        std::swap(head, temp.head);
        return *this;
    }
};


//==================================================================
//  P57–P61: 循环单链表 SimpleCircLinkList
//  - 采用头结点，空表: head->next == head
//  - 遍历与取长：以 head 为“止”，避免空指针
//==================================================================
template <class ElemType>
class SimpleCircLinkList {
protected:
    Node<ElemType>* head;  // 头结点（哨兵，位序0；指向自身表示空）

    Node<ElemType>* GetElemPtr(int position) const {
        // position=0 返回 head；1..Length 返回相应结点；否则 nullptr
        if (position < 0) return nullptr;
        if (position == 0) return head;
        Node<ElemType>* p = head->next;
        int j = 1;
        while (p != head && j < position) {
            p = p->next;
            ++j;
        }
        return (p == head ? nullptr : p);
    }

public:
    SimpleCircLinkList() : head(new Node<ElemType>()) {
        head->next = head;                 // 循环：空表指向自己（P58）
    }

    ~SimpleCircLinkList() {
        Clear();
        delete head;
        head = nullptr;
    }

    bool Empty() const { return head->next == head; }

    int Length() const {
        int len = 0;
        for (Node<ElemType>* p = head->next; p != head; p = p->next) ++len;
        return len;
    }

    void Clear() {
        // 逐个摘链：始终删除 head->next，直到回到 head
        while (head->next != head) {
            Node<ElemType>* p = head->next;
            head->next = p->next;
            delete p;
        }
    }

    void Traverse(void (*Visit)(ElemType&)) const {
        for (Node<ElemType>* p = head->next; p != head; p = p->next) {
            Visit(const_cast<ElemType&>(p->data));
        }
    }

    bool GetElem(int position, ElemType& e) const {
        if (position < 1) return false;
        Node<ElemType>* p = GetElemPtr(position);
        if (!p) return false;
        e = p->data;
        return true;
    }

    bool SetElem(int position, const ElemType& e) {
        if (position < 1) return false;
        Node<ElemType>* p = GetElemPtr(position);
        if (!p) return false;
        p->data = e;
        return true;
    }

    // P50/P60：插入 1..Length()+1
    bool Insert(int position, const ElemType& e) {
        if (position < 1 || position > Length() + 1) return false;
        Node<ElemType>* prev = (position == 1 ? head : GetElemPtr(position - 1));
        if (!prev) return false;
        Node<ElemType>* newPtr = new Node<ElemType>(e, prev->next);
        prev->next = newPtr;
        return true;
    }

    // P53：删除 1..Length()
    bool Delete(int position, ElemType& e) {
        if (position < 1 || position > Length()) return false;
        Node<ElemType>* prev   = (position == 1 ? head : GetElemPtr(position - 1));
        Node<ElemType>* target = prev->next;
        if (target == head) return false;           // 防御：不删头结点
        e = target->data;
        prev->next = target->next;
        delete target;
        return true;
    }

    bool Delete(int position) {
        ElemType tmp;
        return Delete(position, tmp);
    }

    // 复制构造
    SimpleCircLinkList(const SimpleCircLinkList& other) : SimpleCircLinkList() {
        for (Node<ElemType>* p = other.head->next; p != other.head; p = p->next) {
            Insert(Length() + 1, p->data);
        }
    }

    // 赋值重载
    SimpleCircLinkList& operator=(const SimpleCircLinkList& other) {
        if (this == &other) return *this;
        SimpleCircLinkList temp;
        for (Node<ElemType>* p = other.head->next; p != other.head; p = p->next) {
            temp.Insert(temp.Length() + 1, p->data);
        }
        std::swap(head, temp.head);
        return *this;
    }
};


//==================================================================
//  P72–P74: 带“当前位置缓存”和“元素个数”的链表 LinkList
//  - curPosition / curPtr 用于加速 GetElemPtr（顺序访问快）
//==================================================================
template <class ElemType>
class LinkList {
protected:
    Node<ElemType>* head;                 // 头结点
    mutable int curPosition;              // 当前缓存的位序（可在 const 中更新）
    mutable Node<ElemType>* curPtr;       // 指向“当前位置”的指针
    int count;                            // 元素个数

    // 关键：带缓存的定位（只前向移动；若目标在 curPosition 之前则从 head 重新走）
    Node<ElemType>* GetElemPtr(int position) const {
        if (position < 0 || position > count) return nullptr;       // 0..count
        Node<ElemType>* p;
        int j;
        if (position >= curPosition) {
            // 复用缓存，从当前结点继续向前走
            p = curPtr;
            j = curPosition;
        } else {
            // 从头结点重新走（P72 思想）
            p = head;
            j = 0;
        }
        while (j < position && p) { p = p->next; ++j; }
        // 更新缓存
        curPtr = p;
        curPosition = position;
        return p;   // position==0 返回 head；>=1 返回相应数据结点
    }

public:
    LinkList()
        : head(new Node<ElemType>()), curPosition(0), curPtr(nullptr), count(0) {
        head->next = nullptr;
        curPtr = head;  // 初始缓存为头结点（位序0）
    }

    virtual ~LinkList() {
        Clear();
        delete head;
        head = nullptr;
        curPtr = nullptr;
        curPosition = 0;
        count = 0;
    }

    bool Empty() const { return count == 0; }
    int  Length() const { return count; }

    void Clear() {
        while (head->next) {
            Node<ElemType>* p = head->next;
            head->next = p->next;
            delete p;
        }
        count = 0;
        curPosition = 0;
        curPtr = head;
    }

    void Traverse(void (*Visit)(ElemType&)) const {
        for (Node<ElemType>* p = head->next; p; p = p->next) {
            Visit(const_cast<ElemType&>(p->data));
        }
    }

    bool GetElem(int position, ElemType& e) const {
        if (position < 1 || position > count) return false;
        Node<ElemType>* p = GetElemPtr(position);
        if (!p) return false;
        e = p->data;
        return true;
    }

    bool SetElem(int position, const ElemType& e) {
        if (position < 1 || position > count) return false;
        Node<ElemType>* p = GetElemPtr(position);
        if (!p) return false;
        p->data = e;
        return true;
    }

    bool Insert(int position, const ElemType& e) {
        if (position < 1 || position > count + 1) return false;
        Node<ElemType>* prev = GetElemPtr(position - 1);            // 0..count
        if (!prev) return false;
        Node<ElemType>* newPtr = new Node<ElemType>(e, prev->next);
        prev->next = newPtr;
        ++count;

        // 维护缓存：把“当前位置”设为新结点，便于顺序插入/访问
        curPosition = position;
        curPtr = newPtr;
        return true;
    }

    bool Delete(int position, ElemType& e) {
        if (position < 1 || position > count) return false;
        Node<ElemType>* prev = GetElemPtr(position - 1);
        Node<ElemType>* target = prev->next;
        e = target->data;
        prev->next = target->next;
        delete target;
        --count;

        // 维护缓存：删除后，缓存放到“前驱”
        curPosition = position - 1;
        curPtr = prev;
        return true;
    }

    bool Delete(int position) {
        ElemType tmp;
        return Delete(position, tmp);
    }

    // 复制构造 / 赋值重载
    LinkList(const LinkList& other) : LinkList() {
        for (Node<ElemType>* p = other.head->next; p; p = p->next) {
            Insert(count + 1, p->data);
        }
    }

    LinkList& operator=(const LinkList& other) {
        if (this == &other) return *this;
        LinkList temp;
        for (Node<ElemType>* p = other.head->next; p; p = p->next) {
            temp.Insert(temp.count + 1, p->data);
        }
        std::swap(head, temp.head);
        std::swap(curPtr, temp.curPtr);
        std::swap(curPosition, temp.curPosition);
        std::swap(count, temp.count);
        return *this;
    }
};


//=========================== P62–P66: 约瑟夫问题 =========================
// 与课件一致：用循环链表表示人环，反复数到 m 删除
void Josephus(int n, int m) {
    SimpleCircLinkList<int> la;              // 定义空循环链表（P64）
    for (int k = 1; k <= n; ++k) la.Insert(k, k);    // 建表（1..n）（P65）

    std::cout << "出列者: ";
    int pos = 0, out = 0, winner = 0;
    for (int i = 1; i < n; ++i) {            // 循环 n-1 次让 n-1 人出列（P65）
        for (int j = 1; j <= m; ++j) {       // 从 1 数到 m（P65）
            ++pos;
            if (pos > la.Length()) pos = 1;
        }
        la.Delete(pos--, out);               // 报到 m 的人出列（P66）
        std::cout << out << ' ';
    }
    la.GetElem(1, winner);                   // 最后剩下的为优胜者（P66）
    std::cout << "\n优胜者: " << winner << "\n";
}


//=============================== 辅助打印 ===============================
template <class T>
void PrintElem(T& x) { std::cout << x << ' '; }

template <class L>
void PrintList(const char* title, const L& list) {
    std::cout << std::left << std::setw(18) << title << ": ";
    list.Traverse(PrintElem<typename std::remove_reference<decltype(*((typename std::remove_reference<L>::type*)nullptr))>::type::value_type>); // 不使用，避免模板萃取复杂度
    // 上面这一行为了通用性会很复杂；下面提供两个重载更简单
}


// 为了简洁，针对本文件中的三个容器分别提供一个打印重载
template <class T>
void PrintList(const char* title, const SimpleLinkList<T>& list) {
    std::cout << std::left << std::setw(18) << title << ": ";
    list.Traverse(PrintElem<T>);
    std::cout << "(len=" << list.Length() << ")\n";
}
template <class T>
void PrintList(const char* title, const SimpleCircLinkList<T>& list) {
    std::cout << std::left << std::setw(18) << title << ": ";
    list.Traverse(PrintElem<T>);
    std::cout << "(len=" << list.Length() << ")\n";
}
template <class T>
void PrintList(const char* title, const LinkList<T>& list) {
    std::cout << std::left << std::setw(18) << title << ": ";
    list.Traverse(PrintElem<T>);
    std::cout << "(len=" << list.Length() << ")\n";
}


//=============================== 演示主程序 ==============================
int main() {
    std::cout << "==== 链式线性表：单链表 / 循环链表 / 带缓存链表 ====\n\n";

    // --------- 1) 单链表（P49–P55）---------
    SimpleLinkList<int> sl;
    for (int x : {10, 20, 30}) sl.Insert(sl.Length() + 1, x);  // 尾插
    sl.Insert(2, 15);                                          // 在位序2前插入
    PrintList("SimpleLinkList", sl);

    int removed = 0;
    sl.Delete(3, removed);
    std::cout << "Delete(3) -> " << removed << "\n";
    PrintList("After Delete", sl);

    // --------- 2) 循环单链表（P57–P61）---------
    SimpleCircLinkList<int> cl;
    for (int x : {1, 2, 3, 4, 5}) cl.Insert(cl.Length() + 1, x);
    PrintList("CircularList", cl);
    cl.Delete(1);   // 删头（第一个数据结点）
    cl.Insert(1, 100);
    PrintList("After ops", cl);

    // --------- 3) 带缓存的 LinkList（P72–P74）---------
    LinkList<int> ll;
    for (int i = 1; i <= 5; ++i) ll.Insert(i, i * 10);         // 顺序插入：缓存有利
    PrintList("LinkList", ll);
    int val = 0; ll.GetElem(4, val);
    std::cout << "GetElem(4) = " << val << "\n";
    ll.SetElem(4, 99);
    PrintList("SetElem(4,99)", ll);
    ll.Delete(2);
    PrintList("Delete(2)", ll);

    // --------- 4) 约瑟夫问题（P62–P66）---------
    std::cout << "\n[约瑟夫问题] n=8, m=3\n";
    Josephus(8, 3);

    std::cout << "\n==== 演示结束 ====\n";
    return 0;
}
