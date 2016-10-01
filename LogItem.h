#ifndef LOG_ITEM_H
#define LOG_ITEM_H

#include <QString>
#include <QList>
#include <QRegExp>

class LogItem
{
public:
    enum Type { Info, Warning, Error, Debug };

    int index = 0;
    Type type;
    QString moment;
    QString text;
    QString header;

    LogItem() { type = Info; }
    LogItem(Type t) { type = t; }
    int number() const { return index+1; }
    QString str() const;
    QString typeStr() const;
};

//--------------------------------------------------------------------------------------------------

class LogItems
{
public:
    ~LogItems();
    void append(LogItem* item) { _items.append(item); }
    const QList<LogItem*>& items() const { return _items; }
    int count() const { return _items.size(); }
    QString str() const;
private:
    QList<LogItem*> _items;
};

//--------------------------------------------------------------------------------------------------

class LogFilterBase
{
public:
    virtual ~LogFilterBase() {}
    virtual bool accept(const LogItem*) const = 0;
    void enable(bool on) { _enabled = on; }
    bool enabled() const { return _enabled; }
private:
    bool _enabled = true;
};

//--------------------------------------------------------------------------------------------------

class LogItemTypeFilter : public LogFilterBase
{
public:
    LogItemTypeFilter(LogItem::Type type): _type(type) {}

    bool accept(const LogItem *item) const override
    {
        return enabled() && item->type == _type;
    }
private:
    LogItem::Type _type;
};

//--------------------------------------------------------------------------------------------------

class LogItemTextFilter : public LogFilterBase
{
public:
    const QString& text() const { return _text; }
    void setText(const QString& text) { _text = text; updateRegexp(); }
    void setUseRegex(bool use) {  _useRegex = use; updateRegexp(); }
    bool useRegex() const { return _useRegex; }
protected:
    QString _text;
    QRegExp _regex;
    bool _useRegex = false;
    void updateRegexp() { if (_useRegex) _regex = QRegExp(_text); }
};

//--------------------------------------------------------------------------------------------------

class LogItemTextIncludingFilter : public LogItemTextFilter
{
public:
    bool accept(const LogItem *item) const override
    {
        if (!enabled() || _text.isEmpty()) return true;

        bool contains = _useRegex
                ? item->text.contains(_regex)
                : item->text.contains(_text, Qt::CaseInsensitive);

        return contains;
    }
};

//--------------------------------------------------------------------------------------------------

class LogItemTextExcludingFilter : public LogItemTextFilter
{
public:
    bool accept(const LogItem *item) const override
    {
        if (!enabled() || _text.isEmpty()) return true;

        bool contains = _useRegex
                ? item->text.contains(_regex)
                : item->text.contains(_text, Qt::CaseInsensitive);

        return !contains;
    }
};

//--------------------------------------------------------------------------------------------------

typedef QList<LogFilterBase*> FilterList;
typedef QList<LogFilterBase*>* PFilterList;

class LogFilters
{
public:
    ~LogFilters();

    PFilterList including() { return &_includingFilters; }
    PFilterList excluding() { return &_excludingFilters; }
    PFilterList searching() { return &_searchingFilters; }

    bool accept(const LogItem* item) const;

private:
    FilterList _includingFilters;
    FilterList _excludingFilters;
    FilterList _searchingFilters;
};

//--------------------------------------------------------------------------------------------------

#endif // LOG_ITEM_H
