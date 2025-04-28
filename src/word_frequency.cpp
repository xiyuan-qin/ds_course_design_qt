#include <iostream>
#include <fstream>
#include <unordered_map>
#include <chrono>
#include <cctype>
#include <algorithm>
#include <iomanip>
#include <numeric>  
#include <cmath>   
using namespace std;
using namespace std::chrono;

/**
 * 伸展树词频统计与性能测试程序
 * 
 * 目的：
 * 1. 使用伸展树(Splay Tree)统计文本文件中单词出现的频率
 * 2. 与普通二叉搜索树(BST)进行性能对比，验证伸展树在访问热点数据时的优势
 * 3. 收集并分析树结构特性（平均深度、旋转次数等）
 * 
 * 主要测试内容：
 * - 构建树的时间性能比较
 * - 重复访问高频词的性能比较
 * - 树结构特性（平均深度）的比较
 */

// 增加缓冲区大小，提高文件读取效率
const int BUFFER_SIZE = 8192;

/**
 * 伸展树节点定义
 * 包含键值、计数、左右子节点和父节点指针
 */
struct Node {
    string key;        // 单词
    int count = 1;     // 出现次数
    Node* left = nullptr;    // 左子节点
    Node* right = nullptr;   // 右子节点
    Node* parent = nullptr;  // 父节点（用于伸展操作）

    Node(const string& k) : key(k) {}
};

class SplayTree {
private:
    Node* root = nullptr;
    int operationCount = 0;    // 记录旋转操作次数，用于性能分析
    int totalOperations = 0;   // 用于控制伸展频率
    static const int SPLAY_THRESHOLD = 100;  // 伸展阈值，每100次操作才进行一次伸展，减少开销

    /**
     * 条件伸展操作 - 优化策略
     * 不是每次访问都伸展，而是每SPLAY_THRESHOLD次操作才伸展一次
     * 这样可以减少频繁旋转带来的开销，同时保持伸展树的自调整特性
     * 
     * @param x 需要考虑伸展的节点
     */
    void conditionalSplay(Node* x) {
        totalOperations++;
        if (totalOperations % SPLAY_THRESHOLD == 0) {
            splay(x);
        }
    }

    /**
     * 伸展操作 - 将节点x旋转到根部位置
     * 通过一系列旋转，使得被访问的节点成为树的根节点
     * 时间复杂度: 平摊O(log n)
     * 
     * @param x 需要伸展到根部的节点
     */
    void splay(Node* x) {
        if (!x) return;

        while (x->parent) {
            Node* p = x->parent;
            Node* g = p->parent;

            if (!g) {
                singleRotate(x);
            } else {
                doubleRotate(x);
            }
        }
        root = x;
    }

    /**
     * 单旋转优化 - 处理父节点是根的情况
     * 对应Zig或Zag操作
     * 时间复杂度: O(1)
     * 
     * @param x 需要旋转的节点
     */
    void singleRotate(Node* x) {
        Node* p = x->parent;
        if (x == p->left) {
            rotateRight(p);
        } else {
            rotateLeft(p);
        }
    }

    /**
     * 双旋转优化 - 处理父节点不是根的情况
     * 对应Zig-Zig、Zig-Zag、Zag-Zig、Zag-Zag四种情况
     * 时间复杂度: O(1)
     * 
     * @param x 需要旋转的节点
     */
    void doubleRotate(Node* x) {
        Node* p = x->parent;
        Node* g = p->parent;
        
        if (x == p->left && p == g->left) {
            rotateRight(g);
            rotateRight(p);
        } else if (x == p->right && p == g->right) {
            rotateLeft(g);
            rotateLeft(p);
        } else if (x == p->right && p == g->left) {
            rotateLeft(p);
            rotateRight(g);
        } else {
            rotateRight(p);
            rotateLeft(g);
        }
    }

    /**
     * 左旋转操作 - 时间复杂度: O(1)
     * 将节点x的右子节点y提升为其位置，x变为y的左子节点
     * 
     *    x          y
     *   / \        / \
     *  a   y  =>  x   c
     *     / \    / \
     *    b   c  a   b
     * 
     * @param x 需要左旋的节点
     */
    void rotateLeft(Node* x) {
        Node* y = x->right;
        operationCount++;
        
        x->right = y->left;
        if (y->left) y->left->parent = x;
        
        y->parent = x->parent;
        if (!x->parent) root = y;
        else if (x == x->parent->left) x->parent->left = y;
        else x->parent->right = y;
        
        y->left = x;
        x->parent = y;
    }

    /**
     * 右旋转操作 - 时间复杂度: O(1)
     * 将节点x的左子节点y提升为其位置，x变为y的右子节点
     * 
     *    x          y
     *   / \        / \
     *  y   c  =>  a   x
     * / \            / \
     *a   b          b   c
     * 
     * @param x 需要右旋的节点
     */
    void rotateRight(Node* x) {
        Node* y = x->left;
        operationCount++;
        
        x->left = y->right;
        if (y->right) y->right->parent = x;
        
        y->parent = x->parent;
        if (!x->parent) root = y;
        else if (x == x->parent->right) x->parent->right = y;
        else x->parent->left = y;
        
        y->right = x;
        x->parent = y;
    }

    // 添加计算平均深度的方法
    int totalDepth = 0;
    int nodeCount = 0;
    int compareCount = 0;  // 添加比较次数计数器
    int accessCount = 0;   // 添加访问计数器
    
    void calculateDepth(Node* node, int depth) {
        if (!node) return;
        totalDepth += depth;
        nodeCount++;
        calculateDepth(node->left, depth + 1);
        calculateDepth(node->right, depth + 1);
    }

    void deleteTree(Node* node) {
        if (node) {
            deleteTree(node->left);
            deleteTree(node->right);
            delete node;
        }
    }

public:
    // 插入并伸展
    void insert(const string& key) {
        accessCount++;
        Node* current = root;
        Node* parent = nullptr;

        // 查找插入位置
        while (current) {
            compareCount++;
            parent = current;
            if (key < current->key) {
                current = current->left;
            } else if (key > current->key) {
                current = current->right;
            } else {
                current->count++;
                splay(current);
                return;
            }
        }

        // 新建节点
        Node* newNode = new Node(key);
        newNode->parent = parent;

        if (!parent) root = newNode;
        else if (key < parent->key) parent->left = newNode;
        else parent->right = newNode;

        splay(newNode);
    }

    // 批量插入方法
    void batchInsert(const vector<string>& words) {
        if (words.empty()) return;
        
        for (const auto& word : words) {
            if (!word.empty()) {
                insertWithoutSplay(word);
            }
        }

        // 最后进行一次平衡，添加空指针检查
        if (root && getSize(root) > 0) {
            Node* mid = findMid(root);
            if (mid) {
                splay(mid);
            }
        }
    }

    // 不立即伸展的插入
    void insertWithoutSplay(const string& key) {
        Node* current = root;
        Node* parent = nullptr;

        while (current) {
            parent = current;
            if (key < current->key) {
                current = current->left;
            } else if (key > current->key) {
                current = current->right;
            } else {
                current->count++;
                conditionalSplay(current);
                return;
            }
        }

        Node* newNode = new Node(key);
        newNode->parent = parent;

        if (!parent) {
            root = newNode;
        } else if (key < parent->key) {
            parent->left = newNode;
        } else {
            parent->right = newNode;
        }

        conditionalSplay(newNode);
    }

    // 找到中间节点用于平衡
    Node* findMid(Node* node) {
        if (!node) return nullptr;
        
        int size = getSize(node);
        int target = size / 2;
        
        return findKthNode(node, target);
    }

    // 获取子树大小
    int getSize(Node* node) {
        if (!node) return 0;
        return 1 + getSize(node->left) + getSize(node->right);
    }

    // 找到第k个节点
    Node* findKthNode(Node* node, int k) {
        if (!node) return nullptr;
        
        int leftSize = getSize(node->left);
        
        if (k == leftSize) return node;
        if (k < leftSize) return findKthNode(node->left, k);
        return findKthNode(node->right, k - leftSize - 1);
    }

    // 修改遍历方法，收集词频信息
    void traverse(Node* node, vector<pair<string, int>>& freq) {
        if (!node) return;
        if (node->left) traverse(node->left, freq);
        freq.push_back({node->key, node->count});
        if (node->right) traverse(node->right, freq);
    }

    // 性能指标获取
    int getOperations() const { return operationCount; }
    Node* getRoot() { return root; }

    // 将getAverageDepth移动到public部分
    double getAverageDepth() {
        totalDepth = 0;
        nodeCount = 0;
        calculateDepth(root, 0);
        return nodeCount > 0 ? (double)totalDepth / nodeCount : 0;
    }

    void resetCounters() {
        operationCount = 0;
        compareCount = 0;
        accessCount = 0;
    }

    int getCompareCount() const { return compareCount; }
    int getAccessCount() const { return accessCount; }

    // 将析构函数移到public部分
    ~SplayTree() {
        deleteTree(root);
    }
};

// 二叉树
class BST {
private:
    struct BSTNode {
        string key;
        int count = 1;
        BSTNode* left = nullptr;
        BSTNode* right = nullptr;
        
        BSTNode(const string& k) : key(k) {}
    };
    
    BSTNode* root = nullptr;
    int totalDepth = 0;
    int nodeCount = 0;
    int accessCount = 0;  // 添加访问计数器
    int compareCount = 0; // 添加比较次数计数器

    void deleteTree(BSTNode* node) {
        if (node) {
            deleteTree(node->left);
            deleteTree(node->right);
            delete node;
        }
    }

public:
    void insert(const string& key) {
        root = insertRec(root, key);
    }
    
    double getAverageDepth() {
        totalDepth = 0;
        nodeCount = 0;
        calculateDepth(root, 0);
        return nodeCount > 0 ? (double)totalDepth / nodeCount : 0;
    }
    
    void traverse(BSTNode* node, vector<pair<string, int>>& freq) {
        if (!node) return;
        traverse(node->left, freq);
        freq.push_back({node->key, node->count});
        traverse(node->right, freq);
    }
    
    vector<pair<string, int>> getFrequencies() {
        vector<pair<string, int>> freq;
        traverse(root, freq);
        return freq;
    }

    void resetCounters() {
        accessCount = 0;
        compareCount = 0;
    }
    
    int getAccessCount() const { return accessCount; }
    int getCompareCount() const { return compareCount; }

    BSTNode* search(const string& key) {
        accessCount++;
        BSTNode* current = root;
        while (current) {
            compareCount++;
            if (key < current->key)
                current = current->left;
            else if (key > current->key)
                current = current->right;
            else
                return current;
        }
        return nullptr;
    }

    // 将析构函数移到public部分
    ~BST() {
        deleteTree(root);
    }

private:
    BSTNode* insertRec(BSTNode* node, const string& key) {
        accessCount++;
        if (!node) return new BSTNode(key);
        
        compareCount++;
        if (key < node->key)
            node->left = insertRec(node->left, key);
        else if (key > node->key)
            node->right = insertRec(node->right, key);
        else
            node->count++;
            
        return node;
    }
    
    void calculateDepth(BSTNode* node, int depth) {
        if (!node) return;
        totalDepth += depth;
        nodeCount++;
        calculateDepth(node->left, depth + 1);
        calculateDepth(node->right, depth + 1);
    }
};

// 文本处理
vector<string> processFile(const string& filename) {
    vector<string> words;
    words.reserve(100000);  // 预分配空间
    
    ifstream file(filename, ios::in | ios::binary);
    if (!file) return words;

    char buffer[BUFFER_SIZE];
    string word;
    word.reserve(50);  // 预分配单词空间

    while (file.read(buffer, BUFFER_SIZE) || file.gcount()) {
        int count = file.gcount();
        
        for (int i = 0; i < count; i++) {
            char c = buffer[i];
            if (isalpha(c)) {
                word += tolower(c);
            } else if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
    }

    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

// 随机单词生成函数
string generateRandomWord(int len) {
    static const char charset[] = "abcdefghijklmnopqrstuvwxyz";
    string word;
    for (int i = 0; i < len; ++i) {
        word += charset[rand() % 26];
    }
    return word;
}

int main() {
    try {
        SplayTree splayTree;
        BST bst;
        
        // 读取测试文件
        auto words = processFile("../data/test.txt");
        if (words.empty()) {
            cout << "Error: 无法读取文件或文件为空" << endl;
            return 1;
        }

        cout << "词频统计与性能测试开始..." << endl;
        cout << "总单词数: " << words.size() << endl;

        // 先构建主树
        splayTree.batchInsert(words);
        for (const auto& word : words) {
            bst.insert(word);
        }

        // 收集词频统计
        vector<pair<string, int>> frequencies;
        frequencies.reserve(words.size()); // 预分配空间
        
        if (splayTree.getRoot()) {
            splayTree.traverse(splayTree.getRoot(), frequencies);
        }
        
        if (frequencies.empty()) {
            cout << "Error: 未能生成词频统计" << endl;
            return 1;
        }

        sort(frequencies.begin(), frequencies.end(),
             [](const pair<string,int>& a, const pair<string,int>& b) { 
                 return a.second > b.second; 
             });

        // 性能测试部分
        vector<long long> splayTimes, bstTimes;
        vector<long long> splayHotTimes, bstHotTimes;
        const int TEST_ITERATIONS = 5;

        // 多次测试插入性能
        for (int iter = 0; iter < TEST_ITERATIONS; iter++) {
            SplayTree testSplay;
            BST testBst;

            // Splay Tree测试
            auto start = high_resolution_clock::now();
            testSplay.batchInsert(words);
            auto end = high_resolution_clock::now();
            splayTimes.push_back(duration_cast<nanoseconds>(end - start).count());

            // BST测试
            start = high_resolution_clock::now();
            for (const auto& word : words) {
                testBst.insert(word);
            }
            end = high_resolution_clock::now();
            bstTimes.push_back(duration_cast<nanoseconds>(end - start).count());
        }

        // 热点词性能测试
        string hotWord = frequencies[0].first;
        const int HOT_TEST_COUNT = 10000;
        
        for (int iter = 0; iter < TEST_ITERATIONS; iter++) {
            SplayTree testSplay;
            BST testBst;
            
            // 先构建树
            testSplay.batchInsert(words);
            for (const auto& word : words) {
                testBst.insert(word);
            }

            // 重置计数器
            testSplay.resetCounters();
            testBst.resetCounters();

            // Splay Tree热点词测试
            auto start = high_resolution_clock::now();
            for(int i = 0; i < HOT_TEST_COUNT; i++) {
                testSplay.insert(hotWord);
            }
            auto end = high_resolution_clock::now();
            splayHotTimes.push_back(duration_cast<nanoseconds>(end - start).count());

            // BST热点词测试
            start = high_resolution_clock::now();
            for(int i = 0; i < HOT_TEST_COUNT; i++) {
                testBst.insert(hotWord);
            }
            end = high_resolution_clock::now();
            bstHotTimes.push_back(duration_cast<nanoseconds>(end - start).count());
        }

        // 计算统计数据
        auto calcStats = [](const vector<long long>& times) -> pair<double, double> {
            if (times.empty()) return {0.0, 0.0};
            double avg = accumulate(times.begin(), times.end(), 0.0) / times.size();
            double variance = 0;
            for (auto t : times) {
                double diff = t - avg;
                variance += diff * diff;
            }
            variance /= times.size();
            return {avg / 1000.0, sqrt(variance) / 1000.0}; // 转换为微秒
        };

        auto splayStats = calcStats(splayTimes);
        auto bstStats = calcStats(bstTimes);
        auto splayHotStats = calcStats(splayHotTimes);
        auto bstHotStats = calcStats(bstHotTimes);

        // 输出结果到文件
        ofstream outFile("../data/result.txt");
        
        // 1. 输出词频统计
        outFile << "=== 词频统计结果 ===" << endl;
        outFile << "总单词数：" << words.size() << endl;
        outFile << "不同单词数：" << frequencies.size() << endl << endl;
        
        outFile << "词频排序（前20个）：" << endl;
        outFile << "单词\t\t出现次数\t占比" << endl;
        outFile << "----------------------------------------" << endl;
        
        int totalWords = 0;
        for (const auto& freq : frequencies) {
            totalWords += freq.second;
        }
        
        for (int i = 0; i < min(20, (int)frequencies.size()); i++) {
            const auto& freq = frequencies[i];
            double percentage = (freq.second * 100.0) / totalWords;
            outFile << freq.first << "\t\t" 
                    << freq.second << "\t\t"
                    << fixed << setprecision(2) << percentage << "%" << endl;
        }

        // 2. 输出性能指标
        outFile << "\n=== 性能测试结果 ===" << endl;
        outFile << "1. 插入性能：" << endl;
        outFile << "   Splay Tree: " << fixed << setprecision(2) 
                << splayStats.first << " ± " << splayStats.second << " 微秒/操作" << endl;
        outFile << "   BST: " << bstStats.first << " ± " << bstStats.second << " 微秒/操作" << endl;
        outFile << "   性能比: " << (bstStats.first / splayStats.first) << endl;

        outFile << "\n2. 热点词访问性能：" << endl;
        outFile << "   测试词: " << hotWord << endl;
        outFile << "   测试次数: " << HOT_TEST_COUNT << endl;
        outFile << "   Splay Tree: " << fixed << setprecision(3) 
                << splayHotStats.first / HOT_TEST_COUNT << " ± " 
                << splayHotStats.second / HOT_TEST_COUNT << " 微秒/操作" << endl;
        outFile << "   BST: " << bstHotStats.first / HOT_TEST_COUNT << " ± " 
                << bstHotStats.second / HOT_TEST_COUNT << " 微秒/操作" << endl;
        outFile << "   访问性能提升: " << fixed << setprecision(2)
                << ((bstHotStats.first / splayHotStats.first) - 1.0) * 100 << "%" << endl;

        outFile << "\n3. 操作计数统计：" << endl;
        outFile << "   总旋转次数: " << splayTree.getOperations() << endl;
        outFile << "   平均深度: " << splayTree.getAverageDepth() << endl;

        // 输出性能对比结果
        outFile << "\n=== 树结构性能对比 ===" << endl;
        outFile << "1. 插入性能对比：" << endl;
        outFile << "   Splay Tree总时间: " << splayStats.first << " 微秒" << endl;
        outFile << "   BST总时间: " << bstStats.first << " 微秒" << endl;
        outFile << "   性能提升: " << fixed << setprecision(2) 
                << ((double)bstStats.first / splayStats.first - 1.0) * 100 
                << "%" << endl;

        outFile << "\n2. 树结构特性对比：" << endl;
        outFile << "   Splay Tree平均深度: " << splayTree.getAverageDepth() << endl;
        outFile << "   BST平均深度: " << bst.getAverageDepth() << endl;
        outFile << "   深度优化程度: " << fixed << setprecision(2)
                << ((bst.getAverageDepth() / splayTree.getAverageDepth()) - 1.0) * 100
                << "%" << endl;

        outFile << "\n3. 热点词访问性能对比：" << endl;
        outFile << "   测试词: " << hotWord << endl;
        outFile << "   测试次数: " << HOT_TEST_COUNT << endl;
        outFile << "   Splay Tree平均访问时间: " 
                << splayHotStats.first / HOT_TEST_COUNT << " 微秒" << endl;
        outFile << "   BST平均访问时间: " 
                << bstHotStats.first / HOT_TEST_COUNT << " 微秒" << endl;
        outFile << "   访问性能提升: " << fixed << setprecision(2)
                << ((double)bstHotStats.first / splayHotStats.first - 1.0) * 100 
                << "%" << endl;

        cout << "\n测试完成！详细结果已保存到 ../data/result.txt" << endl;
        cout << "文件包含：词频统计、性能对比数据、树的特性分析" << endl;

    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}