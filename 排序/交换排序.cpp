// 交换排序.cpp
// ------------------------------------------------------------
// 依据课件《Ch09 排序 2024》（孙奕髦，四川大学）编写的配套示例代码。
// 本文件实现：
//   1) 冒泡排序 BubbleSort（9.3.1，课件第30-33页）
//   2) 快速排序 QuickSort + Partition（9.3.2，课件第35-38页）
//
// 交换类排序的思想：通过“交换”无序序列中的元素，使某个极值元素进入有序区，逐步扩大有序区（课件第9页）。
//
// 稳定性提醒：
//   - 冒泡排序稳定（课件第79页）
//   - 快速排序不稳定（课件第78-79页）
//
// 编译运行（示例）：
//   g++ -std=c++17 -O2 -Wall 交换排序.cpp -o exchange_sort
//   ./exchange_sort
// 输入格式：
//   n
//   a0 a1 ... a(n-1)
//
// ------------------------------------------------------------

#include <iostream>
#include <vector>
#include <algorithm>

template <class ElemType>
void PrintArray(const ElemType elem[], int n, const std::string& title) {
    std::cout << title << "\n";
    for (int i = 0; i < n; ++i) std::cout << elem[i] << (i + 1 == n ? '\n' : ' ');
}

// ============================================================
// 9.3.1 冒泡排序（课件第30-33页）
// ------------------------------------------------------------
// 课件对第 i 趟冒泡的描述：比较相邻元素，把关键字最大的元素交换到 n-i 位置（课件第30页）。
// 课件代码模板在第33页：BubbleSort(ElemType elem[], int n)
// ============================================================
template <class ElemType>
void BubbleSort(ElemType elem[], int n) {
    for (int i = 1; i < n; ++i) {               // 第 i 趟冒泡（课件第33页）
        for (int j = 0; j < n - i; ++j) {       // 依次比较相邻元素（课件第33页）
            if (elem[j] > elem[j + 1]) {        // 出现逆序则交换（课件第33页）
                ElemType temp = elem[j];
                elem[j] = elem[j + 1];
                elem[j + 1] = temp;
            }
        }
    }
}

// ============================================================
// 9.3.2 快速排序（课件第35-38页）
// ------------------------------------------------------------
// 课件用“一趟快速排序(一次划分)”来解释快速排序（课件第35页）：
//   - 选一个元素作为“支点(pivot)”
//   - 小于支点的移到左边，大于支点的移到右边
//   - 一趟划分后，子序列 elem[s..t] 被分成两部分 elem[s..i-1], elem[i+1..t]，满足：
//       elem[j] <= elem[i] <= elem[k]（课件第35页）
//
// 课件的划分过程用两个指针 low/high（课件第36-37页）：
//   - high 从右向左找 < pivot 的元素
//   - low  从左向右找 > pivot 的元素
//   - 不满足时交换，直到 low == high
//
// 下面的 Partition 使用“挖坑填数”(hole)的写法：本质上等价于课件描述的 low/high 交换推进。
// ============================================================
template <class ElemType>
int Partition(ElemType elem[], int low, int high) {
    // pivot 取子序列第一个元素 elem[low]（课件第36页“设 elem[s] 为支点”）
    ElemType pivot = elem[low];

    while (low < high) {
        // high 从右往左找：直到找到 < pivot 的元素（课件第36页：elem[high] >= pivot 则 high--）
        while (low < high && elem[high] >= pivot) --high;
        elem[low] = elem[high];  // 把更小元素填到左侧“坑”里

        // low 从左往右找：直到找到 > pivot 的元素（课件第36页：elem[low] <= pivot 则 low++）
        while (low < high && elem[low] <= pivot) ++low;
        elem[high] = elem[low];  // 把更大元素填到右侧“坑”里
    }

    // low == high 时，坑位就是支点最终位置 i（课件第35页的 i）
    elem[low] = pivot;
    return low;
}

template <class ElemType>
void QuickSort(ElemType elem[], int low, int high) {
    // 课件指出：一次划分后，对两段子序列分别递归快速排序（课件第38页）
    if (low >= high) return;
    int pivotPos = Partition(elem, low, high);
    QuickSort(elem, low, pivotPos - 1);
    QuickSort(elem, pivotPos + 1, high);
}

template <class ElemType>
void QuickSort(ElemType elem[], int n) {
    if (n <= 1) return;
    QuickSort(elem, 0, n - 1);
}

int main() {
    int n;
    std::cout << "Input n and n integers:\n";
    if (!(std::cin >> n) || n <= 0) return 0;

    std::vector<int> a(n);
    for (int i = 0; i < n; ++i) std::cin >> a[i];

    std::vector<int> b = a;
    BubbleSort(b.data(), n);
    PrintArray(a.data(), n, "[Original]");
    PrintArray(b.data(), n, "[BubbleSort] (课件 9.3.1 第30-33页)");

    std::vector<int> c = a;
    QuickSort(c.data(), n);
    PrintArray(c.data(), n, "[QuickSort] (课件 9.3.2 第35-38页)");

    return 0;
}
