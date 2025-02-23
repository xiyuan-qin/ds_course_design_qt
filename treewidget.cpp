#include "treewidget.h"
#include <QPainterPath>
#include <QtMath>

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
    
    // 设置渐变背景 - 使用更柔和的白色渐变
    QLinearGradient gradient(0, 0, width(), height());
    gradient.setColorAt(0, QColor(252, 253, 255));
    gradient.setColorAt(1, QColor(246, 247, 251));
    painter.fillRect(rect(), gradient);

    // 添加更淡的网格线
    painter.setPen(QPen(QColor(238, 240, 245), 1, Qt::DotLine));
    for(int i = 0; i < width(); i += 40) {
        painter.drawLine(i, 0, i, height());
    }
    for(int i = 0; i < height(); i += 40) {
        painter.drawLine(0, i, width(), i);
    }

    if (m_trees.empty() || std::all_of(m_trees.begin(), m_trees.end(), 
        [](auto t) { return !t || !t->root; })) {
        // 空树显示优化
        QFont font("Microsoft YaHei", 16);
        painter.setFont(font);
        painter.setPen(QColor(150, 150, 150));
        
        QString message = "空树\n点击\"插入\"按钮添加节点";
        QFontMetrics fm(font);
        QRect textRect = fm.boundingRect(rect(), Qt::AlignCenter | Qt::TextWordWrap, message);
        painter.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, message);
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
    
    // 计算水平间距
    int horizontalSpacing = NODE_SIZE * 6 / (level + 2);

    // 先绘制连线
    if (node->left || node->right) {
        // 设置连线样式 - 使用更柔和的蓝灰色
        QPen linePen(QColor(145, 158, 171, 140), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(linePen);
        
        if (node->left) {
            int childX = relX - horizontalSpacing;
            int childY = relY + VERTICAL_SPACING;
            
            // 计算控制点 - 采用更自然的曲线
            int midY = relY + VERTICAL_SPACING * 0.5;
            
            QPainterPath path;
            path.moveTo(relX + NODE_SIZE/2, relY + NODE_SIZE/2);
            // 使用更短的贝塞尔曲线控制点，让曲线更自然
            path.quadTo(
                relX + NODE_SIZE/2, midY,  // 控制点
                childX + NODE_SIZE/2, childY + NODE_SIZE/2  // 终点
            );
            
            painter.drawPath(path);
            drawNode(painter, node->left, childX, childY, level + 1);
        }

        if (node->right) {
            int childX = relX + horizontalSpacing;
            int childY = relY + VERTICAL_SPACING;
            
            // 对称处理右侧连线
            int midY = relY + VERTICAL_SPACING * 0.5;
            
            QPainterPath path;
            path.moveTo(relX + NODE_SIZE/2, relY + NODE_SIZE/2);
            path.quadTo(
                relX + NODE_SIZE/2, midY,
                childX + NODE_SIZE/2, childY + NODE_SIZE/2
            );
            
            painter.drawPath(path);
            drawNode(painter, node->right, childX, childY, level + 1);
        }
    }

    // 绘制节点
    bool isRoot = false;
    for (auto tree : m_trees) {
        if (tree && node == tree->root) {
            isRoot = true;
            break;
        }
    }
    
    // 优化节点外观 - 使用更优雅的配色
    if (isRoot) {
        // 根节点样式 - 优雅的蓝色
        QLinearGradient gradientRoot(relX, relY, relX, relY + NODE_SIZE);
        gradientRoot.setColorAt(0, QColor(25, 118, 210));
        gradientRoot.setColorAt(1, QColor(21, 101, 192));
        painter.setBrush(gradientRoot);
        painter.setPen(QPen(QColor(13, 71, 161), 2));
    } else {
        // 普通节点样式 - 清爽的薄荷色
        QLinearGradient gradientNode(relX, relY, relX, relY + NODE_SIZE);
        gradientNode.setColorAt(0, QColor(0, 151, 167));
        gradientNode.setColorAt(1, QColor(0, 131, 143));
        painter.setBrush(gradientNode);
        painter.setPen(QPen(QColor(0, 96, 100), 2));
    }
    
    // 添加非常柔和的阴影效果
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 15));
    painter.drawEllipse(relX + 3, relY + 3, NODE_SIZE, NODE_SIZE);
    painter.restore();
    
    // 绘制节点主体
    painter.drawEllipse(relX, relY, NODE_SIZE, NODE_SIZE);
    
    // 优化文字显示 - 使用更明亮的白色
    painter.setPen(QColor(255, 255, 255, 240));
    painter.setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
    painter.drawText(QRect(relX, relY, NODE_SIZE, NODE_SIZE),
                    Qt::AlignCenter,
                    QString::number(node->key));
                    
    // 添加更细腻的高光效果
    painter.setPen(QPen(QColor(255, 255, 255, 40), 1));
    painter.drawArc(QRectF(relX + 5, relY + 5, NODE_SIZE - 10, NODE_SIZE - 10), 45 * 16, 180 * 16);
}