#include "LogTableWidget.h"
#include "helpers/OriWidgets.h"

#include <QDebug>
#include <QHeaderView>
#include <QItemSelection>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QTableView>

#define TABLE_COL_COUNT 3
#define TABLE_COL_INDEX 0
#define TABLE_COL_MOMENT 1
#define TABLE_COL_MESSAGE 2

class LogTableItemDelegate : public QStyledItemDelegate
{
public:
    const LogItems* items = nullptr;

    LogTableItemDelegate() : QStyledItemDelegate() {}

    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);
        QStyleOptionViewItemV4 *optionV4 = qstyleoption_cast<QStyleOptionViewItemV4*>(option);
        int idx = index.sibling(index.row(), TABLE_COL_INDEX).data().toInt();
        LogItem* item = items->items().at(idx);
        switch (item->type)
        {
        case LogItem::Error:
            optionV4->backgroundBrush = QColor(255, 0, 0, 55);
            break;

        case LogItem::Warning:
            optionV4->backgroundBrush = QColor(255, 153, 0, 55);
            break;

        case LogItem::Debug:
            optionV4->backgroundBrush = QColor(0, 0, 0, 35);
            break;

        case LogItem::Info:
            break;
        }

        switch (index.column())
        {
        case TABLE_COL_INDEX:
            option->text = QString::number(item->index+1);
            option->displayAlignment = Qt::AlignCenter;
            break;
        }
    }
};

//--------------------------------------------------------------------------------------------------

class LogTableModel : public QAbstractTableModel
{
public:
    LogTableModel(const LogItems* items) : _items(items) {}

    int columnCount(const QModelIndex&) const override { return TABLE_COL_COUNT; }
    int rowCount(const QModelIndex&) const override { return _items->items().size(); }

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        return index.isValid() ? Qt::ItemIsEnabled | Qt::ItemIsSelectable : Qt::NoItemFlags;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        static QString number = tr("Num");
        static QString moment = tr("Moment");
        static QString message = tr("Message");

        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
            switch (section)
            {
            case TABLE_COL_INDEX: return number;
            case TABLE_COL_MOMENT: return moment;
            case TABLE_COL_MESSAGE: return message;
            }
        return QVariant();
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        LogItem* item = _items->items().at(index.row());

        if (index.isValid() && role == Qt::DisplayRole)
            switch (index.column())
            {
            case TABLE_COL_INDEX: return item->index;
            case TABLE_COL_MOMENT: return item->moment;
            case TABLE_COL_MESSAGE: return item->header;
            }
        return QVariant();
    }

private:
    const LogItems* _items;
};

//--------------------------------------------------------------------------------------------------

class LogItemFilterProxyModel : public QSortFilterProxyModel
{
public:
    LogItemFilterProxyModel(const LogItems* items, const LogFilters* filters) : _items(items), _filters(filters) {}

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex&) const override
    {
        return !_filters || _filters->accept(_items->items().at(sourceRow));
    }

private:
    const LogItems* _items;
    const LogFilters* _filters;
};

//--------------------------------------------------------------------------------------------------

LogTableWidget::LogTableWidget(QWidget *parent) : Ori::TableWidgetBase(parent)
{
    //hiddenColumns.append(columnIndexId);

    itemDelegate = new LogTableItemDelegate();
}

LogTableWidget::~LogTableWidget()
{
    if (sourceModel) delete sourceModel;
}

void LogTableWidget::adjustHeader()
{
    Ori::Gui::resizeColumnToContent(tableView, TABLE_COL_MOMENT);
    Ori::Gui::stretchColumn(tableView, TABLE_COL_MESSAGE);
    tableView->horizontalHeader()->resizeSection(TABLE_COL_INDEX, 48);
}

void LogTableWidget::populate(const LogItems *items, const LogFilters* filters)
{
    _items = items;
    _filters = filters;
    update();
}

QAbstractItemModel* LogTableWidget::createTableModel()
{
    if (sourceModel) delete sourceModel;
    sourceModel = new LogTableModel(_items);
    if (itemDelegate)
        ((LogTableItemDelegate*)itemDelegate)->items =_items;

    proxyModel = new LogItemFilterProxyModel(_items, _filters);
    proxyModel->setSourceModel(sourceModel);
    return proxyModel;
}

void LogTableWidget::tableCreated()
{
    connect(tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(selectionChanged(QItemSelection,QItemSelection)));
}

int LogTableWidget::filteredRowCount() const
{
    return proxyModel? proxyModel->rowCount(): 0;
}

const LogItem* LogTableWidget::selectedItem()
{
    return item(selectedRow());
}

const LogItem *LogTableWidget::item(int row)
{
    if (row < 0) return nullptr;
    int index = selectedId();
    if (index < 0) return nullptr;
    if (index >= _items->count()) return nullptr;
    return _items->items().at(index);
}

void LogTableWidget::updateFilter()
{
    if (!proxyModel) return;
    proxyModel->invalidate();
    updateHiddenColumns();
}

void LogTableWidget::selectionChanged(const QItemSelection&, const QItemSelection&)
{
    auto it = selectedItem();
    if (it) emit onLogItemSelected(it);
}

LogItems* LogTableWidget::filteredItems() const
{
    auto items = new LogItems;
    if (!proxyModel) return items;

    for (int row = 0; row < proxyModel->rowCount(); row++)
    {
        int index = proxyModel->data(proxyModel->index(row, TABLE_COL_INDEX)).toInt();
        items->append(_items->items().at(index));
    }
    return items;
}
