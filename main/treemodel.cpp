
#include <QMenu>
#include <QDebug>

#include "treeitem.h"
#include "treemodel.h"

TreeModel::TreeModel( QObject * parent )
    : QAbstractItemModel( parent ),
    rootItem( nullptr )
{
}
void TreeModel::load( const QString & data )
{
    QList<QString> rootData;
    rootData << "Title" << "Summary";
    rootItem = new TreeItem( rootData );
    auto parentStack = QList< TreeItem * >() << rootItem;
    setupModelData( data.split( QString( "\n" ) ), parentStack );
}
TreeModel::~TreeModel()
{
    delete rootItem;
}

TreeItem * TreeModel::getItem( const QModelIndex & index ) const
{
    if ( index.isValid() )
        return static_cast<TreeItem *>(index.internalPointer());
    return rootItem;
}

int TreeModel::columnCount( const QModelIndex & parent ) const
{
    auto item = getItem( parent );
    if ( item )
        return item->columnCount();
    return 0;
}

QVariant TreeModel::data( const QModelIndex & index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();

    if ( role != Qt::DisplayRole )
        return QVariant();

    auto item = getItem( index );
    if ( !item )
        return QVariant();

    return item->data( index.column() );
}
Qt::ItemFlags TreeModel::flags( const QModelIndex & index ) const
{
    if ( !index.isValid() )
        return QAbstractItemModel::flags( index );

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
QVariant TreeModel::headerData( int section, Qt::Orientation orientation,
                                int role ) const
{
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
        return rootItem->data( section );

    return QVariant();
}

void TreeModel::setCurrentChildCount( TreeItem * item, int count )
{
    itemToChildShownCount[ item ] = count;
}

int TreeModel::getCurrentChildCount( TreeItem * item, bool * cached ) const
{
    auto it = itemToChildShownCount.find( item );
    if ( cached )
        *cached = false;
    if ( it != itemToChildShownCount.end() )
    {
        if ( cached )
            *cached = true;
        int count = it->second;
        return count;
    }
    return 0;
}

bool TreeModel::canFetchMore( const QModelIndex & parent ) const
{
    auto item = getItem( parent );
    if ( item )
    {
        int currCount = getCurrentChildCount( item, nullptr );
        bool retVal = currCount < item->childCount();
        return retVal;
    }
    return false;
}

void TreeModel::fetchMore( const QModelIndex & parent )
{
    auto item = getItem( parent );
    if ( !item )
        return;

    int currCount = getCurrentChildCount( item, nullptr );
    int remainder = item->childCount() - currCount;
    int itemsToFetch = qMin( 1, remainder );
    if ( itemsToFetch <= 0 )
        return;

    beginInsertRows( parent, currCount, currCount + itemsToFetch - 1 );
    setCurrentChildCount( item, currCount + itemsToFetch );
    endInsertRows();
}

//! [8]
int TreeModel::rowCount( const QModelIndex & parent ) const
{
    if ( parent.column() > 0 )
        return 0;

    auto parentItem = getItem( parent );
    if ( !parentItem )
        return 0;

    bool cached = false;
    auto count = getCurrentChildCount( parentItem, &cached );
    if ( cached )
        return count;
    return 0;
}

QModelIndex TreeModel::index( int row, int column, const QModelIndex & parent ) const
{
    if ( !hasIndex( row, column, parent ) )
        return QModelIndex();

    TreeItem * parentItem = getItem( parent );
    if ( !parentItem )
        return QModelIndex();

    TreeItem * childItem = parentItem->child( row );
    if ( childItem )
        return createIndex( row, column, childItem );
    else
        return QModelIndex();
}

QModelIndex TreeModel::parent( const QModelIndex & index ) const
{
    if ( !index.isValid() )
        return QModelIndex();

    auto childItem = getItem( index );
    if ( !childItem )
        return QModelIndex();

    TreeItem * parentItem = childItem->parent();

    if ( parentItem == rootItem )
        return QModelIndex();

    return createIndex( parentItem->row(), 0, parentItem );
}

void TreeModel::setupModelData( const QStringList & lines, QList< TreeItem * > & parentStack )
{
    Q_ASSERT( !parentStack.isEmpty() );
    int prevDepth = -1;
    int ii = 0; 
    TreeItem * prevItem = parentStack.back();
    int topParentNum = 0;
    for( auto && currLine : lines )
    {
        auto columns = currLine.split( "\t", Qt::KeepEmptyParts );
        int depth = 0;
        while( !columns.isEmpty() && columns[ 0 ].isEmpty() )
        {
            depth++;
            columns.pop_front();
        }
        columns.removeAll( QString() );
        if ( columns.isEmpty() )
            continue;
        
        if ( depth > prevDepth ) // new parent
        {
            parentStack.push_back( prevItem );
        }
        else if ( depth < prevDepth ) // pop parent
        {
            if ( parentStack.count() > 1 )
                parentStack.pop_back();
        }
        auto parentItem = parentStack.back();
        prevDepth = depth;

        prevItem = new TreeItem( columns, parentItem );
        if ( parentStack.count() <= 2 )
        {
            prevItem->addSuffix( topParentNum++ ); 
        }

        parentItem->appendChild( prevItem );
    }
}

bool TreeModel::hasChildren( const QModelIndex & parent ) const
{
    bool retVal = QAbstractItemModel::hasChildren( parent ) || canFetchMore( parent );
    return retVal;
}

