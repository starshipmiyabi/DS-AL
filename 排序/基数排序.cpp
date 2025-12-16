// 基数排序.cpp
// ------------------------------------------------------------
// 依据课件《Ch09 排序 2024》（孙奕髦，四川大学）编写的配套示例代码。
// 本文件实现：
//   - LSD（最低位优先）“链式基数排序”（9.6，课件第62-71页）
//
// 课件要点：
//   1) 基数排序是一种借助“多关键字排序”思想来实现“单关键字排序”的内部排序算法（课件第62页）
//   2) 多关键字排序常见两种：LSD(最低位优先) 与 MSD(最高位优先)（课件第64页）
//   3) 对数字/字符型单关键字，可视为由多个“位”构成的多关键字；用“分配-收集”反复处理每一位（课件第66-68页）
//   4) 链式实现：分配时按当前位放入不同链表；收集时按位值从小到大拼接链表；对每一位重复（课件第68页）
//   5) 时间复杂度 O(d(2n+r)) ≈ O(dn)，其中 r 为“基”（如十进制 r=10）（课件第71页）
//
// 本实现：
//   - 只处理【非负整数】（与课件示例两位数 01..97 一致，课件第67-70页）
//   - 基数 r 默认 10（十进制）
//   - 稳定：因为“分配”到桶链表时采用尾插，保证同一桶内相对次序不变（对应课件第76页稳定性定义、第79页“基数排序稳定”）
//
// 编译运行（示例）：
//   g++ -std=c++17 -O2 -Wall 基数排序.cpp -o radix_sort
//   ./radix_sort
// 输入格式：
//   n
//   a0 a1 ... a(n-1)
//
// ------------------------------------------------------------

#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>

struct Node {
    int key;
    Node* next;
};

// 计算非负整数 x 在给定进制 radix 下的位数（d）
// 对应课件：d 为“分配-收集”的趟数（课件第71页）
int DigitsInBase(int x, int radix) {
    if (x == 0) return 1;
    int d = 0;
    while (x > 0) {
        x /= radix;
        ++d;
    }
    return d;
}

// 释放链表
void FreeList(Node* head) {
    while (head) {
        Node* nxt = head->next;
        delete head;
        head = nxt;
    }
}

// ============================================================
// LSD 链式基数排序（课件第66-70页的“分配-收集”过程）
//
// 参数：
//   a[]：待排序数组（非负整数）
//   n  ：元素数量
//   radix：基（十进制为10）
// ============================================================
void RadixSortLSD(int a[], int n, int radix = 10) {
    if (n <= 1) return;
    if (radix <= 1) throw std::invalid_argument("radix must be >= 2");

    // 1) 找出最大值，确定需要处理的位数 d（课件第71页）
    int maxVal = 0;
    for (int i = 0; i < n; ++i) {
        if (a[i] < 0) throw std::invalid_argument("This demo supports non-negative integers only.");
        maxVal = std::max(maxVal, a[i]);
    }
    const int d = DigitsInBase(maxVal, radix);

    // 2) 将数组转换成单链表，便于“链式分配-收集”（课件第68页）
    Node* head = nullptr;
    Node* tail = nullptr;
    for (int i = 0; i < n; ++i) {
        Node* node = new Node{a[i], nullptr};
        if (!head) head = tail = node;
        else { tail->next = node; tail = node; }
    }

    // 3) 对每一位进行一次 “分配-收集”（LSD：从最低位开始，课件第64-65页）
    //    exp 表示当前位的权重：1(个位), radix(十位), radix^2(百位)...
    int exp = 1;
    for (int pass = 0; pass < d; ++pass) {
        // buckets[i] 存储“当前位 = i”的链表的 (head, tail)
        std::vector<Node*> bucketHead(radix, nullptr);
        std::vector<Node*> bucketTail(radix, nullptr);

        // ---------- 分配（课件第67-68页） ----------
        for (Node* cur = head; cur != nullptr; ) {
            Node* nxt = cur->next;     // 先保存 next，因为我们要把 cur 从原链表“摘下来”
            cur->next = nullptr;

            int digit = (cur->key / exp) % radix;  // 当前位关键字
            // 尾插，保证稳定性：同桶内元素相对次序不变（课件第76页稳定性概念）
            if (!bucketHead[digit]) bucketHead[digit] = bucketTail[digit] = cur;
            else { bucketTail[digit]->next = cur; bucketTail[digit] = cur; }

            cur = nxt;
        }

        // ---------- 收集（课件第67-70页） ----------
        head = tail = nullptr;
        for (int digit = 0; digit < radix; ++digit) {
            if (!bucketHead[digit]) continue;
            if (!head) {
                head = bucketHead[digit];
                tail = bucketTail[digit];
            } else {
                tail->next = bucketHead[digit];
                tail = bucketTail[digit];
            }
        }

        exp *= radix;  // 进入更高一位
    }

    // 4) 拷贝回数组，并释放链表
    Node* cur = head;
    for (int i = 0; i < n; ++i) {
        a[i] = cur->key;
        cur = cur->next;
    }
    FreeList(head);
}

void PrintArray(const int a[], int n, const std::string& title) {
    std::cout << title << "\n";
    for (int i = 0; i < n; ++i) std::cout << a[i] << (i + 1 == n ? '\n' : ' ');
}

int main() {
    int n;
    std::cout << "Input n and n non-negative integers:\n";
    if (!(std::cin >> n) || n <= 0) return 0;

    std::vector<int> a(n);
    for (int i = 0; i < n; ++i) std::cin >> a[i];

    std::vector<int> b = a;
    RadixSortLSD(b.data(), n, 10);

    PrintArray(a.data(), n, "[Original]");
    PrintArray(b.data(), n, "[RadixSortLSD] (课件 9.6 第62-71页)");

    return 0;
}
