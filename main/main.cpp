#include "treemodel.h"
#include "window.h"
#include "SABUtils/AutoFetch.h"

#include <QMainWindow>
#include <QApplication>
#include <QHeaderView>
#include <QFile>
#include <QAbstractItemModelTester>
#include <QLoggingCategory>
#include <QTreeView>
#include <QScrollBar>

#include <cstdint>
#include <iostream>

MainWindow::MainWindow( QWidget * parent )
    : QMainWindow( parent )
{
    QFile file( ":/default.txt" );
    file.open( QIODevice::ReadOnly );
    TreeModel * model = new TreeModel( this );
    //new QAbstractItemModelTester( model, QAbstractItemModelTester::FailureReportingMode::Fatal, this );
    model->load( file.readAll() );

    file.close();

    fView = new QTreeView( this );
    new NQtUtils::CAutoFetchMore( fView );
    fView->setModel( model );
    setCentralWidget( fView );
    fView->show();

    fView->installEventFilter( this );
}

MainWindow::~MainWindow()
{
}

int main( int argc, char * argv[] )
{
    Q_INIT_RESOURCE( simpletreemodel );
    QApplication app( argc, argv );

    QLoggingCategory::setFilterRules( QStringLiteral( "qt.modeltest.debug=true" ) );

    MainWindow w;
    w.showMaximized();
    w.show();

    //Window w2;
    //w2.show();

    return app.exec();
}


