// 哈希表.cpp
// 对应课件：Chapter 8.4 哈希表（Hash Table）
//   - 8.4.1 哈希表的概念：第 60–65 页
//   - 8.4.2 构造哈希函数的方法：第 67–70 页
//   - 8.4.3 处理冲突的方法：第 72–82 页
//
// 本文件实现：
//   1. 哈希表的基本结构（数组 + 记录存储位置与关键字之间的映射关系）；
//   2. 三种典型哈希函数形式（平方取中法、除留余数法、伪随机法接口）；
//   3. 开放定址法（线性探测、二次探测），对应第 76–78 页；
//   4. 链地址法（分离链接法），对应第 79–80 页；
//   5. 简单展示“装载因子”和“扩容”思想，对应第 81–82 页。
//
// 注意：
//   这里的实现以 int 类型关键字为例，便于与课件中的数字示例对应。

#include <iostream>
#include <vector>
#include <list>
#include <cmath>
#include <random>

//-------------------------------------------------------------
// 一、哈希函数实现（对应 8.4.2）
//-------------------------------------------------------------

/**
 * @brief 平方取中法（对应第 68 页）
 * 简化实现：
 *   1. 计算 key * key；
 *   2. 将平方值转为字符串，取中间若干位；
//   3. 将中间位转换为整数后再对 tableSize 取模。
 */
size_t hashSquareMiddle(int key, size_t tableSize)
{
    long long sq = 1LL * key * key; // 使用 long long 防止溢出
    std::string s = std::to_string(std::llabs(sq));
    if (s.size() <= 2)
        return static_cast<size_t>(sq % static_cast<long long>(tableSize));

    // 取中间 2~3 位（依据长度适配）
    size_t mid = s.size() / 2;
    size_t start = (mid > 1) ? mid - 1 : 0;
    size_t len = std::min(static_cast<size_t>(3), s.size() - start);
    int middle = std::stoi(s.substr(start, len));

    return static_cast<size_t>(middle % static_cast<int>(tableSize));
}

/**
 * @brief 除留余数法（对应第 69 页）
 *   H(key) = key % p，其中 p <= m（表长），p 一般取不大于 m 的素数。
 */
size_t hashModPrime(int key, size_t p)
{
    long long v = key >= 0 ? key : -1LL * key;
    return static_cast<size_t>(v % static_cast<long long>(p));
}

/**
 * @brief 伪随机哈希（对应第 70 页 Random(key)）
 *
 * 出于可复现性与教学需要，这里使用固定种子的伪随机数引擎。
 * 课件中提醒：“伪随机数法可移植性差，应慎用”。
 */
size_t hashPseudoRandom(int key, size_t tableSize)
{
    static std::mt19937 rng(2024); // 固定种子，保证可复现
    std::uniform_int_distribution<int> dist(0, static_cast<int>(tableSize - 1));
    // 将 key 作为扰动，不同 key 产生不同序列
    rng.discard(static_cast<unsigned long long>(std::abs(key)) % 9973);
    return static_cast<size_t>(dist(rng));
}

//-------------------------------------------------------------
// 二、开放定址哈希表（对应 8.4.3 中“开放定址法/闭域法”）
//-------------------------------------------------------------

/**
 * @brief 开放定址哈希表中的槽状态
 */
enum class SlotState
{
    EMPTY,      // 从未占用
    OCCUPIED,   // 当前有元素
    DELETED     // 曾经有元素，被删除
};

/**
 * @brief 哈希表槽
 */
struct HashSlot
{
    int key;
    SlotState state;

    HashSlot() : key(0), state(SlotState::EMPTY) {}
};

/**
 * @brief 开放定址哈希表（线性探测/二次探测）
 *
 * 对应课件第 76–78 页：
 *   - H_i = (H(key) + d_i) % m
 *   - 线性探测：d_i = i
 *   - 二次探测：d_i = i^2
 *
 * 这里仅支持“插入 + 查找”，删除用“标记删除”的方式。
 */
class OpenAddressHashTable
{
public:
    enum class ProbeType
    {
        LINEAR,     // 线性探测 d_i = i
        QUADRATIC   // 二次探测 d_i = i^2
    };

    explicit OpenAddressHashTable(size_t tableSize,
                                  ProbeType type = ProbeType::LINEAR)
        : table(tableSize), probeType(type), elemCount(0)
    {
    }

    size_t size() const { return table.size(); }

    // 装载因子 = 填入表中的元素个数 / 表长（对应第 81 页）
    double loadFactor() const
    {
        return static_cast<double>(elemCount) / static_cast<double>(table.size());
    }

    bool insert(int key)
    {
        if (loadFactor() > 0.7)
        {
            // 简单示意：装载因子过大时进行“扩容”，对应第 82 页
            rehash(table.size() * 2 + 1);
        }

        size_t m = table.size();
        size_t h0 = hashModPrime(key, m); // 这里用除留余数法作为基础哈希
        for (size_t i = 0; i < m; ++i)
        {
            size_t di = probeOffset(i);
            size_t h = (h0 + di) % m;

            if (table[h].state == SlotState::EMPTY ||
                table[h].state == SlotState::DELETED)
            {
                table[h].key = key;
                table[h].state = SlotState::OCCUPIED;
                ++elemCount;
                return true;
            }
            else if (table[h].state == SlotState::OCCUPIED &&
                     table[h].key == key)
            {
                // 已存在
                return true;
            }
        }
        return false; // 表已满
    }

    bool find(int key) const
    {
        size_t m = table.size();
        size_t h0 = hashModPrime(key, m);
        for (size_t i = 0; i < m; ++i)
        {
            size_t di = probeOffset(i);
            size_t h = (h0 + di) % m;

            if (table[h].state == SlotState::EMPTY)
            {
                // 遇到 EMPTY 说明 key 从未插入过，可提前结束
                return false;
            }
            if (table[h].state == SlotState::OCCUPIED && table[h].key == key)
            {
                return true;
            }
            // 遇到 DELETED 或其他 key，继续探测
        }
        return false;
    }

    bool erase(int key)
    {
        size_t m = table.size();
        size_t h0 = hashModPrime(key, m);
        for (size_t i = 0; i < m; ++i)
        {
            size_t di = probeOffset(i);
            size_t h = (h0 + di) % m;

            if (table[h].state == SlotState::EMPTY)
            {
                return false;
            }
            if (table[h].state == SlotState::OCCUPIED && table[h].key == key)
            {
                table[h].state = SlotState::DELETED;
                --elemCount;
                return true;
            }
        }
        return false;
    }

    void debugPrint() const
    {
        std::cout << "开放定址哈希表（size = " << table.size()
                  << ", 装载因子 ≈ " << loadFactor() << "）\n";
        for (size_t i = 0; i < table.size(); ++i)
        {
            std::cout << i << ": ";
            if (table[i].state == SlotState::OCCUPIED)
                std::cout << table[i].key;
            else if (table[i].state == SlotState::DELETED)
                std::cout << "DELETED";
            else
                std::cout << "EMPTY";
            std::cout << "\n";
        }
        std::cout << "\n";
    }

private:
    std::vector<HashSlot> table;
    ProbeType probeType;
    size_t elemCount;

    size_t probeOffset(size_t i) const
    {
        if (probeType == ProbeType::LINEAR)
        {
            // d_i = i
            return i;
        }
        else
        {
            // d_i = i^2
            return i * i;
        }
    }

    // 简单的扩容 & 重新散列（对应第 82 页“扩容的方法”）
    void rehash(size_t newSize)
    {
        std::vector<HashSlot> oldTable = table;
        table.assign(newSize, HashSlot());
        elemCount = 0;

        for (const auto &slot : oldTable)
        {
            if (slot.state == SlotState::OCCUPIED)
                insert(slot.key);
        }
    }
};

//-------------------------------------------------------------
// 三、链地址（分离链接）哈希表（对应 8.4.3 中“链地址法/开域法”）
//-------------------------------------------------------------

/**
 * @brief 链地址哈希表：每个槽是一个链表，对应第 79–80 页。
 *
 * 适合大对象/大数据量，因为单个链表上可以挂很多元素，
 * 并且可以灵活替换为红黑树等结构（课件第 79–80 页的说明）。
 */
class ChainedHashTable
{
public:
    explicit ChainedHashTable(size_t tableSize)
        : buckets(tableSize)
    {
    }

    void insert(int key)
    {
        size_t index = hashModPrime(key, buckets.size());
        auto &lst = buckets[index];
        for (int x : lst)
        {
            if (x == key)
                return; // 已存在
        }
        lst.push_front(key);
    }

    bool find(int key) const
    {
        size_t index = hashModPrime(key, buckets.size());
        const auto &lst = buckets[index];
        for (int x : lst)
        {
            if (x == key)
                return true;
        }
        return false;
    }

    void erase(int key)
    {
        size_t index = hashModPrime(key, buckets.size());
        auto &lst = buckets[index];
        lst.remove(key);
    }

    void debugPrint() const
    {
        std::cout << "链地址哈希表（桶数 = " << buckets.size() << "）\n";
        for (size_t i = 0; i < buckets.size(); ++i)
        {
            std::cout << i << ": ";
            for (int x : buckets[i])
                std::cout << x << " -> ";
            std::cout << "NULL\n";
        }
        std::cout << "\n";
    }

private:
    std::vector<std::list<int>> buckets;
};

//-------------------------------------------------------------
 // 四、演示 main
//-------------------------------------------------------------

int main()
{
    std::cout << "================ 哈希表示例 ================\n\n";

    //----------- 1. 展示不同哈希函数的输出（对应 8.4.2） -----------
    {
        std::cout << "[1] 哈希函数示例（平方取中 / 除留余数 / 伪随机）\n";
        int keys[] = {19, 1, 23, 14, 55, 68, 11, 82, 36};
        size_t n = sizeof(keys) / sizeof(keys[0]);
        size_t tableSize = 11;

        std::cout << "表长 m = " << tableSize << "\n";
        for (size_t i = 0; i < n; ++i)
        {
            int k = keys[i];
            std::cout << "key = " << k
                      << ", square-middle = " << hashSquareMiddle(k, tableSize)
                      << ", mod-prime = " << hashModPrime(k, tableSize)
                      << ", pseudo-rand = " << hashPseudoRandom(k, tableSize)
                      << "\n";
        }
        std::cout << "\n";
    }

    //----------- 2. 开放定址法示例（线性探测，对应 8.4.3 第 78 页示例） -----------
    {
        std::cout << "[2] 开放定址哈希表示例（线性探测）\n";

        int keys[] = {19, 1, 23, 14, 55, 68, 11, 82, 36};
        size_t n = sizeof(keys) / sizeof(keys[0]);

        OpenAddressHashTable ht(11, OpenAddressHashTable::ProbeType::LINEAR);
        for (size_t i = 0; i < n; ++i)
        {
            ht.insert(keys[i]);
        }

        ht.debugPrint();

        int searchKey = 55;
        std::cout << "查找 " << searchKey << "："
                  << (ht.find(searchKey) ? "存在\n" : "不存在\n");

        searchKey = 100;
        std::cout << "查找 " << searchKey << "："
                  << (ht.find(searchKey) ? "存在\n" : "不存在\n");

        std::cout << "\n";
    }

    //----------- 3. 链地址法示例（对应 8.4.3 第 80 页示意图） -----------
    {
        std::cout << "[3] 链地址哈希表示例\n";
        int keys[] = {19, 1, 23, 14, 55, 68, 11, 82, 36};
        size_t n = sizeof(keys) / sizeof(keys[0]);

        ChainedHashTable cht(7);
        for (size_t i = 0; i < n; ++i)
            cht.insert(keys[i]);

        cht.debugPrint();

        int searchKey = 55;
        std::cout << "查找 " << searchKey << "："
                  << (cht.find(searchKey) ? "存在\n" : "不存在\n");

        searchKey = 100;
        std::cout << "查找 " << searchKey << "："
                  << (cht.find(searchKey) ? "存在\n" : "不存在\n");
    }

    std::cout << "\n提示：\n"
              << "  - 哈希表查找的平均时间复杂度期望为 O(1)，对应课件第 63–65 页。\n"
              << "  - 良好的哈希函数 + 合理的装载因子 + 适当的冲突处理策略，是高效哈希表的关键。\n"
              << "  - 开放定址法适合装载因子较小的情况；链地址法更适合大对象、大数据量场景。\n";

    return 0;
}
