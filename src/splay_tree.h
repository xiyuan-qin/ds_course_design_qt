#pragma once

#include <functional>
#include <vector>
#include <algorithm>

template<typename T, typename Comp = std::less<T>>
class SplayTree {
public:
    struct node {
        node *left, *right;
        node *parent;
        T key;
        int x = 0, y = 0;  // 节点绘制坐标
        node(const T& init = T()) : 
            left(nullptr), right(nullptr), parent(nullptr), key(init) {}
    };

    Comp comp;
    unsigned long p_size;
    node *root;

    SplayTree() : root(nullptr), p_size(0) {}
    ~SplayTree() { 
        clear(root); 
        root = nullptr;
        p_size = 0;
    }  // 析构函数释放内存

    // 核心接口
    void insert(const T &key) {
        if (!root) {
            root = allocate_node(key);
            if (!root) return;  // 内存池已满
            p_size++;
            return;
        }

        node *z = root;
        node *p = nullptr;
        
        while (z) {
            p = z;
            if (comp(key, z->key))
                z = z->left;
            else if (comp(z->key, key))
                z = z->right;
            else {
                splay(z);  // 如果找到相同的键，将其旋转到根
                return;
            }
        }
        
        z = allocate_node(key);
        if (!z) return;  // 内存池已满
        z->parent = p;
        
        if (comp(key, p->key))
            p->left = z;
        else
            p->right = z;
        
        splay(z);
        p_size++;
    }

    node* find(const T &key) {
        node *z = root;
        while(z) {
            if(comp(z->key, key)) z = z->right;
            else if(comp(key, z->key)) z = z->left;
            else {
                splay(z);  // 将找到的节点旋转到根
                root = z;  // 确保root指针更新
                return z;
            }
        }
        
        // 如果没找到，将最后访问的节点旋转到根
        if (root) {
            splay(root);
        }
        return nullptr;
    }

    void erase(const T &key) {
        node *z = find( key );
        if( !z ) return;
        
        splay( z );
        
        if( !z->left ) replace( z, z->right );
        else if( !z->right ) replace( z, z->left );
        else {
          node *y = subtree_minimum( z->right );
          if( y->parent != z ) {
            replace( y, y->right );
            y->right = z->right;
            y->right->parent = y;
          }
          replace( z, y );
          y->left = z->left;
          y->left->parent = y;
        }
        
        deallocate_node(z);
        p_size--;
    }

    // 树操作
    void left_rotate(node *x) { 
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
    void right_rotate(node *x) { 
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
    void splay(node *x) {
        if (!x) return;
        
        while (x->parent) {
            node *p = x->parent;
            node *g = p->parent;
            
            if (!g) {  // Zig
                if (p->left == x)
                    right_rotate(p);
                else
                    left_rotate(p);
            }
            else if (g->left == p && p->left == x) {  // Zig-Zig
                right_rotate(g);
                right_rotate(p);
            }
            else if (g->right == p && p->right == x) {  // Zig-Zig
                left_rotate(g);
                left_rotate(p);
            }
            else if (g->left == p && p->right == x) {  // Zig-Zag
                left_rotate(p);
                right_rotate(g);
            }
            else {  // Zig-Zag
                right_rotate(p);
                left_rotate(g);
            }
        }
        root = x;  // 确保根节点更新
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
    // 拆分操作：将树拆分为两棵子树，一棵包含小于等于key的节点，另一棵包含大于key的节点
    std::pair<SplayTree*, SplayTree*> split(const T& key) {
        if (!root) return {new SplayTree(), new SplayTree()};

        // 找到拆分点并旋转到根
        find(key);  // 这会将最接近key的节点旋转到根
        
        SplayTree* left = new SplayTree();
        SplayTree* right = new SplayTree();
        
        if (!root || comp(root->key, key)) {
            // 当前根节点及其左子树属于左树
            left->root = root;
            if (root && root->right) {
                right->root = root->right;
                root->right->parent = nullptr;
                root->right = nullptr;
                // 更新大小
                right->p_size = countNodes(right->root);
                left->p_size = p_size - right->p_size;
            } else {
                left->p_size = p_size;
            }
        } else {
            // 当前根节点及其右子树属于右树
            right->root = root;
            if (root->left) {
                left->root = root->left;
                root->left->parent = nullptr;
                root->left = nullptr;
                // 更新大小
                left->p_size = countNodes(left->root);
                right->p_size = p_size - left->p_size;
            } else {
                right->p_size = p_size;
            }
        }
        
        root = nullptr;
        p_size = 0;
        return {left, right};
    }

    // 修改合并操作
    static SplayTree* merge(SplayTree* t1, SplayTree* t2) {
        if (!t1 || !t1->root) {
            if (t2) {
                auto* result = new SplayTree();
                result->root = t2->root;
                result->p_size = t2->p_size;
                t2->root = nullptr;
                t2->p_size = 0;
                delete t1;
                delete t2;
                return result;
            }
            delete t1;
            delete t2;
            return nullptr;
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

        // 检查合并条件：左树最大值必须小于右树最小值
        auto max_left = t1->subtree_maximum(t1->root);
        auto min_right = t2->subtree_minimum(t2->root);
        if (max_left->key >= min_right->key) {
            delete t1;
            delete t2;
            return nullptr;
        }

        // 创建新树并执行合并
        auto* result = new SplayTree();
        
        // 将左树最大节点旋转到根
        t1->splay(max_left);
        
        // 合并两棵树
        result->root = t1->root;
        result->root->right = t2->root;
        if (t2->root) {
            t2->root->parent = result->root;
        }
        result->p_size = t1->p_size + t2->p_size;

        // 清理原树
        t1->root = nullptr;
        t2->root = nullptr;
        t1->p_size = t2->p_size = 0;
        delete t1;
        delete t2;

        return result;
    }

private:
    // 计算节点数的辅助函数
    unsigned long countNodes(node* x) const {
        if (!x) return 0;
        unsigned long count = 1;
        if (x->left) count += countNodes(x->left);
        if (x->right) count += countNodes(x->right);
        return count;
    }

private: // 以下保持原实现，但根据需要改为public
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

private:
    // 添加内存池管理
    static const size_t MAX_NODES = 1000;  // 限制最大节点数
    static std::vector<node*> node_pool;   // 节点池
    
    node* allocate_node(const T& key) {
        if (node_pool.size() >= MAX_NODES) return nullptr;
        node* new_node = new node(key);
        node_pool.push_back(new_node);
        return new_node;
    }
    
    void deallocate_node(node* n) {
        if (!n) return;
        auto it = std::find(node_pool.begin(), node_pool.end(), n);
        if (it != node_pool.end()) {
            node_pool.erase(it);
            delete n;
        }
    }

public:
    // 添加垃圾回收机制
    static void cleanup() {
        for (auto* n : node_pool) {
            delete n;
        }
        node_pool.clear();
    }

    // 添加容量检查
    bool is_full() const {
        return node_pool.size() >= MAX_NODES;
    }
};

template<typename T, typename Comp>
std::vector<typename SplayTree<T, Comp>::node*> SplayTree<T, Comp>::node_pool;
