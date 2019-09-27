#include "LogFilterPanel.h"

#include "helpers/OriWidgets.h"
#include "helpers/OriDialogs.h"
#include "helpers/OriLayouts.h"
#include "tools/OriSettings.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QBoxLayout>
#include <QDebug>
#include <QGroupBox>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>

using namespace Ori::Layouts;

LogItemTypeFilterView::LogItemTypeFilterView(LogFilterBase *filter, const QString& title)
{
    _filter = filter;
    setText(title);
    setChecked(_filter->enabled());
    connect(this, SIGNAL(toggled(bool)), this, SLOT(applyFilter(bool)));
}

void LogItemTypeFilterView::applyFilter(bool on)
{
    _filter->enable(on);
    emit changed();
}

//--------------------------------------------------------------------------------------------------

LogItemTextFilterView::LogItemTextFilterView(LogItemTextFilter* filter, PFilterList targetList)
{
    _filter = filter;
    _targetList = targetList;

    _menu = new QMenu(this);
    _menu->addAction(tr("Edit..."), this, SLOT(editFilter()));
    _menu->addAction(tr("Remove"), this, SLOT(removeFilter()));

    LayoutH({
                _flag = new QCheckBox,
                _text = new QLabel,
                Stretch()
            }).setMargin(0).setSpacing(6).useFor(this);

    _flag->setChecked(_filter->enabled());
    _text->setText(_filter->text());
    _text->setWordWrap(true);

    connect(_flag, SIGNAL(toggled(bool)), this, SLOT(applyFilter(bool)));
}

void LogItemTextFilterView::applyFilter(bool on)
{
    _filter->enable(on);
    emit changed();
}

void LogItemTextFilterView::editFilter()
{
    auto useRegex = new QCheckBox(tr("Treat as regular expression"));
    auto textEdit = new QPlainTextEdit;
    useRegex->setChecked(_filter->useRegex());
    textEdit->setPlainText(_filter->text());
    Ori::Gui::setFontMonospace(textEdit);
    Ori::Dlg::Dialog d(Ori::Gui::widgetV({ useRegex, textEdit }));
    //Ori::Settings::restoreWindow("TextFilterDialog", &d);
    textEdit->setFocus();
    if (d.exec())
    {
        //Ori::Settings::storeWindow("TextFilterDialog", &d);
        auto text = textEdit->toPlainText().trimmed();
        if (!text.isEmpty() && text != _filter->text())
        {
            _text->setText(text);
            _filter->setText(text);
            _filter->setUseRegex(useRegex->isChecked());
            emit changed();
        }
    }
}

void LogItemTextFilterView::removeFilter()
{
    if (Ori::Dlg::yes(tr("Remove filter '%1'").arg(_filter->text())))
        emit removeRequested(this);
}

void LogItemTextFilterView::contextMenuEvent(QContextMenuEvent* event)
{
    event->accept();
    _menu->exec(event->globalPos());
}

//--------------------------------------------------------------------------------------------------

QLabel* headerLabel(const QString& text)
{
    auto label = new QLabel(text);
    auto font = label->font();
    font.setBold(true);
    label->setFont(font);
    return label;
}

//--------------------------------------------------------------------------------------------------

LogFilterPanel::LogFilterPanel(QWidget *parent) : QWidget(parent)
{
    LayoutV({
        headerLabel(tr("Include")),
        makeItemTypeFilter(LogItem::Info, tr("Info")),
        makeItemTypeFilter(LogItem::Warning, tr("Warning")),
        makeItemTypeFilter(LogItem::Error, tr("Error")),
        makeItemTypeFilter(LogItem::Debug, tr("Debug")),
        Space(6),
        _searchingFilters = new QVBoxLayout,
        Ori::Gui::button(tr("Append..."), this, SLOT(appendSearchingFilter())),
        Space(6),
        headerLabel(tr("Exclude")),
        _excludingFilters = new QVBoxLayout,
        Ori::Gui::button(tr("Append..."), this, SLOT(appendExcludingFilter())),
        Stretch(),
    }).useFor(this);
}

LogItemTypeFilterView* LogFilterPanel::makeItemTypeFilter(LogItem::Type type, const QString& title)
{
    auto filter = new LogItemTypeFilter(type);
    auto view = new LogItemTypeFilterView(filter, title);
    connect(view, SIGNAL(changed()), this, SLOT(raiseChanged()));
    _filters.including()->append(filter);
    return view;
}

void LogFilterPanel::raiseChanged()
{
    emit changed();
}

void LogFilterPanel::appendExcludingFilter()
{
    appendTextFilter(new LogItemTextExcludingFilter, _filters.excluding(), _excludingFilters);
}

void LogFilterPanel::appendSearchingFilter()
{
    appendTextFilter(new LogItemTextIncludingFilter, _filters.searching(), _searchingFilters);
}

void LogFilterPanel::appendTextFilter(LogItemTextFilter* filter, PFilterList targetList, QVBoxLayout *targetPlace)
{
    auto view = new LogItemTextFilterView(filter, targetList);
    view->editFilter();
    if (filter->text().isEmpty())
    {
        delete view;
        delete filter;
        return;
    }
    connect(view, SIGNAL(changed()), this, SLOT(raiseChanged()));
    connect(view, SIGNAL(removeRequested(LogItemTextFilterView*)), this, SLOT(removeTextFilter(LogItemTextFilterView*)));
    targetList->append(filter);
    targetPlace->addWidget(view);
    raiseChanged();
}

void LogFilterPanel::removeTextFilter(LogItemTextFilterView* view)
{
    view->targetList()->removeOne(view->filter());
    view->deleteLater();
    delete view->filter();
    raiseChanged();
}

