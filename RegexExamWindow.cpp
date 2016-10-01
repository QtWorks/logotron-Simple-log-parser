#include "RegexExamWindow.h"
#include "helpers/OriWidgets.h"
#include "helpers/OriDialogs.h"
#include "helpers/OriWindows.h"
#include "tools/OriSettings.h"

#include <QBoxLayout>
#include <QDebug>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegExp>

RegexExamWindow::RegexExamWindow(QWidget *parent) : QWidget(parent)
{
    Ori::Wnd::initWindow(this, tr("Regex Test Window"), ":/main_icon", true);
    setObjectName("RegexExamWindow");

    Ori::Gui::layoutV(this, {
                          new QLabel(tr("Processing text:")),
                          _text = new QPlainTextEdit,
                          new QLabel(tr("Regex:")),
                          _code = new QPlainTextEdit,
                          Ori::Gui::layoutH({
                              new QLabel(tr("Search results:")),
                              0,
                              Ori::Gui::button(tr("Find"), this, SLOT(processText())),
                          }),
                          _results = new QPlainTextEdit,
                      });

    Ori::Gui::setFontMonospace(_text);
    Ori::Gui::setFontMonospace(_code);
    Ori::Gui::setFontMonospace(_results);

    Ori::Settings s;
    s.restoreWindowGeometry(this, QSize(500, 500));
    s.beginGroup(objectName());
    _text->setPlainText(s.strValue("Text"));
    _code->setPlainText(s.strValue("Regex"));
}

RegexExamWindow::~RegexExamWindow()
{
    Ori::Settings s;
    s.storeWindowGeometry(this);
    s.beginGroup(objectName());
    s.setValue("Text", _text->toPlainText());
    s.setValue("Regex", _code->toPlainText());
}

void RegexExamWindow::processText()
{
    QRegExp re(_code->toPlainText());
    if (!re.isValid())
    {
        Ori::Dlg::error(re.errorString());
        return;
    }
    _results->clear();
    auto text = _text->toPlainText();
    int index = 0;
    int count = 0;
    while ((index = re.indexIn(text, index)) != -1)
    {
        int len = re.matchedLength();
        _results->appendHtml(QString("<p>pos: %1; len: %2; text: <span style='color:blue'>%3</span>")
                             .arg(index).arg(len).arg(QStringRef(&text, index, len).toString()));
        index += len;
        count++;
    }
    if (count > 0)
        _results->appendPlainText(tr("Matches count: %1").arg(count));
    else
        _results->appendHtml(QString("<p style='color:red'>%1").arg(tr("No matches found")));
}
