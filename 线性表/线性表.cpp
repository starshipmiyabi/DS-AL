/************************************************************
 * 线性表（顺序存储）— 适配课件的完整实现与演示
 * 对应课件：
 *  - P22-P24：顺序表类的定义与方法原型
 *  - P25-P26：构造/析构的实现思路
 *  - P28-P33：Insert / Delete 的实现与复杂度
 *  - P36-P38：顺序表应用：集合差集 Difference(A,B)
 *
 * 说明：
 *  1) 位序使用 1-based（从 1 开始），与课件一致（见 P13、P19）。
 *  2) 本实现为“固定容量”的顺序表：当 count==maxSize 时，插入失败。
 *    （与课件 Insert 伪码一致：已满返回 false，见 P28）
 *  3) 所有接口命名、签名均与课件对齐，便于学习与对照。
 ************************************************************/
#include <iostream>
#include <iomanip>

// -------------------------- 配置 ---------------------------
#ifndef DEFAULT_SIZE
#define DEFAULT_SIZE 100   // 课件原型：SqList(int size=DEFAULT_SIZE)（见 P23）
#endif

// 为了便于替换元素类型，定义一个 ElemType（也可直接在模板上使用）
template <class ElemType>
class SqList {
protected:
    // ---------- P22：顺序表实现的数据成员 ----------
    int        count;    // 元素个数（当前长度）
    int        maxSize;  // 最大可容纳元素个数（容量）
    ElemType*  elems;    // 元素存储空间（连续内存）

public:
    // ---------- P23：抽象数据类型方法声明 ----------
    // 构造：构造一个最大元素个数为 size 的空顺序表（P25）
    explicit SqList(int size = DEFAULT_SIZE)
        : count(0), maxSize(size), elems(nullptr)
    {
        if (maxSize <= 0) maxSize = DEFAULT_SIZE;
        elems = new ElemType[maxSize];        // 分配顺序存储空间（连续数组）
    }

    // 析构：销毁线性表，释放 elems（P26、P9）
    virtual ~SqList() {
        delete[] elems;
        elems = nullptr;
        count = maxSize = 0;
    }

    // 复制构造：深拷贝，确保两个表互不影响（P24 原型）
    SqList(const SqList& source)
        : count(source.count), maxSize(source.maxSize), elems(new ElemType[maxSize])
    {
        for (int i = 0; i < count; ++i) elems[i] = source.elems[i];
    }

    // 赋值重载：深拷贝（P24 原型）
    SqList& operator=(const SqList& source) {
        if (this == &source) return *this;
        ElemType* newBuf = new ElemType[source.maxSize];
        for (int i = 0; i < source.count; ++i) newBuf[i] = source.elems[i];
        delete[] elems;
        elems   = newBuf;
        maxSize = source.maxSize;
        count   = source.count;
        return *this;
    }

    // -------------------- Ⅲ. 引用型操作（P10-P14） --------------------
    // 判空：若空返回 true，否则 false（P11）
    bool Empty() const { return count == 0; }

    // 求长度：返回元素个数（P12）
    int Length() const { return count; }

    // 按位序（1-based）取值：e 返回第 position 个元素值（P13）
    bool GetElem(int position, ElemType& e) const {
        if (position < 1 || position > count) return false;      // 位置非法
        e = elems[position - 1];
        return true;
    }

    // 遍历：依次对每个元素调用 visit(x)（P14）
    void Traverse(void (*visit)(const ElemType&)) const {
        for (int i = 0; i < count; ++i) visit(elems[i]);
    }

    // -------------------- Ⅳ. 加工型操作（P15-P19） --------------------
    // 置空：将线性表重置为长度 0（容量保留）（P16）
    void Clear() { count = 0; }

    // 置值：将第 position 个元素改为 e（P17）
    bool SetElem(int position, const ElemType& e) {
        if (position < 1 || position > count) return false;      // 位置非法
        elems[position - 1] = e;
        return true;
    }

    // 插入：在第 position 个位置“前”插入 e（1 ≤ position ≤ Length()+1）（P19、P28-P29）
    // 成功返回 true；若已满或位置非法返回 false。
    bool Insert(int position, const ElemType& e) {
        if (count == maxSize) return false;                       // 表满（与 P28 一致）
        if (position < 1 || position > count + 1) return false;   // 位置非法（与 P28 一致）

        // 从尾到 position 依次右移，空出插入位（P29 动图思想）
        for (int i = count; i >= position; --i) {
            elems[i] = elems[i - 1];
        }
        elems[position - 1] = e;
        ++count;
        return true;
    }

    // 删除（带出参）：删除第 position 个元素，用 e 返回其值（1 ≤ position ≤ Length()）（P18、P31-P32）
    bool Delete(int position, ElemType& e) {
        if (position < 1 || position > count) return false;       // 位置非法（与 P31 一致）

        e = elems[position - 1];                                  // 返回被删值（P32）
        // 从 position+1 起的元素整体左移填补空位（P32）
        for (int i = position; i < count; ++i) {
            elems[i - 1] = elems[i];
        }
        --count;
        return true;
    }

    // 删除（无出参重载）：仅删除，不关心被删值（P24 原型含此重载）
    bool Delete(int position) {
        ElemType tmp;
        return Delete(position, tmp);
    }
};

// ------------------------ 与课件一致的遍历打印 ------------------------
template <class T>
void PrintElem(const T& x) { std::cout << x << ' '; }

template <class T>
void PrintList(const char* title, const SqList<T>& L) {
    std::cout << std::left << std::setw(18) << title << ": ";
    L.Traverse(PrintElem<T>);
    std::cout << "(len=" << L.Length() << ")\n";
}

// ------------------------ 顺序表应用：差集（P36-P38） ------------------
template <class ElemType>
void Difference(const SqList<ElemType>& la,
                const SqList<ElemType>& lb,
                SqList<ElemType>& lc)
{
    lc.Clear();                           // 清空 lc（P36）
    ElemType aElem, bElem;

    for (int aPos = 1; aPos <= la.Length(); ++aPos) {
        la.GetElem(aPos, aElem);          // 取出 la 的一个元素（P37）
        bool isExist = false;             // 是否在 lb 中出现（P37）

        for (int bPos = 1; bPos <= lb.Length(); ++bPos) {
            lb.GetElem(bPos, bElem);      // 取出 lb 的一个元素（P37）
            if (aElem == bElem) {         // 同时出现在 la 与 lb，则不纳入差集（P38）
                isExist = true;
                break;
            }
        }
        if (!isExist) {
            // 插入到 lc（按课件做法在“尾部”位置插入：Length()+1，见 P38）
            lc.Insert(lc.Length() + 1, aElem);
        }
    }
}

// ----------------------------- 演示主程序 ------------------------------
int main() {
    std::cout << "==== 顺序表 SqList 演示（与课件接口一致） ====\n\n";

    // 1) 构造空表，容量为 10（P25）
    SqList<int> L(10);
    std::cout << "构造空表：Empty? " << (L.Empty() ? "true" : "false") << ", Length=" << L.Length() << "\n";

    // 2) Insert：在表尾插入（position = Length()+1），然后在中间插入（P19/P28）
    L.Insert(1, 10);
    L.Insert(2, 20);
    L.Insert(3, 30);
    L.Insert(4, 40);
    PrintList("初始表 L", L);

    L.Insert(2, 15);           // 在第 2 个位置前插入 15
    L.Insert(5, 35);           // 在第 5 个位置前插入 35
    PrintList("多次 Insert 后", L);

    // 3) GetElem / SetElem（P13/P17）
    int val = -1;
    if (L.GetElem(3, val)) {
        std::cout << "GetElem(3) = " << val << "\n";
    }
    L.SetElem(3, 200);
    L.GetElem(3, val);
    std::cout << "SetElem(3,200) 后 GetElem(3) = " << val << "\n";
    PrintList("修改后 L", L);

    // 4) Delete（P18/P31-P32）：带出参与不带出参两种
    int removed = 0;
    if (L.Delete(5, removed)) {
        std::cout << "Delete(5, e) 成功，e=" << removed << "\n";
    }
    L.Delete(1);  // 删除第 1 个（不关心被删值）
    PrintList("两次 Delete 后", L);

    // 5) Clear（P16）
    L.Clear();
    std::cout << "Clear() 后：Empty? " << (L.Empty() ? "true" : "false")
              << ", Length=" << L.Length() << "\n\n";

    // 6) 课件应用：差集 Difference（P36-P38）
    SqList<int> A(10), B(10), C(10);
    // A = {1,2,3,4,5}, B = {2,4,6}
    for (int x : {1,2,3,4,5}) A.Insert(A.Length()+1, x);
    for (int y : {2,4,6})     B.Insert(B.Length()+1, y);

    PrintList("集合 A", A);
    PrintList("集合 B", B);
    Difference(A, B, C);      // C = A - B = {1,3,5}
    PrintList("差集 C=A-B", C);

    std::cout << "\n==== 演示结束 ====\n";
    return 0;                  // 析构自动触发（P26/P9）
}
