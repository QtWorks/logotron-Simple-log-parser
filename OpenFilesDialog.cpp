#include "OpenFilesDialog.h"
#include "helpers/OriWidgets.h"
#include "helpers/OriDialogs.h"
#include "tools/OriSettings.h"
#include "helpers/OriLayouts.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QDateTime>
#include <QDebug>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QToolButton>

#define TAB_LOG_FORMAT 1

//--------------------------------------------------------------------------------------------------

PreviewFileReader::PreviewFileReader(QPlainTextEdit* target, int maxLines, const QString& file, const QString& encoding)
    : FileReader(file, encoding), _target(target), _maxLines(maxLines)
{
}

bool PreviewFileReader::processLine(const QString& line)
{
    auto text = _target->toPlainText();
    _target->setPlainText(text.isEmpty()? line: (text + "\n" + line));
    return ++_lineCount < _maxLines;
}

//--------------------------------------------------------------------------------------------------

PersistentCombo::PersistentCombo(const QString& settingPrefix, bool editable)
{
    _keyItems = settingPrefix + ".Items";
    _keyCurrent = settingPrefix + ".Current";

    setEditable(editable);
    setMaxVisibleItems(32);
}

void PersistentCombo::load(QSettings* s, const QString &defaultValue)
{
    _storedItems = s->value(_keyItems).toStringList();
    populate(s->value(_keyCurrent, defaultValue).toString());
}

void PersistentCombo::save(QSettings* s)
{
    s->setValue(_keyItems, _storedItems);
    s->setValue(_keyCurrent, currentText());
}

void PersistentCombo::populate(const QString &current)
{
    blockSignals(true);
    for (const QString& item : _storedItems)
        addItem(item);
    blockSignals(false);

    if (!current.isEmpty())
        setCurrentText(current);
    if (currentText().isEmpty())
        setCurrentIndex(0);
}

void PersistentCombo::append(const QString& item, bool repopulate)
{
    if (_storedItems.contains(item))
        _storedItems.removeAll(item);
    _storedItems.insert(0, item);

    if (repopulate) populate(item);
}

QSize PersistentCombo::sizeHint() const
{
    auto sz = QComboBox::sizeHint();
    if (_preferredWidth > 0)
        sz.setWidth(_preferredWidth);
    return sz;
}

//--------------------------------------------------------------------------------------------------

HeaderLabel::HeaderLabel(const QString& title) : QLabel(title)
{
    auto f = font();
    f.setBold(true);
    setFont(f);
}

//--------------------------------------------------------------------------------------------------

LogParams OpenFilesDialog::selectFiles(QWidget* parent)
{
    OpenFilesDialog dlg(parent);
    return Ori::Dlg::show(&dlg)? dlg.result(): LogParams();
}

OpenFilesDialog::OpenFilesDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Open log files"));
    setObjectName("OpenFilesDialog");

    auto tabs = new QTabWidget;
    tabs->addTab(Ori::Gui::widgetV
                 ({
                      Ori::Gui::layoutH({
                          _comboPath = new PersistentCombo("Dirs", false),
                          Ori::Gui::iconToolButton(tr("Select directory"), ":/open_folder", 24, this, SLOT(selectDirectory())),
                      }),
                      Ori::Gui::layoutH({
                          _listFiles = new QListWidget,
                          Ori::Gui::layoutV({
                              Ori::Gui::iconToolButton(tr("Select all files"), ":/check_all", 24, this, SLOT(selectFilesAll())),
                              Ori::Gui::iconToolButton(tr("Deselect all files"), ":/check_none", 24, this, SLOT(selectFilesNone())),
                              //Ori::Gui::iconToolButton(tr("Invert selection"), ":/check_invert", 24, this, SLOT(selectFilesInvert())),
                              0
                          })
                      }),
                      Ori::Gui::layoutH({
                          new QLabel(tr("Filter:")),
                          _filterEdit = new PersistentCombo("Filter"),
                          _caseSensitiveFiles = new QCheckBox(tr("Case sensitive file list")),
                          0
                      })
                  }),
                 tr("Files"));
    tabs->addTab(Ori::Gui::widgetV
                 ({

                      new HeaderLabel(tr("Encoding:")),
                      _encoding = new PersistentCombo("Encoding"),
                      Ori::Gui::defaultSpacing(),
                      new HeaderLabel(tr("Message type left marker:")),
                      Ori::Gui::layoutH
                      ({
                          _leftMarker = new PersistentCombo("LeftMarker"),
                          _leftMarkerRegexp = new QCheckBox(tr("Regex"))
                      }),
                      Ori::Gui::defaultSpacing(),
                      new HeaderLabel(tr("Message type right marker:")),
                      Ori::Gui::layoutH
                      ({
                          _rightMarker = new PersistentCombo("RightMarker"),
                          _rightMarkerRegexp = new QCheckBox(tr("Regex"))
                      }),
                      Ori::Gui::defaultSpacing(),
                      new HeaderLabel(tr("Preview:")),
                      _logPreviewTitle = new QLabel(tr("(Select a file on the tab 'Files' to preview)")),
                      _logPreview = new QPlainTextEdit,
                      Ori::Gui::layoutH
                      ({
                          new HeaderLabel(tr("Parsing result:")),
                          0,
                          Ori::Gui::button(tr("Test"), this, SLOT(testParsing()))
                      }),
                      _parseResults = new QPlainTextEdit
                  }),
                 tr("Format"));

    _filterEdit->setPreferredWidth(150);
    _leftMarkerRegexp->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    _rightMarkerRegexp->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    Ori::Gui::setFontMonospace(_leftMarker);
    Ori::Gui::setFontMonospace(_rightMarker);
    Ori::Gui::setFontMonospace(_logPreview);
    Ori::Gui::setFontMonospace(_parseResults);
    _logPreview->setReadOnly(true);
    _parseResults->setReadOnly(true);

    _listFiles->setAlternatingRowColors(true);
    _listFiles->setSelectionMode(QAbstractItemView::MultiSelection);

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Open | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    Ori::Layouts::LayoutV({tabs, buttons}).useFor(this);

    restoreState();

    connect(_comboPath, SIGNAL(currentTextChanged(QString)), this, SLOT(populateFileList()));
    connect(_filterEdit, SIGNAL(editTextChanged(QString)), this, SLOT(filterChanged()));
    connect(tabs, SIGNAL(currentChanged(int)), this, SLOT(tabActivated(int)));
    connect(_caseSensitiveFiles, SIGNAL(clicked(bool)), this, SLOT(populateFileList()));
}

OpenFilesDialog::~OpenFilesDialog()
{
    if (QDialog::result() == QDialog::Accepted)
    {
        _leftMarker->append(selectedLeftMarker(), false);
        _rightMarker->append(selectedRightMarker(), false);
        _encoding->append(selectedEncoding(), false);
        _filterEdit->append(selectedFilter(), false);
    }

    saveState();
}

void OpenFilesDialog::saveState()
{
    Ori::Settings s;
    s.storeWindowGeometry(this);
    s.beginDefaultGroup();
    _comboPath->save(s.settings());
    _leftMarker->save(s.settings());
    _rightMarker->save(s.settings());
    _encoding->save(s.settings());
    _filterEdit->save(s.settings());
    s.settings()->setValue("LeftMarkerRegexp", selectedLeftMarkerRegexp());
    s.settings()->setValue("RightMarkerRegexp", selectedRightMarkerRegexp());
    s.settings()->setValue("CaseSensitiveFiles", _caseSensitiveFiles->isChecked());
}

void OpenFilesDialog::restoreState()
{
    Ori::Settings s;
    s.restoreWindowGeometry(this, QSize(550, 550));
    s.beginDefaultGroup();

    _comboPath->load(s.settings());
    _filterEdit->load(s.settings(), "*.log");
    _caseSensitiveFiles->setChecked(s.settings()->value("CaseSensitiveFiles").toBool());
    populateFileList();

    _leftMarker->load(s.settings());
    _leftMarkerRegexp->setChecked(s.settings()->value("LeftMarkerRegexp").toBool());

    _rightMarker->load(s.settings());
    _rightMarkerRegexp->setChecked(s.settings()->value("RightMarkerRegexp").toBool());

    _encoding->load(s.settings(), "UTF-8");
}

LogParams OpenFilesDialog::result() const
{
    LogParams params;
    params.encoding = selectedEncoding();
    params.files = selectedFiles();
    params.marker = selectedMarkerParams();
    return params;
}

void OpenFilesDialog::selectDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Logs Directory"), selectedDir());
    if (dir.isEmpty()) return;

    _comboPath->append(dir);
    populateFileList();
}

void OpenFilesDialog::selectFilesAll()
{
    _listFiles->selectAll();
}

void OpenFilesDialog::selectFilesNone()
{
    _listFiles->selectionModel()->clear();
}

void OpenFilesDialog::selectFilesInvert()
{
}

void OpenFilesDialog::populateFileList()
{
    QDir dir(selectedDir(), selectedFilter());
    _listFiles->clear();
    auto files = dir.entryList();
    sortFiles(files);
    for (const QString& file: files)
        _listFiles->addItem(file);
}

void OpenFilesDialog::filterChanged()
{
    _lastTimerTick = QDateTime::currentMSecsSinceEpoch();
    if (_updateFilterTimerId == 0)
        _updateFilterTimerId = startTimer(500);
}

void OpenFilesDialog::timerEvent(QTimerEvent*)
{
    auto now = QDateTime::currentMSecsSinceEpoch();
    if (now - _lastTimerTick >= 500)
    {
        killTimer(_updateFilterTimerId);
        _updateFilterTimerId = 0;
        populateFileList();
    }
    _lastTimerTick = now;
}

QStringList OpenFilesDialog::selectedFiles() const
{
    QStringList files;
    QString path = _comboPath->currentText();
    for (QListWidgetItem* it : _listFiles->selectedItems())
        files << path % '/' % it->text();
    sortFiles(files);
    return files;
}

QString OpenFilesDialog::selectedDir() const { return _comboPath->currentText(); }
QString OpenFilesDialog::selectedFilter() const { return _filterEdit->currentText(); }
QString OpenFilesDialog::selectedEncoding() const { return _encoding->currentText(); }
QString OpenFilesDialog::selectedLeftMarker() const { return _leftMarker->currentText(); }
QString OpenFilesDialog::selectedRightMarker() const { return _rightMarker->currentText(); }
bool OpenFilesDialog::selectedLeftMarkerRegexp() const { return _leftMarkerRegexp->isChecked(); }
bool OpenFilesDialog::selectedRightMarkerRegexp() const { return _rightMarkerRegexp->isChecked(); }

LogMarkersParams OpenFilesDialog::selectedMarkerParams() const
{
    LogMarkersParams marker;
    marker.left.marker = selectedLeftMarker();
    marker.left.regexp = selectedLeftMarkerRegexp();
    marker.right.marker = selectedRightMarker();
    marker.right.regexp = selectedRightMarkerRegexp();
    return marker;
}

void OpenFilesDialog::tabActivated(int index)
{
    if (index == TAB_LOG_FORMAT)
        loadLogPreview();
}

void OpenFilesDialog::loadLogPreview()
{
    auto files = selectedFiles();
    if (files.empty()) return;
    auto file = files.first();

    _logPreviewTitle->setText(file);
    _logPreview->clear();
    PreviewFileReader reader(_logPreview, 100, file, selectedEncoding());
    QString res = reader.read();
    if (!res.isEmpty())
        _logPreview->setPlainText(tr("ERROR: %1").arg(res));
}

void OpenFilesDialog::testParsing()
{
    auto files = selectedFiles();
    if (files.empty()) return;
    auto file = files.first();

    auto marker = selectedMarkerParams();
    LogItems log;
    LogFileReader reader(&marker, &log, file, selectedEncoding());
    QString res = reader.read();
    if (!res.isEmpty())
        res = tr("ERROR: %1").arg(res);
    else if (log.items().empty())
        res = tr("No records where recognized");
    else
        res = log.str();
    _parseResults->setPlainText(res);
}

void OpenFilesDialog::sortFiles(QStringList& files) const
{
    files.sort(_caseSensitiveFiles->isChecked()? Qt::CaseSensitive: Qt::CaseInsensitive);
}
