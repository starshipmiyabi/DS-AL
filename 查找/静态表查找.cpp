// 静态表查找.cpp
// 对应课件：Chapter 8.2 静态表的查找（顺序查找与有序表的折半查找）
// 主要参考：
//   - 8.2.1 顺序查找：第 9–12 页（顺序查找思想与 SqSerach 模板代码）
//   - 基于有序顺序表的折半查找与两版 BinSerach 代码：第 13–20 页
//
// 本文件实现了：
//   1. 顺序查找（无序/有序静态表通用）
//   2. 折半查找（基础版 mid = (low + high) / 2，对应第 16 页）
//   3. 折半查找（溢出安全版 mid = low + ((high - low) >> 1)，对应第 18 页）
//
// 为便于实验与理解，本文件使用简单的 int 数组作为“静态查找表”，
// 并在 main 函数中给出示例。
//
// 注意：
//   静态表查找只做“是否存在、位置在哪里”的查询，不做插入/删除。
//   若要支持插入/删除并保持高效，需要使用“动态查找表”结构
//  （见：动态表查找.cpp 中的二叉排序树/AVL 树等实现）。

#include <iostream>
#include <vector>
#include <iomanip>

//---------------------------------------------------------------------
// 1. 顺序查找（对应课件第 8.2.1 节，第 9–12 页）
//---------------------------------------------------------------------
// 时间复杂度：O(n)
// 适用场景：
//   - 表长不太大；
//   - 不要求表必须有序；
//   - 实现简单，便于理解、调试。
//
// AL_success = (n + 1) / 2（假设每个位置被查到的概率相同），
// 这一点在第 12 页中“顺序查找平均查找长度”处有给出。

/**
 * @brief 顺序查找（线性扫描）
 * @param elem  顺序表（静态表）的首地址
 * @param n     顺序表中元素个数
 * @param key   要查找的关键字
 * @return      查找成功：返回下标；查找失败：返回 -1
 *
 * 对应课件代码：
 *   template <class ElemType, class KeyType>
 *   int SqSerach(ElemType elem[], int n, KeyType key)
 *   （见第 11 页）
 */
template <class ElemType, class KeyType>
int SeqSearch(const ElemType elem[], int n, const KeyType &key)
{
    // temPos 对应课件中的“临时变量”
    int temPos;
    // 从 0 开始依次比较，直到找到 elem[temPos] == key 或扫描完 n 个元素
    for (temPos = 0; temPos < n && elem[temPos] != key; ++temPos)
        ;

    if (temPos < n)
    {
        // 查找成功：返回元素下标
        return temPos;
    }
    else
    {
        // 查找失败：返回 -1（课件中也是如此约定）
        return -1;
    }
}

//---------------------------------------------------------------------
// 2. 折半查找（基础版，对应课件第 16 页 BinSerach 代码）
//---------------------------------------------------------------------
// 折半查找要求：
//   1. 以“有序顺序表”表示静态查找表（第 13–15 页的示意图）；
//   2. 可以按下标 O(1) 随机访问（因此一般用数组实现）。
//
// 时间复杂度：O(log n)
// 平均查找长度也为 O(log n)，明显优于顺序查找。

/**
 * @brief 折半查找（基础版）：mid = (low + high) / 2
 * @param elem  有序数组（从小到大）
 * @param n     元素个数
 * @param key   要查找的关键字
 * @return      查找成功返回下标，失败返回 -1
 *
 * 对应课件代码：第 16 页 BinSerach
 *   int low = 0, high = n - 1;
 *   while (low <= high) {
 *       int mid = (low + high) / 2;
 *       ...
 *   }
 */
template <class ElemType, class KeyType>
int BinarySearchBasic(const ElemType elem[], int n, const KeyType &key)
{
    int low = 0;
    int high = n - 1; // 查找区间初值 [low, high]

    while (low <= high)
    {
        // 课件中的写法：mid = (low + high) / 2
        // 在 n 较大时 (low + high) 有理论上的溢出风险，
        // 但教学中常用此形式，便于理解“中间位置”的概念。
        int mid = (low + high) / 2;

        if (key == elem[mid])
        {
            // 匹配成功
            return mid;
        }
        else if (key < elem[mid])
        {
            // 在左半区间继续查找
            high = mid - 1;
        }
        else
        {
            // 在右半区间继续查找
            low = mid + 1;
        }
    }

    // 查找区间缩小为空仍然没找到，查找失败
    return -1;
}

//---------------------------------------------------------------------
// 3. 折半查找（溢出安全版，对应课件第 18 页改进代码）
//---------------------------------------------------------------------
// 课件在第 18 页给出的改进：
//   int mid = low + ((high - low) >> 1);
// 这是为了避免 low + high 在整型范围内产生加法溢出。
//
// >> 1 表示除以 2（右移 1 位），在整数环境中比 /2 更贴近底层实现。

/**
 * @brief 折半查找（安全版）：mid = low + ((high - low) >> 1)
 * @param elem  有序数组（从小到大）
 * @param n     元素个数
 * @param key   要查找的关键字
 * @return      查找成功返回下标，失败返回 -1
 *
 * 对应课件：第 18 页 BinSerach 改进写法。
 */
template <class ElemType, class KeyType>
int BinarySearchSafe(const ElemType elem[], int n, const KeyType &key)
{
    int low = 0;
    int high = n - 1;

    while (low <= high)
    {
        // 防溢出的中点写法：
        int mid = low + ((high - low) >> 1);

        if (key < elem[mid])
        {
            // 在左半区间继续查找
            high = mid - 1;
        }
        else if (key > elem[mid])
        {
            // 在右半区间继续查找
            low = mid + 1;
        }
        else
        {
            // 查找成功
            return mid;
        }
    }
    return -1; // 查找失败
}

//---------------------------------------------------------------------
// 4. 打印辅助函数与演示 main
//---------------------------------------------------------------------

template <typename T>
void printArray(const T *a, int n)
{
    std::cout << "[";
    for (int i = 0; i < n; ++i)
    {
        std::cout << a[i];
        if (i + 1 < n)
            std::cout << ", ";
    }
    std::cout << "]";
}

int main()
{
    // 示例 1：无序表上的顺序查找（对应第 10 页的示意表）
    int unorderedElem[] = {21, 37, 88, 19, 92, 5, 64, 56, 80, 75, 13};
    int n1 = sizeof(unorderedElem) / sizeof(unorderedElem[0]);
    int key1 = 64;

    std::cout << "================ 静态表查找示例 ================\n";
    std::cout << "无序顺序表（示例源自课件第 10 页）：\n";
    printArray(unorderedElem, n1);
    std::cout << "\n要查找的 key = " << key1 << "\n\n";

    int posSeq = SeqSearch(unorderedElem, n1, key1);
    std::cout << "[顺序查找] 结果：";
    if (posSeq != -1)
        std::cout << "找到，位置下标 = " << posSeq << "\n";
    else
        std::cout << "未找到\n";

    // 示例 2：有序表上的折半查找（对应第 15 页的例子）
    int orderedElem[] = {5, 13, 19, 21, 37, 56, 64, 75, 80, 88, 92};
    int n2 = sizeof(orderedElem) / sizeof(orderedElem[0]);
    int key2 = 64;

    std::cout << "\n有序顺序表（示例源自课件第 15 页）：\n";
    printArray(orderedElem, n2);
    std::cout << "\n要查找的 key = " << key2 << "\n\n";

    int posBinBasic = BinarySearchBasic(orderedElem, n2, key2);
    std::cout << "[折半查找-基础版] 结果：";
    if (posBinBasic != -1)
        std::cout << "找到，位置下标 = " << posBinBasic << "\n";
    else
        std::cout << "未找到\n";

    int posBinSafe = BinarySearchSafe(orderedElem, n2, key2);
    std::cout << "[折半查找-安全版] 结果：";
    if (posBinSafe != -1)
        std::cout << "找到，位置下标 = " << posBinSafe << "\n";
    else
        std::cout << "未找到\n";

    std::cout << "\n提示：\n"
              << "  - 顺序查找适用于小规模或无序的数据集合；\n"
              << "  - 折半查找要求数据有序且以顺序表存储，平均查找长度为 O(log n)，\n"
              << "    对应课件第 12、16、18、20 页关于折半查找和局限性的分析。\n";

    return 0;
}
