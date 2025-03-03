#include "treewidget.h"
#include <QPainterPath>
#include <QtMath>

TreeWidget::TreeWidget(QWidget *parent) : QWidget(parent) {
    m_lastRotation.start();  // 初始化计时器
}

void TreeWidget::setTree(SplayTree<int>* tree) {
    m_trees.clear();
    if (tree) {
        m_trees.push_back(tree);
        m_tree = tree;  // 保存当前树的引用
        m_lastRotation.restart();  // 重启计时器
        
        // 强制立即进行一次重绘
        update();
        
        // 添加额外的延时更新以确保动画效果
        QTimer::singleShot(50, this, [this]() {
            update();
        });
    }
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
    gradient.setColorAt(0, QColor(255, 255, 255));
    gradient.setColorAt(1, QColor(248, 250, 252));
    painter.fillRect(rect(), gradient);

    // 添加更淡的网格线
    painter.setPen(QPen(QColor(240, 242, 245), 1, Qt::DotLine));
    for(int i = 0; i < width(); i += 50) {
        painter.drawLine(i, 0, i, height());
    }
    for(int i = 0; i < height(); i += 50) {
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
    static const int ANIMATION_DURATION = 1000; // 动画持续时间（毫秒）
    
    // 计算水平间距
    int horizontalSpacing = NODE_SIZE * 6 / (level + 2);

    // 修改判断逻辑，使动画效果更明显
    bool isRecentlyRotated = false;
    if (m_tree && node == m_tree->root) {
        qint64 elapsed = m_lastRotation.elapsed();
        isRecentlyRotated = (elapsed < ANIMATION_DURATION);
    }
    
    // 先绘制连线
    if (node->left || node->right) {
        // 增加线条宽度和对比度
        QPen linePen;
        if (isRecentlyRotated) {
            linePen = QPen(QColor(33, 150, 243), 3.5); // 加粗的蓝色，更明显
        } else {
            linePen = QPen(QColor(52, 73, 94), 3.0); // 加粗的深灰色，提高对比度
        }
        linePen.setCapStyle(Qt::RoundCap);
        linePen.setJoinStyle(Qt::RoundJoin);
        painter.setPen(linePen);
        
        if (node->left) {
            int childX = relX - horizontalSpacing;
            int childY = relY + VERTICAL_SPACING;
            
            // 创建更明显的连线
            QPainterPath path;
            path.moveTo(relX + NODE_SIZE/2, relY + NODE_SIZE/2);
            path.cubicTo(
                relX + NODE_SIZE/2, relY + NODE_SIZE + 10,        // 第一个控制点
                childX + NODE_SIZE/2, childY - 30,                // 第二个控制点
                childX + NODE_SIZE/2, childY + NODE_SIZE/2        // 终点
            );
            
            painter.drawPath(path);
            drawNode(painter, node->left, childX, childY, level + 1);
        }

        if (node->right) {
            int childX = relX + horizontalSpacing;
            int childY = relY + VERTICAL_SPACING;
            
            // 创建更明显的连线
            QPainterPath path;
            path.moveTo(relX + NODE_SIZE/2, relY + NODE_SIZE/2);
            path.cubicTo(
                relX + NODE_SIZE/2, relY + NODE_SIZE + 10,        // 第一个控制点
                childX + NODE_SIZE/2, childY - 30,                // 第二个控制点
                childX + NODE_SIZE/2, childY + NODE_SIZE/2        // 终点
            );
            
            painter.drawPath(path);
            drawNode(painter, node->right, childX, childY, level + 1);
        }
    }

    // 增强节点样式
    QColor nodeColor;
    if (isRecentlyRotated) {
        int alpha = qMax(0, 255 - (int)(m_lastRotation.elapsed() / 4));
        nodeColor = QColor(52, 152, 219, alpha); // 蓝色
    } else if (m_tree && node == m_tree->root) {
        nodeColor = QColor(41, 128, 185); // 更深的蓝色
    } else {
        nodeColor = QColor(26, 188, 156); // 绿松石色
    }

    // 创建节点渐变
    QRadialGradient gradient(relX + NODE_SIZE/2, relY + NODE_SIZE/2,
                            NODE_SIZE/2);
    gradient.setColorAt(0, nodeColor.lighter(130));
    gradient.setColorAt(1, nodeColor);
    
    // 美化节点效果
    painter.save();
    
    // 添加阴影
    QColor shadowColor(0, 0, 0, 30);
    painter.setPen(Qt::NoPen);
    painter.setBrush(shadowColor);
    painter.drawEllipse(relX + 4, relY + 4, NODE_SIZE, NODE_SIZE);
    
    painter.setBrush(gradient);
    painter.setPen(QPen(nodeColor.darker(110), 2));
    
    // 绘制主节点
    painter.drawEllipse(relX, relY, NODE_SIZE, NODE_SIZE);
    
    // 添加内层亮环，提升3D效果
    QPen innerRingPen(QColor(255, 255, 255, 100), 2);
    painter.setPen(innerRingPen);
    painter.drawEllipse(relX + 5, relY + 5, NODE_SIZE - 10, NODE_SIZE - 10);
    
    // 优化文字效果
    painter.setPen(QColor(255, 255, 255));
    QFont nodeFont("Microsoft YaHei", 14, QFont::Bold);
    painter.setFont(nodeFont);
    painter.drawText(QRect(relX, relY, NODE_SIZE, NODE_SIZE),
                    Qt::AlignCenter,
                    QString::number(node->key));
                    
    painter.restore();
    
    // 递归绘制子节点
    if (node->left) {
        drawNode(painter, node->left,
                relX - horizontalSpacing,
                relY + VERTICAL_SPACING,
                level + 1);
    }
    if (node->right) {
        drawNode(painter, node->right,
                relX + horizontalSpacing,
                relY + VERTICAL_SPACING,
                level + 1);
    }
}