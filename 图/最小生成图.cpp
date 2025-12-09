// 最小生成图.cpp
// 对应课件 7.4 图的最小代价生成树：
//  - 网的邻接矩阵存储（7.2 P33–34）
//  - Prim 算法（7.4.1 P62–71）
//  - Kruskal 算法思想（7.4.2 P73–79）

#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>

// ===================== 无向网的邻接矩阵存储（对应 7.2 P33–34）=====================

template<typename ElemType, typename WeightType>
class AdjMatrixUndirNetwork {
public:
    static constexpr WeightType ZERO = static_cast<WeightType>(0); // 0 表示“无边”（课件 P33）

    AdjMatrixUndirNetwork(const std::vector<ElemType>& vertices)
        : vexNum(static_cast<int>(vertices.size())),
          edgeNum(0),
          matrix(vexNum, std::vector<WeightType>(vexNum, ZERO)),
          elems(vertices),
          tag(vexNum, false) {}

    // 顶点基本操作（类似 7.1.3 P23）
    bool GetElem(int v, ElemType& e) const {
        if (!checkVertex(v)) return false;
        e = elems[v];
        return true;
    }

    bool SetElem(int v, const ElemType& e) {
        if (!checkVertex(v)) return false;
        elems[v] = e;
        return true;
    }

    int GetVexNum() const { return vexNum; }
    int GetEdgeNum() const { return edgeNum; }

    // 5. FirstAdjVex —— 返回第一个邻接点（7.1.3 P24）
    int FirstAdjVex(int v) const {
        if (!checkVertex(v)) return -1;
        for (int j = 0; j < vexNum; ++j) {
            if (matrix[v][j] > ZERO) return j;
        }
        return -1;
    }

    // 6. NextAdjVex —— 返回相对于 v2 的下一个邻接点（7.1.3 P24）
    int NextAdjVex(int v1, int v2) const {
        if (!checkVertex(v1) || !checkVertex(v2)) return -1;
        for (int j = v2 + 1; j < vexNum; ++j) {
            if (matrix[v1][j] > ZERO) return j;
        }
        return -1;
    }

    // 7. InsertEdge —— 插入无向边 (v1, v2)，权值为 w（7.1.3 P24）
    void InsertEdge(int v1, int v2, WeightType w) {
        if (!checkVertex(v1) || !checkVertex(v2) || v1 == v2) {
            throw std::out_of_range("InsertEdge: 顶点下标非法");
        }
        if (w <= ZERO) {
            throw std::invalid_argument("边权必须为正");
        }
        if (matrix[v1][v2] == ZERO) {
            ++edgeNum;
        }
        matrix[v1][v2] = matrix[v2][v1] = w;
    }

    // 8. DeleteEdge —— 删除边（7.1.3 P25）
    void DeleteEdge(int v1, int v2) {
        if (!checkVertex(v1) || !checkVertex(v2)) return;
        if (matrix[v1][v2] != ZERO) {
            --edgeNum;
            matrix[v1][v2] = matrix[v2][v1] = ZERO;
        }
    }

    // 9. GetWeight —— 返回边 (v1, v2) 的权值（7.1.3 P25）
    WeightType GetWeight(int v1, int v2) const {
        if (!checkVertex(v1) || !checkVertex(v2)) {
            throw std::out_of_range("GetWeight: 顶点下标非法");
        }
        return matrix[v1][v2];
    }

    // 10. SetWeight —— 设置边权（7.1.3 P25）
    void SetWeight(int v1, int v2, WeightType w) {
        if (!checkVertex(v1) || !checkVertex(v2)) {
            throw std::out_of_range("SetWeight: 顶点下标非法");
        }
        matrix[v1][v2] = matrix[v2][v1] = w;
    }

    // 11. GetTag / 12. SetTag —— 用于算法中标记集合 U / V-U（7.1.3 P25–26）
    bool GetTag(int v) const {
        if (!checkVertex(v)) throw std::out_of_range("GetTag: 顶点下标非法");
        return tag[v];
    }

    bool SetTag(int v, bool val) const {
        if (!checkVertex(v)) return false;
        tag[v] = val;
        return true;
    }

private:
    int vexNum;
    int edgeNum;
    std::vector<std::vector<WeightType>> matrix;
    std::vector<ElemType> elems;
    mutable std::vector<bool> tag;

    bool checkVertex(int v) const {
        return v >= 0 && v < vexNum;
    }
};

// ===================== Prim 算法（对应 7.4.1 P62–71）=====================

// 对应课件 P67–68 的 MinVertex：
// 在 V-U 中寻找一条连接 U 的“最小权值边 (w, adjVex[w])” 的端点 w
template<typename ElemType, typename WeightType>
int MinVertex(const AdjMatrixUndirNetwork<ElemType, WeightType>& net,
              const std::vector<int>& adjVex) {
    int n = net.GetVexNum();
    int w = -1;

    // 先找到第一个满足条件的 v（课件 P67）
    for (int v = 0; v < n; ++v) {
        if (!net.GetTag(v) && net.GetWeight(v, adjVex[v]) > AdjMatrixUndirNetwork<ElemType, WeightType>::ZERO) {
            w = v;
            break;
        }
    }
    if (w == -1) return -1;

    // 再在 V-U 中继续寻找权值更小的边（课件 P68）
    for (int v = w + 1; v < n; ++v) {
        if (!net.GetTag(v) &&
            net.GetWeight(v, adjVex[v]) > AdjMatrixUndirNetwork<ElemType, WeightType>::ZERO &&
            net.GetWeight(v, adjVex[v]) < net.GetWeight(w, adjVex[w])) {
            w = v;
        }
    }
    return w;
}

// Prim 最小生成树（课件 7.4.1 P69–71）
template<typename ElemType, typename WeightType>
void MiniSpanTreePrim(const AdjMatrixUndirNetwork<ElemType, WeightType>& net, int u0) {
    int n = net.GetVexNum();
    if (u0 < 0 || u0 >= n) {
        std::cout << "u0 不合法!" << std::endl;
        return;
    }

    std::vector<int> adjVex(n);

    // ① 初始化：U = {u0}，V-U = 其他顶点（课件 P70）
    for (int v = 0; v < n; ++v) {
        if (v != u0) {
            adjVex[v] = u0;          // 当前认为与 U 相连的边是 (v, u0)
            net.SetTag(v, false);    // v ∈ V-U
        } else {
            net.SetTag(v, true);     // u0 ∈ U
            adjVex[v] = u0;
        }
    }

    // ② 重复 n-1 次，从 V-U 中选出最小边并更新（课件 P71）
    for (int i = 1; i < n; ++i) {  // 需要再选择 n-1 条边
        int w = MinVertex(net, adjVex);   // 在 V-U 中选出最小边 (w, adjVex[w])
        if (w == -1) return;              // U 与 V-U 之间无边 —— 图非连通
        std::cout << "edge:(" << adjVex[w] << "," << w
                  << ") weight:" << net.GetWeight(w, adjVex[w]) << std::endl;
        net.SetTag(w, true);              // w 并入 U

        // 用新加入的顶点 w 更新各个 v 的最小边（课件 P71）
        for (int v = net.FirstAdjVex(w); v >= 0; v = net.NextAdjVex(w, v)) {
            if (!net.GetTag(v) &&     // v ∈ V-U
                (net.GetWeight(v, w) < net.GetWeight(v, adjVex[v]) ||
                 net.GetWeight(v, adjVex[v]) == AdjMatrixUndirNetwork<ElemType, WeightType>::ZERO)) {
                adjVex[v] = w;        // 更新 v 连接到 U 的最小边
            }
        }
    }
}

// ===================== Kruskal 算法（对应 7.4.2 P73–79）=====================

// 课件 7.4.2 只给出了思路：
//  1）按权从小到大排序边；
//  2）依次尝试加入“安全边”，用并查集判断是否构成回路。
// 这里给出完整实现，便于实验。

template<typename WeightType>
struct KruskalEdge {
    int u, v;
    WeightType w;
};

class DisjointSet {
public:
    explicit DisjointSet(int n) : parent(n), rank(n, 0) {
        for (int i = 0; i < n; ++i) parent[i] = i;
    }
    int Find(int x) {
        if (parent[x] != x) parent[x] = Find(parent[x]);
        return parent[x];
    }
    void Union(int x, int y) {
        x = Find(x); y = Find(y);
        if (x == y) return;
        if (rank[x] < rank[y]) parent[x] = y;
        else if (rank[x] > rank[y]) parent[y] = x;
        else { parent[y] = x; ++rank[x]; }
    }
private:
    std::vector<int> parent, rank;
};

// Kruskal 最小生成树（课件 7.4.2 总结 P78–79）
template<typename ElemType, typename WeightType>
void MiniSpanTreeKruskal(const AdjMatrixUndirNetwork<ElemType, WeightType>& net) {
    int n = net.GetVexNum();
    std::vector<KruskalEdge<WeightType>> edges;

    // 收集所有边（只取 i<j，避免重复）
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            WeightType w = net.GetWeight(i, j);
            if (w > AdjMatrixUndirNetwork<ElemType, WeightType>::ZERO) {
                edges.push_back({i, j, w});
            }
        }
    }

    // 按权值从小到大排序（课件 P78）
    std::sort(edges.begin(), edges.end(),
              [](const KruskalEdge<WeightType>& a,
                 const KruskalEdge<WeightType>& b) {
                  return a.w < b.w;
              });

    DisjointSet dsu(n);
    int accepted = 0;
    for (const auto& e : edges) {
        if (accepted == n - 1) break;
        int ru = dsu.Find(e.u);
        int rv = dsu.Find(e.v);
        if (ru != rv) { // 不在同一棵自由树中，加入不会形成回路（课件 P74）
            dsu.Union(ru, rv);
            ++accepted;
            std::cout << "edge:(" << e.u << "," << e.v
                      << ") weight:" << e.w << std::endl;
        }
    }
}

int main() {
    // 使用课件 7.4.1 P65 的例子（城市/顶点命名略）
    std::vector<char> vs = {'a','b','c','d','e','f','g'};
    AdjMatrixUndirNetwork<char, int> net(vs);

    net.InsertEdge(0,1,19);
    net.InsertEdge(0,5,16);
    net.InsertEdge(0,6,14);
    net.InsertEdge(1,2,5);
    net.InsertEdge(1,5,18);
    net.InsertEdge(1,6,8);
    net.InsertEdge(2,3,12);
    net.InsertEdge(2,4,6);
    net.InsertEdge(2,5,7);
    net.InsertEdge(3,4,3);
    net.InsertEdge(4,5,2);
    net.InsertEdge(4,6,21);

    std::cout << "Prim MST:\n";
    MiniSpanTreePrim(net, 0);

    std::cout << "\nKruskal MST:\n";
    MiniSpanTreeKruskal(net);
    return 0;
}