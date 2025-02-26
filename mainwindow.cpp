#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 设置树控件
    ui->treeWidget->setTree(&m_tree);

    // 连接信号和槽
    connect(ui->insertButton, &QPushButton::clicked, this, &MainWindow::onInsertClicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteClicked);
    connect(ui->searchButton, &QPushButton::clicked, this, &MainWindow::onSearchClicked);
    connect(ui->splitButton, &QPushButton::clicked, this, &MainWindow::onSplitClicked);
    connect(ui->mergeButton, &QPushButton::clicked, this, &MainWindow::onMergeClicked);
    
    // 添加回车键支持
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &MainWindow::onInsertClicked);

    // 设置初始状态
    ui->textBrowser->append("欢迎使用伸展树演示程序!");

    // 强制初始刷新
    QTimer::singleShot(100, [this](){
        ui->treeWidget->update();
    });
}

MainWindow::~MainWindow() {
    // 清理所有节点
    SplayTree<int>::cleanup();
    delete ui;
    delete m_leftTree;
    delete m_rightTree;
}

void MainWindow::updateTreeDisplay()
{
    // 减少不必要的更新
    static unsigned long lastSize = 0;  // 修改为unsigned long以匹配size()返回类型
    if (m_tree.size() == lastSize && !m_isInSplitState) return;
    lastSize = m_tree.size();
    
    ui->treeWidget->update();
    
    QString status = QString("树的节点数: %1").arg(m_tree.size());
    statusBar()->showMessage(status);
}

void MainWindow::onInsertClicked() {
    static const int MAX_NODES = 50;  // 限制最大节点数
    
    bool ok;
    int val = ui->lineEdit->text().toInt(&ok);
    
    if (!ok) {
        QMessageBox::warning(this, "错误", "请输入有效的数字!");
        return;
    }
    
    if (m_tree.size() >= MAX_NODES) {
        QMessageBox::warning(this, "警告", "节点数量已达到上限!");
        return;
    }
    
    m_tree.insert(val);
    
    // 使用定时器延迟更新显示
    QTimer::singleShot(0, this, &MainWindow::updateTreeDisplay);
    
    ui->textBrowser->append(QString("插入节点: %1 (当前大小: %2)")
                          .arg(val).arg(m_tree.size()));
    ui->lineEdit->clear();
    ui->lineEdit->setFocus();
}

void MainWindow::onDeleteClicked() {
    bool ok;
    int val = ui->lineEdit->text().toInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, "错误", "请输入有效的数字!");
        return;
    }

    // 先保存当前根节点值用于比较
    int oldRootVal = m_tree.root ? m_tree.root->key : -1;
    
    m_tree.erase(val);
    
    int newRootVal = m_tree.root ? m_tree.root->key : -1;
    
    if (oldRootVal == val) {
        ui->textBrowser->append(QString("已删除节点 %1").arg(val));
    } else if (oldRootVal != newRootVal) {
        ui->textBrowser->append(QString("未找到节点 %1，最后访问的节点 %2 已旋转至根")
                              .arg(val).arg(newRootVal));
    } else {
        ui->textBrowser->append(QString("未找到节点 %1，树为空或无需旋转").arg(val));
    }
    
    updateTreeDisplay();
}

void MainWindow::onSearchClicked() {
    bool ok;
    int val = ui->lineEdit->text().toInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, "错误", "请输入有效的数字!");
        return;
    }

    // 先保存当前根节点值用于比较
    int oldRootVal = m_tree.root ? m_tree.root->key : -1;
    
    auto node = m_tree.find(val);
    int newRootVal = m_tree.root ? m_tree.root->key : -1;
    
    // 更新显示并触发动画
    ui->treeWidget->setTree(&m_tree);
    
    if (node) {
        ui->textBrowser->append(QString("找到节点 %1 并旋转至根节点").arg(val));
    } else {
        if (oldRootVal != newRootVal) {
            ui->textBrowser->append(QString("未找到节点 %1，最后访问的节点 %2 已旋转至根")
                                  .arg(val).arg(newRootVal));
        } else {
            ui->textBrowser->append(QString("未找到节点 %1，树为空或无需旋转").arg(val));
        }
    }
    
    // 强制更新显示并添加延迟更新以确保动画效果
    updateTreeDisplay();
    QTimer::singleShot(50, this, &MainWindow::updateTreeDisplay);
}

void MainWindow::onSplitClicked() {
    if (m_isInSplitState) {
        QMessageBox::warning(this, "错误", "树已经处于拆分状态!");
        return;
    }

    if (!m_tree.root) {
        QMessageBox::warning(this, "错误", "当前树为空!");
        return;
    }

    bool ok;
    int val = ui->lineEdit->text().toInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, "错误", "请输入有效的数字!");
        return;
    }

    // 清理旧的拆分树
    delete m_leftTree;
    delete m_rightTree;
    m_leftTree = m_rightTree = nullptr;

    auto [left, right] = m_tree.split(val);
    if (!left || !right) {
        QMessageBox::warning(this, "错误", "拆分失败!");
        delete left;
        delete right;
        return;
    }

    m_leftTree = left;
    m_rightTree = right;
    m_isInSplitState = true;

    // 清空主树但保留其分配的内存
    m_tree.root = nullptr;
    m_tree.p_size = 0;

    // 显示拆分后的两棵树
    std::vector<SplayTree<int>*> trees = {m_leftTree, m_rightTree};
    ui->treeWidget->setTrees(trees);
    
    QString status = QString("左树节点数: %1, 右树节点数: %2")
                        .arg(m_leftTree->size())
                        .arg(m_rightTree->size());
    statusBar()->showMessage(status);
    
    ui->textBrowser->append(QString("已按键值 %1 拆分树为 [≤%1] 和 [>%1] 两部分")
                          .arg(val));
    
    // 添加内存使用状态检查
    QString memStatus = QString("当前节点数: %1, 总分配次数: %2")
                        .arg(m_tree.get_current_nodes())
                        .arg(m_tree.get_total_allocations());
    ui->textBrowser->append(memStatus);
}

void MainWindow::onMergeClicked() {
    if (!m_isInSplitState) {
        QMessageBox::warning(this, "错误", "请先拆分树!");
        return;
    }

    // 禁用所有按钮，防止用户重复操作
    setControlsEnabled(false);

    // 使用QTimer延迟执行合并操作
    QTimer::singleShot(0, this, [this]() {
        // 安全检查
        if (!m_leftTree || !m_rightTree) {
            cleanup_split_state();
            setControlsEnabled(true);
            return;
        }

        // 尝试合并
        SplayTree<int>* merged = SplayTree<int>::merge(m_leftTree, m_rightTree);
        if (!merged) {
            cleanup_split_state();
            QMessageBox::warning(this, "错误", "合并失败：左树的最大值必须小于右树的最小值!");
            setControlsEnabled(true);
            return;
        }

        // 更新主树
        m_tree.clear(m_tree.root);
        m_tree.root = merged->root;
        m_tree.p_size = merged->p_size;
        merged->root = nullptr;
        merged->p_size = 0;
        delete merged;

        m_leftTree = m_rightTree = nullptr;
        m_isInSplitState = false;

        // 强制垃圾回收
        SplayTree<int>::cleanup_unused();

        // 更新UI
        ui->treeWidget->setTree(&m_tree);
        updateTreeDisplay();

        // 恢复按钮状态
        setControlsEnabled(true);

        ui->textBrowser->append("合并完成");
    });
}

// 新增：控制UI元素启用状态的辅助函数
void MainWindow::setControlsEnabled(bool enabled) {
    ui->insertButton->setEnabled(enabled);
    ui->deleteButton->setEnabled(enabled);
    ui->searchButton->setEnabled(enabled);
    ui->splitButton->setEnabled(enabled);
    ui->mergeButton->setEnabled(enabled);
    ui->lineEdit->setEnabled(enabled);
}

void MainWindow::cleanup_split_state() {
    delete m_leftTree;
    delete m_rightTree;
    m_leftTree = m_rightTree = nullptr;
    m_isInSplitState = false;
    SplayTree<int>::cleanup_unused();
}