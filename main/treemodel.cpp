
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
    QList<QVariant> rootData;
    rootData << "Title" << "Summary";
    rootItem = new TreeItem( rootData );
    setupModelData( data.split( QString( "\n" ) ), rootItem );
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

void TreeModel::setupModelData( const QStringList & lines, TreeItem * parent )
{
    QList<TreeItem *> parents;
    QList<int> indentations;
    parents << parent;
    indentations << 0;

    int number = 0;

    while ( number < lines.count() )
    {
        int position = 0;
        while ( position < lines[ number ].length() )
        {
            if ( lines[ number ].mid( position, 1 ) != " " )
                break;
            position++;
        }

        QString lineData = lines[ number ].mid( position ).trimmed();

        if ( !lineData.isEmpty() )
        {
            // Read the column data from the rest of the line.
            QStringList columnStrings = lineData.split( "\t", Qt::SkipEmptyParts );
            QList<QVariant> columnData;
            for ( int column = 0; column < columnStrings.count(); ++column )
                columnData << columnStrings[ column ];

            if ( position > indentations.last() )
            {
                // The last child of the current parent is now the new parent
                // unless the current parent has no children.

                if ( parents.last()->childCount() > 0 )
                {
                    parents << parents.last()->child( parents.last()->childCount() - 1 );
                    indentations << position;
                }
            }
            else
            {
                while ( position < indentations.last() && parents.count() > 0 )
                {
                    parents.pop_back();
                    indentations.pop_back();
                }
            }

            // Append a new item to the current parent's list of children.
            TreeItem * item_1 = new TreeItem( columnData, parents.last() );
            parents.last()->appendChild( item_1 );

            //auto child1 = new TreeItem( QList< QVariant >() << "1-Column1" << "1-Column2", item_1 );
            //auto child2 = new TreeItem( QList< QVariant >() << "2-Column1" << "2-Column2", item_1 );
            //auto child3 = new TreeItem( QList< QVariant >() << "3-Column1" << "3-Column2", item_1 );
            //item_1->appendChild( child1 );
            //item_1->appendChild( child2 );
            //item_1->appendChild( child3 );
        }

        number++;
    }
}

bool TreeModel::hasChildren( const QModelIndex & parent ) const
{
    bool retVal = QAbstractItemModel::hasChildren( parent ) || canFetchMore( parent );
    return retVal;
}


