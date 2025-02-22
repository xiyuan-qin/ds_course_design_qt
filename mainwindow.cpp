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
    delete ui;
}

void MainWindow::updateTreeDisplay()
{
    ui->treeWidget->update();
    qApp->processEvents();  // 确保立即更新显示

    QString status = QString("树的节点数: %1").arg(m_tree.size());
    statusBar()->showMessage(status);

    if (m_tree.root) {
        qDebug() << "当前树状态 - 根节点:" << m_tree.root->key
                 << " 大小:" << m_tree.size();
    }
}

void MainWindow::onInsertClicked() {
    bool ok;
    int val = ui->lineEdit->text().toInt(&ok);
    
    if (!ok) {
        QMessageBox::warning(this, "错误", "请输入有效的数字!");
        return;
    }
    
    m_tree.insert(val);
    updateTreeDisplay();
    ui->textBrowser->append(QString("插入节点: %1 (当前大小: %2)").arg(val).arg(m_tree.size()));
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
    if (ok) {
        auto node = m_tree.find(val);
        if (node) {
            ui->textBrowser->append(QString("找到节点 %1 并旋转至根节点").arg(val));
            updateTreeDisplay();  // 更新显示以反映伸展操作
        } else {
            ui->textBrowser->append(QString("未找到节点 %1").arg(val));
        }
    } else {
        ui->textBrowser->append("输入无效!");
    }
}
