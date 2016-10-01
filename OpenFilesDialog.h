#ifndef OPEN_FILES_DIALOG_H
#define OPEN_FILES_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QComboBox>

#include "LogProcessor.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QListWidget;
class QPlainTextEdit;
class QSettings;
class QTextEdit;
QT_END_NAMESPACE

//--------------------------------------------------------------------------------------------------

class PreviewFileReader : public FileReader
{
public:
    PreviewFileReader(QPlainTextEdit* target, int maxLines, const QString& file, const QString& encoding);

    bool processLine(const QString& line) override;

private:
    QPlainTextEdit* _target;
    int _maxLines, _lineCount = 0;
};

//--------------------------------------------------------------------------------------------------

class PersistentCombo : public QComboBox
{
    Q_OBJECT

public:
    explicit PersistentCombo(const QString& settingPrefix, bool editable = true);

    void load(QSettings* s, const QString &defaultValue = QString());
    void save(QSettings* s);

    void append(const QString& item, bool repopulate = true);

    QSize sizeHint() const override;

    void setPreferredWidth(int w) { _preferredWidth = w; }

private:
    QStringList _storedItems;
    QString _keyItems, _keyCurrent;
    int _preferredWidth = 0;

    void populate(const QString& current);
};

//--------------------------------------------------------------------------------------------------

class HeaderLabel : public QLabel
{
public:
    explicit HeaderLabel(const QString& title);
};

//--------------------------------------------------------------------------------------------------

class OpenFilesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OpenFilesDialog(QWidget *parent = 0);
    ~OpenFilesDialog();

    static LogParams selectFiles(QWidget *parent);

    LogParams result() const;

protected:
    void timerEvent(QTimerEvent*);

private slots:
    void selectDirectory();
    void selectFilesAll();
    void selectFilesNone();
    void selectFilesInvert();
    void populateFileList();
    void filterChanged();
    void tabActivated(int index);
    void testParsing();

private:
    PersistentCombo* _comboPath;
    QListWidget* _listFiles;
    PersistentCombo* _filterEdit;
    PersistentCombo *_leftMarker, *_rightMarker;
    QCheckBox *_leftMarkerRegexp, *_rightMarkerRegexp;
    QCheckBox *_caseSensitiveFiles;
    PersistentCombo *_encoding;
    QPlainTextEdit *_logPreview, *_parseResults;
    QLabel* _logPreviewTitle;
    qint64 _lastTimerTick = 0;
    int _updateFilterTimerId = 0;

    void saveState();
    void restoreState();
    QString selectedDir() const;
    QString selectedFilter() const;
    QStringList selectedFiles() const;
    QString selectedEncoding() const;
    QString selectedLeftMarker() const;
    QString selectedRightMarker() const;
    bool selectedLeftMarkerRegexp() const;
    bool selectedRightMarkerRegexp() const;
    LogMarkersParams selectedMarkerParams() const;

    void loadLogPreview();
    void sortFiles(QStringList& files) const;
};

#endif // OPEN_FILES_DIALOG_H
