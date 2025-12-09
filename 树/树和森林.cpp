/**
 * 树和森林.cpp
 * ------------------------------------------------------------
 * 自包含的“树与森林（孩子-兄弟表示）”与“二叉树”的互转与遍历实现，覆盖课件“6.5 树和森林”的要点：
 * - 一般树结点（孩子-兄弟表示）：GTNode<T> { firstChild, nextSibling }
 * - 二叉树结点（最小化定义）：BNode<T> { left, right }
 * - 遍历：
 *   * 树的先序 / 后序
 *   * 森林的先序 / 中序
 * - 互转：
 *   * ForestToBinary(root_of_forest)  —— 规则见课件：第一棵树根→B的根；其子树森林→B左子树；其余树森林→B右子树
 *   * BinaryToForest(root_of_binary)  —— 逆变换
 *
 * 编译：g++ -std=c++17 -O2 -Wall -Wextra 树和森林.cpp -o tf_demo
 */
#include <iostream>
#include <queue>
#include <stack>
#include <stdexcept>
#include <vector>
#include <algorithm>
using namespace std;

// ------------------ 一般树（孩子-兄弟表示） ------------------
template <class T>
struct GTNode
{
    T data{};
    GTNode *firstChild{nullptr};  // 第一个孩子
    GTNode *nextSibling{nullptr}; // 下一个兄弟
    GTNode() = default;
    explicit GTNode(const T &v) : data(v) {}
};

// ------------------ 简化版二叉树 ------------------
template <class T>
struct BNode
{
    T data{};
    BNode *left{nullptr}, *right{nullptr};
    BNode() = default;
    explicit BNode(const T &v) : data(v) {}
};

// ------------------ 遍历（树/森林） ------------------
template <class T, class F>
void PreOrderTree(GTNode<T> *r, F &&visit)
{
    if (!r)
        return;
    visit(r->data);
    for (auto c = r->firstChild; c; c = c->nextSibling)
        PreOrderTree(c, visit);
}
template <class T, class F>
void PostOrderTree(GTNode<T> *r, F &&visit)
{
    if (!r)
        return;
    for (auto c = r->firstChild; c; c = c->nextSibling)
        PostOrderTree(c, visit);
    visit(r->data);
}

template <class T, class F>
void PreOrderForest(GTNode<T> *forest, F &&visit)
{
    // forest = 多棵树的“链表”：第一棵树的根是 forest，其他树用 nextSibling 串起
    if (!forest)
        return;
    // 访问第一棵树的根
    visit(forest->data);
    // 先序遍历第一棵树的子树森林
    for (auto c = forest->firstChild; c; c = c->nextSibling)
        PreOrderTree(c, visit);
    // 先序遍历其它树组成的森林
    PreOrderForest(forest->nextSibling, visit);
}

template <class T, class F>
void InOrderForest(GTNode<T> *forest, F &&visit)
{
    // 中序（森林）：按课件定义——子树森林 → 根 → 其余森林
    if (!forest)
        return;
    for (auto c = forest->firstChild; c; c = c->nextSibling)
        PreOrderTree(c, visit); // 子树森林中序的常见教学版本可定义为：对每棵子树做中序（此处沿课件简化）
    visit(forest->data);
    InOrderForest(forest->nextSibling, visit);
}

// ------------------ 森林 ↔ 二叉树 转换 ------------------
// 规则（森林→二叉树）：第一棵树根→B的根；根的子树森林→B左子树；其余树森林→B右子树
template <class T>
BNode<T> *ForestToBinary(GTNode<T> *forest)
{
    if (!forest)
        return nullptr;
    BNode<T> *b = new BNode<T>(forest->data);
    // 左子树：第一棵树根的子树森林
    if (forest->firstChild)
    {
        // 把第一棵子树作为左孩子，其余孩子连成“右兄弟链”
        b->left = ForestToBinary(forest->firstChild); // 第一个孩子作为左子树根
        // 处理兄弟：在二叉树里串到右边
        GTNode<T> *sib = forest->firstChild->nextSibling;
        BNode<T> *cur = b->left;
        while (sib)
        {
            cur->right = ForestToBinary(sib);
            cur = cur->right;
            sib = sib->nextSibling;
        }
    }
    // 右子树：森林其余树
    b->right = ForestToBinary(forest->nextSibling);
    return b;
}

// 规则（二叉树→森林）：B根→第一棵树根；B左子树→其子树森林；B右子树→其余树森林
template <class T>
GTNode<T> *BinaryToForest(BNode<T> *b)
{
    if (!b)
        return nullptr;
    GTNode<T> *root = new GTNode<T>(b->data);
    // 左链还原为“孩子链”
    if (b->left)
    {
        root->firstChild = BinaryToForest(b->left);
        // 右指针表示兄弟：将 b->left 的“右边链”变回兄弟链
        BNode<T> *curB = b->left->right;
        GTNode<T> *curG = root->firstChild;
        while (curB)
        {
            curG->nextSibling = BinaryToForest(curB);
            curG = curG->nextSibling;
            curB = curB->right;
        }
    }
    // 右子树对应“其余树森林”：作为 root 的 nextSibling
    root->nextSibling = BinaryToForest(b->right);
    return root;
}

// ------------------ 内存释放 ------------------
template <class T>
void DestroyForest(GTNode<T> *&f)
{
    if (!f)
        return;
    // 先递归释放所有孩子
    DestroyForest(f->firstChild);
    // 再递归释放所有兄弟
    DestroyForest(f->nextSibling);
    // 最后释放当前节点
    delete f;
    f = nullptr;
}
template <class T>
void DestroyBTree(BNode<T> *&r)
{
    if (!r)
        return;
    DestroyBTree(r->left);
    DestroyBTree(r->right);
    delete r;
    r = nullptr;
}

// 演示主函数：直接运行森林↔二叉树互转示例
static void print_char(const char &c) { cout << c << ' '; }
int main()
{
    // 构造一棵示例“树的森林”：两棵树 T1(A) 与 T2(H)
    // T1: A 的孩子 B,C,D；B 的孩子 E,F；D 的孩子 G
    GTNode<char> *A = new GTNode<char>('A');
    GTNode<char> *B = new GTNode<char>('B');
    GTNode<char> *C = new GTNode<char>('C');
    GTNode<char> *D = new GTNode<char>('D');
    GTNode<char> *E = new GTNode<char>('E');
    GTNode<char> *F = new GTNode<char>('F');
    GTNode<char> *G = new GTNode<char>('G');
    A->firstChild = B;
    B->nextSibling = C;
    C->nextSibling = D;
    B->firstChild = E;
    E->nextSibling = F;
    D->firstChild = G;
    // 第二棵树 T2: H 的孩子 I,J
    GTNode<char> *H = new GTNode<char>('H');
    GTNode<char> *I = new GTNode<char>('I');
    GTNode<char> *J = new GTNode<char>('J');
    H->firstChild = I;
    I->nextSibling = J;
    // 森林：A 和 H
    A->nextSibling = H;

    cout << "森林先序：";
    PreOrderForest(A, print_char);
    cout << "\n";

    auto b = ForestToBinary(A);
    // 将二叉树做“伪中序”：左链->根->右链（只是为了打印效果）
    function<void(BNode<char> *)> inorderB = [&](BNode<char> *r)
    {
        if (!r)
            return;
        inorderB(r->left);
        cout << r->data << ' ';
        inorderB(r->right);
    };
    cout << "转换为二叉树后中序打印：";
    inorderB(b);
    cout << "\n";

    auto f2 = BinaryToForest(b);
    cout << "还原森林先序：";
    PreOrderForest(f2, print_char);
    cout << "\n";

    // 释放内存：A是原始森林，b是转换后的二叉树，f2是还原的森林
    // 它们各自独立分配，需要分别释放
    DestroyForest(A);
    DestroyBTree(b);
    DestroyForest(f2);
    return 0;
}