#include "LogProcessor.h"
#include "helpers/OriDialogs.h"
#include "tools/OriSettings.h"
#include "tools/OriWaitCursor.h"

#include <QApplication>
#include <QDebug>
#include <QDir>

//--------------------------------------------------------------------------------------------------

LogMarker::LogMarker(const LogMarkerParams& params)
{
    if (params.regexp)
        regexp = QRegExp(params.marker, Qt::CaseInsensitive);
    else
    {
        marker = params.marker;
        len = marker.length();
    }
    simple = !params.regexp;
}

QString LogMarker::validate() const
{
    if (simple)
    {
        if (marker.isEmpty())
            return qApp->tr("Marker is empty");
    }
    else
    {
        if (regexp.isEmpty() || !regexp.isValid())
            return qApp->tr("Invalid regular expression");
    }
    return QString();
}

bool LogMarker::process(const QString& s, int offset)
{
    if (simple)
    {
        pos = s.indexOf(marker, offset);
        return pos > -1;
    }
    else
    {
        pos = regexp.indexIn(s, offset);
        if (pos > -1)
        {
            len = regexp.matchedLength();
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------

FileReader::FileReader(const QString& file, const QString& encoding): _file(file), _encoding(encoding)
{
}

QString FileReader::read()
{
    QFile input(_file);
    if (!input.open(QIODevice::Text | QIODevice::ReadOnly))
        return input.errorString();

    QString res = processStart();
    if (!res.isEmpty()) return res;

    QTextStream stream(&input);
    stream.setCodec(_encoding.toStdString().data());
    stream.setAutoDetectUnicode(true);

    QString line("");
    while (!line.isNull())
    {
        line = stream.readLine();
        if (!line.isEmpty())
            if (!processLine(line))
                break;
    }
    processDone();

    return _errors.isEmpty()? QString(): _errors.join("\n");
}

//--------------------------------------------------------------------------------------------------

LogFileReader::LogFileReader(LogMarkersParams* params, LogItems* log, const QString& file, const QString& encoding)
    : FileReader(file, encoding), _log(log), _params(params)
{
}

QString LogFileReader::processStart()
{
    _leftMarker = LogMarker(_params->left);
    QString leftErr = _leftMarker.validate();
    if (!leftErr.isEmpty())
        return qApp->tr("Invalid left marker: %1").arg(leftErr);

    _rightMarker = LogMarker(_params->right);
    QString rightErr = _rightMarker.validate();
    if (!rightErr.isEmpty())
        return qApp->tr("Invalid right marker: %1").arg(rightErr);

    return QString();
}

bool LogFileReader::processLine(const QString& line)
{
    auto item = newItem(line);
    if (item)
    {
        finishItem();
        _item = item;
    }
    _message.append(line);
    return true;
}

void LogFileReader::processDone()
{
    finishItem();
}

LogItem* LogFileReader::newItem(const QString& s)
{
    if (!_leftMarker.process(s)) return nullptr;
    int markerStart = _leftMarker.pos + _leftMarker.len;

    if (!_rightMarker.process(s, markerStart)) return nullptr;
    int markerEnd = _rightMarker.pos;

    auto item = makeItem(s, markerStart, markerEnd);
    if (item)
    {
        item->moment = s.leftRef(markerStart-1).trimmed().toString();
        item->header = s.rightRef(s.length()-markerEnd-1).trimmed().toString();
    }
    return item;
}

LogItem* LogFileReader::makeItem(const QString& s, int markerStart, int markerEnd) const
{
    static QString markerError("error");
    static QString markerInfo("info");
    static QString markerDebug("debug");
    static QString markerWarning("warning");

    QStringRef marker(&s, markerStart, markerEnd - markerStart);

    if (marker.compare(markerError, Qt::CaseInsensitive) == 0)
        return new LogItem(LogItem::Error);

    if (marker.compare(markerInfo, Qt::CaseInsensitive) == 0)
        return new LogItem(LogItem::Info);

    if (marker.compare(markerDebug, Qt::CaseInsensitive) == 0)
        return new LogItem(LogItem::Debug);

    if (marker.compare(markerWarning, Qt::CaseInsensitive) == 0)
        return new LogItem(LogItem::Warning);

    return nullptr;
}

void LogFileReader::finishItem()
{
    if (!_item) return;

    _item->text = _message.join('\n');
    _message.clear();

    _log->append(_item);
    _item->index = _log->items().size()-1;

    auto type = _item->type;
    int count = _countByType.contains(type)? _countByType[type]: 0;
    _countByType[type] = count+1;
}

//--------------------------------------------------------------------------------------------------

LogProcessor::LogProcessor(QObject* parent) : QObject(parent)
{
}

bool LogProcessor::open(const LogParams &params)
{
    if (params.files.empty()) return false;

    _params = params;
    _path = QFileInfo(params.files.first()).absolutePath();

    Ori::WaitCursor wait;

    _filesCount = 0;
    for (const QString& file: params.files)
    {
        QString res = processFile(file);
        if (!res.isEmpty())
        {
            Ori::Dlg::error(tr("Error while processing file\n%1:\n\n%2\n\nFile is skipped").arg(file, res));
            continue;
        }
        _filesCount++;
    }
    return true;
}

QString LogProcessor::processFile(const QString& file)
{
    LogFileReader reader(&_params.marker, &_log, file, _params.encoding);
    QString res = reader.read();
    _countByType = reader.countByType();
    return res;
}


