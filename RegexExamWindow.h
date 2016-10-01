#ifndef REGEX_EXAM_WINDOW_H
#define REGEX_EXAM_WINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QPlainTextEdit;
QT_END_NAMESPACE

class RegexExamWindow : public QWidget
{
    Q_OBJECT
public:
    explicit RegexExamWindow(QWidget *parent = 0);
    ~RegexExamWindow();

private:
    QPlainTextEdit *_text, *_code, *_results;

private slots:
    void processText();
};

#endif // REGEX_EXAM_WINDOW_H
