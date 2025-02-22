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
    painter.fillRect(rect(), Qt::white);

    // 增加空树提示
    if (!m_tree || !m_tree->root) {
        painter.drawText(rect(), Qt::AlignCenter, "树为空");
        return;
    }

        // 修改初始化坐标为动态计算（原坐标会导致节点重叠）
        int startX = width()/2;
        int startY = 80;  // 原30太小导致节点重叠
        int maxValidLevel = qMax(1, calculateTreeDepth(m_tree->root));
        float scaleFactor = qMin(width()/(pow(2,maxValidLevel)*180.0), 1.5f); // 增加最大缩放限制
        m_globalOffset = QPoint(startX, startY);

        // 添加坐标系平移
        painter.translate(width()/2 * (1 - scaleFactor), 50);
        painter.scale(scaleFactor, scaleFactor);

        drawNode(painter, m_tree->root, 0, 0, 1); // 使用相对坐标
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
    const int HORIZONTAL_SPACING = 280 / (1 << qMin(level, 4));  // 增加基础间距
    const int VERTICAL_SPACING = 80;  // 减少垂直间距

    // 添加节点存在性验证
    if(!node || !node->left || !node->right) {
        qDebug() << "异常节点：" << (node?node->key:-1)
                 << " Left:" << (node->left?node->left->key:-1)
                 << " Right:" << (node->right?node->right->key:-1);
    }

    // 修正绝对坐标计算
    int absX = relX;  // 移除错误m_globalOffset
    int absY = relY;

    // 先绘制子节点连线
    if (node->left) {
        int childX = relX - HORIZONTAL_SPACING;
        int childY = relY + VERTICAL_SPACING;
        painter.drawLine(absX +15, absY +30,
                         m_globalOffset.x()+childX+15,
                         m_globalOffset.y()+childY);
        drawNode(painter, node->left, childX, childY, level+1);
    }

    if (node->right) {
        int childX = relX + HORIZONTAL_SPACING;
        int childY = relY + VERTICAL_SPACING;
        painter.drawLine(absX +15, absY +30,
                         m_globalOffset.x()+childX+15,
                         m_globalOffset.y()+childY);
        drawNode(painter, node->right, childX, childY, level+1);
    }

    // 节点绘制（添加选中状态边框）
    painter.setPen(node == m_tree->root ? Qt::red : Qt::darkBlue);
    painter.setBrush(QColor(173, 216, 230));
    painter.drawEllipse(absX, absY, 30, 30);

    // 文字绘制（防止缩放时模糊）
    painter.save();
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 12));
    painter.drawText(QRect(absX, absY, 30, 30),
                     Qt::AlignCenter,
                     QString::number(node->key));
    painter.restore();
}
