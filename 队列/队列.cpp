// 队列.cpp
// 参考教材：《数据结构与算法分析》Chapter 3 栈和队列（四川大学 计算机学院 孙奕髦）
// 本文件根据“3.2 队列”部分实现两种队列：链队列（LinkQueue）与循环顺序队列（CircQueue）。
// 关键定义、判空/判满条件与入队/出队规则均严格依据讲义：
//   - 队列定义与FIFO特性：第34页（3.2.1 队列的基本概念，front/rear，两端约束，先进先出）
//   - 基本操作（Length/Empty/Clear/Traverse/OutQueue/GetHead/InQueue）：第35-38页接口列表
//   - 链队列：第40-42页（front==rear 为队空；类模板接口）
//   - 循环队列：第45-46页（取模实现首尾相接；front, rear 自加；count==0 空，count==maxSize 满）
//   - 循环队列入/出队伪码与遍历：第50-52页
// 代码中在相应处附有页码与要点提示。
//
// 编译：g++ -std=c++17 队列.cpp -o queue_demo
// 运行：./queue_demo
//
// ---------------------------------------------------------------

#include <iostream>
#include <functional>
#include <string>
#include <cassert>
using namespace std;

#ifndef DEFAULT_SIZE
#define DEFAULT_SIZE 8  // 默认容量，可在构造时覆盖（参见循环队列类说明，P46）
#endif

// ==============================
// 基础结点：单链表结点（用于链队列）
// ==============================
template <class T>
struct Node {
    T data;
    Node<T>* next;
    explicit Node(const T& d, Node<T>* n=nullptr) : data(d), next(n) {}
};

// ==============================
// 链队列（LinkQueue） —— 参考 P40~P42
// 设计要点：
//   1) 采用“带头结点”的单链表，保持 front 指向头结点，rear 指向最后一个实际结点。
//   2) 判空条件：front == rear，对应讲义“队空条件为 front == rear”（P40）。
//   3) 入队：在 rear 之后追加结点；出队：删除头结点后的第一个实际结点。
//   4) 计数器 count 维护队列长度（P42 接口列表中的 Length/Empty 语义）。
// ==============================
template <class T>
class LinkQueue {
private:
    Node<T>* front; // 头结点（不存有效数据）—— 队头前驱（P40 图示）
    Node<T>* rear;  // 指向最后一个实际结点（队尾）
    int count;      // 元素个数
public:
    // 构造/析构
    LinkQueue() : front(new Node<T>(T{})), rear(front), count(0) {
        // 初始为空队列：front==rear（P40）
    }
    ~LinkQueue() { Clear(); delete front; front = rear = nullptr; }

    // 拷贝构造（深拷贝）
    LinkQueue(const LinkQueue& other) : LinkQueue() {
        for (Node<T>* p = other.front->next; p != nullptr; p = p->next) {
            InQueue(p->data);
        }
    }
    // 赋值运算（深拷贝）
    LinkQueue& operator=(const LinkQueue& other) {
        if (this == &other) return *this;
        Clear();
        for (Node<T>* p = other.front->next; p != nullptr; p = p->next) {
            InQueue(p->data);
        }
        return *this;
    }

    // 基本操作 —— 接口名称与讲义保持一致（P35-38, P42）
    int Length() const { return count; }                       // P35 (1)
    bool Empty() const { return front == rear; }               // P40 队空条件
    void Clear() {                                             // P35 (3)
        T tmp;
        while (OutQueue(tmp)) {}
    }

    // 从队头到队尾依次访问（P36 (4)：Traverse）
    void Traverse(void (*visit)(const T&)) const {
        for (Node<T>* p = front->next; p != nullptr; p = p->next) visit(p->data);
    }

    // 出队（删除队头元素，用 e 返回其值）—— P36 (5)、P37 (6)
    bool OutQueue(T& e) {
        if (Empty()) return false;
        Node<T>* first = front->next;   // 真实队头
        e = first->data;
        front->next = first->next;
        if (rear == first) rear = front; // 若删除了最后一个元素，恢复到空队列：front==rear
        delete first;
        --count;
        return true;
    }

    // 无返回值版本（P36-37），语义：仅删除队头
    bool OutQueue() {
        T dummy;
        return OutQueue(dummy);
    }

    // 取队头（不删除）—— P37 (7)
    bool GetHead(T& e) const {
        if (Empty()) return false;
        e = front->next->data;
        return true;
    }

    // 入队（在队尾插入）—— P38 (8)
    bool InQueue(const T& e) {
        Node<T>* node = new (std::nothrow) Node<T>(e);
        if (!node) return false; // 动态内存耗尽
        rear->next = node;
        rear = node;
        ++count;
        return true;
    }
};

// ==============================
// 循环顺序队列（CircQueue） —— 参考 P45~P52
// 设计要点：
//   1) 顺序表首尾相接（Circular）：front / rear 自加采用取模（P46）。
//   2) 判空：count == 0；判满：count == maxSize（P46）。
//   3) 入队：elem[rear]=e; rear=(rear+1)%maxSize; count++（P52）。
//   4) 出队：e=elem[front]; front=(front+1)%maxSize; count--（P51）。
//   5) 遍历：i 从 front 起，i!=rear，i=(i+1)%maxSize（P50）。
// ==============================
template <class T>
class CircQueue {
private:
    T* elems;
    int front;
    int rear;
    int maxSize;
    int count;
public:
    explicit CircQueue(int size=DEFAULT_SIZE)
        : elems(new T[size]), front(0), rear(0),
          maxSize(size), count(0) {}

    ~CircQueue() { delete[] elems; }

    // 拷贝构造
    CircQueue(const CircQueue& other)
        : elems(new T[other.maxSize]), front(other.front), rear(other.rear),
          maxSize(other.maxSize), count(other.count) {
        // 拷贝底层数组（注意：仅拷贝有效区间并不必要，但简洁起见全拷贝）
        for (int i = 0; i < maxSize; ++i) elems[i] = other.elems[i];
    }
    // 赋值运算
    CircQueue& operator=(const CircQueue& other) {
        if (this == &other) return *this;
        if (maxSize != other.maxSize) {
            delete[] elems;
            elems = new T[other.maxSize];
            maxSize = other.maxSize;
        }
        front = other.front; rear = other.rear; count = other.count;
        for (int i = 0; i < maxSize; ++i) elems[i] = other.elems[i];
        return *this;
    }

    // 基本操作 —— 名称与讲义一致（P49）
    int  Length() const { return count; }             // P35 (1) / P49
    bool Empty() const  { return count == 0; }        // P46
    void Clear() { front = rear = 0; count = 0; }     // P46 初始化语义

    void Traverse(void (*visit)(const T&)) const {    // P50
        for (int i = front; i != rear; i = (i + 1) % maxSize) visit(elems[i]);
    }

    // 出队（删除队头并返回）—— P51
    bool OutQueue(T& e) {
        if (Empty()) return false;                    // P45：队空时出队需处理
        e = elems[front];
        front = (front + 1) % maxSize;                // P46
        --count;
        return true;
    }
    bool OutQueue() {
        T dummy;
        return OutQueue(dummy);
    }

    // 取队头（不删除）—— P49 （GetHead）
    bool GetHead(T& e) const {
        if (Empty()) return false;
        e = elems[front];
        return true;
    }

    // 入队（在队尾插入）—— P52
    bool InQueue(const T& e) {
        if (count == maxSize) return false;           // P46：队满条件
        elems[rear] = e;
        rear = (rear + 1) % maxSize;                  // P46
        ++count;
        return true;
    }
};

// ==============================
// 演示与自检（可按需删除 main）
// ==============================
template <class T>
void print_item(const T& x) { cout << x << ' '; }

int main() {
    cout << "==== LinkQueue<int> 演示（P40~P42 语义） ====\n";
    LinkQueue<int> lq;
    assert(lq.Empty());
    for (int i = 1; i <= 5; ++i) {
        bool ok = lq.InQueue(i);
        assert(ok);
    }
    cout << "长度: " << lq.Length() << "，遍历：";
    lq.Traverse(&print_item<int>);
    cout << "\n";

    int head;
    bool ok = lq.GetHead(head);
    assert(ok && head == 1);
    cout << "队头元素（不删除）: " << head << "\n";

    int x;
    ok = lq.OutQueue(x); assert(ok && x == 1);
    ok = lq.OutQueue(x); assert(ok && x == 2);
    cout << "出队两个后，长度: " << lq.Length() << "，遍历：";
    lq.Traverse(&print_item<int>);
    cout << "\n\n";

    cout << "==== CircQueue<string> 演示（P45~P52 语义） ====\n";
    CircQueue<string> cq(5); // 容量5（注意：count==maxSize 判满，P46）
    assert(cq.Empty());
    for (string s : {"A","B","C","D"}) {
        bool ok2 = cq.InQueue(s);
        assert(ok2);
    }
    cout << "长度: " << cq.Length() << "，遍历：";
    cq.Traverse(&print_item<string>);
    cout << "\n";

    string hs;
    ok = cq.GetHead(hs); assert(ok && hs=="A");
    cout << "队头元素（不删除）: " << hs << "\n";

    string y;
    ok = cq.OutQueue(y); assert(ok && y=="A");
    ok = cq.OutQueue(y); assert(ok && y=="B");
    cout << "出队两个后入队 E,F：";
    bool okE = cq.InQueue("E"); assert(okE);
    bool okF = cq.InQueue("F"); // 容量5，此时应刚好满
    cout << (okF ? "成功" : "失败(队满)") << "\n";

    cout << "当前长度: " << cq.Length() << "，遍历：";
    cq.Traverse(&print_item<string>);
    cout << "\n";

    // 再尝试入队（应失败，因为满）
    bool okX = cq.InQueue("X");
    cout << "继续入队 X：" << (okX ? "成功" : "失败(队满，P46)") << "\n";

    cout << "全部演示通过。\n";
    return 0;
}
