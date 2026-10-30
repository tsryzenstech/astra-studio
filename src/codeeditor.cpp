// SPDX-License-Identifier: Apache-2.0
#include "codeeditor.h"
#include "highlighter.h"

#include <QAbstractItemView>
#include <QCompleter>
#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QSaveFile>
#include <QScrollBar>
#include <QStringListModel>
#include <QTextBlock>

namespace {
class LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(CodeEditor *editor) : QWidget(editor), m_editor(editor) {}
    QSize sizeHint() const override { return {m_editor->lineNumberAreaWidth(), 0}; }

protected:
    void paintEvent(QPaintEvent *event) override { m_editor->paintLineNumbers(event); }

private:
    CodeEditor *m_editor;
};

// What Ctrl+Space offers: the SDK's components, Theme's tokens and the
// common properties (docs/sdk/REFERENCE.md has them all).
QStringList completions()
{
    return {
        QStringLiteral("Page"), QStringLiteral("PageStack"), QStringLiteral("ListGroup"), QStringLiteral("ListItem"),
        QStringLiteral("Button"), QStringLiteral("IconButton"), QStringLiteral("TextField"), QStringLiteral("Switch"),
        QStringLiteral("Slider"), QStringLiteral("SegmentedControl"), QStringLiteral("Sheet"), QStringLiteral("Card"),
        QStringLiteral("TabBar"), QStringLiteral("EmptyState"), QStringLiteral("Spinner"), QStringLiteral("ProgressBar"),
        QStringLiteral("Icon"), QStringLiteral("AppIcon"), QStringLiteral("WidgetTile"), QStringLiteral("ShareSheet"),
        QStringLiteral("Theme.text"), QStringLiteral("Theme.textSecondary"), QStringLiteral("Theme.textTertiary"),
        QStringLiteral("Theme.background"), QStringLiteral("Theme.surface"), QStringLiteral("Theme.surfaceRaised"),
        QStringLiteral("Theme.separator"), QStringLiteral("Theme.accent"), QStringLiteral("Theme.danger"),
        QStringLiteral("Theme.fontFamily"), QStringLiteral("Theme.bodySize"), QStringLiteral("Theme.titleSize"),
        QStringLiteral("Theme.headlineSize"), QStringLiteral("Theme.captionSize"), QStringLiteral("Theme.largeTitleSize"),
        QStringLiteral("Theme.margin"), QStringLiteral("Theme.spacing"), QStringLiteral("Theme.radius"),
        QStringLiteral("Theme.cardRadius"), QStringLiteral("Theme.sheetRadius"),
        QStringLiteral("Application.accent"), QStringLiteral("Application.args"), QStringLiteral("Files.appData"),
        QStringLiteral("Files.readText"), QStringLiteral("Files.writeText"), QStringLiteral("Notifications.post"),
        QStringLiteral("Settings.values"), QStringLiteral("Settings.setValue"), QStringLiteral("Permissions.request"),
        QStringLiteral("Dialogs.show"), QStringLiteral("Packages.launch"),
        QStringLiteral("anchors.fill"), QStringLiteral("anchors.centerIn"), QStringLiteral("iconName"),
        QStringLiteral("chevron"), QStringLiteral("subtitle"), QStringLiteral("placeholder"), QStringLiteral("separator"),
        QStringLiteral("onClicked"), QStringLiteral("qsTr"),
    };
}
}

CodeEditor::CodeEditor(const QString &path, QWidget *parent)
    : QPlainTextEdit(parent)
    , m_path(path)
    , m_lineNumbers(new LineNumberArea(this))
{
    QFont mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    mono.setFamilies({QStringLiteral("Cascadia Code"), QStringLiteral("Consolas"), mono.family()});
    mono.setPointSize(11);
    setFont(mono);
    setTabStopDistance(fontMetrics().horizontalAdvance(QLatin1Char(' ')) * 4);
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setFrameShape(QFrame::NoFrame);

    const QString suffix = QFileInfo(path).suffix().toLower();
    const Highlighter::Language lang = suffix == QLatin1String("json") ? Highlighter::Json
                                     : (suffix == QLatin1String("qml") || suffix == QLatin1String("js")) ? Highlighter::Qml
                                                                                                       : Highlighter::Plain;
    m_highlighter = new Highlighter(lang, document());

    if (lang == Highlighter::Qml) {
        m_completer = new QCompleter(new QStringListModel(completions(), this), this);
        m_completer->setWidget(this);
        m_completer->setCaseSensitivity(Qt::CaseInsensitive);
        m_completer->setFilterMode(Qt::MatchContains);
        connect(m_completer, qOverload<const QString &>(&QCompleter::activated), this, &CodeEditor::insertCompletion);
    }

    connect(this, &QPlainTextEdit::blockCountChanged, this, [this] { setViewportMargins(lineNumberAreaWidth(), 0, 0, 0); });
    connect(this, &QPlainTextEdit::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
    highlightCurrentLine();
}

bool CodeEditor::load()
{
    QFile f(m_path);
    if (!f.open(QIODevice::ReadOnly))
        return false;
    setPlainText(QString::fromUtf8(f.readAll()));
    document()->setModified(false);
    return true;
}

bool CodeEditor::save()
{
    QSaveFile f(m_path);
    if (!f.open(QIODevice::WriteOnly))
        return false;
    f.write(toPlainText().toUtf8());
    if (!f.commit())
        return false;
    document()->setModified(false);
    Q_EMIT saved(m_path);
    return true;
}

void CodeEditor::goToLine(int line)
{
    const QTextBlock block = document()->findBlockByNumber(qMax(0, line - 1));
    QTextCursor c(block);
    setTextCursor(c);
    centerCursor();
    setFocus();
}

int CodeEditor::lineNumberAreaWidth() const
{
    int digits = 1;
    for (int max = qMax(1, blockCount()); max >= 10; max /= 10)
        ++digits;
    return 16 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * qMax(3, digits);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        m_lineNumbers->scroll(0, dy);
    else
        m_lineNumbers->update(0, rect.y(), m_lineNumbers->width(), rect.height());
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    const QRect cr = contentsRect();
    m_lineNumbers->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::paintLineNumbers(QPaintEvent *event)
{
    QPainter p(m_lineNumbers);
    p.fillRect(event->rect(), palette().base().color().darker(110));
    QTextBlock block = firstVisibleBlock();
    int number = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    const int current = textCursor().blockNumber();
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            p.setPen(number == current ? palette().text().color() : palette().placeholderText().color());
            p.drawText(0, top, m_lineNumbers->width() - 8, fontMetrics().height(), Qt::AlignRight,
                       QString::number(number + 1));
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++number;
    }
}

void CodeEditor::highlightCurrentLine()
{
    QTextEdit::ExtraSelection line;
    line.format.setBackground(palette().base().color().lighter(125));
    line.format.setProperty(QTextFormat::FullWidthSelection, true);
    line.cursor = textCursor();
    line.cursor.clearSelection();
    setExtraSelections({line});
}

QString CodeEditor::wordUnderCursor() const
{
    QTextCursor c = textCursor();
    const QString line = c.block().text().left(c.positionInBlock());
    int i = int(line.length());
    while (i > 0 && (line.at(i - 1).isLetterOrNumber() || line.at(i - 1) == QLatin1Char('.') || line.at(i - 1) == QLatin1Char('_')))
        --i;
    return line.mid(i);
}

void CodeEditor::insertCompletion(const QString &word)
{
    QTextCursor c = textCursor();
    const QString prefix = wordUnderCursor();
    c.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, int(prefix.length()));
    c.insertText(word);
    setTextCursor(c);
}

void CodeEditor::keyPressEvent(QKeyEvent *event)
{
    if (m_completer && m_completer->popup()->isVisible()) {
        switch (event->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
            event->ignore();
            return;
        default:
            break;
        }
    }
    if (event->key() == Qt::Key_Space && event->modifiers() & Qt::ControlModifier && m_completer) {
        m_completer->setCompletionPrefix(wordUnderCursor());
        QRect r = cursorRect();
        r.setWidth(m_completer->popup()->sizeHintForColumn(0) + 40);
        m_completer->complete(r);
        return;
    }
    if (event->key() == Qt::Key_Tab && !(event->modifiers() & Qt::ShiftModifier)) {
        insertPlainText(QStringLiteral("    "));
        return;
    }
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        // Keep the indentation, one level more after '{'.
        const QString line = textCursor().block().text();
        QString indent;
        for (QChar ch : line) {
            if (ch != QLatin1Char(' ') && ch != QLatin1Char('\t'))
                break;
            indent += ch;
        }
        if (line.trimmed().endsWith(QLatin1Char('{')) || line.trimmed().endsWith(QLatin1Char('[')))
            indent += QStringLiteral("    ");
        QPlainTextEdit::keyPressEvent(event);
        insertPlainText(indent);
        return;
    }
    if (event->key() == Qt::Key_BraceRight) {
        // '}' on an empty line goes back one level.
        QTextCursor c = textCursor();
        const QString before = c.block().text().left(c.positionInBlock());
        if (before.trimmed().isEmpty() && before.endsWith(QStringLiteral("    "))) {
            for (int i = 0; i < 4; ++i)
                c.deletePreviousChar();
        }
    }
    QPlainTextEdit::keyPressEvent(event);
}
