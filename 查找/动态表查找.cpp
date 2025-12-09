// 动态表查找.cpp
// 对应课件：Chapter 8.3 动态查找表
//   - 8.3.1 二叉排序树（二叉查找树）：第 23–34 页
//   - 8.3.2 二叉平衡树（AVL 树）：第 36–47 页
//   - 8.3.3 B 树：第 49–54 页（概念性介绍，本文件仅实现简化版查找）
//   - 8.3.4 B+ 树：第 56–58 页（概念性介绍，本文件仅实现简化版查找接口）
//
// 课件给出了典型代码：
//   template <class ElemType, class KeyType>
//   BinTreeNode<ElemType> *BinarySortTree<ElemType, KeyType>::SearchHelp(...)
//   （见第 29 页）
// 本文件在此基础上：
//   1. 定义二叉排序树（BST）的结点结构与查找/插入操作；
//   2. 定义 AVL 树，在 BST 基础上增加平衡因子、单旋/双旋（LL, RR, LR, RL 型），
//      对应第 36–47 页中的“平衡因子与平衡化旋转”；
//   3. 定义一个极简 B 树/B+ 树查找接口以帮助理解“多路平衡查找树”的查找思想。
//
// 注意：
//   这些代码主要用于教学演示数据结构与查找原理，不追求工业级完整性。

#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>

//-------------------------------------------------------------
// 一、二叉排序树（Binary Search Tree，BST）
//    对应 8.3.1 二叉排序树，第 23–34 页
//-------------------------------------------------------------

/**
 * @brief 二叉排序树节点
 *
 * 对应课件：
 *   - 第 23 页的定义：左子树关键字 < 根结点关键字 < 右子树关键字；
 *   - 第 25 页：中序遍历可以得到有序序列。
 */
struct BSTNode
{
    int key;          // 关键字
    BSTNode *left;    // 左子树
    BSTNode *right;   // 右子树

    BSTNode(int k) : key(k), left(nullptr), right(nullptr) {}
};

/**
 * @brief 在二叉排序树中查找 key
 *
 * 对应课件第 26–29 页：
 *   - 若 key == 根结点关键字：查找成功；
 *   - 若 key < 根结点关键字：在左子树继续查找；
 *   - 若 key > 根结点关键字：在右子树继续查找；
 *   - 一直走到空指针则查找失败。
 *
 * @param root  根结点
 * @param key   要查找的关键字
 * @param parent 如果需要，可返回找到结点的父节点（模仿 SearchHelp 中的 f）
 * @return      若找到，返回对应指针；否则返回 nullptr
 */
BSTNode *BSTSearch(BSTNode *root, int key, BSTNode *&parent)
{
    BSTNode *p = root;     // 当前结点
    parent = nullptr;      // 当前结点的父结点

    while (p != nullptr && p->key != key)
    {
        parent = p;        // 记录父亲
        if (key < p->key)
            p = p->left;   // 在左子树继续查找
        else
            p = p->right;  // 在右子树继续查找
    }
    return p; // 找到返回结点指针；找不到则为 nullptr
}

/**
 * @brief BST 插入
 *
 * 插入规则：沿着查找路径找到空位置插入新结点，
 * 保证插入后仍满足“左 < 根 < 右”的性质。
 *
 * 对应课件：第 31–32 页“根据关键字建立二叉查找树的过程”。
 */
BSTNode *BSTInsert(BSTNode *&root, int key)
{
    BSTNode *parent = nullptr;
    BSTNode *p = BSTSearch(root, key, parent);

    if (p != nullptr)
    {
        // 关键字已存在，这里示例中直接返回原结点
        return p;
    }

    BSTNode *newNode = new BSTNode(key);
    if (parent == nullptr)
    {
        // 原树为空，新结点作为根
        root = newNode;
    }
    else if (key < parent->key)
    {
        parent->left = newNode;
    }
    else
    {
        parent->right = newNode;
    }
    return newNode;
}

/**
 * @brief 中序遍历 BST，并输出关键字
 * 对应课件第 25 页：中序遍历得到有序序列。
 */
void inorderBST(BSTNode *root)
{
    if (!root) return;
    inorderBST(root->left);
    std::cout << root->key << " ";
    inorderBST(root->right);
}

//-------------------------------------------------------------
// 二、AVL 树（平衡二叉查找树）
//    对应 8.3.2 二叉平衡树，第 36–47 页
//-------------------------------------------------------------

/**
 * @brief AVL 树结点
 *
 * 对应课件：
 *   - 第 36 页：AVL 树定义；
 *   - 第 37 页：平衡因子（balance factor = 左子树高 - 右子树高）；
 *   - 第 38–47 页：单旋/双旋示意图与 LL, RR, LR, RL 四类失衡。
 */
struct AVLNode
{
    int key;
    int height;     // 当前结点的高度（空树高度定义为 0）
    AVLNode *left;
    AVLNode *right;

    AVLNode(int k)
        : key(k), height(1), left(nullptr), right(nullptr) {}
};

int height(AVLNode *node)
{
    return node ? node->height : 0;
}

// 更新结点高度：1 + max(左子树高度, 右子树高度)
void updateHeight(AVLNode *node)
{
    if (node)
        node->height = 1 + std::max(height(node->left), height(node->right));
}

// 计算平衡因子 = 左子树高度 - 右子树高度
int balanceFactor(AVLNode *node)
{
    return node ? (height(node->left) - height(node->right)) : 0;
}

//--------------------- 单旋转：右旋（LL 型，对应第 43 页） --------------------
/**
 * @brief 右旋转（Right Rotation）
 * 场景：
//   对应课件中 LL 型失衡：在“某结点 A 的左子树的左子树”插入新结点，
//   导致 A 的平衡因子由 1 变为 2，需要进行一次右旋。
 *
 * 旋转示意（与第 43 页一一对应）：
 *        A                      B
 *       / \                    / \
 *      B   AR      ==>       BL  A
 *     / \                        / \
 *    BL BR                     BR  AR
 */
AVLNode *rotateRight(AVLNode *y)
{
    AVLNode *x = y->left;
    AVLNode *T2 = x->right;

    // 右旋
    x->right = y;
    y->left = T2;

    // 更新高度
    updateHeight(y);
    updateHeight(x);

    return x; // 新根
}

//--------------------- 单旋转：左旋（RR 型，对应第 42 页） --------------------
/**
 * @brief 左旋转（Left Rotation）
 * 场景：
 *   对应课件中 RR 型失衡：在“某结点 A 的右子树的右子树”插入新结点，
 *   导致 A 的平衡因子由 -1 变为 -2，需要进行一次左旋。
 *
 * 旋转示意（与第 42 页一一对应）：
 *      A                           B
 *     / \                         / \
 *    AL  B         ==>           A  BR
 *       / \                     / \
 *      BL BR                  AL BL
 */
AVLNode *rotateLeft(AVLNode *x)
{
    AVLNode *y = x->right;
    AVLNode *T2 = y->left;

    // 左旋
    y->left = x;
    x->right = T2;

    // 更新高度
    updateHeight(x);
    updateHeight(y);

    return y; // 新根
}

//--------------------- 插入并保持平衡 --------------------
/**
 * @brief 向 AVL 树插入 key
 *
 * 对应课件第 39–47 页：
 *   - 插入新结点后，自底向上检查平衡因子；
 *   - 若平衡因子绝对值大于 1，则根据插入所在子树位置做：
 *       LL 型：右旋
 *       RR 型：左旋
 *       LR 型：先左旋再右旋
 *       RL 型：先右旋再左旋
 */
AVLNode *AVLInsert(AVLNode *node, int key)
{
    // 1. 普通 BST 插入
    if (!node)
        return new AVLNode(key);

    if (key < node->key)
        node->left = AVLInsert(node->left, key);
    else if (key > node->key)
        node->right = AVLInsert(node->right, key);
    else
        return node; // 不允许重复关键字

    // 2. 更新高度
    updateHeight(node);

    // 3. 检查平衡因子并做相应旋转
    int bf = balanceFactor(node);

    // LL 型（左左）：在左孩子的左子树插入
    if (bf > 1 && key < node->left->key)
        return rotateRight(node);

    // RR 型（右右）：在右孩子的右子树插入
    if (bf < -1 && key > node->right->key)
        return rotateLeft(node);

    // LR 型（左右）：在左孩子的右子树插入
    if (bf > 1 && key > node->left->key)
    {
        node->left = rotateLeft(node->left);   // 先对左孩子左旋
        return rotateRight(node);             // 再对自己右旋
    }

    // RL 型（右左）：在右孩子的左子树插入
    if (bf < -1 && key < node->right->key)
    {
        node->right = rotateRight(node->right); // 先对右孩子右旋
        return rotateLeft(node);               // 再对自己左旋
    }

    // 若仍平衡，直接返回
    return node;
}

// 中序遍历 AVL 树（应得到有序序列）
void inorderAVL(AVLNode *root)
{
    if (!root) return;
    inorderAVL(root->left);
    std::cout << root->key << " ";
    inorderAVL(root->right);
}

//-------------------------------------------------------------
// 三、极简 B 树 / B+ 树查找接口（概念性实现）
//    对应 8.3.3 / 8.3.4 第 49–58 页
//-------------------------------------------------------------
// 为了突出“多路平衡查找树”的查找思想，本节只实现：
//   - BTreeNode/BPTreeNode 结构；
//   - 在节点内顺序查找/折半查找关键字；
//   - 自顶向下沿指针查找的递归过程。
//
// 真正工业级的 B 树/B+ 树实现非常复杂，尤其是插入、删除和磁盘存储等，
// 超出了本课程 PPT 中代码的范围。

//--------------------- B 树（简化版） --------------------
struct BTreeNode
{
    bool leaf;                     // 是否为叶子
    std::vector<int> keys;         // 关键字 K1 < K2 < ... < Kn
    std::vector<BTreeNode *> c;    // 子树指针 A0..An（size = keys.size() + 1）

    BTreeNode(bool isLeaf = true) : leaf(isLeaf) {}
};

/**
 * @brief 在单个 B 树节点内查找 key 所在位置
 *        若找到，返回 (true, index)
 *        若找不到，返回 (false, index)，其中 index 为下一步应走向的子树编号。
 *
 * 对应课件第 52 页：
 *   - 关键字有序；
 *   - Ai-1 指向的子树上所有关键字 < Ki，Ai 指向的子树上所有关键字 > Ki。
 */
std::pair<bool, size_t> BTreeSearchInNode(BTreeNode *node, int key)
{
    size_t i = 0;
    // 顺序查找，也可改为折半查找
    while (i < node->keys.size() && key > node->keys[i])
        ++i;

    if (i < node->keys.size() && key == node->keys[i])
        return {true, i};
    else
        return {false, i}; // 应该到子树 c[i] 中继续查找
}

/**
 * @brief 在 B 树上查找 key（简化版）
 *
 * 对应课件第 54 页：“从根节点出发，沿指针查找节点并在节点内查找（顺序或折半）”。
 */
BTreeNode *BTreeSearch(BTreeNode *root, int key)
{
    if (!root)
        return nullptr;

    BTreeNode *p = root;
    while (p)
    {
        auto [found, index] = BTreeSearchInNode(p, key);
        if (found)
            return p;

        if (p->leaf)
            return nullptr; // 已到叶子节点仍未找到，查找失败

        p = p->c[index]; // 沿指针继续向下
    }
    return nullptr;
}

//--------------------- B+ 树（极简版） --------------------
struct BPlusTreeNode
{
    bool leaf;
    std::vector<int> keys;
    std::vector<BPlusTreeNode *> c; // 非叶：子节点；叶：指向记录或下层结构
    BPlusTreeNode *next;            // 叶子链表，用于范围/顺序查找（对应第 56–57 页）

    BPlusTreeNode(bool isLeaf = true)
        : leaf(isLeaf), next(nullptr) {}
};

/**
 * @brief 在 B+ 树节点内查找 key 所在范围
 *        与 B 树类似，但需要注意 B+ 树非叶节点中的 Ki 通常是子树中的“最大值/分界值”（第 57 页）。
 */
std::pair<bool, size_t> BPlusTreeSearchInNode(BPlusTreeNode *node, int key)
{
    size_t i = 0;
    while (i < node->keys.size() && key > node->keys[i])
        ++i;

    if (node->leaf && i < node->keys.size() && key == node->keys[i])
        return {true, i};

    return {false, i};
}

/**
 * @brief 在 B+ 树上查找 key（简化版）
 *
 * 对应课件第 58 页：
 *   - 缩小范围查找：从根到叶；
 *   - 不论成功与否，一定查到叶子结点为止。
 */
BPlusTreeNode *BPlusTreeSearch(BPlusTreeNode *root, int key)
{
    if (!root)
        return nullptr;

    BPlusTreeNode *p = root;
    while (true)
    {
        auto [found, index] = BPlusTreeSearchInNode(p, key);
        if (p->leaf)
        {
            // 到叶子结点，如果 found 则成功，否则失败
            return found ? p : nullptr;
        }
        else
        {
            // 非叶结点，根据“Ki 为对应子树最大关键字”的规则
            // 若 key <= Ki，则应走到 Ai-1；否则走到最后一棵子树。
            if (index >= p->c.size())
                index = p->c.size() - 1;
            p = p->c[index];
        }
    }
}

//-------------------------------------------------------------
// 四、演示 main
//-------------------------------------------------------------

int main()
{
    std::cout << "================ 动态查找表示例 ================\n\n";

    //------------- 1. 二叉排序树示例（对应第 31–34 页） -------------
    {
        std::cout << "[1] 二叉排序树 BST 示例（对应课件 8.3.1）\n";
        int keys[] = {53, 78, 65, 17, 87, 9, 81, 15};
        int n = sizeof(keys) / sizeof(keys[0]);

        BSTNode *root = nullptr;
        for (int i = 0; i < n; ++i)
            BSTInsert(root, keys[i]);

        std::cout << "中序遍历（应为有序）：";
        inorderBST(root);
        std::cout << "\n";

        BSTNode *parent = nullptr;
        int searchKey = 81;
        BSTNode *res = BSTSearch(root, searchKey, parent);
        if (res)
            std::cout << "查找关键字 " << searchKey << " 成功，结点地址 = " << res << "\n";
        else
            std::cout << "查找关键字 " << searchKey << " 失败\n";
        std::cout << "\n";
    }

    //------------- 2. AVL 树示例（对应第 36–47 页插入+旋转） -------------
    {
        std::cout << "[2] AVL 树示例（对应课件 8.3.2）\n";
        int keysAVL[] = {5, 4, 2, 8, 6, 9}; // PPT 第 46–47 页的插入序列
        int n = sizeof(keysAVL) / sizeof(keysAVL[0]);

        AVLNode *rootAVL = nullptr;
        for (int i = 0; i < n; ++i)
        {
            rootAVL = AVLInsert(rootAVL, keysAVL[i]);
            std::cout << "插入 " << keysAVL[i] << " 后，中序遍历：";
            inorderAVL(rootAVL);
            std::cout << "  （高度 = " << height(rootAVL)
                      << ", 平衡因子 = " << balanceFactor(rootAVL) << "）\n";
        }
        std::cout << "\n";
    }

    //------------- 3. 极简 B 树/B+ 树查找演示 -------------
    {
        std::cout << "[3] B 树 / B+ 树查找思路演示（对应课件 8.3.3 / 8.3.4）\n";

        // 手工构造一棵极简 B 树（3 层）用于演示
        BTreeNode *rootB = new BTreeNode(false);
        rootB->keys = {50};
        rootB->c.resize(2);
        // 左子树
        rootB->c[0] = new BTreeNode(true);
        rootB->c[0]->keys = {15, 26, 43};
        // 右子树
        rootB->c[1] = new BTreeNode(true);
        rootB->c[1]->keys = {56, 78, 89};

        int searchKeyB = 43;
        BTreeNode *resB = BTreeSearch(rootB, searchKeyB);
        std::cout << "在简化 B 树中查找 " << searchKeyB
                  << (resB ? " 成功\n" : " 失败\n");

        // 手工构造一棵极简 B+ 树：根 + 两个叶子
        BPlusTreeNode *rootBp = new BPlusTreeNode(false);
        rootBp->keys = {43, 78}; // 这里让根结点保存各子树的最大值以作分界
        rootBp->c.resize(3);
        // 叶子 1
        rootBp->c[0] = new BPlusTreeNode(true);
        rootBp->c[0]->keys = {3, 8, 20};
        // 叶子 2
        rootBp->c[1] = new BPlusTreeNode(true);
        rootBp->c[1]->keys = {26, 43, 32};
        // 叶子 3
        rootBp->c[2] = new BPlusTreeNode(true);
        rootBp->c[2]->keys = {56, 62, 78, 89};

        // 连接叶子形成有序链表
        rootBp->c[0]->next = rootBp->c[1];
        rootBp->c[1]->next = rootBp->c[2];

        int searchKeyBp = 62;
        BPlusTreeNode *resBp = BPlusTreeSearch(rootBp, searchKeyBp);
        std::cout << "在简化 B+ 树中查找 " << searchKeyBp
                  << (resBp ? " 成功（到达叶子并找到）\n" : " 失败（到达叶子未找到）\n");

        std::cout << "从最左叶子开始顺序遍历 B+ 树叶子链表：";
        BPlusTreeNode *leaf = rootBp->c[0];
        while (leaf)
        {
            for (int k : leaf->keys)
                std::cout << k << " ";
            leaf = leaf->next;
        }
        std::cout << "\n";
    }

    std::cout << "\n提示：\n"
              << "  - BST/AVL/B 树/B+ 树都可以看作“动态查找表”的实现；\n"
              << "  - 查找时都沿着从根到叶的一条路径前进，但树的高度和节点分支数不同；\n"
              << "  - AVL 通过旋转保持“严格平衡”，高度为 O(log n)；\n"
              << "  - B/B+ 树通过在单个节点中存多个关键字，并保持多路平衡，\n"
              << "    特别适合磁盘/SSD 等块设备的索引结构（课件第 49–53 页）。\n";

    return 0;
}
