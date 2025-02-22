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
    static int lastSize = -1;
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
    if (ok) {
        m_tree.erase(val);
        updateTreeDisplay();
        ui->textBrowser->append("已删除: " + QString::number(val));
    } else {
        ui->textBrowser->append("输入无效!");
    }
}

void MainWindow::onSearchClicked() {
    bool ok;
    int val = ui->lineEdit->text().toInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, "错误", "请输入有效的数字!");
        return;
    }

    auto node = m_tree.find(val);
    if (node) {
        ui->textBrowser->append(QString("找到节点 %1 并旋转至根节点").arg(val));
    } else {
        ui->textBrowser->append(QString("未找到节点 %1，最后访问的节点已旋转至根").arg(val));
    }
    
    // 强制更新显示
    ui->treeWidget->update();
    qApp->processEvents();
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
}

void MainWindow::onMergeClicked() {
    if (!m_isInSplitState) {
        QMessageBox::warning(this, "错误", "请先拆分树!");
        return;
    }

    // 合并前检查
    if (!m_leftTree || !m_rightTree) {
        QMessageBox::warning(this, "错误", "拆分树状态异常!");
        m_isInSplitState = false;
        return;
    }

    // 执行合并
    SplayTree<int>* merged = SplayTree<int>::merge(m_leftTree, m_rightTree);
    if (!merged) {
        QMessageBox::warning(this, "错误", "合并失败：左树的最大值必须小于右树的最小值!");
        // 重置状态
        delete m_leftTree;
        delete m_rightTree;
        m_leftTree = m_rightTree = nullptr;
        m_isInSplitState = false;
        return;
    }

    // 更新主树
    m_tree = *merged;
    delete merged;

    // 清理状态
    m_leftTree = m_rightTree = nullptr;
    m_isInSplitState = false;

    // 更新显示
    ui->treeWidget->setTree(&m_tree);
    ui->textBrowser->append("已成功合并树");
    updateTreeDisplay();
}
