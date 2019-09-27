#include "LogItemWidget.h"
#include "LogItem.h"
#include "helpers/OriWidgets.h"

#include <QPlainTextEdit>
#include <QVBoxLayout>

LogItemWidget::LogItemWidget(const LogItem* item) : _item(item)
{
    _browser = new QPlainTextEdit;
    Ori::Gui::setFontMonospace(_browser);

    Ori::Gui::layoutV(this, 3, 3, { _browser });

    _browser->setPlainText(item->text);
}
