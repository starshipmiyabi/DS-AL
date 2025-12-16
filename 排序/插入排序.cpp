// 插入排序.cpp
// ------------------------------------------------------------
// 依据课件《Ch09 排序 2024》（孙奕髦，四川大学）编写的配套示例代码。
// 本文件实现：
//   1) 直接插入排序 StraightInsertSort（9.2.1，课件第14-21页）
//   2) 希尔排序 ShellInsert + ShellSort（9.2.2，课件第23-27页）
//
// 额外补充（便于你把“代码”和“算法思想”对应起来）：
//   - 插入类排序的核心：把无序区的一个元素“插入”到有序区中，使有序区逐步扩大。
//     这正是课件对“插入类”方法的概括（课件第8页）。
//   - 希尔排序可以看成“带间隔(incr)的插入排序”，先宏观(大增量)再微观(小增量)（课件第23-24页）。
//
// 稳定性提醒：
//   - 直接插入排序稳定（课件第79页）
//   - 希尔排序不稳定（课件第78-79页）
//
// 编译运行（示例）：
//   g++ -std=c++17 -O2 -Wall 插入排序.cpp -o insert_sort
//   ./insert_sort
// 输入格式（示例程序）：
//   n
//   a0 a1 ... a(n-1)
//
// ------------------------------------------------------------

#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>

// 打印数组（仅用于演示）
template <class ElemType>
void PrintArray(const ElemType elem[], int n, const std::string& title) {
    std::cout << title << "\n";
    for (int i = 0; i < n; ++i) std::cout << elem[i] << (i + 1 == n ? '\n' : ' ');
}

// ============================================================
// 9.2.1 直接插入排序（课件第14-21页）
// ------------------------------------------------------------
// 一趟直接插入排序（课件第15页“三步”思想）：
// 1) 在有序区 elem[0..i-1] 中找 elem[i] 的插入位置（从后往前顺序查找）
// 2) 将比 elem[i] 大的元素整体后移一个位置
// 3) 把 elem[i] 放入最终插入位置
//
// 课件代码模板在第21页：StraightInsertSort(ElemType elem[], int n)
// ============================================================
template <class ElemType>
void StraightInsertSort(ElemType elem[], int n) {
    // 对照课件第21页：外层 i=1..n-1，每趟把 elem[i] 插入到 elem[0..i-1] 的合适位置
    for (int i = 1; i < n; ++i) {
        ElemType e = elem[i];  // 暂存 elem[i]（课件第21页）
        int j;
        // 从后向前找插入位置，并在查找过程中把更大的元素后移（课件第18-19页）
        for (j = i - 1; j >= 0 && e < elem[j]; --j) {
            elem[j + 1] = elem[j];
        }
        elem[j + 1] = e;       // j+1 为插入位置（课件第21页）
    }
}

// ============================================================
// 9.2.2 希尔排序（Shell Sort）（课件第23-27页）
// ------------------------------------------------------------
// 课件对希尔排序的关键描述：
//   - 将序列按“增量 d(incr)”分成若干子序列（课件第24页）
//   - 分别对每个子序列做插入排序，但子序列相邻元素间距为 incr（课件第26页 ShellInsert）
//   - 依次使用一个“从大到小，最后为1”的增量序列 inc[]（课件第24-25页；代码第27页 ShellSort）
//
// 注意：希尔排序的“核心变化”就是把插入排序里 j-- 改成 j -= incr（课件第26页）。
// ============================================================
template <class ElemType>
void ShellInsert(ElemType elem[], int n, int incr) {
    // 对数组 elem 做“一趟”增量为 incr 的 Shell 排序（课件第26页）
    for (int i = incr; i < n; ++i) {
        ElemType e = elem[i];
        int j;
        for (j = i - incr; j >= 0 && e < elem[j]; j -= incr) {
            elem[j + incr] = elem[j];
        }
        elem[j + incr] = e;
    }
}

template <class ElemType>
void ShellSort(ElemType elem[], int n, const int inc[], int t) {
    // 按增量序列 inc[0..t-1] 对 elem 做 Shell 排序（课件第27页）
    for (int k = 0; k < t; ++k) {
        ShellInsert(elem, n, inc[k]);
    }
}

// 一个简单的示例 main：演示同一组输入分别用直接插入、希尔排序后的结果
int main() {
    int n;
    std::cout << "Input n and n integers:\n";
    if (!(std::cin >> n) || n <= 0) return 0;

    std::vector<int> a(n);
    for (int i = 0; i < n; ++i) std::cin >> a[i];

    std::vector<int> b = a;
    StraightInsertSort(b.data(), n);
    PrintArray(a.data(), n, "[Original]");
    PrintArray(b.data(), n, "[StraightInsertSort] (课件 9.2.1 第14-21页)");

    // 课件示例给出的增量序列（示例中 d=5,3,1）体现“从大到小，最后为1”（课件第25页）。
    // 这里给一个常见的示例增量序列：n/2, n/4, ..., 1（你也可以换成课件示例 5,3,1）
    std::vector<int> c = a;
    std::vector<int> inc;
    for (int d = n / 2; d >= 1; d /= 2) inc.push_back(d);
    ShellSort(c.data(), n, inc.data(), static_cast<int>(inc.size()));
    PrintArray(c.data(), n, "[ShellSort] (课件 9.2.2 第23-27页)");

    return 0;
}
