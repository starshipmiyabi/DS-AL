/******************************************************
 * 矩阵.cpp —— 参照《Ch05 数组和广义表 2024.pdf》矩阵部分
 * 目标：
 *   1) 实现通用稠密矩阵 Matrix（行主序存储，外部1-based下标）。
 *   2) 实现“特殊矩阵”的三种经典压缩存储：
 *        • 对称矩阵 SymmetricMatrix —— 仅存下(或上)三角（P28）。
 *        • 下/上三角矩阵 TriangularMatrix —— 仅存一个三角（P29）。
 *        • 三对角矩阵 TridiagonalMatrix —— 仅存主对角及上下各一条（P30–P31）。
 *   3) 实现稀疏矩阵三元组表 TriSparseMatrix 及“简单/快速”转置（P36–P59）。
 *
 * 重要说明（与课件一致的约定）：
 *   • 本文件所有“矩阵对外下标”均采用 1 开始（P24）。
 *     若你更习惯 0 开始，可把对外接口处做一次 ±1 的映射。
 *   • 稠密矩阵行优先（Row-major）内存布局（P10、P11）。
 *   • 示例 main() 覆盖：稠密矩阵、对称/三角/三对角、三元组转置，便于 IDE 直接运行验证。
 ******************************************************/

#include <iostream>
#include <vector>
#include <stdexcept>
#include <iomanip>
#include <cstring>   // memset
#include <limits>
#include <utility>

/*======================================================
=            一、通用稠密矩阵 Matrix<T> (P24–P25)        =
======================================================*/
template <class ElemType>
class Matrix {
    int rows, cols;            // 行/列（对外1-based）
    std::vector<ElemType> buf; // 行主序存储区
    // 将1-based (i,j) 映射到线性位置（行主序）：(i-1)*cols + (j-1)（P10）
    int pos_(int i, int j) const {
        if (i < 1 || i > rows || j < 1 || j > cols)
            throw std::out_of_range("Matrix index out of range (1-based).");
        return (i - 1) * cols + (j - 1);
    }
public:
    Matrix(int r=0, int c=0, const ElemType& init=ElemType{})
        : rows(r), cols(c), buf(r*c, init) {}

    int GetRows() const { return rows; }   //（P25）
    int GetCols() const { return cols; }   //（P25）

    // 元素读写（1-based）——（P25）
    ElemType& operator()(int i, int j) { return buf[pos_(i,j)]; }
    const ElemType& operator()(int i, int j) const { return buf[pos_(i,j)]; }

    // 演示：填充 A(i,j) = base + (i-1)*cols + (j-1)
    void FillSequence(ElemType base=ElemType{}) {
        for (int i=1;i<=rows;++i)
            for (int j=1;j<=cols;++j)
                (*this)(i,j) = base + static_cast<ElemType>((i-1)*cols + (j-1));
    }

    void Print(const std::string& name="A") const {
        std::cout << name << " (" << rows << "x" << cols << ")\n";
        for (int i=1;i<=rows;++i) {
            for (int j=1;j<=cols;++j)
                std::cout << std::setw(4) << (*this)(i,j);
            std::cout << "\n";
        }
    }
};

/*======================================================
=      二、特殊矩阵：对称/三角/三对角的压缩存储 (P27–P31)   =
======================================================*/

/*------------------ 1) 对称矩阵（SymmetricMatrix）(P28) ------------------
 * 思想：只存下三角（含主对角）。访问任意 (i,j) 时转为 (max(i,j), min(i,j)).
 * 映射（行序为主）：
 *   对 1 ≤ j ≤ i ≤ n，线性下标 k = i*(i-1)/2 + j - 1   （P28公式）
 *   证明见教材，此为等差求和结果。
 */
template <class ElemType>
class SymmetricMatrix {
    int n;
    std::vector<ElemType> buf; // 长度 n(n+1)/2
    static int lower_index_(int i, int j, int n) {
        if (i < 1 || i > n || j < 1 || j > n)
            throw std::out_of_range("SymmetricMatrix index out of range (1-based).");
        if (i < j) std::swap(i, j); // 转为下三角
        // k = i(i-1)/2 + j - 1
        return (i * (i - 1)) / 2 + (j - 1);
    }
public:
    SymmetricMatrix(int n_, const ElemType& init=ElemType{}): n(n_), buf(n_*(n_+1)/2, init) {}
    int Size() const { return n; }
    ElemType& operator()(int i, int j) { return buf[lower_index_(i,j,n)]; }
    const ElemType& operator()(int i, int j) const { return buf[lower_index_(i,j,n)]; }
};

/*------------------ 2) 三角矩阵（TriangularMatrix）(P29) ------------------
 * 仅存一个三角区域。这里给出“下三角/上三角”二选一的实现：
 *   下三角：存 i≥j 的元素，k = i(i-1)/2 + j - 1   （P29公式）
 *   上三角：可映射到下三角形式：存 j≥i 的元素，k = j(j-1)/2 + i - 1
 * 未存区域：get 时返回 0 值；set 时若写未存区域，抛出异常提醒。
 */
template <class ElemType>
class TriangularMatrix {
    int n;
    bool upper;                     // false=下三角，true=上三角
    std::vector<ElemType> buf;      // n(n+1)/2
    int index_(int i, int j) const {
        if (i<1||i>n||j<1||j>n) throw std::out_of_range("Triangular index OOR.");
        if (!upper) {               // 下三角：i>=j
            if (i < j) throw std::logic_error("Write to zero region of lower-triangular.");
            return (i*(i-1))/2 + (j-1);
        } else {                    // 上三角：j>=i
            if (j < i) throw std::logic_error("Write to zero region of upper-triangular.");
            return (j*(j-1))/2 + (i-1);
        }
    }
public:
    TriangularMatrix(int n_, bool upper_=false, const ElemType& init=ElemType{})
        : n(n_), upper(upper_), buf(n_*(n_+1)/2, init) {}
    int Size() const { return n; }

    // set：仅允许写入存储区域
    void set(int i, int j, const ElemType& v) { buf[index_(i,j)] = v; }

    // get：未存区域视为 0（学习友好，可方便打印/验证）
    ElemType get(int i, int j) const {
        if (i<1||i>n||j<1||j>n) throw std::out_of_range("Triangular index OOR.");
        if (!upper) {
            if (i < j) return ElemType{};
            return buf[(i*(i-1))/2 + (j-1)];
        } else {
            if (j < i) return ElemType{};
            return buf[(j*(j-1))/2 + (i-1)];
        }
    }

    void Print(const std::string& name="T") const {
        std::cout << name << " (n="<<n<<")\n";
        for (int i=1;i<=n;++i) {
            for (int j=1;j<=n;++j) std::cout << std::setw(4) << get(i,j);
            std::cout << "\n";
        }
    }
};

/*------------------ 3) 三对角矩阵（TridiagonalMatrix）(P30–P31) ------------------
 * 仅主对角与上下各一条：|i-j| ≤ 1。
 * 单数组“按行序存放”的常见实现（与P31右图“另一种存储方法”匹配）：
 *   行1： a11, a12                                  —— 2个
 *   行i(2..n-1)： a(i,i-1), a(i,i), a(i,i+1)        —— 3个
 *   行n： a(n,n-1), a(n,n)                          —— 2个
 *   总元素 3n - 2，线性下标 k 范围 0..(3n-3)（P31 页脚标注）
 *   统一映射：令 rowOffset(i) = (i==1 ? 0 : 3*(i-1)-1) = 3i-4
 *              （i=1时 offset=0；i=2时 offset=2；i=3时 offset=5；……）
 *     则：
 *       if i==1:         j∈{1,2}       => k = (j-1)
 *       else if i==n:    j∈{n-1,n}     => k = (3n-4) + (j-(n-1))
 *       else (2..n-1):
 *            j∈{i-1,i,i+1}            => k = (3i-4) + (j-(i-1)) = 2i + j - 3
 */
template <class ElemType>
class TridiagonalMatrix {
    int n;
    std::vector<ElemType> buf;        // 长度 3n-2
    int index_(int i, int j) const {
        if (i<1||i>n||j<1||j>n) throw std::out_of_range("Tridiagonal index OOR.");
        if (std::abs(i-j)>1) throw std::logic_error("Accessing zero region of tridiagonal.");
        if (i==1) return j-1;                       // j=1/2
        if (i==n) return (3*n-4) + (j-(n-1));       // j=n-1/n
        // middle rows
        return 3*i - 4 + (j-(i-1));                 // j=i-1,i,i+1
    }
public:
    explicit TridiagonalMatrix(int n_, const ElemType& init=ElemType{})
        : n(n_), buf((n_>=1? 3*n_-2:0), init) {}
    int Size() const { return n; }
    ElemType& at(int i, int j) { return buf[index_(i,j)]; }
    ElemType get(int i, int j) const {
        if (i<1||i>n||j<1||j>n) throw std::out_of_range("Tridiagonal index OOR.");
        if (std::abs(i-j)>1) return ElemType{};   // 未存区域视为 0
        if (i==1) return buf[j-1];
        if (i==n) return buf[(3*n-4) + (j-(n-1))];
        return buf[3*i - 4 + (j-(i-1))];
    }
    void Print(const std::string& name="TD") const {
        std::cout << name << " (n="<<n<<")\n";
        for (int i=1;i<=n;++i) {
            for (int j=1;j<=n;++j) std::cout << std::setw(4) << get(i,j);
            std::cout << "\n";
        }
    }
};

/*======================================================
=          三、稀疏矩阵：三元组顺序表 (P36–P59)          =
======================================================*/

// 三元组（P40）
template <class ElemType>
struct Triple {
    int row{}, col{};   // 1-based
    ElemType value{};
    Triple() = default;
    Triple(int r, int c, ElemType v): row(r), col(c), value(v) {}
};

// 稀疏矩阵三元组顺序表（P43–P44，接口完全对齐）
template <class ElemType>
class TriSparseMatrix {
protected:
    Triple<ElemType>* triElems;  // 三元组数组
    int maxSize;
    int rows, cols, num;         // 行、列、非零元个数

public:
    explicit TriSparseMatrix(int rs=0, int cs=0, int size=0)
        : triElems(nullptr), maxSize(size), rows(rs), cols(cs), num(0) {
        if (maxSize < 0) maxSize = 0;
        triElems = (maxSize>0)? new Triple<ElemType>[maxSize]: nullptr;
    }
    ~TriSparseMatrix() { delete [] triElems; }

    // 基本查询（P36）
    int GetRows() const { return rows; }
    int GetCols() const { return cols; }
    int GetNum()  const { return num;  }
    bool Empty()  const { return num==0; }  //（P37）

    // 设置/获取单元（P45–P48，完整对齐课件伪码/思路）
    bool SetElem(int r, int c, const ElemType &v) {
        if (r<1 || r>rows || c<1 || c>cols) return false;  // 下标范围错（P45①）
        // 在有序三元组表中查找插入/删除位置（按 row, col 增序）
        int pos;
        for (pos = num-1; pos>=0 &&
             (r < triElems[pos].row ||
             (r == triElems[pos].row && c < triElems[pos].col)); --pos) {}

        if (pos >= 0 && triElems[pos].row == r && triElems[pos].col == c) {
            // ②③：位置存在
            if (v == ElemType{}) {
                // 删除该三元组（向前搬移）
                for (int k = pos+1; k < num; ++k) triElems[k-1] = triElems[k];
                --num;
            } else {
                // 修改值
                triElems[pos].value = v;
            }
            return true;
        } else {
            // ④⑤：位置不存在
            if (v == ElemType{}) return true;  // 不做任何修改（P48⑤）
            if (num >= maxSize) return false;  // 溢出（P48）
            // 插入到 pos+1 位置（先把 [pos+1..num-1] 整体后移一格）
            for (int k = num-1; k > pos; --k) triElems[k+1] = triElems[k];
            triElems[pos+1] = Triple<ElemType>(r,c,v);
            ++num;
            return true;
        }
    }

    bool GetElem(int r, int c, ElemType &v) const {
        if (r<1 || r>rows || c<1 || c>cols) return false;
        // 顺序查找（也可改为二分）
        for (int k=0;k<num;++k) {
            if (triElems[k].row==r && triElems[k].col==c) {
                v = triElems[k].value; return true;
            } else if (triElems[k].row>r || (triElems[k].row==r && triElems[k].col>c)) {
                break;
            }
        }
        v = ElemType{}; // 缺省为0
        return true;
    }

    // 赋值（深拷贝）
    TriSparseMatrix& operator=(const TriSparseMatrix& src) {
        if (this==&src) return *this;
        delete [] triElems;
        rows=src.rows; cols=src.cols; num=src.num; maxSize=src.maxSize;
        triElems = (maxSize>0)? new Triple<ElemType>[maxSize]: nullptr;
        for (int i=0;i<num;++i) triElems[i]=src.triElems[i];
        return *this;
    }

    // —— 稀疏矩阵转置 ——

    // a) 简单转置（P51–P55）：O(cols * num)
    static void SimpleTranspose(const TriSparseMatrix& source,
                                TriSparseMatrix& dest) {
        dest.rows = source.cols;
        dest.cols = source.rows;
        dest.num  = source.num;
        dest.maxSize = source.maxSize;
        delete [] dest.triElems;
        dest.triElems = (dest.maxSize>0)? new Triple<ElemType>[dest.maxSize]: nullptr;

        if (dest.num==0) return;
        int destPos = 0;
        for (int col=1; col<=source.cols; ++col) {           // 列序扫描（P54）
            for (int s=0; s<source.num; ++s) {
                if (source.triElems[s].col == col) {
                    dest.triElems[destPos].row   = source.triElems[s].col; // 列→行
                    dest.triElems[destPos].col   = source.triElems[s].row; // 行→列
                    dest.triElems[destPos].value = source.triElems[s].value;
                    ++destPos;
                }
            }
        }
    }

    // b) 快速转置（P56–P59）：O(cols + num)
    static void FastTranspose(const TriSparseMatrix& source,
                              TriSparseMatrix& dest) {
        dest.rows = source.cols;
        dest.cols = source.rows;
        dest.num  = source.num;
        dest.maxSize = source.maxSize;
        delete [] dest.triElems;
        dest.triElems = (dest.maxSize>0)? new Triple<ElemType>[dest.maxSize]: nullptr;
        if (dest.num==0) return;

        std::vector<int> cNum(source.cols+1, 0); // 1..cols（P58）
        std::vector<int> cPos(source.cols+1, 0);
        // 统计每一列的非零元个数（P59）
        for (int s=0; s<source.num; ++s) ++cNum[source.triElems[s].col];
        // 递推求每列在目标三元组中的起始位置（P58、P59）
        cPos[1] = 0;
        for (int col=2; col<=source.cols; ++col) cPos[col] = cPos[col-1] + cNum[col-1];
        // 逐个搬运（行列号互换），并推进当前列写入位置（P59）
        for (int s=0; s<source.num; ++s) {
            int col = source.triElems[s].col;
            int dp  = cPos[col]++;                 // 在该列“当前可写位置”
            dest.triElems[dp].row   = source.triElems[s].col;
            dest.triElems[dp].col   = source.triElems[s].row;
            dest.triElems[dp].value = source.triElems[s].value;
        }
    }

    // 打印为稠密形式（教学用）
    void PrintDense(const std::string& name="S") const {
        std::cout << name << " ("<<rows<<"x"<<cols<<", nnz="<<num<<")\n";
        int k=0;
        for (int i=1;i<=rows;++i) {
            for (int j=1;j<=cols;++j) {
                ElemType v{};
                GetElem(i,j,v);
                std::cout << std::setw(4) << v;
            }
            std::cout << "\n";
        }
    }
};

/*======================================================
=                        示例 main()                    =
======================================================*/
int main() {
    std::cout << "==== 稠密矩阵 Matrix 示例（行主序，1-based） ====\n";
    Matrix<int> A(3,4);
    A.FillSequence(0);
    A.Print("A");  // 验证行主序的填充顺序（P10、P11）

    std::cout << "\n==== 对称矩阵 SymmetricMatrix 示例（仅存下三角） ====\n";
    SymmetricMatrix<int> S(4);
    // 设置几个元素（注意下三角/上三角统一访问）
    S(1,1)=11; S(4,1)=41; S(3,2)=32; S(2,4)=24; // (2,4) 会映射到 (4,2)
    for (int i=1;i<=4;++i) {
        for (int j=1;j<=4;++j) std::cout<<std::setw(4)<<S(i,j);
        std::cout<<"\n";
    }

    std::cout << "\n==== 下三角矩阵 TriangularMatrix(lower) 示例 ====\n";
    TriangularMatrix<int> L(4, /*upper=*/false);
    for (int i=1;i<=4;++i) for (int j=1;j<=i;++j) L.set(i,j, 10*i + j);
    L.Print("Lower");

    std::cout << "\n==== 上三角矩阵 TriangularMatrix(upper) 示例 ====\n";
    TriangularMatrix<int> U(4, /*upper=*/true);
    for (int j=1;j<=4;++j) for (int i=1;i<=j;++i) U.set(i,j, 100*j + i);
    U.Print("Upper");

    std::cout << "\n==== 三对角矩阵 TridiagonalMatrix 示例（3n-2 压缩） ====\n";
    TridiagonalMatrix<int> TD(5);
    // 仅对 |i-j|<=1 的位置赋值
    for (int i=1;i<=5;++i) {
        if (i>1)     TD.at(i, i-1) = -1*i;    // 下对角
        TD.at(i, i)      = 10*i;              // 主对角
        if (i<5)     TD.at(i, i+1) =  i;      // 上对角
    }
    TD.Print("TD");

    std::cout << "\n==== 稀疏矩阵 TriSparseMatrix：设置 + 快速转置 ====\n";
    // 以课件 P42 的 5x6 示例为基准，创建并设置若干非零元
    TriSparseMatrix<int> SM(5,6,16); // rows, cols, max nnz
    // P42 示例： (1,3)=2; (2,6)=8; (3,1)=1; (3,3)=3; (5,1)=4; (5,3)=6
    SM.SetElem(1,3,2);
    SM.SetElem(2,6,8);
    SM.SetElem(3,1,1);
    SM.SetElem(3,3,3);
    SM.SetElem(5,1,4);
    SM.SetElem(5,3,6);
    SM.PrintDense("SM");

    TriSparseMatrix<int> ST;
    TriSparseMatrix<int>::FastTranspose(SM, ST);   // P56–P59
    ST.PrintDense("ST=SM^T");

    std::cout << "\n提示：本程序所有映射/接口均在注释中标明对应课件页码，便于核对学习。\n";
    return 0;
}
