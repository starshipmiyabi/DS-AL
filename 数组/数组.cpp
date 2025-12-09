/******************************************************
 * 数组.cpp  —— 参照《数据结构与算法分析 · 第5章 数组》
 * 目标：
 *   1) 用类模板实现一个 n 维数组的顺序存储（行优先，Row-major）。
 *   2) 完成课件中 Array 的所有方法：
 *      - Array(int dm, ...)            // 构造：给定维数及各维长度
 *      - ~Array()                      // 析构
 *      - ElemType& operator()(...)     // 重载() 下标访问（可变参数）
 *      - Array(const Array&)           // 复制构造
 *      - Array& operator=(const Array&)// 赋值运算
 *      - int Locate(int, va_list&)     // 私有：下标→线性位置
 *
 * 约定与实现细节（和课件一致）：
 *   • 存储采用行优先（Row-major），即最右边下标变化最快
 *     —— 参见课件 5.1.2 二维数组“以行序为主序(按行排列)”与 n 维推广（P9, P14）。
 *   • n 维映射（行优先）公式（见 P14）：
 *       Map(j1,...,jn) = j1*b2*...*bn + j2*b3*...*bn + ... + j(n-1)*bn + jn
 *     我们在构造时预计算 constants[i] = ∏_{k=i+1..n-1} b_{k}，以 O(n) 求位址。
 *   • 下标一律从 0 开始（与 C/C++ 一致），越界将抛出 std::out_of_range。
 *
 * 教学提示：
 *   • 观察 &A(i,...)-&A(0,...) 的差值即可得其线性下标（单位：元素个数）
 *     —— 这能直观看出 Loc(0,0)+(b2*i + j)*L（见 P10）的“+ (b2*i+j)”部分。
 ******************************************************/

#include <iostream>
#include <cstdarg>     // va_list, va_start, va_end
#include <stdexcept>   // std::out_of_range, std::bad_alloc
#include <iomanip>     // std::setw
#include <utility>     // std::swap

/***************
 * Array 类模板
 ***************/
template <class ElemType>
class Array {
protected:
    // 数据成员 —— 与课件一致（见 P20）：
    ElemType *base;   // 数组元素基址
    int dim;          // 数组维数
    int *bounds;      // 各维长度 b[i]
    int *constants;   // 映射函数常量（行优先）c[i] = ∏_{k=i+1..dim-1} b[k]
    int total;        // 元素总数（便于调试/演示）

    // 私有辅助：根据可变参数下标，计算线性位置（见 P20：Locate 原型）
    int Locate(int sub0, va_list &va) const {
        // 边界：必须至少 1 维
        if (dim <= 0) throw std::out_of_range("Array dimension must be positive.");

        // 处理第 0 维下标
        int idx = 0;
        int sub = sub0;
        if (sub < 0 || sub >= bounds[0])
            throw std::out_of_range("Subscript out of range on dimension 0.");
        idx += sub * constants[0];

        // 依次读取剩余 dim-1 个下标，并累加线性偏移
        for (int i = 1; i < dim; ++i) {
            sub = va_arg(va, int);
            if (sub < 0 || sub >= bounds[i])
                throw std::out_of_range("Subscript out of range on dimension " + std::to_string(i) + ".");
            idx += sub * constants[i];
        }
        return idx;  // 行优先映射位置（元素下标，不含字节大小）
    }

    // 释放内部资源
    void release_() noexcept {
        delete[] base;      base = nullptr;
        delete[] bounds;    bounds = nullptr;
        delete[] constants; constants = nullptr;
        dim = 0; total = 0;
    }

    // 深拷贝（供复制构造/赋值使用）
    void deepcopy_from_(const Array &src) {
        dim = src.dim;
        total = src.total;

        bounds    = new int[dim];
        constants = new int[dim];
        for (int i = 0; i < dim; ++i) {
            bounds[i]    = src.bounds[i];
            constants[i] = src.constants[i];
        }
        base = new ElemType[total];
        for (int i = 0; i < total; ++i) base[i] = src.base[i];
    }

public:
    /***********************
     * 构造函数（见 P21）
     * 参数：
     *   dm  —— 维数
     *   ... —— 紧随其后依次给出每一维的长度（int），共 dm 个
     ***********************/
    Array(int dm, ...) : base(nullptr), dim(dm), bounds(nullptr), constants(nullptr), total(0) {
        if (dm <= 0) throw std::out_of_range("Array dimension must be positive.");

        // 读取各维长度 b[i]
        bounds = new int[dim];
        va_list ap;
        va_start(ap, dm);
        for (int i = 0; i < dim; ++i) {
            int bi = va_arg(ap, int);
            if (bi <= 0) {
                va_end(ap);
                delete[] bounds; bounds = nullptr;
                throw std::out_of_range("Each dimension length must be positive.");
            }
            bounds[i] = bi;
        }
        va_end(ap);

        // 计算总元素数 total = ∏ b[i]
        long long prod = 1;
        for (int i = 0; i < dim; ++i) {
            prod *= bounds[i];
            if (prod > static_cast<long long>(std::numeric_limits<int>::max()))
                throw std::overflow_error("Array is too large.");
        }
        total = static_cast<int>(prod);

        // 预计算 constants（行优先，见 P14 “n 维数组行序映射”）
        constants = new int[dim];
        constants[dim - 1] = 1;                  // 最右维乘子 = 1
        for (int i = dim - 2; i >= 0; --i) {     // c[i] = b[i+1] * c[i+1]
            constants[i] = bounds[i + 1] * constants[i + 1];
        }

        // 分配并零初始化线性存储
        base = new ElemType[total](); // () 确保对基础类型置零，对类类型调用默认构造

        // —— 对应课件思想：顺序存储且“数组元素的存储位置是其下标的线性函数”（P14）
    }

    // 析构（见 P21）
    virtual ~Array() { release_(); }

    // 复制构造（见 P21）
    Array(const Array &source)
        : base(nullptr), dim(0), bounds(nullptr), constants(nullptr), total(0) {
        deepcopy_from_(source);
    }

    // 赋值重载（见 P21）
    Array &operator=(const Array &source) {
        if (this == &source) return *this;
        // 强异常安全：先复制，再交换
        Array temp(source);
        std::swap(base, temp.base);
        std::swap(bounds, temp.bounds);
        std::swap(constants, temp.constants);
        std::swap(dim, temp.dim);
        std::swap(total, temp.total);
        return *this;
    }

    // 重载 () —— 元素访问（见 P21）
    // 可写版本
    ElemType &operator()(int sub0, ...) {
        va_list ap;
        va_start(ap, sub0);
        int pos = Locate(sub0, ap);   // 计算行优先映射位置
        va_end(ap);
        return base[pos];
    }
    // 只读版本（便于在 const 环境下访问）
    const ElemType &operator()(int sub0, ...) const {
        va_list ap;
        va_start(ap, sub0);
        int pos = Locate(sub0, ap);
        va_end(ap);
        return base[pos];
    }

    /**************
     * —— 以下为教学/演示的便捷接口（非课件必需）——
     **************/
    int Dimensions() const { return dim; }
    int Length(int d) const {
        if (d < 0 || d >= dim) throw std::out_of_range("dimension index invalid.");
        return bounds[d];
    }
    int Size() const { return total; }

    // 打印形状
    void PrintShape(std::ostream &os = std::cout) const {
        os << "shape=(";
        for (int i = 0; i < dim; ++i) {
            os << bounds[i];
            if (i + 1 < dim) os << ",";
        }
        os << ")\n";
    }
};

/*********************************************
 *                示例与测试
 * main() 将验证一维 / 二维 / 三维情形下的
 * 行优先映射（与课件 P8/P9/P10/P13 一致）。
 *********************************************/
int main() {
    try {
        // === 1) 一维数组：Map(i) = i  （见 P8）
        Array<int> A1(1, 10); // 10 个元素，缺省初始化为 0
        for (int i = 0; i < 10; ++i) A1(i) = (i + 1) * 3;
        std::cout << "[一维] "; A1.PrintShape();
        std::cout << "A1: ";
        for (int i = 0; i < 10; ++i) std::cout << A1(i) << (i + 1 < 10 ? " " : "\n");
        std::cout << "验证 Map(i)=i，例如 &A1(7)-&A1(0) = "
                  << (&A1(7) - &A1(0)) << "（应为 7）\n\n";

        // === 2) 二维数组（m 行 n 列）行优先：
        //     Loc(i,j) = Loc(0,0) + (n*i + j)*L  （见 P10）
        const int m = 3, n = 4;
        Array<int> A2(2, m, n);
        // 按 “行优先（行序）” 填充：A2(i,j) = i*10 + j
        for (int i = 0; i < m; ++i)
            for (int j = 0; j < n; ++j)
                A2(i, j) = i * 10 + j;

        std::cout << "[二维] "; A2.PrintShape();
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j)
                std::cout << std::setw(3) << A2(i, j) << " ";
            std::cout << "\n";
        }
        // 行优先验证：&A2(1,2)-&A2(0,0) == 1*n + 2
        std::ptrdiff_t off12 = &A2(1,2) - &A2(0,0);
        std::cout << "验证 Row-major：&A2(1,2)-&A2(0,0) = " << off12
                  << "；公式 n*i+j = " << (n*1 + 2) << "\n\n";

        // === 3) 三维数组（页 i / 行 j / 列 k）行优先：
        //     Map(i,j,k) = i*(m2*m3) + j*(m3) + k  （见 P13）
        const int p = 2, q = 3, r = 4; // 2×3×4
        Array<int> A3(3, p, q, r);
        // 简单填充并验证一个位置
        for (int i = 0; i < p; ++i)
            for (int j = 0; j < q; ++j)
                for (int k = 0; k < r; ++k)
                    A3(i, j, k) = (i+1)*100 + (j+1)*10 + (k+1);

        int ti=1, tj=2, tk=3; // 目标下标（1,2,3）
        std::ptrdiff_t off123 = &A3(ti, tj, tk) - &A3(0,0,0);
        int formula = ti*(q*r) + tj*(r) + tk;
        std::cout << "[三维] "; A3.PrintShape();
        std::cout << "A3(" << ti << "," << tj << "," << tk << ") = " << A3(ti,tj,tk) << "\n";
        std::cout << "验证 Row-major：&A3(" << ti << "," << tj << "," << tk
                  << ")-&A3(0,0,0) = " << off123
                  << "；公式 i*(q*r)+j*(r)+k = " << formula << "\n\n";

        // === 4) 拷贝与赋值测试（见 P21）
        Array<int> B = A2;      // 复制构造
        Array<int> C(2, 1, 1);  // 先建一个 1x1
        C = A2;                 // 赋值
        std::cout << "[拷贝/赋值] B(2,3) = " << B(2,3) << ", C(2,3) = " << C(2,3) << "\n";

        // 越界演示（可注释掉观察）：应抛出异常
        // std::cout << A2(m, 0) << "\n"; // m 越界
    }
    catch (const std::exception &ex) {
        std::cerr << "异常： " << ex.what() << "\n";
    }
    return 0;
}

/*
编译建议：
  g++ -std=c++17 -O2 数组.cpp -o 数组

运行输出要点（示例）：
  [一维] shape=(10)
  A1: 3 6 9 12 15 18 21 24 27 30
  验证 Map(i)=i，例如 &A1(7)-&A1(0) = 7（应为 7）

  [二维] shape=(3,4)
    0   1   2   3
   10  11  12  13
   20  21  22  23
  验证 Row-major：&A2(1,2)-&A2(0,0) = 6；公式 n*i+j = 6

  [三维] shape=(2,3,4)
  A3(1,2,3) = 233
  验证 Row-major：差值与公式一致
*/
