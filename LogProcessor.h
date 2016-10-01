#ifndef LOG_PROCESOR_H
#define LOG_PROCESOR_H

#include <QObject>
#include <QMap>
#include <QStringList>

#include "LogItem.h"

//--------------------------------------------------------------------------------------------------

struct LogMarkerParams
{
    QString marker;
    bool regexp;
};

struct LogMarkersParams
{
    LogMarkerParams left;
    LogMarkerParams right;
};

struct LogMarker
{
    QRegExp regexp;
    QString marker;
    bool simple;

    int pos;
    int len;

    LogMarker() {}
    LogMarker(const LogMarkerParams& params);
    bool process(const QString& s, int offset = 0);
    QString validate() const;
};

struct LogParams
{
    QString encoding;
    QStringList files;
    LogMarkersParams marker;

    bool ok() const { return !files.empty(); }
};

//--------------------------------------------------------------------------------------------------

class FileReader
{
public:
    FileReader(const QString& file, const QString& encoding);

    QString read();

protected:
    virtual QString processStart() { return QString(); }
    virtual bool processLine(const QString&) { return true; }
    virtual void processDone() {}

    void addError(const QString& s) { _errors.append(s); }

private:
    QString _file, _encoding;
    QStringList _errors;
};

//--------------------------------------------------------------------------------------------------

class LogFileReader : public FileReader
{
public:
    LogFileReader(LogMarkersParams* params, LogItems* log, const QString& file, const QString& encoding);

    const QMap<LogItem::Type, int>& countByType() const { return _countByType; }

protected:
    QString processStart() override;
    bool processLine(const QString& line) override;
    void processDone() override;

    virtual LogItem* newItem(const QString& s);

    LogItem* makeItem(const QString& s, int markerStart, int markerEnd) const;

private:
    LogItems* _log;
    QStringList _message;
    LogItem* _item = nullptr;
    LogMarkersParams* _params;
    LogMarker _leftMarker, _rightMarker;
    QMap<LogItem::Type, int> _countByType;

    void finishItem();
};

//--------------------------------------------------------------------------------------------------

class LogProcessor : public QObject
{
    Q_OBJECT

public:
    LogProcessor(QObject* parent = 0);

    const QString& path() const { return _path; }
    const LogItems* log() const { return &_log; }
    int filesCount() const { return _filesCount; }
    int recordsCount() const { return _log.items().size(); }
    const QMap<LogItem::Type, int>& countByType() const { return _countByType; }

    bool open(const LogParams& params);

private:
    QString _path;
    int _filesCount;
    LogItems _log;
    LogParams _params;
    QMap<LogItem::Type, int> _countByType;

    QString processFile(const QString& file);
    void addItem(LogItem* item, QStringList& strs);
};

//--------------------------------------------------------------------------------------------------

#endif // LOG_PROCESOR_H
