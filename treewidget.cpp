#include "treewidget.h"

TreeWidget::TreeWidget(QWidget *parent) : QWidget(parent) {}

void TreeWidget::setTree(SplayTree<int>* tree) {
    m_tree = tree;
    update();
}

void TreeWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 设置背景
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, QColor(240, 240, 255));
    gradient.setColorAt(1, QColor(255, 255, 255));
    painter.fillRect(rect(), gradient);

    if (!m_tree || !m_tree->root) {
        painter.setPen(Qt::gray);
        painter.setFont(QFont("Arial", 14));
        painter.drawText(rect(), Qt::AlignCenter, "空树");
        return;
    }

    // 计算合适的缩放比例
    int maxDepth = calculateTreeDepth(m_tree->root);
    float hScale = width() / (pow(2, maxDepth) * 50.0f);
    float vScale = height() / ((maxDepth + 1) * 100.0f);
    float scale = qMin(qMin(hScale, vScale), 1.0f);

    // 应用变换
    painter.translate(width()/2, 50);
    painter.scale(scale, scale);

    drawNode(painter, m_tree->root, 0, 0, 1);
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

    const int HORIZONTAL_SPACING = 280 / (1 << qMin(level, 4));
    const int VERTICAL_SPACING = 80;

    int absX = relX;
    int absY = relY;

    // 先绘制子节点连线
    if (node->left) {
        int childX = relX - HORIZONTAL_SPACING;
        int childY = relY + VERTICAL_SPACING;
        painter.drawLine(absX + 15, absY + 15,
                        childX + 15, childY + 15);
        drawNode(painter, node->left, childX, childY, level+1);
    }

    if (node->right) {
        int childX = relX + HORIZONTAL_SPACING;
        int childY = relY + VERTICAL_SPACING;
        painter.drawLine(absX + 15, absY + 15,
                        childX + 15, childY + 15);
        drawNode(painter, node->right, childX, childY, level+1);
    }

    // 节点绘制
    painter.setPen(node == m_tree->root ? Qt::red : Qt::darkBlue);
    painter.setBrush(QColor(173, 216, 230));
    painter.drawEllipse(absX, absY, 30, 30);

    painter.setPen(Qt::black);
    painter.drawText(QRect(absX, absY, 30, 30),
                     Qt::AlignCenter,
                     QString::number(node->key));
}
