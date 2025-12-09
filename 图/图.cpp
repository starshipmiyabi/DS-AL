// 图.cpp
// 根据《数据结构与算法分析》Ch7 图 课件实现：
// 1）图的基本操作（课件 7.1.2：P18–21）
// 2）图的遍历：DFS / BFS（课件 7.3：P44–48, P53–54）

#include <iostream>
#include <vector>
#include <queue>
#include <stdexcept>

// ===================== 图的邻接表存储结构（对应 7.2 P37–40）=====================
// 这里实现一个既可以表示有向图也可以表示无向图的邻接表图。
// 课件中将有向图/无向图拆成不同类，这里通过 directed 标志统一处理。

template<typename ElemType>
class AdjListGraph {
public:
    // 构造函数：给定顶点元素数组，指定是否为有向图
    // 对应 “顶点表 + 每个顶点一条邻接链表”的思想（课件 7.2 P37 图示）
    AdjListGraph(const std::vector<ElemType>& vertices, bool directed = false)
        : vexNum(static_cast<int>(vertices.size())),
          edgeNum(0),
          adjList(vexNum),
          elems(vertices),
          tag(vexNum, false),
          directed(directed) {}

    // 1. GetElem —— 返回顶点 v 的元素值（课件 7.1.2 P18）
    bool GetElem(int v, ElemType& e) const {
        if (!checkVertex(v)) return false;
        e = elems[v];
        return true;
    }

    // 2. SetElem —— 设置顶点 v 的元素值（课件 7.1.2 P18）
    bool SetElem(int v, const ElemType& e) {
        if (!checkVertex(v)) return false;
        elems[v] = e;
        return true;
    }

    // 3. GetVexNum —— 返回顶点个数（课件 7.1.2 P18）
    int GetVexNum() const { return vexNum; }

    // 4. GetEdgeNum —— 返回边数（课件 7.1.2 P18）
    int GetEdgeNum() const { return edgeNum; }

    // 5. FirstAdjVex —— 返回顶点 v 的第一个邻接点（课件 7.1.2 P19）
    int FirstAdjVex(int v) const {
        if (!checkVertex(v) || adjList[v].empty()) return -1;
        return adjList[v][0];
    }

    // 6. NextAdjVex —— 返回相对于 v2 的下一个邻接点（课件 7.1.2 P19）
    int NextAdjVex(int v1, int v2) const {
        if (!checkVertex(v1) || !checkVertex(v2)) return -1;
        const auto& neighbors = adjList[v1];
        for (std::size_t i = 0; i + 1 < neighbors.size(); ++i) {
            if (neighbors[i] == v2) {
                return neighbors[i + 1];
            }
        }
        return -1;  // 没有“下一个邻接点”
    }

    // 7. InsertEdge —— 插入边 <v1, v2>（课件 7.1.2 P19）
    // 无向图会自动插入 (v2, v1)
    void InsertEdge(int v1, int v2) {
        if (!checkVertex(v1) || !checkVertex(v2)) {
            throw std::out_of_range("InsertEdge: 顶点下标非法");
        }
        // 避免平行边：若已存在则不重复添加
        if (!isNeighbor(v1, v2)) {
            adjList[v1].push_back(v2);
            ++edgeNum;
        }
        if (!directed) {
            if (!isNeighbor(v2, v1)) {
                adjList[v2].push_back(v1);
            }
        }
    }

    // 8. DeleteEdge —— 删除边 <v1, v2>（课件 7.1.2 P20）
    void DeleteEdge(int v1, int v2) {
        if (!checkVertex(v1) || !checkVertex(v2)) return;
        if (eraseNeighbor(v1, v2)) {
            --edgeNum;
        }
        if (!directed) {
            eraseNeighbor(v2, v1);
        }
    }

    // 9. GetTag —— 获取顶点 v 的访问标志（课件 7.1.2 P20）
    bool GetTag(int v) const {
        if (!checkVertex(v)) {
            throw std::out_of_range("GetTag: 顶点下标非法");
        }
        return tag[v];
    }

    // 10. SetTag —— 设置顶点 v 的访问标志（课件 7.1.2 P20）
    // 这里用 mutable + const 方法，方便在 const 遍历函数中修改 tag
    bool SetTag(int v, bool val) const {
        if (!checkVertex(v)) return false;
        tag[v] = val;
        return true;
    }

    // 11. DFSTraverse —— 深度优先遍历（课件 7.3 P44–48）
    // 思路：
    //  1）将所有顶点标志 tag 置为 false（P44）
    //  2）从每个未访问的顶点出发，递归 DFS（P46）
    void DFSTraverse(void (*visit)(const ElemType& e)) const {
        for (int v = 0; v < vexNum; ++v) {
            tag[v] = false;
        }
        // 非连通图：对每个未访问的顶点调用 DFS（课件 P46）
        for (int v = 0; v < vexNum; ++v) {
            if (!tag[v]) {
                DFS(v, visit);
            }
        }
    }

    // 12. BFSTraverse —— 广度优先遍历（课件 7.3 P52–54）
    // 思路类似树的层次遍历（课件 P51），使用队列（P53）
    void BFSTraverse(void (*visit)(const ElemType& e)) const {
        for (int v = 0; v < vexNum; ++v) {
            tag[v] = false;
        }
        std::queue<int> q;
        for (int v = 0; v < vexNum; ++v) {
            if (!tag[v]) {
                BFSFrom(v, visit, q);
            }
        }
    }

private:
    int vexNum;                                   // 顶点个数
    int edgeNum;                                  // 边数
    std::vector<std::vector<int>> adjList;        // 邻接表（每个顶点一条链表）
    std::vector<ElemType> elems;                  // 顶点数据
    mutable std::vector<bool> tag;                // 访问标志（DFS/BFS 使用）
    bool directed;                                // 是否为有向图

    bool checkVertex(int v) const {
        return v >= 0 && v < vexNum;
    }

    bool isNeighbor(int from, int to) const {
        const auto& neighbors = adjList[from];
        for (int w : neighbors) if (w == to) return true;
        return false;
    }

    bool eraseNeighbor(int from, int to) {
        auto& neighbors = adjList[from];
        for (auto it = neighbors.begin(); it != neighbors.end(); ++it) {
            if (*it == to) {
                neighbors.erase(it);
                return true;
            }
        }
        return false;
    }

    // 递归 DFS —— 对应课件中 AdjListUndirGraph::DFS 实现（P48）
    void DFS(int v, void (*visit)(const ElemType& e)) const {
        tag[v] = true;          // SetTag(v, true)
        visit(elems[v]);        // 访问顶点数据元素
        // 对所有尚未访问的邻接点递归 DFS
        for (int w : adjList[v]) {
            if (!tag[w]) DFS(w, visit);
        }
    }

    // 单源 BFS —— 对应课件 AdjListDirGraph::BFS 实现（P53–54）
    void BFSFrom(int start,
                 void (*visit)(const ElemType& e),
                 std::queue<int>& q) const {
        tag[start] = true;
        visit(elems[start]);
        q.push(start);
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (int w : adjList[u]) {
                if (!tag[w]) {
                    tag[w] = true;
                    visit(elems[w]);
                    q.push(w);
                }
            }
        }
    }
};

// 示例 visit 函数
template<typename ElemType>
void PrintElem(const ElemType& e) {
    std::cout << e << " ";
}

// 如需简单测试，可在编译时定义 GRAPH_TEST_MAIN 开关。
// g++ 图.cpp -DGRAPH_TEST_MAIN
int main() {
    std::vector<char> vs = {'A','B','C','D','E','F'};
    AdjListGraph<char> g(vs, false); // 无向图

    // 构造课件 7.1.1 中类似的无向图（P7、P10）
    g.InsertEdge(0,1); // A-B
    g.InsertEdge(0,4); // A-E
    g.InsertEdge(1,4); // B-E
    g.InsertEdge(1,5); // B-F
    g.InsertEdge(2,3); // C-D
    g.InsertEdge(2,5); // C-F
    g.InsertEdge(3,5); // D-F

    std::cout << "DFS: ";
    g.DFSTraverse(PrintElem<char>);
    std::cout << "\nBFS: ";
    g.BFSTraverse(PrintElem<char>);
    std::cout << std::endl;
    return 0;
}