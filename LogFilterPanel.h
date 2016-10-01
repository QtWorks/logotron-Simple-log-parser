#ifndef LOG_FILTER_PANEL_H
#define LOG_FILTER_PANEL_H

#include <QCheckBox>

#include "LogItem.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QMenu;
class QVBoxLayout;
QT_END_NAMESPACE

class LogItemTypeFilterView : public QCheckBox
{
    Q_OBJECT

public:
    LogItemTypeFilterView(LogFilterBase* filter, const QString& title);

private:
    LogFilterBase *_filter;

signals:
    void changed();

private slots:
    void applyFilter(bool on);
};

//--------------------------------------------------------------------------------------------------

class LogItemTextFilterView : public QWidget
{
    Q_OBJECT

public:
    LogItemTextFilterView(LogItemTextFilter* filter, PFilterList targetList);

    LogItemTextFilter* filter() const { return _filter; }
    PFilterList targetList() const { return _targetList; }

signals:
    void changed();
    void removeRequested(LogItemTextFilterView*);

public slots:
    void editFilter();

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    LogItemTextFilter *_filter;
    PFilterList _targetList;
    QCheckBox* _flag;
    QLabel* _text;
    QMenu* _menu;

private slots:
    void applyFilter(bool on);
    void removeFilter();
};

//--------------------------------------------------------------------------------------------------

class LogFilterPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LogFilterPanel(QWidget *parent = 0);

    const LogFilters* filters() const { return &_filters; }

signals:
    void changed();

private:
    LogFilters _filters;
    QVBoxLayout *_excludingFilters, *_searchingFilters;

    LogItemTypeFilterView* makeItemTypeFilter(LogItem::Type type, const QString& title);

private slots:
    void raiseChanged();
    void appendExcludingFilter();
    void appendSearchingFilter();
    void removeTextFilter(LogItemTextFilterView*);
    void appendTextFilter(LogItemTextFilter* filter, PFilterList targetList, QVBoxLayout* targetPlace);
};

#endif // LOG_FILTER_PANEL_H
