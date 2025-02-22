#pragma once

#include <functional>

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
    ~SplayTree() { clear(root); }  // 析构函数释放内存

    // 核心接口
    void insert(const T &key) {
        if (!root) {
            root = new node(key);
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
        
        z = new node(key);
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
                splay(z);  // 找到节点后执行伸展操作
                return z;
            }
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
            delete x;
        }
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

        delete z; // 修复内存泄漏
        p_size--;
    }

    const T& minimum( ) { return subtree_minimum( root )->key; }
    const T& maximum( ) { return subtree_maximum( root )->key; }
    
    bool empty( ) const { return root == 0; }
public: unsigned long size( ) const { return p_size; }
};
