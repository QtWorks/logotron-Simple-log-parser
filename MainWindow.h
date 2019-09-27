#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <functional>

QT_BEGIN_NAMESPACE
class QBoxLayout;
class QLabel;
class QPlainTextEdit;
QT_END_NAMESPACE

class LogItem;
class LogFilterPanel;
class LogTableWidget;
class LogParams;
class LogProcessor;
class LogItemWidget;
class QuestWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    QTabWidget *_tabs;
    LogTableWidget *_logTable;
    LogFilterPanel *_filterPanel;
    LogProcessor *_processor = nullptr;
    QLabel *_statusPath, *_statusCountFiles, *_statusCountTotal, *_statusCountVisible;
    QLabel *_statusCountInfo, *_statusCountWarning, *_statusCountError, *_statusCountDebug;
    bool _justStarted = true;
    QString _recentPath;
    QPlainTextEdit* _logItemView;
    QDockWidget *_dockRecordText, *_dockfilterPanel;
    QAction *_actionOpenDir;

    void createMenu();
    void createStatusBar();
    QWidget* createStartPage();
    QWidget* createRecordsPage();
    QBoxLayout* makeStartPageCommand(QAction *action);

    void displayEmptyProcessor();
    void displayCurrentProcessor();

    LogItemWidget* recordPage(int i);
    LogItemWidget* recordPageById(int id);
    void closePages();
    void closePage(int index);

    void loadSettings();
    void saveSettings();

    void openLogs(const LogParams &params);

    void showStatus(QLabel* place, const QString& name, int value);

private slots:
    void openLogsDir();
    void showSelectedItem();
    //void showAboutBox();
    void tabCloseRequested(int index);
    void updateFilter();
    void showCurrentItem(const LogItem*);
    void showRegexTool();
    void gotoRecord();
    //void plotRecordIntervals();
    //void plotFilteredRecordIntervals();
};

#endif // MAIN_WINDOW_H
