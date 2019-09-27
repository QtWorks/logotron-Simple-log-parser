#ifndef LOG_RECORD_WIDGET_H
#define LOG_RECORD_WIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QPlainTextEdit;
QT_END_NAMESPACE

class LogItem;

class LogItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LogItemWidget(const LogItem* item);

    const LogItem* item() const { return _item; }

private:
    QPlainTextEdit* _browser;
    const LogItem* _item;
};

#endif // LOG_RECORD_WIDGET_H
