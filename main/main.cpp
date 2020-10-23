#include "treemodel.h"
#include "window.h"

#include <QMainWindow>
#include <QApplication>
#include <QHeaderView>
#include <QFile>
#include <QAbstractItemModelTester>
#include <QLoggingCategory>

bool isVisible( QAbstractItemView * view, const QModelIndex & idx )
{
    if ( !view )
        return false;
    if ( !idx.isValid() )
        return false;

    auto visRegion = view->viewport()->visibleRegion();
    auto idxRect = view->visualRect( idx );
    bool isVisible = visRegion.contains( idxRect ) || visRegion.intersects( idxRect );
    return isVisible;
}

void autoFetchMore( QAbstractItemView * view, const QModelIndex & idx )
{
    // here is the problem, need to fetch more, as long as the last child is not visible
    QAbstractItemModel * model = view->model();
    auto rowCount = model->rowCount( idx );
    auto lastChild = model->index( rowCount - 1, 0, idx );

    while ( model->canFetchMore( idx ) && ::isVisible( view, lastChild ) )
    {
        model->fetchMore( idx );
        auto rowCount = model->rowCount( idx );
        lastChild = model->index( rowCount - 1, 0, idx );
    }
    if ( ::isVisible( view, lastChild ) && model->hasChildren( lastChild ) )
    {
        auto childIdx = model->index( 0, 0, lastChild );
        while ( !childIdx.isValid() && model->canFetchMore( lastChild ) )
        {
            model->fetchMore( lastChild );
            auto rowCount = model->rowCount( lastChild );
            childIdx = model->index( 0, 0, lastChild );
        }
        if ( childIdx.isValid() )
            autoFetchMore( view, childIdx );
    }
}

MainWindow::MainWindow( QWidget * parent )
    : QMainWindow( parent )
{
    QFile file( ":/default.txt" );
    file.open( QIODevice::ReadOnly );
    TreeModel * model = new TreeModel( this );
    //new QAbstractItemModelTester( model, QAbstractItemModelTester::FailureReportingMode::Fatal, this );
    model->load( file.readAll() );

    file.close();

    view = new QTreeView( this );
    connect( dynamic_cast< QTreeView* >( view ), &QTreeView::expanded, 
             [this]( const QModelIndex & idx )
             {
                 autoFetchMore( view, idx );
             } );
    view->setModel( model );
    setCentralWidget( view );
    view->show();

    view->installEventFilter( this );
}

MainWindow::~MainWindow()
{
}


bool MainWindow::eventFilter( QObject * object, QEvent * event )
{
    if ( object == view )
    {
        if (
            ( event->type() == QEvent::Resize )  || 
            (event->type() == QEvent::Show)
            )
        {
            autoFetchMore( view, QModelIndex() );
        }
    }
    return QMainWindow::eventFilter( object, event );
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


