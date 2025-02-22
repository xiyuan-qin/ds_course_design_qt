#pragma once

#include <QWidget>
#include <QPainter>
#include "src/splay_tree.h"

class TreeWidget : public QWidget {
    Q_OBJECT
public:
    // 添加显式构造函数
    explicit TreeWidget(QWidget *parent = nullptr);

    // 添加树指针设置方法
    void setTree(SplayTree<int>* tree);

protected:
    // 添加重写声明
    void paintEvent(QPaintEvent* event) override;

private:
    int m_treeVersion = 0; // 添加版本标记
    SplayTree<int>* m_tree = nullptr;
    QPoint m_globalOffset;
    int calculateTreeDepth(typename SplayTree<int>::node* node);
    void drawNode(QPainter& painter,
                  typename SplayTree<int>::node* node,
                  int x, int y, int level);
};


// void TreeWidget::setTree(SplayTree<int>* tree) {
//     m_tree = tree;
//     m_treeVersion++;  // 每次设置递增版本
//     update();
// }
