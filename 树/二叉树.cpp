/**
 * 二叉树.cpp
 * ------------------------------------------------------------
 * 自包含的二叉树模板实现，覆盖课件“6.2 二叉树”“6.3 二叉树遍历”的主要接口：
 * - 结点：BinTreeNode<T>
 * - 类：BinaryTree<T>
 *   方法清单（与课件接口对应）：
 *   (1)  GetRoot() const
 *   (2)  Empty() const
 *   (3)  GetElem(node*, T&) const
 *   (4)  SetElem(node*, const T&)
 *   (5)  InOrder(void (*visit)(const T&)) const
 *   (6)  PreOrder(void (*visit)(const T&)) const
 *   (7)  PostOrder(void (*visit)(const T&)) const
 *   (8)  LevelOrder(void (*visit)(const T&)) const
 *   (9)  NodeCount() const
 *   (10) LeftChild(const node*) const
 *   (11) RightChild(const node*) const
 *   (12) Parent(const node*) const
 *   (13) InsertLeftChild(node*, const T&)
 *   (14) InsertRightChild(node*, const T&)
 *   (15) DeleteLeftChild(node*)
 *   (16) DeleteRightChild(node*)
 *   (17) Height() const
 *   —— 以及课件 6.3 拓展：
 *   - 非递归先/中/后序遍历 (NonRecurPreOrder / NonRecurInOrder / NonRecurPostOrder)
 *   - 由先序+中序构建二叉树 (CreateFromPreIn / CreateFromPreInSpan)
 *
 * 设计说明：
 * 1) parent 指针：便于 O(1) 求双亲（与课件 6.2.3(b) 三叉链表一致）。
 * 2) LevelOrder 使用 std::queue；非递归遍历使用 std::stack。
 * 3) 全部接口为强异常安全：若 new 失败会抛出 std::bad_alloc；析构/删除保证释放完整子树。
 *
 * 编译：g++ -std=c++17 -O2 -Wall -Wextra 二叉树.cpp -o btree_demo
 * 运行：./btree_demo
 */
#include <iostream>
#include <queue>
#include <stack>
#include <stdexcept>
#include <vector>
#include <algorithm>
using namespace std;

template <class T>
struct BinTreeNode {
    T data{};
    BinTreeNode* left{nullptr};
    BinTreeNode* right{nullptr};
    BinTreeNode* parent{nullptr};

    BinTreeNode() = default;
    explicit BinTreeNode(const T& v, BinTreeNode* l=nullptr, BinTreeNode* r=nullptr, BinTreeNode* p=nullptr)
        : data(v), left(l), right(r), parent(p) {}
};

template <class T>
class BinaryTree {
public:
    using Node = BinTreeNode<T>;

private:
    Node* root{nullptr};

    // --- 辅助：复制、销毁、遍历、统计、高度、查找双亲 ---
    static Node* copyTree(Node* r, Node* parent=nullptr) {
        if (!r) return nullptr;
        Node* n = new Node(r->data);
        n->parent = parent;
        n->left  = copyTree(r->left,  n);
        n->right = copyTree(r->right, n);
        return n;
    }

    static void destroy(Node*& r) {
        if (!r) return;
        destroy(r->left);
        destroy(r->right);
        delete r;
        r = nullptr;
    }

    static void preOrder(Node* r, void (*visit)(const T&)) {
        if (!r) return;
        visit(r->data);
        preOrder(r->left, visit);
        preOrder(r->right, visit);
    }
    static void inOrder(Node* r, void (*visit)(const T&)) {
        if (!r) return;
        inOrder(r->left, visit);
        visit(r->data);
        inOrder(r->right, visit);
    }
    static void postOrder(Node* r, void (*visit)(const T&)) {
        if (!r) return;
        postOrder(r->left, visit);
        postOrder(r->right, visit);
        visit(r->data);
    }

    static int height(Node* r) {
        if (!r) return 0;
        return max(height(r->left), height(r->right)) + 1;
    }
    static int nodeCount(Node* r) {
        if (!r) return 0;
        return nodeCount(r->left) + nodeCount(r->right) + 1;
    }

public:
    // --- 构造/析构/拷贝 ---
    BinaryTree() = default;
    explicit BinaryTree(const T& e) { root = new Node(e); }
    explicit BinaryTree(Node* r) : root(r) {}
    BinaryTree(const BinaryTree& other) { root = copyTree(other.root); }
    BinaryTree& operator=(const BinaryTree& other) {
        if (this == &other) return *this;
        Node* newRoot = copyTree(other.root);
        destroy(root);
        root = newRoot;
        return *this;
    }
    ~BinaryTree() { destroy(root); }

    // (1) 根
    const Node* GetRoot() const { return root; }
          Node* GetRoot()       { return root; }

    // (2) 判空
    bool Empty() const { return root == nullptr; }

    // (3) 取值
    bool GetElem(Node* cur, T& e) const {
        if (!cur) return false;
        e = cur->data;
        return true;
    }
    // (4) 设值
    bool SetElem(Node* cur, const T& e) {
        if (!cur) return false;
        cur->data = e;
        return true;
    }

    // (5)(6)(7) 递归遍历
    void PreOrder(void (*visit)(const T&)) const { preOrder(root, visit); }
    void InOrder (void (*visit)(const T&)) const { inOrder (root, visit); }
    void PostOrder(void (*visit)(const T&)) const{ postOrder(root, visit); }

    // (8) 层次遍历
    void LevelOrder(void (*visit)(const T&)) const {
        if (!root) return;
        queue<Node*> q;
        q.push(root);
        while (!q.empty()) {
            Node* cur = q.front(); q.pop();
            visit(cur->data);
            if (cur->left) q.push(cur->left);
            if (cur->right) q.push(cur->right);
        }
    }

    // (9) 结点计数
    int NodeCount() const { return nodeCount(root); }

    // (10)(11)(12) 左/右孩子、双亲
    Node* LeftChild (const Node* cur) const { return cur ? cur->left  : nullptr; }
    Node* RightChild(const Node* cur) const { return cur ? cur->right : nullptr; }
    Node* Parent    (const Node* cur) const { return cur ? cur->parent: nullptr; }

    // (13) 插入左孩子：若已有左子树，则成为新结点的左子树（与课件一致）
    void InsertLeftChild(Node* cur, const T& e) {
        if (!cur) throw invalid_argument("InsertLeftChild: cur is null");
        Node* n = new Node(e);
        n->left = cur->left;
        if (n->left) n->left->parent = n;
        n->right = nullptr;
        n->parent = cur;
        cur->left = n;
    }
    // (14) 插入右孩子：若已有右子树，则成为新结点的右子树
    void InsertRightChild(Node* cur, const T& e) {
        if (!cur) throw invalid_argument("InsertRightChild: cur is null");
        Node* n = new Node(e);
        n->right = cur->right;
        if (n->right) n->right->parent = n;
        n->left = nullptr;
        n->parent = cur;
        cur->right = n;
    }
    // (15) 删除左子树
    void DeleteLeftChild(Node* cur) {
        if (!cur) return;
        destroy(cur->left);
    }
    // (16) 删除右子树
    void DeleteRightChild(Node* cur) {
        if (!cur) return;
        destroy(cur->right);
    }

    // (17) 高度
    int Height() const { return height(root); }

    // ------------------ 6.3 非递归遍历 ------------------
    void NonRecurPreOrder(void (*visit)(const T&)) const {
        if (!root) return;
        stack<Node*> st;
        st.push(root);
        while (!st.empty()) {
            Node* cur = st.top(); 
            st.pop();
            visit(cur->data);
            if (cur->right) st.push(cur->right); // 先右后左，保证左先出栈
            if (cur->left)  st.push(cur->left);
        }
    }

    void NonRecurInOrder(void (*visit)(const T&)) const {
        stack<Node*> st;
        Node* cur = root;
        while (cur || !st.empty()) {
            while (cur) { 
                st.push(cur); cur = cur->left;
            }
            cur = st.top(); 
            st.pop();
            visit(cur->data);
            cur = cur->right;
        }
    }

    // 采用“标记法”后序：second=true 表示已访问过右子树/即可输出
    void NonRecurPostOrder(void (*visit)(const T&)) const {
        stack<pair<Node*, bool>> st;
        Node* cur = root;
        while (cur || !st.empty()) {
            while (cur) {
                st.push({cur, false});
                cur = cur->left;
            }
            auto [node, visitedRight] = st.top(); st.pop();
            if (!visitedRight) {
                st.push({node, true});
                cur = node->right;
            } else {
                visit(node->data);
            }
        }
    }

    // -------------- 由先序 + 中序 构建二叉树 --------------
    // 传入区间 [pl, pr] / [il, ir]（含端点）
    static Node* CreateFromPreInSpan(const vector<T>& pre, int pl, int pr,
                                     const vector<T>& in,  int il, int ir,
                                     Node* parent=nullptr) {
        if (pl > pr || il > ir) return nullptr;
        Node* r = new Node(pre[pl]);
        r->parent = parent;
        // 定位根在中序的位置
        int mid = il;
        while (mid <= ir && !(in[mid] == pre[pl])) ++mid;
        if (mid > ir) throw runtime_error("序列不匹配：先序/中序不一致");
        int leftSize = mid - il;
        r->left  = CreateFromPreInSpan(pre, pl+1, pl+leftSize, in, il, mid-1, r);
        r->right = CreateFromPreInSpan(pre, pl+leftSize+1, pr, in, mid+1, ir, r);
        return r;
    }

    static BinaryTree CreateFromPreIn(const vector<T>& pre, const vector<T>& in) {
        if (pre.size() != in.size()) throw invalid_argument("长度不一致");
        Node* r = CreateFromPreInSpan(pre, 0, (int)pre.size()-1, in, 0, (int)in.size()-1, nullptr);
        return BinaryTree(r);
    }
};

// ------------------ 演示 (可选) ------------------
static void print_char(const char& c){ cout<<c<<' '; }
int main(){
    // 由先序/中序创建：a b c d e f g / c b d a e g f（课件示例）
    vector<char> pre = {'a','b','c','d','e','f','g'};
    vector<char> in  = {'c','b','d','a','e','g','f'};
    auto bt = BinaryTree<char>::CreateFromPreIn(pre, in);

    cout<<"递归先序："; bt.PreOrder(print_char); cout<<"\n";
    cout<<"递归中序："; bt.InOrder(print_char);  cout<<"\n";
    cout<<"递归后序："; bt.PostOrder(print_char);cout<<"\n";
    cout<<"层次遍历："; bt.LevelOrder(print_char);cout<<"\n";
    cout<<"非递归先序："; bt.NonRecurPreOrder(print_char); cout<<"\n";
    cout<<"非递归中序："; bt.NonRecurInOrder(print_char);  cout<<"\n";
    cout<<"非递归后序："; bt.NonRecurPostOrder(print_char);cout<<"\n";
    cout<<"节点数："<<bt.NodeCount()<<", 高度："<<bt.Height()<<"\n";
    return 0;
}