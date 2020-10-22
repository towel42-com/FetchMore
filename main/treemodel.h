

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QMainWindow>
#include <QApplication>
#include <map>
#include <QTreeView>
using namespace std;


class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
public:
    MainWindow(QWidget *parent = NULL);
    virtual ~MainWindow();
};

class TreeItem;

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TreeModel(const QString &data, QObject *parent = NULL);
    ~TreeModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    void emitLayoutChangedSignal();

private:
    void setupModelData(const QStringList &lines, TreeItem *parent);
    map<TreeItem*, int> itemToChildShownCount;
    TreeItem *rootItem;

    int getCurrentChildCount(TreeItem* item) const;

protected:
    bool canFetchMore(const QModelIndex &parent) const ;
    void fetchMore(const QModelIndex &parent) ;
};
