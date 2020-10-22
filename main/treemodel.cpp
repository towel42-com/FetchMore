
#include <QMenu>
#include <QDebug>

#include "treeitem.h"
#include "treemodel.h"

TreeModel::TreeModel(const QString &data, QObject *parent)
    : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    rootData << "Title" << "Summary";
    rootItem = new TreeItem(rootData);
    itemToChildShownCount[rootItem] = 0;
    setupModelData(data.split(QString("\n")), rootItem);
}
TreeModel::~TreeModel()
{
    delete rootItem;
}
int TreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}
QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    if(!item)
        return QVariant();

    return item->data(index.column());
}
Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

int TreeModel::getCurrentChildCount(TreeItem* item) const
{
    map<TreeItem*, int>::const_iterator it = itemToChildShownCount.find(item);
    if(it != itemToChildShownCount.end())
    {
        int count = it->second;
        return count;
    }
    return 0;
}

bool TreeModel::canFetchMore(const QModelIndex & parent ) const
{
    if (parent.isValid())
    {
        TreeItem* item = static_cast<TreeItem*>(parent.internalPointer());
        int currCount = getCurrentChildCount(item);
        qDebug()<< currCount << item->childCount();
        return  currCount < item->childCount();
    }
    return false;
}

void TreeModel::fetchMore(const QModelIndex &  parent )
{
    if (parent.isValid())
    {
        TreeItem* item = static_cast<TreeItem*>(parent.internalPointer());
        int currCount = getCurrentChildCount(item);

        int remainder = item->childCount() - currCount;
    
        int itemsToFetch = qMin(3, remainder);
       beginInsertRows(parent, currCount, currCount + itemsToFetch - 1);

        itemToChildShownCount[item] = currCount + itemsToFetch;

        endInsertRows();

    }

}

//! [8]
int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    if(0 == getCurrentChildCount(parentItem))
        return parentItem->childCount();

     return getCurrentChildCount(parentItem);
}
QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}
QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    if(!childItem)
        return QModelIndex();

    TreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

void TreeModel::setupModelData(const QStringList &lines, TreeItem *parent)
{
    QList<TreeItem*> parents;
    QList<int> indentations;
    parents << parent;
    indentations << 0;

    int number = 0;

    while (number < lines.count()) {
        int position = 0;
        while (position < lines[number].length()) {
            if (lines[number].mid(position, 1) != " ")
                break;
            position++;
        }

        QString lineData = lines[number].mid(position).trimmed();

        if (!lineData.isEmpty()) {
            // Read the column data from the rest of the line.
            QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
            QList<QVariant> columnData;
            for (int column = 0; column < columnStrings.count(); ++column)
                columnData << columnStrings[column];

            if (position > indentations.last()) {
                // The last child of the current parent is now the new parent
                // unless the current parent has no children.

                if (parents.last()->childCount() > 0) {
                    parents << parents.last()->child(parents.last()->childCount()-1);
                    indentations << position;
                }
            } else {
                while (position < indentations.last() && parents.count() > 0) {
                    parents.pop_back();
                    indentations.pop_back();
                }
            }

            // Append a new item to the current parent's list of children.
            TreeItem* item_1 = new TreeItem(columnData, parents.last());
            parents.last()->appendChild(item_1);
            itemToChildShownCount[item_1] = 0;
        }

        number++;
    }
}


void TreeModel::emitLayoutChangedSignal()
{
    emit layoutChanged();
}
