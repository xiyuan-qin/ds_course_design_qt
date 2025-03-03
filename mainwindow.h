#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "src/splay_tree.h"
#include "treewidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onInsertClicked();
    void onDeleteClicked();
    void onSearchClicked();
    void onSplitClicked();
    void onMergeClicked();
    void onClearClicked();  // 新增清空树的槽函数
    void onExitClicked();   // 新增退出程序的槽函数

private:
    void cleanup_split_state();  // 只声明，不实现
    void setControlsEnabled(bool enabled);

    SplayTree<int>* m_leftTree = nullptr;
    SplayTree<int>* m_rightTree = nullptr;
    bool m_isInSplitState = false;
    Ui::MainWindow *ui;
    SplayTree<int> m_tree;
    void updateTreeDisplay();
};
#endif // MAINWINDOW_H