// 选择排序.cpp
// ------------------------------------------------------------
// 依据课件《Ch09 排序 2024》（孙奕髦，四川大学）编写的配套示例代码。
// 本文件实现：
//   1) 简单选择排序 SimpleSelectionSort（9.4.1，课件第41-43页）
//   2) 堆排序 HeapSort（9.4.2，课件第45-51页，按“大顶堆”实现升序排序）
//
// 选择类排序的思想：每趟从无序区“选择”关键字最小/最大元素放入有序区（课件第10页）。
//
// 稳定性提醒：
//   - 简单选择排序不稳定（课件第79页）
//   - 堆排序不稳定（课件第78-79页）
//
// 编译运行（示例）：
//   g++ -std=c++17 -O2 -Wall 选择排序.cpp -o select_sort
//   ./select_sort
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
// 9.4.1 简单选择排序（课件第41-43页）
// ------------------------------------------------------------
// 课件图示：第 i 趟从无序区 elem[i..n-1] 选出关键字最小元素，放到位置 i（课件第41页）
// 课件代码模板在第43页：SimpleSelectionSort(ElemType elem[], int n)
// ============================================================
template <class ElemType>
void SimpleSelectionSort(ElemType elem[], int n) {
    for (int i = 0; i < n - 1; ++i) {              // 第 i 趟选择（课件第43页）
        int minPos = i;                            // 记录最小元素下标（课件第43页）
        for (int j = i + 1; j < n; ++j) {
            if (elem[j] < elem[minPos]) minPos = j;
        }
        if (i != minPos) {
            ElemType temp = elem[i];
            elem[i] = elem[minPos];
            elem[minPos] = temp;
        }
    }
}

// ============================================================
// 9.4.2 堆排序（课件第45-51页）
// ------------------------------------------------------------
// 课件给出“堆”的定义（课件第45页）：
//   小顶堆：e_i <= e_{2i+1} 且 e_i <= e_{2i+2}
//   大顶堆：e_i >= e_{2i+1} 且 e_i >= e_{2i+2}
//
// 堆排序的步骤（课件第47页）：
//   1) 将无序序列构建成一个堆（建堆）
//   2) 输出堆顶元素，并对剩余元素做“筛选”调整成新堆（筛选/调整）
//
// 课件解释“筛选”(sift down)（课件第49-50页）：
//   - 假设左右子树都是堆，自上而下调整根，使整棵树也成为堆
//
// 下面实现采用“大顶堆”得到升序：
//   - 大顶堆堆顶是最大值
//   - 每次把堆顶(最大)交换到数组末尾，然后对剩余区间再 sift down
// ============================================================
template <class ElemType>
void SiftDown(ElemType elem[], int start, int end) {
    // start：需要下滤的根结点下标；end：当前堆的最后一个元素下标
    // 对应课件“筛选（自上而下）”的过程（第49-50页）
    int root = start;
    while (true) {
        int child = root * 2 + 1;          // 左孩子 2i+1（课件第46页）
        if (child > end) break;

        // 如果右孩子更大，则选择右孩子
        if (child + 1 <= end && elem[child] < elem[child + 1]) child++;

        // 若根已经 >= 最大孩子，则满足大顶堆性质
        if (!(elem[root] < elem[child])) break;

        std::swap(elem[root], elem[child]);
        root = child;                      // 继续向下调整
    }
}

template <class ElemType>
void BuildMaxHeap(ElemType elem[], int n) {
    // 课件指出：建堆是从下往上进行“筛选”的过程（课件第51页）
    // 最后一个非叶子结点下标 = (n-2)/2
    for (int i = (n - 2) / 2; i >= 0; --i) {
        SiftDown(elem, i, n - 1);
    }
}

template <class ElemType>
void HeapSort(ElemType elem[], int n) {
    if (n <= 1) return;
    BuildMaxHeap(elem, n);

    // 反复把堆顶(最大)放到末尾，并缩小堆范围
    for (int end = n - 1; end > 0; --end) {
        std::swap(elem[0], elem[end]);       // 交换堆顶与末尾（课件第48页示例“交换”思想）
        SiftDown(elem, 0, end - 1);          // 对剩余元素筛选（课件第47-50页）
    }
}

int main() {
    int n;
    std::cout << "Input n and n integers:\n";
    if (!(std::cin >> n) || n <= 0) return 0;

    std::vector<int> a(n);
    for (int i = 0; i < n; ++i) std::cin >> a[i];

    std::vector<int> b = a;
    SimpleSelectionSort(b.data(), n);
    PrintArray(a.data(), n, "[Original]");
    PrintArray(b.data(), n, "[SimpleSelectionSort] (课件 9.4.1 第41-43页)");

    std::vector<int> c = a;
    HeapSort(c.data(), n);
    PrintArray(c.data(), n, "[HeapSort] (课件 9.4.2 第45-51页)");

    return 0;
}
