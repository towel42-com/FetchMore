#include <QMainWindow>
#include <QApplication>
#include <QHeaderView>
#include <QFile>
#include "treemodel.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QFile file(":/default.txt");
    file.open(QIODevice::ReadOnly);
    TreeModel* modelPtr = new TreeModel (file.readAll());
    file.close();

    QTreeView *view = new QTreeView(this);
    view->setModel(modelPtr);
    setCentralWidget(view);
    view->show();
}

MainWindow::~MainWindow()
{
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(simpletreemodel);
    QApplication app(argc, argv);

    MainWindow w;
    w.showMaximized();
    w.show();

    return app.exec();
}


