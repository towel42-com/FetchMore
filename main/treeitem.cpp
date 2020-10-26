#include <QStringList>
#include <stdio.h>
#include "treeitem.h"

TreeItem::TreeItem(const QList<QString> &data, TreeItem *parent)
{
    parentItem = parent;
    for ( auto && jj : data )
        itemData << jj;
}
TreeItem::~TreeItem()
{
    qDeleteAll(childItems);
}
void TreeItem::addSuffix( int cnt )
{
    auto string = itemData[ 0 ].toString();
    string += ": " + QString::number( cnt );
    itemData[ 0 ] = string;
}

void TreeItem::appendChild(TreeItem *item)
{
    childItems.append(item);
}
TreeItem *TreeItem::child(int row)
{
    return childItems.value(row);
}
int TreeItem::childCount() const
{
    return childItems.count();
}
int TreeItem::columnCount() const
{
    return itemData.count();
}
QVariant TreeItem::data(int column) const
{
    return itemData.value(column);
}
TreeItem *TreeItem::parent()
{
    return parentItem;
}
int TreeItem::row() const
{
    if (parentItem)
    {
        return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));
    }

    return 0;
}
//! [8]
