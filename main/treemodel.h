

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QMainWindow>
#include <QApplication>
#include <map>
#include <QTreeView>

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
public:
    MainWindow(QWidget *parent = NULL);
    virtual ~MainWindow();

    virtual bool eventFilter( QObject * object, QEvent * event ) override;
private:
    QAbstractItemView * view;
};

class TreeItem;

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TreeModel(QObject *parent = NULL);
    ~TreeModel();

    void load( const QString & data );
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &index) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual bool hasChildren( const QModelIndex & parent ) const override;

    virtual bool canFetchMore( const QModelIndex & parent ) const override;
    virtual void fetchMore( const QModelIndex & parent ) override;
    //    void emitLayoutChangedSignal();

private:
    void setupModelData(const QStringList &lines, TreeItem *parent);
    std::map<TreeItem*, int> itemToChildShownCount;
    TreeItem *rootItem;

    void setCurrentChildCount( TreeItem * item, int count );
    int getCurrentChildCount(TreeItem* item, bool * cached ) const;

protected:
    TreeItem * getItem( const QModelIndex & index ) const;

    bool fFetchingMore{false};

};
