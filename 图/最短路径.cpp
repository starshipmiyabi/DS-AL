/****************************************************
 * 文件名：最短路径.cpp
 * 功能：实现
 *   1）单源点最短路径：Dijkstra 算法
 *   2）所有顶点对之间的最短路径：Floyd 算法
 *
 * 主要知识点对应课件：
 *   - 7.6 最短路径 (Shortest Path)          （课件 P107 以后）
 *   - Dijkstra 算法思想、dist/path 数组    （P109–P116）
 *   - 课件给出的 ShortestPathDIJ 伪代码    （P118–P120）
 *   - Floyd 算法的 D^(k)[i][j] 递推公式    （P123–P127）
 ****************************************************/

#include <iostream>
#include <vector>
#include <iomanip>
#include <limits>

using namespace std;

const int INF = 1000000000; // 代表“无穷大”（不可达），对应课件中的 MY_INFINITY

/************************************************************
 * 一、Dijkstra 单源最短路径 —— 对应课件 7.6.1（P109–P120）
 *
 * 基本思想（课件描述简化版）：
 *   - 用 dist[v] 存放当前已知的从源点 s 到 v 的最短路径长度；
 *   - 用 path[v] 存放该最短路径上 v 的前驱顶点；
 *   - 集合 U 存放“已经确定最短路径”的顶点；
 *   - 每次从 V-U 中选 dist 最小的顶点 v1 加入 U，
 *     然后用 v1 作为“中间点”尝试松弛其它顶点 v2 的 dist。
 *
 * 前提：
 *   - 图中边权必须非负（课件 P110），否则 Dijkstra 可能失效。
 ************************************************************/

// 函数：Dijkstra 最短路径
// 参数：
//   graph - 邻接矩阵 graph[u][v] 表示边 u->v 的权值，若为 INF 表示无边
//   s     - 源点编号
//   dist  - 输出：源点到各点的最短距离
//   path  - 输出：源点到各点的最短路径上，当前顶点的直接前驱
void dijkstra(const vector<vector<int>> &graph, int s,
              vector<int> &dist, vector<int> &path) {
    int n = static_cast<int>(graph.size());
    vector<bool> visited(n, false); // 对应课件中的顶点标志 tag / 集合 U

    dist.assign(n, INF);
    path.assign(n, -1);

    // ① 初始化 —— 对应课件 P114 中 dist[] 和 path[] 的初值
    for (int v = 0; v < n; ++v) {
        dist[v] = graph[s][v];       // 若没有边则为 INF；若 v==s 则 graph[s][s] = 0
        if (v != s && graph[s][v] < INF) {
            path[v] = s;             // 如果存在边 s -> v，则 v 的前驱为 s
        } else {
            path[v] = -1;            // 不存在直接边的，前驱初始化为 -1
        }
    }
    dist[s] = 0;        // 源点到自身距离为 0
    visited[s] = true;  // U 中只有源点 s

    // ② 外层循环：执行 n-1 次，每次确定一个新的最短路径顶点
    for (int i = 1; i < n; ++i) {
        int v1 = -1;
        int minVal = INF;
        // ③ 在 V-U 中寻找 dist 最小的顶点 v1 —— 对应课件 P115/P119 中“选 v_j 使 dist[j] 最小”
        for (int v = 0; v < n; ++v) {
            if (!visited[v] && dist[v] < minVal) {
                minVal = dist[v];
                v1 = v;
            }
        }

        if (v1 == -1) {
            // 剩余顶点均不可达
            break;
        }

        visited[v1] = true; // 将 v1 并入 U

        // ④ 用 v1 作为中间点，尝试改善 v2 的 dist[v2] —— 对应课件 P115/P120
        for (int v2 = 0; v2 < n; ++v2) {
            // 只考虑还不在 U 中、且存在边 v1->v2 的顶点
            if (!visited[v2] && graph[v1][v2] < INF) {
                // 如果通过 v1 到 v2 的路径更短，则更新
                if (dist[v1] + graph[v1][v2] < dist[v2]) {
                    dist[v2] = dist[v1] + graph[v1][v2];
                    path[v2] = v1; // 更新前驱
                }
            }
        }
    }
}

// 辅助函数：打印从源点到某个终点的路径（递归方式）
// path[v] 记录的是 v 的直接前驱
void printPath(const vector<int> &path, int v) {
    if (v == -1) return;
    if (path[v] == -1) {
        cout << v;  // 源点
    } else {
        printPath(path, path[v]);
        cout << " -> " << v;
    }
}

/************************************************************
 * 二、Floyd 多源最短路径 —— 对应课件 7.6.2（P122–P127）
 *
 * 核心递推（课件 P123–P126）：
 *   设 D^(k)[i][j] 表示从 i 到 j 的最短路径长度，
 *   中间点只允许使用 {v0, v1, ..., vk}。
 *
 *   初始 (k = -1)：中间点集合为空：
 *      D^(-1)[i][j] = 0                , i==j
 *                      graph[i][j]     , 存在边 i->j
 *                      INF             , 否则
 *
 *   递推：
 *      D^(k)[i][j] = min( D^(k-1)[i][j],
 *                         D^(k-1)[i][k] + D^(k-1)[k][j] )
 *
 * 实际实现中，我们直接用 dist[i][j] 原地更新，
 * 三重循环顺序为 k -> i -> j，等价于上述公式。
 ************************************************************/

// 函数：Floyd 算法，求所有顶点对之间的最短路径
// 参数：
//   graph - 邻接矩阵，graph[i][j] 为边权，无边则为 INF
//   dist  - 输出：dist[i][j] 为 i 到 j 的最短距离
//   path  - 输出：path[i][j] 为 i 到 j 的最短路径上，j 的前驱顶点
void floyd(const vector<vector<int>> &graph,
           vector<vector<int>> &dist,
           vector<vector<int>> &path) {
    int n = static_cast<int>(graph.size());
    dist = graph;
    path.assign(n, vector<int>(n, -1));

    // ① 初始化 dist 和 path —— 对应课件 P123 "开始：D^(-1)[i][j] = ..."
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i == j) {
                dist[i][j] = 0;
                path[i][j] = -1;       // 自己到自己，无前驱
            } else if (graph[i][j] < INF) {
                dist[i][j] = graph[i][j];
                path[i][j] = i;        // i 直接到 j，则 j 的前驱为 i
            } else {
                dist[i][j] = INF;
                path[i][j] = -1;       // 不可达
            }
        }
    }

    // ② 三重循环 —— 对应课件 P126 "第 k 步：D^(k)[i][j] = Min{ D^(k-1)[i][j], D^(k-1)[i][k] + D^(k-1)[k][j] }"
    for (int k = 0; k < n; ++k) {
        for (int i = 0; i < n; ++i) {
            if (dist[i][k] == INF) continue; // 剪枝：i->k 不可达
            for (int j = 0; j < n; ++j) {
                if (dist[k][j] == INF) continue; // 剪枝：k->j 不可达
                if (dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    path[i][j] = path[k][j]; // j 的前驱改为原来 k->j 路径上的前驱
                }
            }
        }
    }
}

// 辅助函数：输出 i 到 j 的路径
void printFloydPath(const vector<vector<int>> &path, int i, int j) {
    if (i == j) {
        cout << i;
        return;
    }
    if (path[i][j] == -1) {
        cout << "（不可达）";
        return;
    }
    // 倒着回溯：先把路径存下来，再正序输出
    vector<int> seq;
    int v = j;
    while (v != -1 && v != i) {
        seq.push_back(v);
        v = path[i][v];
    }
    if (v == -1) {
        cout << "（不可达）";
        return;
    }
    cout << i;
    for (int k = static_cast<int>(seq.size()) - 1; k >= 0; --k) {
        cout << " -> " << seq[k];
    }
}

/************************************************************
 * 三、简单主函数，演示 Dijkstra 与 Floyd
 *
 * 输入格式示例：
 *
 * 1）统一先输入一个有向网：
 *    n m
 *    接下来 m 行：u v w  （有向边 u -> v，权值 w）
 *    不存在的边默认权值为 INF
 *
 * 2）然后指定 Dijkstra 源点 s。
 *
 * 代码会输出：
 *   - 对每个顶点 v：从 s 到 v 的最短距离 + 路径
 *   - Floyd 的所有顶点对最短距离矩阵
 ************************************************************/

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "================ 最短路径：Dijkstra & Floyd 示例 ================\n";
    cout << "请先输入一个有向网，顶点编号为 0..n-1\n";
    cout << "格式：n m\n";
    cout << "然后输入 m 行：u v w （表示边 u -> v ，权值 w > 0，若有多条边取最小）\n\n";

    int n, m;
    if (!(cin >> n >> m)) {
        cerr << "输入错误。\n";
        return 0;
    }

    // 初始化邻接矩阵
    vector<vector<int>> graph(n, vector<int>(n, INF));
    for (int i = 0; i < n; ++i) {
        graph[i][i] = 0;
    }

    cout << "请输入 " << m << " 条边：\n";
    for (int i = 0; i < m; ++i) {
        int u, v, w;
        cin >> u >> v >> w;
        if (u < 0 || u >= n || v < 0 || v >= n) {
            cerr << "边顶点编号非法，跳过该边。\n";
            continue;
        }
        // 若有多条边，只保留最小权值
        graph[u][v] = min(graph[u][v], w);
    }

    /*********************** Dijkstra 演示 ************************/
    cout << "\n===== Dijkstra 单源最短路径 =====\n";
    int s;
    cout << "请输入源点编号 s：";
    cin >> s;
    if (s < 0 || s >= n) {
        cerr << "源点编号非法。\n";
        return 0;
    }

    vector<int> dist, path;
    dijkstra(graph, s, dist, path);

    cout << "\n从源点 " << s << " 出发到各顶点的最短路径：\n";
    for (int v = 0; v < n; ++v) {
        cout << "顶点 " << v << " ：";
        if (dist[v] == INF) {
            cout << "不可达\n";
        } else {
            cout << "距离 = " << setw(4) << dist[v] << " ，路径：";
            printPath(path, v);
            cout << "\n";
        }
    }

    /*********************** Floyd 演示 ************************/
    cout << "\n===== Floyd 所有顶点对最短路径 =====\n";
    vector<vector<int>> distFloyd, pathFloyd;
    floyd(graph, distFloyd, pathFloyd);

    cout << "\n最短距离矩阵 dist[i][j]：\n";
    cout << "    ";
    for (int j = 0; j < n; ++j) {
        cout << setw(6) << j;
    }
    cout << "\n";

    for (int i = 0; i < n; ++i) {
        cout << setw(3) << i << " ";
        for (int j = 0; j < n; ++j) {
            if (distFloyd[i][j] == INF) {
                cout << setw(6) << "INF";
            } else {
                cout << setw(6) << distFloyd[i][j];
            }
        }
        cout << "\n";
    }

    cout << "\n示例：打印任意一对顶点 (i, j) 的路径\n";
    cout << "请输入 i 和 j（-1 -1 结束）：\n";
    while (true) {
        int i, j;
        cin >> i >> j;
        if (i == -1 && j == -1) break;
        if (i < 0 || i >= n || j < 0 || j >= n) {
            cout << "编号非法，请重新输入。\n";
            continue;
        }
        cout << "从 " << i << " 到 " << j << " 的路径：";
        if (distFloyd[i][j] == INF) {
            cout << "不可达\n";
        } else {
            printFloydPath(pathFloyd, i, j);
            cout << " ，总权值 = " << distFloyd[i][j] << "\n";
        }
    }

    return 0;
}
