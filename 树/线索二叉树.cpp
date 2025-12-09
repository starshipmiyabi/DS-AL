/**
 * 线索二叉树.cpp
 * ------------------------------------------------------------
 * 自包含的“中序线索二叉树”模板实现，覆盖课件“6.4 线索二叉树”的关键要点：
 * - 结点：ThreadNode<T>，含 ltag/rtag（0=孩子指针，1=前驱/后继线索）
 * - 构造：从普通二叉链复制，然后进行中序线索化（含可选“带头结点”版本）
 * - 遍历：中序线索遍历（无递归、无栈）
 *
 * 编译：g++ -std=c++17 -O2 -Wall -Wextra 线索二叉树.cpp -o tbt_demo
 */
#include <iostream>
#include <queue>
#include <stack>
#include <stdexcept>
#include <vector>
#include <algorithm>
using namespace std;
template <class T>
struct ThreadNode {
    T data{};
    ThreadNode* left{nullptr};
    ThreadNode* right{nullptr};
    // 0 表示指向孩子；1 表示指向前驱/后继（线索）
    int ltag{0};
    int rtag{0};
    ThreadNode() = default;
    explicit ThreadNode(const T& v): data(v) {}
};

/**
 * 普通二叉链 → 线索二叉树（中序）
 * 为了自包含，这里也定义一个最小版普通二叉链节点。
 */
template <class T>
struct RawNode {
    T data{};
    RawNode *left{nullptr}, *right{nullptr};
    RawNode() = default;
    explicit RawNode(const T& v): data(v) {}
};

template <class T>
class ThreadedBinaryTree {
public:
    using TNode = ThreadNode<T>;
    using RNode = RawNode<T>;
private:
    TNode* head{nullptr};   // 可选：带头结点版本的头结点（中序最小前驱/最大后继）
    TNode* root{nullptr};   // 线索化后的根（真实根）

    static void destroy(TNode*& p) {
        if (!p) return;
        // 按照“仅沿着孩子指针递归”来销毁；线索指针不递归进入
        if (p->ltag == 0) destroy(p->left);
        if (p->rtag == 0) destroy(p->right);
        delete p; p=nullptr;
    }

    static TNode* copyFromRaw(const RNode* r) {
        if (!r) return nullptr;
        TNode* n = new TNode(r->data);
        n->left  = copyFromRaw(r->left);
        n->right = copyFromRaw(r->right);
        // 初始均为“孩子”语义
        n->ltag = (n->left  ? 0 : 1);
        n->rtag = (n->right ? 0 : 1);
        return n;
    }

    // 中序线索化：prev 始终指向“中序前驱”
    static void inorderThreading(TNode* p, TNode*& prev) {
        if (!p) return;
        if (p->ltag == 0) inorderThreading(p->left, prev); // 左子树
        // 处理当前结点 p 与前驱 prev 的线索
        if (p->left == nullptr) { p->ltag = 1; p->left  = prev; }
        if (prev && prev->right == nullptr) { prev->rtag = 1; prev->right = p; }
        prev = p;
        if (p->rtag == 0) inorderThreading(p->right, prev); // 右子树
    }

public:
    ThreadedBinaryTree() = default;
    ~ThreadedBinaryTree() { clear(); }

    void clear() {
        // 如果有头结点，则从 head->left 开始销毁
        if (head) {
            TNode* realRoot = head->left;
            destroy(realRoot);
            delete head; head = nullptr; root = nullptr;
        } else {
            destroy(root);
            root = nullptr;
        }
    }

    // 从普通二叉链（RawNode）复制并线索化；useHead=true 生成带头结点版本
    void BuildFromRawInorderThreaded(const RNode* rawRoot, bool useHeadNode = true) {
        clear();
        if (!useHeadNode) {
            root = copyFromRaw(rawRoot);
            TNode* prev = nullptr;
            inorderThreading(root, prev);
        } else {
            head = new TNode(); // 头结点 data 默认
            root = copyFromRaw(rawRoot);
            head->ltag = 0; head->left = root;   // 头结点左指针指向根
            head->rtag = 1; head->right = head;  // 头结点右线索回指自身
            TNode* prev = head;
            inorderThreading(root, prev);
            // 完成后 prev 指向中序最后一个结点，将其后继线索回到 head
            prev->rtag = 1; prev->right = head;
            head->left = root;
        }
    }

    // 中序的第一个结点（最左）
    static TNode* first(TNode* p) {
        if (!p) return nullptr;
        while (p->ltag == 0) p = p->left;
        return p;
    }
    // 中序的后继：若 rtag=1，直接线索；否则到右子树最左
    static TNode* next(TNode* p) {
        if (!p) return nullptr;
        if (p->rtag == 1) return p->right;
        p = p->right;
        while (p && p->ltag == 0) p = p->left;
        return p;
    }

    // 中序线索遍历（不带头/带头都支持）：不使用递归和栈
    void InOrderTraverse(void (*visit)(const T&)) const {
        if (head) {
            // 带头结点：从 head->left 的最左开始，直到回到 head
            TNode* p = head->left;
            if (!p) return;
            p = first(p);
            while (p && p != head) {
                visit(p->data);
                p = next(p);
            }
        } else {
            TNode* p = first(root);
            while (p) {
                visit(p->data);
                p = next(p);
            }
        }
    }

    // 获取真实根（线索化后的树根，便于测试）
    const TNode* GetRoot() const { return root; }

    // ----------- 工具：构造一个示例 RawNode 树（便于独立测试） -----------
    static RNode* MakeSampleRaw() {
        //        A
        //      /   \
        //     B     C
        //    / \     \
        //   D   E     F
        RNode* A = new RNode('A');
        RNode* B = new RNode('B');
        RNode* C = new RNode('C');
        RNode* D = new RNode('D');
        RNode* E = new RNode('E');
        RNode* F = new RNode('F');
        A->left=B; A->right=C; B->left=D; B->right=E; C->right=F;
        return A;
    }

    // 清理 Raw 树（普通二叉链）
    static void DestroyRaw(RNode*& r) {
        if (!r) return;
        DestroyRaw(r->left);
        DestroyRaw(r->right);
        delete r; r=nullptr;
    }
};

// ------------------ 演示 (直接运行线索二叉树演示) ------------------
static void print_char(const char& c){ cout<<c<<' '; }
int main(){
    using TBT = ThreadedBinaryTree<char>;
    auto raw = TBT::MakeSampleRaw();
    TBT tbt;
    tbt.BuildFromRawInorderThreaded(raw, /*useHeadNode=*/true);
    cout<<"中序线索遍历：";
    tbt.InOrderTraverse(print_char);
    cout<<"\n";
    TBT::DestroyRaw(raw);
    return 0;
}