#pragma once

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QElapsedTimer>
#include <QTimer>  // 添加 QTimer 支持
#include <vector>
#include "src/splay_tree.h"

class TreeWidget : public QWidget {
    Q_OBJECT
public:
    explicit TreeWidget(QWidget *parent = nullptr);

    void setTree(SplayTree<int>* tree);  // 移除实现，只保留声明
    void setTrees(std::vector<SplayTree<int>*> trees);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    SplayTree<int>* m_tree = nullptr;
    std::vector<SplayTree<int>*> m_trees;
    QElapsedTimer m_lastRotation;  // 改用 QElapsedTimer
    int calculateTreeDepth(typename SplayTree<int>::node* node);
    void drawNode(QPainter& painter,
                  typename SplayTree<int>::node* node,
                  int x, int y, int level);
};