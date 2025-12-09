// 哈夫曼树与哈夫曼编码.cpp
// ------------------------------------------------------------
// 参考课件：四川大学《数据结构与算法》Ch06《树与二叉树 2024》
//   §6.6 哈夫曼树与哈夫曼编码（p.132 起）
//   • 基本概念：路径长度 / 带权路径长度 WPL（p.133）
//   • 哈夫曼树（最优树）性质（p.134–135）
//   • 构造算法步骤（p.137“构造哈夫曼树的算法”）
//   • 前缀编码与样例 “CAST CAST SAT AT A TASA”（p.147–148）
//   • 实现提示：结点结构、类接口与编码/译码（p.151–157）
//
// 文件目标：给出一个可直接编译运行的、注释完善的哈夫曼树与哈夫曼编码实现，
// 覆盖课件里“所有方法”的核心功能：
//   1) 由字符集和权值（频度）构造哈夫曼树（Build/Select，两最小权合并）
//   2) 为每个字符生成前缀编码（Encode(char)）
//   3) 把比特串译回原字符序列（Decode(bits)）
//   4) 计算并展示 WPL，与样例中的“总编码长度 = WPL”相印证（p.148）
//
// 编译：g++ -std=c++17 -O2 -Wall -Wextra 哈夫曼树与哈夫曼编码.cpp -o huffman_demo
// 运行：./huffman_demo
//
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <queue>
#include <functional>
using namespace std;

/** ===================== 一、理论对照小抄 =====================
 * 1) 路径长度、WPL：见课件 p.133
 *    WPL(T) = sum_{i=1..n} (w_i * l_i)，其中 l_i 为叶结点到根的边数。
 *    对应编码长度：当把“频度/次数”作为权 w_i 时，总编码长度 = WPL（p.148）。
 *
 * 2) 哈夫曼树（最优树）：在所有含 n 个叶子的 m 叉树中，WPL 最小者（p.134）。
 *    性质：权越大的叶结点离根越近（p.135 图示）。
 *
 * 3) 构造算法（以二叉树为例，p.137）：
 *    (1) 初始把 n 个权 w_i 各自作为一棵“只有根”的树；
 *    (2) 反复从当前森林中选出“权值最小的两棵树”合并为新树；
 *    (3) 新树的权 = 两子树权之和；重复直到只剩一棵树。
 *
 * 4) 前缀编码（p.146–148）：把左分支记作 0、右分支记作 1，
 *    从根到叶形成的 0/1 序列即该字符的码字；任一码字都不是另外一码字的前缀。
 *    例：A(7)、T(5)、C(2)、S(4) → 可能得到 A:0, T:10, C:110, S:111；
 *    总长度 7*1 + 5*2 + 2*3 + 4*3 = 35 = WPL。
 * ========================================================== */

/** ===================== 二、结点与树的定义 =====================
 * 课件 p.151–152 给出了 HuffmanTreeNode 与 HuffmanTree 的成员设计。
 * 本实现沿用其思想，但采用更现代 C++ 容器与接口，便于直接运行。
 */

// —— 哈夫曼树结点（对照 p.151 的结构体）：
struct HuffNode
{
    int weight = 0;      // 权（频度）
    int parent = 0;      // 双亲下标（0 表示“无/未连”）
    int left = 0;        // 左孩子下标
    int right = 0;       // 右孩子下标
    char ch = '\0';      // 若为叶结点，存放对应字符；内部结点可为 '\0'
    bool isLeaf = false; // 是否叶结点（用于打印/构码）
};

// —— 哈夫曼树类（对照 p.152 接口：构造、Encode、Decode 等）：
class HuffmanTree
{
public:
    // 使用给定字符集与权（频度）构造哈夫曼树；见“构造算法”p.137。
    HuffmanTree(const vector<char> &chars, const vector<int> &weights)
    {
        build(chars, weights);
        buildCodes(); // 叶到根逆推码字（p.156–157 思想）
    }

    // 返回单个字符的哈夫曼码；若不存在，抛异常。（对照 p.157 Encode 思想）
    string Encode(char c) const
    {
        auto it = codeOf.find(c);
        if (it == codeOf.end())
            throw runtime_error("未知字符，无法编码");
        return it->second;
    }

    // 把输入文本编码为 01 串；顺便返回总长度。
    string EncodeString(const string &text) const
    {
        string out;
        out.reserve(text.size() * 8);
        for (char c : text)
            out += Encode(c);
        return out;
    }

    // 把 01 串译回原文（对照 p.157 Decode 思想：从根沿 0/1 走到叶即输出并回根）
    string Decode(const string &bits) const
    {
        string out;
        int root = (int)nodes.size() - 1; // 按构造，最后一个下标即根
        int cur = root;
        for (char b : bits)
        {
            if (b == '0')
                cur = nodes[cur].left;
            else if (b == '1')
                cur = nodes[cur].right;
            else
                throw runtime_error("比特串中含有非 0/1 字符");
            if (nodes[cur].isLeaf)
            {
                out.push_back(nodes[cur].ch);
                cur = root;
            }
        }
        if (cur != root)
            throw runtime_error("比特串非叶处结束 —— 不是合法的前缀码序列");
        return out;
    }

    // 计算并返回 WPL（p.133 定义）：∑(权 * 叶深度)。
    long long WPL() const
    {
        long long wpl = 0;
        for (int i = 1; i <= n; ++i)
        {
            int depth = 0, x = i;
            while (nodes[x].parent != 0)
            {
                ++depth;
                x = nodes[x].parent;
            }
            wpl += 1LL * nodes[i].weight * depth;
        }
        return wpl;
    }

    // 访问全部码字（字符 → 01 串）。
    const unordered_map<char, string> &Codes() const { return codeOf; }

    // 打印调试信息：结构、码字、WPL。
    void Dump(ostream &os = cout) const
    {
        os << "=== Huffman Codes ===\n";
        for (auto &kv : codeOf)
            os << kv.first << " : " << kv.second << "\n";
        os << "WPL = " << WPL() << "  (∑ w_i * l_i)\n";
        os << "结点个数 m = " << nodes.size() - 1 << " (2n-1), 叶子 n = " << n << "\n";
    }

private:
    // 1-based 存储（与课件一致，便于“parent==0 表示无”）；大小 = 2n-1。
    vector<HuffNode> nodes;
    int n = 0;                          // 叶子个数
    unordered_map<char, int> leafIndex; // 叶字符 → 结点下标
    unordered_map<char, string> codeOf; // 叶字符 → 码字

    // —— 选择函数：在 [1..cur] 中找出 parent==0 的“最小两权”的下标 r1, r2（p.155 Select）。
    pair<int, int> selectTwoMin(int cur)
    {
        int r1 = 0, r2 = 0;
        for (int i = 1; i <= cur; ++i)
        {
            if (nodes[i].parent != 0)
                continue;
            if (r1 == 0 || nodes[i].weight < nodes[r1].weight)
            {
                r2 = r1;
                r1 = i;
            }
            else if (r2 == 0 || nodes[i].weight < nodes[r2].weight)
            {
                r2 = i;
            }
        }
        if (r1 == 0 || r2 == 0)
            throw runtime_error("selectTwoMin 失败：可选节点不足");
        // 为了生成与课件示例更一致的码字，把较小权作为“左孩子”（常见约定）。
        if (nodes[r1].weight > nodes[r2].weight)
            swap(r1, r2);
        return {r1, r2};
    }

    // —— 按 p.137 的三步构造哈夫曼树；并对照 p.151–156 的字段语义。
    void build(const vector<char> &chars, const vector<int> &weights)
    {
        if (chars.size() != weights.size() || chars.empty())
            throw invalid_argument("输入的字符与权值长度不一致或为空");
        n = (int)chars.size();
        int m = 2 * n - 1;
        nodes.assign(m + 1, {}); // 1..m 有效

        // (①) 把 ch 与 w 存入叶结点（p.154–155）。
        for (int i = 1; i <= n; ++i)
        {
            nodes[i].weight = weights[i - 1];
            nodes[i].ch = chars[i - 1];
            nodes[i].isLeaf = true;
            leafIndex[chars[i - 1]] = i;
        }

        // (②) 建树：每次从 [1..k-1] 中取两棵 parent==0 的最小权树合并到 k（p.155–156）。
        for (int k = n + 1; k <= m; ++k)
        {
            auto [r1, r2] = selectTwoMin(k - 1);
            nodes[r1].parent = nodes[r2].parent = k;
            nodes[k].left = r1; // 约定更小权作为左（便于复现 p.148 的编码）
            nodes[k].right = r2;
            nodes[k].weight = nodes[r1].weight + nodes[r2].weight;
        }
    }

    // (③) 由“叶到根”逆向求码字（p.156–157）：左边走 '0'，右边走 '1'。
    void buildCodes()
    {
        codeOf.clear();
        for (int i = 1; i <= n; ++i)
        {
            string code;
            int child = i, parent = nodes[child].parent;
            while (parent != 0)
            {
                code.push_back(nodes[parent].left == child ? '0' : '1');
                child = parent;
                parent = nodes[child].parent;
            }
            reverse(code.begin(), code.end());
            if (code.empty())
                code = "0"; // n=1 时约定编码为 "0"
            codeOf[nodes[i].ch] = code;
        }
    }
};

/** ===================== 三、演示（与课件示例一致） =====================
 * 课件 p.147–148 的文本：
 *   "CAST CAST SAT AT A TASA"
 * 字符集 {C,A,S,T}，频度 W = {2,7,4,5}。
 * 下面程序：
 *   • 用上述频度构造哈夫曼树；
 *   • 打印每个字符的哈夫曼码与 WPL；
 *   • 把样例文本编码后再译码回来，并输出长度对比（WPL vs. 总编码长度）。
 */
int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // —— 样例字符与频度（p.147 的表述）。
    vector<char> chars = {'C', 'A', 'S', 'T'};
    vector<int> freq = {2, 7, 4, 5};

    HuffmanTree ht(chars, freq);
    ht.Dump();

    // —— 编解码样例文本（p.147）。
    string text = "CAST CAST SAT AT A TASA";
    // 移除空格，因为字符集中不包含空格
    string textNoSpace;
    for (char c : text)
    {
        if (c != ' ')
            textNoSpace += c;
    }
    string bits = ht.EncodeString(textNoSpace);
    string back = ht.Decode(bits);

    cout << "\n原文（含空格）: " << text << "\n";
    cout << "原文（无空格）: " << textNoSpace << "\n";
    cout << "编码: " << bits << "\n";
    cout << "译码: " << back << "\n";

    // —— 计算“总编码长度=各字符频度×其码长之和”，对比 WPL（p.148）。
    long long sumLen = 0;
    for (size_t i = 0; i < chars.size(); ++i)
    {
        int w = freq[i];
        int L = (int)ht.Encode(chars[i]).size();
        sumLen += 1LL * w * L;
    }
    cout << "\n按 p.148：总编码长度 = ∑(频度×码长) = " << sumLen
         << "，WPL = " << ht.WPL() << "\n";

    return 0;
}
