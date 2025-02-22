#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 增加初始化日志输出
    qDebug() << "TreeWidget address:" << ui->treeWidget;

    // 修改指针传递方式
    ui->treeWidget->setTree(&m_tree);

    // 添加按钮文本设置
    ui->pushButton->setText("Insert");   // XML中缺少按钮文本设置

    ui->lineEdit->disconnect();

    connet(ui->pushButton, &QPushButton::clicked, this, &MainWindow::onInsertClicked);

    // 强制初始刷新
    QTimer::singleShot(100, [this](){
        ui->treeWidget->update();
    });
}


MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::updateTreeDisplay()
{
    // 强制界面刷新
    ui->treeWidget->update();
    qApp->processEvents();  // 立即处理事件队列

    // 添加详细调试信息
    qDebug() << "当前树状态 - 根节点:" << (m_tree.root?m_tree.root->key:-1)
             << " 左子树:" << (m_tree.root&&m_tree.root->left?m_tree.root->left->key:-1)
             << " 右子树:" << (m_tree.root&&m_tree.root->right?m_tree.root->right->key:-1);
}




void MainWindow::onInsertClicked() {
    bool ok;
    int val = ui->lineEdit->text().toInt(&ok);
    if (ok) {
        m_tree.insert(val);
        updateTreeDisplay();
        ui->textBrowser->append("已插入: " + QString::number(val));
    } else {
        ui->textBrowser->append("输入无效!");
    }
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
    if (ok) {
        auto node = m_tree.find(val);
        ui->textBrowser->append(node ? "找到节点" : "未找到节点");
    } else {
        ui->textBrowser->append("输入无效!");
    }
}
