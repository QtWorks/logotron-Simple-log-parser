#include "MainWindow.h"
#include "LogFilterPanel.h"
#include "LogTableWidget.h"
#include "LogProcessor.h"
#include "LogItemWidget.h"
#include "OpenFilesDialog.h"
#include "RegexExamWindow.h"
#include "helpers/OriWindows.h"
#include "helpers/OriWidgets.h"
#include "tools/OriSettings.h"
#include "tools/OriWaitCursor.h"
#include "widgets/OriBackWidget.h"

#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QDebug>
#include <QDir>
#include <QDockWidget>
#include <QInputDialog>
#include <QFileDialog>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QTextBrowser>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    Ori::Wnd::setWindowIcon(this, ":/main_icon");
    loadSettings();

    createMenu();
    createStatusBar();

    _logTable = new LogTableWidget;
    connect(_logTable, SIGNAL(onDoubleClick()), this, SLOT(showSelectedItem()));
    connect(_logTable, SIGNAL(onLogItemSelected(const LogItem*)), this, SLOT(showCurrentItem(const LogItem*)));

    _tabs = new QTabWidget(this);
    _tabs->setVisible(false);
    _tabs->addTab(createRecordsPage(), tr("Log records"));
    _tabs->setTabsClosable(true);
    connect(_tabs, SIGNAL(tabCloseRequested(int)), this, SLOT(tabCloseRequested(int)));

    setCentralWidget(createStartPage());

    _filterPanel = new LogFilterPanel(this);
    _dockfilterPanel = new QDockWidget(tr("Filters"));
    _dockfilterPanel->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    _dockfilterPanel->setWidget(_filterPanel);
    _dockfilterPanel->setVisible(false);
    addDockWidget(Qt::LeftDockWidgetArea, _dockfilterPanel);
    connect(_filterPanel, SIGNAL(changed()), this, SLOT(updateFilter()));

    _logItemView = Ori::Gui::logView(11);
    _dockRecordText = new QDockWidget(tr("Record"));
    _dockRecordText->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetVerticalTitleBar);
    _dockRecordText->setWidget(_logItemView);
    _dockRecordText->setVisible(false);
    addDockWidget(Qt::BottomDockWidgetArea, _dockRecordText);
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::loadSettings()
{
    Ori::Settings s;
    s.restoreWindowGeometry("MainWindow", this);
    s.beginDefaultGroup();
    _recentPath = s.strValue("RecentPath");
}

void MainWindow::saveSettings()
{
    Ori::Settings s;
    s.storeWindowGeometry("MainWindow", this);
    s.beginDefaultGroup();
    s.setValue("RecentPath", _recentPath);
}

void MainWindow::createMenu()
{
    QMenu *menu = menuBar()->addMenu(tr("File"));
    _actionOpenDir = menu->addAction(QIcon(":/open"), tr("Open Logs Directory..."), this, SLOT(openLogsDir()), QKeySequence::Open);

    menu = menuBar()->addMenu("Log");
    menu->addAction(tr("Go To Record Number..."), this, SLOT(gotoRecord()), QKeySequence("Ctrl+G"));

    menu = menuBar()->addMenu(tr("Tools"));
    menu->addAction(tr("Play With Regex"), this, SLOT(showRegexTool()));
}

QBoxLayout* MainWindow::makeStartPageCommand(QAction* action)
{
    auto icon = new QLabel;
    icon->setPixmap(action->icon().pixmap(48, 48));

    auto link = new QLabel(QString::fromLatin1("<a href='cmd://dummy'>%1</a>").arg(action->text()));
    connect(link, SIGNAL(linkActivated(QString)), action, SLOT(trigger()));
    Ori::Gui::adjustFont(link);

    return Ori::Gui::layoutH({icon, link});
}

QWidget* MainWindow::createStartPage()
{
    auto w = new Ori::Widgets::BackWidget(":/background");
    Ori::Gui::layoutH(w, {0, Ori::Gui::layoutV({ 0, makeStartPageCommand(_actionOpenDir), 0 }), 0 });
    return w;
}

QWidget* MainWindow::createRecordsPage()
{
    return Ori::Gui::widget(Ori::Gui::layoutV(3, 3, {_logTable}));
}

void MainWindow::createStatusBar()
{
    setStatusBar(new QStatusBar);
    statusBar()->addWidget(_statusCountFiles = new QLabel);
    statusBar()->addWidget(_statusCountTotal = new QLabel);
    statusBar()->addWidget(_statusCountInfo = new QLabel);
    statusBar()->addWidget(_statusCountWarning = new QLabel);
    statusBar()->addWidget(_statusCountError = new QLabel);
    statusBar()->addWidget(_statusCountDebug = new QLabel);
    statusBar()->addWidget(_statusCountVisible = new QLabel);
    statusBar()->addWidget(_statusPath = new QLabel);
}

void MainWindow::tabCloseRequested(int index)
{
    if (index > 0) closePage(index);
}

void MainWindow::closePages()
{
    while (_tabs->count() > 1)
        closePage(_tabs->count()-1);
}

void MainWindow::closePage(int index)
{
    QWidget *page = _tabs->widget(index);
    _tabs->removeTab(index);
    delete page;
}

void MainWindow::openLogsDir()
{
    auto params = OpenFilesDialog::selectFiles(this);
    if (params.ok()) openLogs(params);
}

void MainWindow::openLogs(const LogParams& params)
{
    auto processor = new LogProcessor(this);
    if (!processor->open(params)) return;

    _recentPath.clear();
    closePages();
    _processor = processor;
    if (!_processor)
        displayEmptyProcessor();
    else
        displayCurrentProcessor();
    _logTable->populate(_processor->log(), _filterPanel->filters());
    _recentPath = _processor->path();

    if (_justStarted)
    {
        _dockfilterPanel->setVisible(true);
        _dockRecordText->setVisible(true);
        setCentralWidget(_tabs);
        _tabs->setVisible(true);
        _justStarted = false;
    }
}

void MainWindow::displayEmptyProcessor()
{
    _statusCountFiles->clear();
    _statusCountTotal->clear();
    _statusPath->clear();
}

void MainWindow::showStatus(QLabel* place, const QString& name, int value)
{
    place->setText("  " % name % "  " % QString::number(value) % "  ");
}

void MainWindow::displayCurrentProcessor()
{
    showStatus(_statusCountFiles, tr("Files:"), _processor->filesCount());
    showStatus(_statusCountTotal, tr("Records:"), _processor->recordsCount());
    showStatus(_statusCountInfo, tr("Info:"), _processor->countByType()[LogItem::Info]);
    showStatus(_statusCountWarning, tr("Warning:"), _processor->countByType()[LogItem::Warning]);
    showStatus(_statusCountError, tr("Error:"), _processor->countByType()[LogItem::Error]);
    showStatus(_statusCountDebug, tr("Debug:"), _processor->countByType()[LogItem::Debug]);
    showStatus(_statusCountVisible, tr("Visible:"), _logTable->filteredRowCount());
    _statusPath->setText("  " % _processor->path() % "  ");
}

void MainWindow::showSelectedItem()
{
    auto item = _logTable->selectedItem();
    if (!item) return;

    auto page = recordPageById(item->index);
    if (!page)
    {
        page = new LogItemWidget(item);
        _tabs->addTab(page, QString("[%1] %2").arg(item->number()).arg(item->moment));
    }
    _tabs->setCurrentWidget(page);
}

LogItemWidget* MainWindow::recordPage(int i)
{
    return qobject_cast<LogItemWidget*>(_tabs->widget(i));
}

LogItemWidget* MainWindow::recordPageById(int id)
{
    for (int i = 0; i < _tabs->count(); i++)
    {
       auto page = recordPage(i);
       if (page && page->item()->index == id)
       {
           _tabs->setCurrentIndex(i);
           return page;
       }
    }
    return nullptr;
}

//void MainWindow::showAboutBox()
//{
//    QString revision = SVN_REV;
//    QMessageBox::about(this, tr("About %1").arg(qApp->applicationName()), QString::fromUtf8(
//        "<p><font size=4><b>%1</b></font><p>Версия %2.%3.%4<p>Основано на Qt %5<p>Собрано %6 %7<br>&nbsp;")
//        .arg(qApp->applicationName()).arg(APP_VER_MAJOR).arg(APP_VER_MINOR)
//        .arg(revision.contains(':')? QString(revision).section(':', 1): revision)
//        .arg(QT_VERSION_STR).arg(BUILDDATE).arg(BUILDTIME));
//}

void MainWindow::updateFilter()
{
    Ori::WaitCursor wc;
    _logTable->updateFilter();
    showStatus(_statusCountVisible, tr("Visible:"), _logTable->filteredRowCount());
}

void MainWindow::showCurrentItem(const LogItem* item)
{
    _logItemView->setText(item->text);
    _dockRecordText->setWindowTitle(tr("Record [%1]").arg(item->number()));
}

void MainWindow::showRegexTool()
{
    (new RegexExamWindow)->show();
}

void MainWindow::gotoRecord()
{
    static int savedIndex = 0;
    int index = QInputDialog::getInt(this, tr("Go To Record"), tr("Record number:"), savedIndex);
    if (index > 0)
    {
        savedIndex = index;
        _logTable->setSelectedId(index-1);
    }
}
