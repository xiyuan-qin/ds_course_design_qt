#include "treewidget.h"

TreeWidget::TreeWidget(QWidget *parent) : QWidget(parent) {}

void TreeWidget::setTree(SplayTree<int>* tree) {
    m_trees.clear();
    if (tree) m_trees.push_back(tree);
    update();
}

void TreeWidget::setTrees(std::vector<SplayTree<int>*> trees) {
    m_trees = trees;
    update();
}

void TreeWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 设置背景
    painter.fillRect(rect(), QColor(240, 240, 255));

    if (m_trees.empty() || std::all_of(m_trees.begin(), m_trees.end(), 
        [](auto t) { return !t || !t->root; })) {
        painter.setPen(Qt::gray);
        painter.setFont(QFont("Arial", 14));
        painter.drawText(rect(), Qt::AlignCenter, "空树");
        return;
    }

    int treeCount = m_trees.size();
    int treeWidth = width() / treeCount;
    
    for (int i = 0; i < treeCount; i++) {
        if (!m_trees[i] || !m_trees[i]->root) continue;
        
        painter.save();
        
        // 修改缩放和位置计算
        int maxDepth = calculateTreeDepth(m_trees[i]->root);
        float hScale = treeWidth / (pow(2, maxDepth) * 100.0f);
        float vScale = height() / (maxDepth * 150.0f);
        float scale = qMin(qMin(hScale, vScale), 1.0f);
        
        // 移动原点到每棵树的顶部中心
        painter.translate(i * treeWidth + treeWidth/2, 50);
        painter.scale(scale, scale);
        
        // 从根节点开始绘制，使用相对坐标
        drawNode(painter, m_trees[i]->root, -30, 0, 0);
        
        painter.restore();
    }
}

int TreeWidget::calculateTreeDepth(typename SplayTree<int>::node* node) {
    if (!node) return 0;
    return 1 + qMax(calculateTreeDepth(node->left),
                    calculateTreeDepth(node->right));
}

void TreeWidget::drawNode(QPainter& painter,
                          typename SplayTree<int>::node* node,
                          int relX, int relY, int level)
{
    if (!node) return;
    
    static const int NODE_SIZE = 60;
    static const int VERTICAL_SPACING = 100;
    
    // 水平间距随层级增加而减小
    int horizontalSpacing = NODE_SIZE * 6 / (level + 2);

    // 先绘制连线
    if (node->left) {
        int childX = relX - horizontalSpacing;
        int childY = relY + VERTICAL_SPACING;
        painter.setPen(QPen(Qt::black, 2));
        painter.drawLine(relX + NODE_SIZE/2, relY + NODE_SIZE/2,
                        childX + NODE_SIZE/2, childY + NODE_SIZE/2);
        drawNode(painter, node->left, childX, childY, level + 1);
    }

    if (node->right) {
        int childX = relX + horizontalSpacing;
        int childY = relY + VERTICAL_SPACING;
        painter.setPen(QPen(Qt::black, 2));
        painter.drawLine(relX + NODE_SIZE/2, relY + NODE_SIZE/2,
                        childX + NODE_SIZE/2, childY + NODE_SIZE/2);
        drawNode(painter, node->right, childX, childY, level + 1);
    }

    // 绘制节点
    bool isRoot = false;
    for (auto tree : m_trees) {
        if (tree && node == tree->root) {
            isRoot = true;
            break;
        }
    }
    
    painter.setPen(QPen(isRoot ? Qt::red : Qt::darkBlue, 2));
    painter.setBrush(QColor(173, 216, 230));
    painter.drawEllipse(relX, relY, NODE_SIZE, NODE_SIZE);
    
    // 绘制文字
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 12));
    painter.drawText(QRect(relX, relY, NODE_SIZE, NODE_SIZE),
                    Qt::AlignCenter,
                    QString::number(node->key));
}