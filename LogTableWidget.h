#ifndef LOG_TABLE_WIDGET_H
#define LOG_TABLE_WIDGET_H

#include "widgets/OriTableWidgetBase.h"
#include "LogItem.h"

QT_BEGIN_NAMESPACE
class QAbstractTableModel;
class QSortFilterProxyModel;
class QItemSelection;
QT_END_NAMESPACE

class LogTableWidget : public Ori::TableWidgetBase
{
    Q_OBJECT

public:
    explicit LogTableWidget(QWidget *parent = 0);
    ~LogTableWidget();

    void populate(const LogItems *items, const LogFilters *filters);

    LogItems* filteredItems() const;

    const LogItem* selectedItem();
    const LogItem* item(int row);
    void adjustHeader();

    int filteredRowCount() const;

signals:
    void onLogItemSelected(const LogItem*);

public slots:
    void updateFilter();

protected:
    QAbstractItemModel* createTableModel() override;
    void tableCreated() override;

private:
    QAbstractTableModel *sourceModel = nullptr;
    QSortFilterProxyModel *proxyModel = nullptr;
    const LogItems* _items = nullptr;
    const LogFilters* _filters = nullptr;

private slots:
    void selectionChanged(const QItemSelection &, const QItemSelection &);
};

#endif // LOG_TABLE_WIDGET_H
