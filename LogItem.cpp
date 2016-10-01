#include "LogItem.h"

#include <QDebug>

//--------------------------------------------------------------------------------------------------

QString LogItem::typeStr() const
{
    switch (type)
    {
    case Info: return "INFO";
    case Warning: return "WARNING";
    case Error: return "ERROR";
    case Debug: return "DEBUG";
    }
    return QString();
}

QString LogItem::str() const
{
    return QString("%1 [%2]: %3").arg(moment, typeStr(), header);
}

//--------------------------------------------------------------------------------------------------

LogItems::~LogItems()
{
    for (LogItem* i : _items) delete i;
}

QString LogItems::str() const
{
    QStringList messages;
    for (const LogItem* it: _items)
        messages.append(it->str());
    return messages.join("\n");
}

//--------------------------------------------------------------------------------------------------

LogFilters::~LogFilters()
{
   for (LogFilterBase* f : _includingFilters) delete f;
   for (LogFilterBase* f : _excludingFilters) delete f;
   for (LogFilterBase* f : _searchingFilters) delete f;
}

bool LogFilters::accept(const LogItem *item) const
{
    for (LogFilterBase* f : _excludingFilters)
        if (!f->accept(item)) return false;

    if (_includingFilters.isEmpty())
        return true;

    for (LogFilterBase* f : _includingFilters)
        if (f->accept(item))
        {
            if (!_searchingFilters.isEmpty())
                for (LogFilterBase* f : _searchingFilters)
                    if (!f->accept(item)) return false;
            return true;
        }

    return false;
}
