#pragma once

#include <QWidget>
#include <QPainter>
#include <vector>
#include "src/splay_tree.h"

class TreeWidget : public QWidget {
    Q_OBJECT
public:
    explicit TreeWidget(QWidget *parent = nullptr);

    void setTree(SplayTree<int>* tree);
    void setTrees(std::vector<SplayTree<int>*> trees); // 添加多树支持

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::vector<SplayTree<int>*> m_trees;  // 改为树的集合
    int calculateTreeDepth(typename SplayTree<int>::node* node);
    void drawNode(QPainter& painter,
                  typename SplayTree<int>::node* node,
                  int x, int y, int level);
};