# Data Structures and Algorithms (C++ Implementation)

> **Repository:** ds-al
> **Author:** Starship Miyabi
> **Language:** C++17

本仓库包含了一套完整的数据结构与算法核心库的 C++ 实现。项目代码紧密贴合《数据结构与算法分析》课程大纲，旨在通过底层实现深入理解数据结构的逻辑结构、存储结构及算法效率。

所有代码均采用 **Modern C++** 标准编写，广泛应用泛型编程技术，并包含详细的理论注释与页码对照，既可作为学习资料，也可作为算法实战的基础组件。

## 📚 目录结构 (Directory Structure)

仓库按照数据结构类型进行模块化组织，涵盖了从线性表到高级图论算法的完整体系。

### 1. 线性结构 (Linear Structures)
* **线性表 (Linear List)** [`线性表/`]
    * `SqList`: 基于数组的顺序表实现，支持动态插入、删除及集合运算（如差集）。
* **链表 (Linked List)** [`链表/`]
    * `SimpleLinkList`: 单链表的基础实现。
    * `SimpleCircLinkList`: 循环单链表及其应用（如约瑟夫环问题）。
    * `LinkList`: 带当前位置缓存（Cursor）的高效链表实现。
* **栈与队列 (Stack & Queue)** [`栈/`, `队列/`]
    * `ArrayStack` / `LinkedStack`: 栈的顺序与链式实现。
    * `DualStack`: 双栈共享空间实现。
    * `LinkQueue` / `CircQueue`: 链队列与循环顺序队列。
    * **应用**: 括号匹配检查、中缀表达式求值。
* **串 (String)** [`串/`]
    * `CharString`: 动态字符串类封装。
    * **算法**: KMP 模式匹配算法（含 `next` 数组计算）、简易文本编辑器实现。

### 2. 数组与广义表 (Arrays & Generalized Lists)
* **数组 (Arrays)** [`数组/`]
    * `Array`: 支持任意维度的 N 维数组模板类，基于行优先（Row-major）映射实现。
* **矩阵 (Matrices)** [`数组/矩阵/`]
    * `Matrix`: 通用稠密矩阵。
    * **压缩存储**: 对称矩阵、三角矩阵、三对角矩阵的压缩存储实现。
    * **稀疏矩阵**: 基于三元组表 (`Triple`) 的稀疏矩阵实现，包含简单转置与**快速转置**算法。
* **广义表 (Generalized Lists)** [`广义表/`]
    * `RefGenList`: 采用“引用计数法”管理的广义表，支持递归深度计算与字符串解析构造。

### 3. 树与二叉树 (Trees)
* **二叉树 (Binary Tree)** [`树/`]
    * `BinaryTree`: 完整的二叉树模板类。
    * **遍历**: 递归与非递归实现的前序、中序、后序及层序遍历。
* **线索二叉树 (Threaded Binary Tree)** [`树/`]
    * `ThreadedBinaryTree`: 中序线索化二叉树的构建与非递归遍历。
* **树与森林 (Trees & Forests)** [`树/`]
    * 实现了树/森林（孩子-兄弟表示法）与二叉树之间的相互转换及遍历。
* **哈夫曼树 (Huffman Tree)** [`树/`]
    * `HuffmanTree`: 哈夫曼树的构建、前缀编码生成及位串译码实现（含 WPL 计算）。

### 4. 图论 (Graphs)
* **图的存储与遍历** [`图/`]
    * `AdjListGraph`: 基于邻接表的图实现（支持有向/无向）。
    * **遍历**: 深度优先搜索 (DFS) 与广度优先搜索 (BFS)。
* **最小生成树 (MST)** [`图/`]
    * **Prim 算法**: 基于邻接矩阵实现。
    * **Kruskal 算法**: 基于并查集实现。
* **最短路径 (Shortest Path)** [`图/`]
    * **Dijkstra 算法**: 单源最短路径。
    * **Floyd 算法**: 多源最短路径（所有顶点对）。
* **有向无环图 (DAG)** [`图/`]
    * **拓扑排序 (Topological Sort)**: 基于入度表的 Kahn 算法。
    * **关键路径 (Critical Path)**: AOE 网的关键活动分析。

### 5. 查找 (Search)
* **静态查找** [`查找/`]
    * 顺序查找与折半查找（Binary Search）及其溢出安全改进版。
* **动态查找** [`查找/`]
    * **BST**: 二叉排序树的查找与插入。
    * **AVL**: 平衡二叉树的旋转操作（LL/RR/LR/RL）与平衡维护。
    * **B-Tree / B+ Tree**: 多路查找树的基本原理演示。
* **哈希表 (Hashing)** [`查找/`]
    * 哈希函数：平方取中法、除留余数法。
    * 冲突解决：开放定址法（线性/二次探测）与链地址法。

## 🛠️ 技术特点 (Technical Highlights)

1.  **泛型编程 (Generic Programming)**:
    绝大多数数据结构（如 `SqList<T>`, `BinaryTree<T>`) 均使用 C++ 模板实现，支持任意数据类型，极大地提高了代码复用性。

2.  **现代 C++ 规范 (Modern C++)**:
    * 使用 `std::vector`, `std::queue`, `std::stack` 等 STL 容器辅助实现高级逻辑。
    * 遵循 RAII 原则管理内存，注重异常安全（Exception Safety）。
    * 代码包含详尽的注释，精确对应教材的理论知识点（如 Pxx 页码引用）。

3.  **自包含演示 (Self-contained Demos)**:
    每个 `.cpp` 源文件均包含 `main` 函数或测试用例，能够独立编译运行，直观展示算法执行过程。

## 🚀 构建与运行 (Build & Run)

本项目代码不依赖第三方库，仅需支持 C++17 的编译器（GCC, Clang, MSVC）即可。

**编译单个文件示例：**

```bash
# 编译图论部分的 Dijkstra 算法演示
g++ -std=c++17 -O2 图/最短路径.cpp -o dijkstra_demo

# 运行
./dijkstra_demo
