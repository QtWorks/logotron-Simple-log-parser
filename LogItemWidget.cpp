#include "LogItemWidget.h"
#include "LogItem.h"
#include "helpers/OriWidgets.h"

#include <QTextBrowser>
#include <QVBoxLayout>

LogItemWidget::LogItemWidget(const LogItem* item) : _item(item)
{
    Ori::Gui::layoutV(this, 3, 3, { _browser = Ori::Gui::logView(11) });

    _browser->setText(item->text);
}
