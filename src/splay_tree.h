#pragma once

#include <functional>
#include <memory>
#include <set>
#include <vector>

template<typename T, typename Comp = std::less<T>>
class SplayTree {
public:
    struct node {
        node *left = nullptr;
        node *right = nullptr;
        node *parent = nullptr;// 为了方便找到父节点，因为旋转后父节点向下指的指针会变
        T key;
        size_t ref_count = 0;
        
        explicit node(const T& k) : key(k) {}
    };

private:
    // 静态成员声明
    static const size_t MAX_NODES = 100;
    static std::set<node*> node_pool;
    static size_t total_allocations;

    // 节点内存管理
    static node* allocate_node(const T& key) {
        if (node_pool.size() >= MAX_NODES) {
            cleanup_unused();
        }
        if (node_pool.size() >= MAX_NODES) return nullptr;
        
        auto* new_node = new node(key);
        node_pool.insert(new_node);
        total_allocations++;
        return new_node;
    }
    
    static void deallocate_node(node* n) {
        if (!n) return;
        auto it = node_pool.find(n);
        if (it != node_pool.end()) {
            node_pool.erase(it);
            delete n;
        }
    }

public:
    static void cleanup_unused() {
        std::vector<node*> to_remove;
        for (auto* n : node_pool) {
            if (n->ref_count == 0) {
                to_remove.push_back(n);
            }
        }
        for (auto* n : to_remove) {
            deallocate_node(n);
        }
    }

public:
    // 基本属性
    Comp comp;
    unsigned long p_size;// 节点数量
    node* root;

    // 构造和析构函数
    SplayTree() : p_size(0), root(nullptr) {}
    
    // 复制构造
    SplayTree(const SplayTree& other) : p_size(0), root(nullptr) {
        if (other.root) {
            root = copy_tree(other.root);
            p_size = other.p_size;
        }
    }
    
    // 移动构造
    SplayTree(SplayTree&& other) noexcept 
        : root(other.root), p_size(other.p_size) {
        other.root = nullptr;
        other.p_size = 0;
    }
    
    // 复制赋值
    SplayTree& operator=(const SplayTree& other) {
        if (this != &other) {
            clear(root);
            root = nullptr;
            p_size = 0;
            
            if (other.root) {
                root = copy_tree(other.root);
                p_size = other.p_size;
            }
        }
        return *this;
    }

    // 移动赋值
    SplayTree& operator=(SplayTree&& other) noexcept {
        if (this != &other) {
            clear(root);
            root = other.root;
            p_size = other.p_size;
            other.root = nullptr;
            other.p_size = 0;
        }
        return *this;
    }

    // 析构函数
    ~SplayTree() { 
        if (root) clear(root);
        root = nullptr;
        p_size = 0;
    }

    
    /**
     * 插入操作 - 时间复杂度: 平摊 O(log n)，树高
     * 最坏情况: O(n)，当树完全不平衡时
     * 平摊分析: 通过伸展操作，频繁访问的节点会被移动到靠近根部的位置，从而优化后续访问
     */
    void insert(const T &key) {
        if (!root) {// 树为空
            root = allocate_node(key);
            if (!root) return;  // 内存池已满
            p_size++;
            return;
        }

        node *z = root;
        node *p = nullptr;
        
        while (z) {
            p = z;// p是父节点
            if (comp(key, z->key))// key < z->key
                z = z->left;
            else if (comp(z->key, key))// key > z->key
                z = z->right;
            else {
                splay(z);  // 如果找到相同的键，将其旋转到根
                return;
            }
        }
        
        z = allocate_node(key);// 分配一个内存，并创建一个对象
        if (!z) return;  // 内存池已满
        z->parent = p;
        
        if (comp(key, p->key))
            p->left = z;
        else
            p->right = z;
        
        splay(z);// 把插入后的节点旋上去
        p_size++;
    }

    /**
     * 查找操作 - 时间复杂度: 平摊 O(log n) 树高
     * 最坏情况: O(n)，当树完全不平衡时
     * 查找后会进行伸展操作，将查找的节点或最后访问的节点移到根部
     * 这种自调整特性使得频繁访问的元素查找效率更高
     */
    node* find(const T &key) {
        if (!root) return nullptr;
        
        node* last_accessed = root;
        node* current = root;
        
        while (current) {
            last_accessed = current;
            if (comp(current->key, key)) { // current -> key < key
                current = current->right;
            } else if (comp(key, current->key)) {// current -> key > key
                current = current->left;
            } else {
                splay(current);
                return current;
            }
        }
        
        // 将最后访问的节点伸展到根
        splay(last_accessed);
        return nullptr;
    }

    /**
     * 删除操作 - 时间复杂度: 平摊 O(log n)
     * 最坏情况: O(n)，当树完全不平衡时
     * 1. 先通过find操作将目标节点伸展到根部 - O(log n)
     * 2. 分裂为左右子树 - O(1)
     * 3. 合并左右子树 - O(log n)
     */
    void erase(const T &key) {
        // 1. 查找目标节点并伸展到根
        node* target = find(key);
        if (!target) return;  // 节点不存在，find已经将最后访问节点伸展到根
        
        // 2. 分裂为左右子树
        node* left_tree = target->left;
        node* right_tree = target->right;
        
        // 断开父子关系
        if (left_tree) left_tree->parent = nullptr;
        if (right_tree) right_tree->parent = nullptr;
        
        // 3. 合并左右子树
        if (!left_tree) {
            // 如果没有左子树，直接使用右子树
            root = right_tree;
        } else {
            // 找到左子树的最大节点并伸展到根
            node* max_node = left_tree;
            while (max_node->right) {
                max_node = max_node->right;
            }
            splay(max_node);
            
            // 现在max_node是左子树的根，且没有右子树
            max_node->right = right_tree;
            if (right_tree) right_tree->parent = max_node;
            
            root = max_node;
        }
        
        // 删除目标节点
        deallocate_node(target);
        p_size--;
    }

    /**
     * 左旋转操作 - 时间复杂度: O(1)
     * 执行常数时间的指针修改
     */
    void left_rotate(node *x) { //把这个节点左旋下去
        node *y = x->right;
        x->right = y->left;

        if( y->left ) y->left->parent = x;

        y->parent = x->parent;
        if( x->parent ) {
            if( x == x->parent->left ) x->parent->left = y;
            else x->parent->right = y;
        }

        y->left = x;
        x->parent = y;
    }
    /**
     * 右旋转操作 - 时间复杂度: O(1)
     * 执行常数时间的指针修改
     */
    void right_rotate(node *x) { // 把这个节点右旋下去
        node *y = x->left;
        x->left = y->right;

        if( y->right ) y->right->parent = x;

        y->parent = x->parent;
        if( x->parent ) {
            if( x == x->parent->left ) x->parent->left = y;
            else x->parent->right = y;
        }

        y->right = x;
        x->parent = y; 
    }

    /**
     * 伸展操作 - 时间复杂度: 平摊 O(log n)
     * 最坏情况: O(n)，当树完全不平衡时
     * 平摊分析: 对于n次连续操作，总时间复杂度为O(n log n)，平均每次O(log n)
     */
    void splay(node *x) {
        if (!x) return;
        
        while (x->parent) {
            node *p = x->parent;
            node *g = p->parent;
            
            if (!g) {  // Zig
                if (p->left == x)
                    right_rotate(p);
                else   // zag
                    left_rotate(p);
            }
            else if (g->left == p && p->left == x) {  // Zig-Zig，左子树的左子树
                right_rotate(g);
                right_rotate(p);
            }
            else if (g->right == p && p->right == x) {  // Zig-Zig，右子树的右子树
                left_rotate(g);
                left_rotate(p);
            }
            else if (g->left == p && p->right == x) {  // Zig-Zag， 左子树的右子树
                left_rotate(p);
                right_rotate(g);
            }
            else {  // Zig-Zag，右子树的左子树
                right_rotate(p);
                left_rotate(g);
            }
        }
        root = x;  // 每次旋转后，x都会变成根节点
    }

    // 辅助函数
    void replace(node *u, node *v) { 
        if( !u->parent ) root = v;
        else if( u == u->parent->left ) u->parent->left = v;
        else u->parent->right = v;
        if( v ) v->parent = u->parent;
    }
    node* subtree_minimum(node *u) { 
        while( u->left ) u = u->left;
        return u;
    }
    node* subtree_maximum(node *u) {
        while( u->right ) u = u->right;
        return u;
    }
    
    // 新增内存清理
    void clear(node *x) {
        if (x) {
            clear(x->left);
            clear(x->right);
            deallocate_node(x);
        }
    }

public:
    // 拆分
    /**
     * 树的拆分操作 - 时间复杂度: O(log n)
     * 1. 查找并伸展操作 - O(log n)
     * 2. 分割为两棵树 - O(1)
     * 
     * 将树拆分为两棵子树，使得左子树中所有键值小于等于key，
     * 右子树中所有键值大于key
     */
    std::pair<SplayTree*, SplayTree*> split(const T& key) {
        if (!root) return {new SplayTree(), new SplayTree()};

        // 1. 先将最接近key的节点旋转到根
        find(key);  

        // 创建两棵新树
        SplayTree* left = new SplayTree();
        SplayTree* right = new SplayTree();

        // 确保拆分值在左子树
        if (!comp(key, root->key)) {  // 如果 key >= root->key
            // 根节点及其左子树归入左子树
            left->root = root;
            if (root->right) {
                right->root = root->right;
                root->right->parent = nullptr;
                root->right = nullptr;
            }
        } else {  // 如果 key < root->key
            // 右子树和根节点归入右子树
            right->root = root;
            if (root->left) {
                left->root = root->left;
                root->left->parent = nullptr;
                root->left = nullptr;
            }
        }

        // 更新两棵子树的大小
        if (left->root) left->p_size = countNodes(left->root);
        if (right->root) right->p_size = countNodes(right->root);

        // 清空原树
        root = nullptr;
        p_size = 0;

        return {left, right};
    }

    /**
     * 树的合并操作 - 时间复杂度: O(log n)
     * 1. 查找最大/最小节点 - O(log n)
     * 2. 伸展操作 - O(log n)
     * 3. 树的连接 - O(1)
     * 
     * 合并两棵树的前提是t1中的所有键值必须小于t2中的所有键值
     */
    static SplayTree* merge(SplayTree* t1, SplayTree* t2) {
        // 空树快速处理
        if (!t1 || !t1->root) {
            auto* result = new SplayTree();
            if (t2 && t2->root) {
                result->root = t2->root;
                result->p_size = t2->p_size;
                t2->root = nullptr;
                t2->p_size = 0;
            }
            delete t1;
            delete t2;
            return result;
        }
        if (!t2 || !t2->root) {
            auto* result = new SplayTree();
            result->root = t1->root;
            result->p_size = t1->p_size;
            t1->root = nullptr;
            t1->p_size = 0;
            delete t1;
            delete t2;
            return result;
        }

        // 验证合并条件
        node* max_node = t1->subtree_maximum(t1->root);
        node* min_node = t2->subtree_minimum(t2->root);
        if (!(t1->comp(max_node->key, min_node->key))) {// 左边最大的比右边的最小的大
            delete t1;
            delete t2;
            return nullptr;
        }

        // 合并过程
        auto* result = new SplayTree();
        t1->splay(max_node);  // 将最大节点旋转到根
        
        // 直接连接两棵树
        t1->root->right = t2->root;
        if (t2->root) {
            t2->root->parent = t1->root;
        }
        
        result->root = t1->root;
        result->p_size = t1->p_size + t2->p_size;

        // 重置原树
        t1->root = t2->root = nullptr;
        t1->p_size = t2->p_size = 0;
        
        // 批量更新引用计数
        if (result->root) {
            result->batch_update_ref_counts(result->root);
        }

        delete t1;
        delete t2;
        return result;
    }

private:
    // 更新引用计数
    void update_ref_counts(node* n) {
        if (!n) return;
        n->ref_count++;
        update_ref_counts(n->left);
        update_ref_counts(n->right);
    }

    // 减少引用计数
    void decrease_ref_counts(node* n) {
        if (!n) return;
        n->ref_count--;
        decrease_ref_counts(n->left);
        decrease_ref_counts(n->right);
    }

    // 批量更新引用计数
    void batch_update_ref_counts(node* n) {
        if (!n) return;
        
        std::vector<node*> nodes;
        collect_nodes(n, nodes);
        
        for (node* node : nodes) {
            node->ref_count++;
        }
    }

    void collect_nodes(node* n, std::vector<node*>& nodes) {
        if (!n) return;
        nodes.push_back(n);
        collect_nodes(n->left, nodes);
        collect_nodes(n->right, nodes);
    }

    // 添加树复制辅助函数
    node* copy_tree(node* src) {
        if (!src) return nullptr;
        
        node* new_node = allocate_node(src->key);
        if (!new_node) return nullptr;
        
        new_node->ref_count = src->ref_count;
        
        if (src->left) {
            new_node->left = copy_tree(src->left);
            if (new_node->left) {
                new_node->left->parent = new_node;
            }
        }
        
        if (src->right) {
            new_node->right = copy_tree(src->right);
            if (new_node->right) {
                new_node->right->parent = new_node;
            }
        }
        
        return new_node;
    }

    // 辅助函数
    unsigned long countNodes(node* x) const {
        if (!x) return 0;
        return 1 + countNodes(x->left) + countNodes(x->right);
    }

    void erase_impl(const T &key) {
        node *z = find(key);
        if (!z) return;

        splay(z);

        if (!z->left) replace(z, z->right);
        else if (!z->right) replace(z, z->left);
        else {
            node *y = subtree_minimum(z->right);
            if (y->parent != z) {
                replace(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            replace(z, y);
            y->left = z->left;
            y->left->parent = y;
        }
        deallocate_node(z); // 修复内存泄漏
        p_size--;
    }

    const T& minimum( ) { return subtree_minimum( root )->key; }
    const T& maximum( ) { return subtree_maximum( root )->key; }
    
    bool empty( ) const { return root == 0; }
public: unsigned long size( ) const { return p_size; }

public:
    // 垃圾回收
    static void cleanup() {
        for (auto* n : node_pool) {
            delete n;
        }
        node_pool.clear();
    }

    // 容量检查
    bool is_full() const {
        return node_pool.size() >= MAX_NODES;
    }

    // 添加静态方法访问器
    static size_t get_current_nodes() { return node_pool.size(); }
    static size_t get_total_allocations() { return total_allocations; }
};

// 静态成员定义
template<typename T, typename Comp>
std::set<typename SplayTree<T, Comp>::node*> SplayTree<T, Comp>::node_pool;

template<typename T, typename Comp>
size_t SplayTree<T, Comp>::total_allocations = 0;

template<typename T, typename Comp>
const size_t SplayTree<T, Comp>::MAX_NODES;