// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <QPlainTextEdit>

class QCompleter;
class Highlighter;

// A source file: line numbers, the current line, auto-indent, tab = 4
// spaces, and Ctrl+Space completion of astraOS's components and Theme.
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(const QString &path, QWidget *parent = nullptr);

    QString path() const { return m_path; }
    bool load();
    bool save();
    bool modified() const { return document()->isModified(); }
    void goToLine(int line);

    int lineNumberAreaWidth() const;
    void paintLineNumbers(QPaintEvent *event);

Q_SIGNALS:
    void saved(const QString &path);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void updateLineNumberArea(const QRect &rect, int dy);
    void highlightCurrentLine();
    void insertCompletion(const QString &word);
    QString wordUnderCursor() const;

    QString m_path;
    QWidget *m_lineNumbers;
    Highlighter *m_highlighter = nullptr;
    QCompleter *m_completer = nullptr;
};
