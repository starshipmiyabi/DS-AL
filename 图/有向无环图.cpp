/****************************************************
 * 文件名：有向无环图.cpp
 * 功能：实现
 *   1）有向无环图的拓扑排序（零入度算法 / Kahn 算法）
 *   2）AOE 网的关键路径（Critical Path）求解
 *
 * 主要知识点对应课件：
 *   - 有向无环图及应用：7.5 节        （课件 P81 以后）
 *   - 拓扑排序的概念与零入度算法：P84–P91
 *   - TopSort 算法实现示意代码：P92–P93
 *   - 关键路径、ve / vl / ee / el 计算：P95–P105
 *
 * 为了便于自测，本文件使用「顶点从 0 开始编号」的简单图结构，
 * 不依赖课件里的模板类 AdjMatrixDirGraph，逻辑与课件一致。
 ****************************************************/

#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <iomanip>

using namespace std;

/************** 图的基本数据结构（邻接表） ************************
 * 对应课件 7.2 图的存储表示中「邻接表」思想（课件 P37–P40），
 * 这里我们用一个简化版来表示有向图/有向网。
 ******************************************************************/

// 有向网边结构：终点 to，权值 weight（活动持续时间）
struct Edge {
    int to;         // 边的终点
    int weight;     // 边的权值（对于 AOE 网表示活动时间）
    Edge(int t, int w) : to(t), weight(w) {}
};

// 邻接表：graph[u] 存储从 u 出发的所有边
using AdjList = vector<vector<Edge>>;

/************************************************************
 * 一、拓扑排序（Topological Sort）
 *
 * 零入度算法（Kahn 算法）基本思想 —— 对应课件 P87–P91：
 *   1. 统计每个顶点的入度 inDegree[v]。
 *   2. 将所有入度为 0 的顶点入队（或入栈）。
 *   3. 循环：
 *        从队列取出一个顶点 v 输出到拓扑序列；
 *        删除 v 及其发出的边：对每条边 v -> w，
 *        将 inDegree[w]--，如果变为 0 则入队。
 *   4. 如果最后输出的顶点数 < 顶点总数，说明图中存在有向环。
 ************************************************************/

// 函数：对有向图进行拓扑排序
// 参数：
//   graph      - 邻接表表示的有向图（可以是有权或无权，拓扑排序只看结构）
//   topoOrder  - 输出的拓扑有序序列
// 返回值：
//   true  - 拓扑排序成功（图为有向无环图，DAG）
//   false - 拓扑排序失败（图中存在有向环）
bool topologicalSort(const AdjList &graph, vector<int> &topoOrder) {
    int n = static_cast<int>(graph.size());
    vector<int> inDegree(n, 0);  // 入度数组

    // 1. 统计每个顶点的入度 —— 对应课件 P90 "删除顶点及以它为起点的边 ≡ 终点入度减 1"
    for (int u = 0; u < n; ++u) {
        for (const auto &e : graph[u]) {
            ++inDegree[e.to];
        }
    }

    // 2. 将所有入度为 0 的顶点入队 —— “建立入度为零的顶点队列(或栈)”（P90–P91）
    queue<int> q;
    for (int v = 0; v < n; ++v) {
        if (inDegree[v] == 0) {
            q.push(v);
        }
    }

    // 3. BFS 式处理：队列非空，反复取出顶点并“删除其出边”
    while (!q.empty()) {
        int v = q.front();
        q.pop();
        topoOrder.push_back(v);  // 输出顶点到拓扑序列，对应课件 P91 步骤 ①

        // 对应课件 P91 步骤 ②：删去以 v 为起点的边，终点入度减一
        for (const auto &e : graph[v]) {
            int w = e.to;
            if (--inDegree[w] == 0) {
                // 对应课件 P91 步骤 ③：入度减为 0 的顶点进入队列
                q.push(w);
            }
        }
    }

    // 4. 如果输出顶点个数少于图的顶点个数，说明有向图中含有环
    if (static_cast<int>(topoOrder.size()) < n) {
        // 对应课件 P91：“如果输出顶点个数少于有向图的顶点个数，则报告图中存在有向环”
        return false;
    }
    return true;
}

/************************************************************
 * 二、AOE 网关键路径（Critical Path） —— 对应课件 7.5.2
 *
 * 课件核心公式（P98–P101）：
 *   - 事件(顶点)最早发生时间     ve[j]
 *   - 事件(顶点)最迟发生时间     vl[j]
 *   - 活动(边)最早开始时间       ee[jk] = ve[j]
 *   - 活动(边)最迟开始时间       el[jk] = vl[k] - weight(j,k)
 *   - 若某活动满足 ee[jk] == el[jk]，则该活动为关键活动
 *
 * 计算步骤（AOE 网为有向无环图）：
 *   1. 先做拓扑排序，得到一个拓扑有序序列 topoOrder。
 *   2. 按拓扑序正向扫描，递推计算 ve[]：
 *        ve[source] = 0；
 *        对边 <j,k>：ve[k] = max(ve[k], ve[j] + weight(j,k));
 *   3. 按拓扑序逆序扫描，递推计算 vl[]：
 *        vl[sink] = ve[sink]（总工期）；
 *        对边 <j,k>：vl[j] = min(vl[j], vl[k] - weight(j,k));
 *   4. 对每条边 <j,k> 计算 ee, el，判断是否为关键活动。
 ************************************************************/

// 函数：在 AOE 网中求关键路径
// 参数：
//   graph  - AOE 网的邻接表（边权为活动需要时间）
//   source - 源点（唯一开始事件）
//   sink   - 汇点（唯一结束事件）
// 说明：图必须是有向无环图（DAG），否则无法定义拓扑序与关键路径。
void criticalPath(const AdjList &graph, int source, int sink) {
    int n = static_cast<int>(graph.size());

    // 1. 先做拓扑排序 —— 对应课件“关键路径的实现必须在拓扑有序序列基础上进行”（P102–P105）
    vector<int> topoOrder;
    if (!topologicalSort(graph, topoOrder)) {
        cerr << "错误：图中存在有向环，无法进行关键路径计算（不是 AOE DAG）。" << endl;
        return;
    }

    // 2. 计算所有事件的最早发生时间 ve[] —— 对应公式 ve_k = max{ve_j + w_jk}（P99–P100）
    vector<int> ve(n, 0);
    // 初始化源点最早发生时间为 0
    ve[source] = 0;
    // 按拓扑序正向扫描
    for (int idx = 0; idx < n; ++idx) {
        int u = topoOrder[idx];
        for (const auto &e : graph[u]) {
            int v = e.to;
            int w = e.weight;
            if (ve[u] + w > ve[v]) {
                ve[v] = ve[u] + w;
            }
        }
    }

    // 3. 计算所有事件的最迟发生时间 vl[] —— 对应公式 vl_j = min{vl_k - w_jk}（P99–P100）
    vector<int> vl(n, ve[sink]);   // 初始化为总工期 ve[sink]
    // 按拓扑序逆序扫描
    for (int idx = n - 1; idx >= 0; --idx) {
        int u = topoOrder[idx];
        for (const auto &e : graph[u]) {
            int v = e.to;
            int w = e.weight;
            if (vl[v] - w < vl[u]) {
                vl[u] = vl[v] - w;
            }
        }
    }

    cout << "==== 关键路径计算结果（AOE 网） ====" << endl;
    cout << "事件最早发生时间 ve：" << endl;
    for (int i = 0; i < n; ++i) {
        cout << "ve[" << i << "] = " << ve[i] << endl;
    }
    cout << endl;

    cout << "事件最迟发生时间 vl：" << endl;
    for (int i = 0; i < n; ++i) {
        cout << "vl[" << i << "] = " << vl[i] << endl;
    }
    cout << endl;

    cout << "关键活动（ee == el 的边）：" << endl;
    // 4. 找出所有关键活动 —— 对应课件 P101 “关键活动：el_jk = ee_jk”
    for (int u = 0; u < n; ++u) {
        for (const auto &e : graph[u]) {
            int v = e.to;
            int w = e.weight;
            int ee = ve[u];      // 活动最早开始时间
            int el = vl[v] - w;  // 活动最迟开始时间
            if (ee == el) {
                cout << "活动 <" << u << " -> " << v << "> ，工期 = "
                     << setw(2) << w << " ，为关键活动。" << endl;
            }
        }
    }

    cout << "总工期（从源点到汇点的最长路径长度） = " << ve[sink] << endl;
}

/************************************************************
 * 三、简单主函数，用于演示拓扑排序和关键路径
 *
 * 输入格式示例（可按需要调整）：
 *
 * 1）拓扑排序演示（无权边，仅检查是否有环 + 输出拓扑序）
 *    n m
 *    接下来 m 行：u v   （表示一条有向边 u -> v）
 *
 * 2）关键路径演示（AOE 网，有权边）
 *    n m
 *    接下来 m 行：u v w （表示活动 <u,v> ，持续时间为 w）
 *    source sink        （源点编号、汇点编号）
 *
 * 为避免交互过于复杂，这里分两次简单演示。
 ************************************************************/

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "================ 有向无环图：拓扑排序 & 关键路径示例 ================\n";
    cout << "顶点默认编号为 0..n-1。\n\n";

    /************ 一、拓扑排序示例 ************/
    cout << "[拓扑排序] 请输入顶点数 n 和边数 m（仅结构，用于演示拓扑序）：\n";
    int n, m;
    if (!(cin >> n >> m)) {
        cerr << "输入错误。\n";
        return 0;
    }
    AdjList dag(n);
    cout << "请输入 " << m << " 条边 u v （表示有向边 u -> v）：\n";
    for (int i = 0; i < m; ++i) {
        int u, v;
        cin >> u >> v;
        dag[u].push_back(Edge(v, 1)); // 权值在拓扑排序中无关，这里统一填 1
    }

    vector<int> topoOrder;
    bool ok = topologicalSort(dag, topoOrder);
    if (!ok) {
        cout << "图中存在有向环，无法生成拓扑序列（不是 DAG）。\n\n";
    } else {
        cout << "拓扑有序序列（可能不唯一）：\n";
        for (size_t i = 0; i < topoOrder.size(); ++i) {
            cout << topoOrder[i] << (i + 1 == topoOrder.size() ? '\n' : ' ');
        }
        cout << "\n";
    }

    /************ 二、关键路径示例 ************/
    cout << "[关键路径] 请输入 AOE 网的顶点数 n 和边数 m（带权有向网）：\n";
    int n2, m2;
    if (!(cin >> n2 >> m2)) {
        cerr << "输入错误。\n";
        return 0;
    }
    AdjList aoe(n2);
    cout << "请输入 " << m2 << " 条边 u v w （表示活动 <u,v> ，持续时间 w）：\n";
    for (int i = 0; i < m2; ++i) {
        int u, v, w;
        cin >> u >> v >> w;
        aoe[u].push_back(Edge(v, w));
    }
    int source, sink;
    cout << "请输入源点 source 和汇点 sink 的编号（AOE 网通常只有一个源点和一个汇点）：\n";
    cin >> source >> sink;

    criticalPath(aoe, source, sink);

    return 0;
}
