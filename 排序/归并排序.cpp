// 归并排序.cpp
// ------------------------------------------------------------
// 依据课件《Ch09 排序 2024》（孙奕髦，四川大学）编写的配套示例代码。
// 本文件实现：
//   1) MergeSortHelp（递归拆分 + 归并）（课件第58页）
//   2) Merge（两段有序子序列的2-路归并）（课件第59-60页）
//   3) MergeSort（对外封装：申请辅助数组 temElem，再调用 MergeSortHelp）
//
// 课件核心思想（9.5，课件第53-55页）：
//   - 将两个（或多个）有序子序列“归并”为一个有序序列
//   - 对内部排序通常采用 2-路归并：把相邻两段有序子序列归并成一段（课件第54页）
//   - 对无序序列 elem[s..t]：先分别对左右两半归并排序，然后归并（课件第55页）
//
// 稳定性与空间：
//   - 归并排序稳定（课件第79页）
//   - 辅助空间 O(n)（课件第79页）
//
// 编译运行（示例）：
//   g++ -std=c++17 -O2 -Wall 归并排序.cpp -o merge_sort
//   ./merge_sort
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
// Merge：将两个有序子序列归并成一个有序序列（课件第59-60页）
//   - 左段：elem[low .. mid]
//   - 右段：elem[mid+1 .. high]
//   - 输出：归并到 temElem[low .. high]，再复制回 elem[low .. high]
// ============================================================
template <class ElemType>
void Merge(ElemType elem[], ElemType temElem[], int low, int mid, int high) {
    // 对照课件第59页：i/j 指向两段当前元素，k 指向 temElem 写入位置
    int i, j, k;
    for (i = low, j = mid + 1, k = low; i <= mid && j <= high; ++k) {
        // 课件用 <= 保证稳定性：相等时先取左段元素（第59页 if (elem[i] <= elem[j])）
        if (elem[i] <= elem[j]) temElem[k] = elem[i++];
        else                    temElem[k] = elem[j++];
    }
    // 处理剩余元素（课件第60页两段 for 循环）
    for (; i <= mid; ++i, ++k) temElem[k] = elem[i];
    for (; j <= high; ++j, ++k) temElem[k] = elem[j];

    // 复制回原数组（课件第60页最后一个 for）
    for (i = low; i <= high; ++i) elem[i] = temElem[i];
}

// ============================================================
// MergeSortHelp：对子序列 elem[low .. high] 进行归并排序（课件第58页）
// ------------------------------------------------------------
// 递归逻辑（课件第58页）：
//   if (low < high)
//       mid = (low+high)/2
//       递归排序左半
//       递归排序右半
//       Merge 归并两半
// ============================================================
template <class ElemType>
void MergeSortHelp(ElemType elem[], ElemType temElem[], int low, int high) {
    if (low < high) {
        int mid = (low + high) / 2;
        MergeSortHelp(elem, temElem, low, mid);
        MergeSortHelp(elem, temElem, mid + 1, high);
        Merge(elem, temElem, low, mid, high);
    }
}

// 对外封装：申请辅助数组（对应课件第79页“辅助空间 O(n)”）
template <class ElemType>
void MergeSort(ElemType elem[], int n) {
    if (n <= 1) return;
    std::vector<ElemType> temElem(n);
    MergeSortHelp(elem, temElem.data(), 0, n - 1);
}

int main() {
    int n;
    std::cout << "Input n and n integers:\n";
    if (!(std::cin >> n) || n <= 0) return 0;

    std::vector<int> a(n);
    for (int i = 0; i < n; ++i) std::cin >> a[i];

    std::vector<int> b = a;
    MergeSort(b.data(), n);
    PrintArray(a.data(), n, "[Original]");
    PrintArray(b.data(), n, "[MergeSort] (课件 9.5 第53-60页)");

    return 0;
}
