#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QTimer>
#include <QSet>


/**
 * 管理用户交互和树的状态
 * 树操作功能：
 * 插入节点
    * 删除节点
    * 查找节点
    * 拆分树
    * 合并树
 * 界面管理
 * 状态显示 - 显示树的节点数和操作结果
        树的更新 - updateTreeDisplay()
        操作日志 - 显示在文本浏览器中
*/
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
    connect(ui->clearButton, &QPushButton::clicked, this, &MainWindow::onClearClicked);  // 连接清空树按钮
    connect(ui->exitButton, &QPushButton::clicked, this, &MainWindow::onExitClicked);    // 连接退出按钮
    connect(ui->randomButton, &QPushButton::clicked, this, &MainWindow::onRandomClicked); // 连接随机生成按钮
    
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
    static const int MAX_NODES = 15;  // 最大节点数限制为15
    
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
    
    // 尝试插入前先禁用控件，防止多次操作
    setControlsEnabled(false);
    
    try {
        // 插入节点
        m_tree.insert(val);
        
        // 安全地更新显示
        QTimer::singleShot(0, this, [this, val]() {
            try {
                updateTreeDisplay();
                ui->textBrowser->append(QString("插入节点: %1 (当前大小: %2)")
                                      .arg(val).arg(m_tree.size()));
            } catch (const std::exception& e) {
                QMessageBox::critical(this, "错误", QString("更新显示时发生异常: %1").arg(e.what()));
            }
            
            // 恢复控件状态
            setControlsEnabled(true);
        });
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "错误", QString("插入节点时发生异常: %1").arg(e.what()));
        setControlsEnabled(true);
    }
    
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
    
    // 更新显示并触发动画
    ui->treeWidget->setTree(&m_tree);
    
    if (oldRootVal == val) {
        ui->textBrowser->append(QString("已删除节点 %1").arg(val));
    } else if (oldRootVal != newRootVal) {
        ui->textBrowser->append(QString("未找到节点 %1，最后访问的节点 %2 已旋转至根")
                              .arg(val).arg(newRootVal));
    } else {
        ui->textBrowser->append(QString("未找到节点 %1，树为空或无需旋转").arg(val));
    }
    
    // 强制更新显示并添加延迟更新以确保动画效果
    updateTreeDisplay();
    QTimer::singleShot(50, this, &MainWindow::updateTreeDisplay);
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

// 新增清空树的槽函数实现
void MainWindow::onClearClicked() {
    // 创建一个自定义的确认对话框
    QMessageBox msgBox;
    msgBox.setWindowTitle("确认操作");
    msgBox.setText("确定要清空当前树吗？此操作无法撤销。");
    msgBox.setIcon(QMessageBox::Warning);
    
    // 自定义按钮
    QPushButton *yesButton = msgBox.addButton("确定清空", QMessageBox::YesRole);
    QPushButton *noButton = msgBox.addButton("取消", QMessageBox::NoRole);
    
    // 设置按钮样式
    yesButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #e74c3c;"
        "   color: white;"
        "   font-weight: bold;"
        "   padding: 6px 20px;"
        "   border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #c0392b;"
        "}"
    );
    
    noButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #3498db;"
        "   color: white;"
        "   padding: 6px 20px;"
        "   border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #2980b9;"
        "}"
    );
    
    // 设置默认按钮为取消
    msgBox.setDefaultButton(noButton);
    
    // 执行对话框
    msgBox.exec();
    
    // 如果选择了确定清空
    if (msgBox.clickedButton() == yesButton) {
        // 清空当前树
        if (m_isInSplitState) {
            // 如果处于分裂状态，先清理分裂的树
            cleanup_split_state();
            m_isInSplitState = false;
        }
        
        // 清空主树
        if (m_tree.root) {
            m_tree.clear(m_tree.root);
            m_tree.root = nullptr;
            m_tree.p_size = 0;
            
            // 强制垃圾回收
            SplayTree<int>::cleanup_unused();
            
            // 更新界面
            ui->treeWidget->setTree(&m_tree);
            updateTreeDisplay();
            
            ui->textBrowser->append("已清空所有节点");
        } else {
            ui->textBrowser->append("树已为空");
        }
    }
}

// 新增退出程序的槽函数实现
void MainWindow::onExitClicked() {
    // 创建一个自定义的确认对话框
    QMessageBox msgBox;
    msgBox.setWindowTitle("确认退出");
    msgBox.setText("确定要退出程序吗？");
    msgBox.setIcon(QMessageBox::Question);
    
    // 自定义按钮
    QPushButton *yesButton = msgBox.addButton("确定退出", QMessageBox::YesRole);
    QPushButton *noButton = msgBox.addButton("取消", QMessageBox::NoRole);
    
    // 设置按钮样式
    yesButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #e74c3c;"
        "   color: white;"
        "   font-weight: bold;"
        "   padding: 6px 20px;"
        "   border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #c0392b;"
        "}"
    );
    
    noButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #3498db;"
        "   color: white;"
        "   padding: 6px 20px;"
        "   border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #2980b9;"
        "}"
    );
    
    // 设置默认按钮为取消
    msgBox.setDefaultButton(noButton);
    
    // 执行对话框
    msgBox.exec();
    
    // 如果选择了确定退出
    if (msgBox.clickedButton() == yesButton) {
        QApplication::quit();
    }
}

// 新增：控制UI元素启用状态的辅助函数
void MainWindow::setControlsEnabled(bool enabled) {
    ui->insertButton->setEnabled(enabled);
    ui->deleteButton->setEnabled(enabled);
    ui->searchButton->setEnabled(enabled);
    ui->splitButton->setEnabled(enabled);
    ui->mergeButton->setEnabled(enabled);
    ui->clearButton->setEnabled(enabled);  // 添加清空树按钮的控制
    ui->exitButton->setEnabled(enabled);   // 添加退出按钮的控制
    ui->randomButton->setEnabled(enabled); // 添加随机生成按钮的控制
    ui->lineEdit->setEnabled(enabled);
}

void MainWindow::cleanup_split_state() {
    delete m_leftTree;
    delete m_rightTree;
    m_leftTree = m_rightTree = nullptr;
    m_isInSplitState = false;
    SplayTree<int>::cleanup_unused();
}

// 修改随机生成树的槽函数实现
void MainWindow::onRandomClicked() {
    static const int MAX_RANDOM_NODES = 7; // 将最大随机节点数从10减少到7
    static const int MAX_NODE_VALUE = 15;  // 随机节点的最大值
    
    // 如果当前树不为空，询问用户是否清空
    if (m_tree.size() > 0) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("确认操作");
        msgBox.setText("生成随机树将替换当前树。是否继续？");
        msgBox.setIcon(QMessageBox::Question);
        
        QPushButton *yesButton = msgBox.addButton("确定", QMessageBox::YesRole);
        QPushButton *noButton = msgBox.addButton("取消", QMessageBox::NoRole);
        
        yesButton->setStyleSheet(
            "QPushButton {"
            "   background-color: #3498db;"
            "   color: white;"
            "   font-weight: bold;"
            "   padding: 6px 20px;"
            "   border-radius: 4px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #2980b9;"
            "}"
        );
        
        noButton->setStyleSheet(
            "QPushButton {"
            "   background-color: #95a5a6;"
            "   color: white;"
            "   padding: 6px 20px;"
            "   border-radius: 4px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #7f8c8d;"
            "}"
        );
        
        msgBox.setDefaultButton(noButton);
        msgBox.exec();
        
        if (msgBox.clickedButton() != yesButton) {
            return;
        }
        
        // 清空当前树
        m_tree.clear(m_tree.root);
        m_tree.root = nullptr;
        m_tree.p_size = 0;
    }
    
    // 随机生成节点数量 (3-7)，进一步减少节点数量范围
    int numNodes = 3 + (rand() % (MAX_RANDOM_NODES - 2));
    
    // 创建一个集合用于跟踪已插入的值，避免重复
    QSet<int> insertedValues;
    
    // 禁用所有控件，防止用户重复操作
    setControlsEnabled(false);
    
    // 使用QTimer添加动画效果，每隔一段时间插入一个节点
    QTimer *timer = new QTimer(this);
    int nodeIndex = 0;
    
    connect(timer, &QTimer::timeout, this, [this, timer, numNodes, &nodeIndex, &insertedValues]() {
        if (nodeIndex < numNodes) {
            // 生成一个尚未使用的随机值
            int val;
            do {
                val = rand() % MAX_NODE_VALUE + 1;
            } while (insertedValues.contains(val));
            
            insertedValues.insert(val);
            
            // 插入节点
            m_tree.insert(val);
            
            // 更新UI
            updateTreeDisplay();
            
            // 添加日志
            ui->textBrowser->append(QString("随机插入节点: %1 (%2/%3)")
                                  .arg(val).arg(nodeIndex + 1).arg(numNodes));
            
            nodeIndex++;
        } else {
            // 所有节点已插入，停止定时器
            timer->stop();
            timer->deleteLater();
            
            // 恢复控件状态
            setControlsEnabled(true);
            
            // 最终更新显示
            QTimer::singleShot(50, this, &MainWindow::updateTreeDisplay);
            
            // 添加完成日志
            ui->textBrowser->append(QString("随机生成树完成，共 %1 个节点").arg(numNodes));
        }
    });
    
    // 每200毫秒插入一个节点
    timer->start(200);
}