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

public:
    Ui::MainWindow *ui;
    SplayTree<int> m_tree; // 改为成员变量形式
    void updateTreeDisplay();
};
#endif // MAINWINDOW_H
