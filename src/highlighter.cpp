// SPDX-License-Identifier: Apache-2.0
#include "highlighter.h"

namespace {
QTextCharFormat makeFormat(const char *color, bool bold = false, bool italic = false)
{
    QTextCharFormat f;
    f.setForeground(QColor(QString::fromLatin1(color)));
    if (bold)
        f.setFontWeight(QFont::DemiBold);
    f.setFontItalic(italic);
    return f;
}
}

Highlighter::Highlighter(Language language, QTextDocument *document)
    : QSyntaxHighlighter(document)
    , m_language(language)
{
    m_comment = makeFormat("#7f8792", false, true);
    if (language == Json) {
        m_rules.append({QRegularExpression(QStringLiteral("\"(?:[^\"\\\\]|\\\\.)*\"(?=\\s*:)")), makeFormat("#ff9f0a")});
        m_rules.append({QRegularExpression(QStringLiteral(":\\s*(\"(?:[^\"\\\\]|\\\\.)*\")")), makeFormat("#a3d977"), 1});
        m_rules.append({QRegularExpression(QStringLiteral("\\b(true|false|null)\\b")), makeFormat("#bf5af2", true)});
        m_rules.append({QRegularExpression(QStringLiteral("-?\\b\\d+(\\.\\d+)?\\b")), makeFormat("#64d2ff")});
        return;
    }
    if (language != Qml)
        return;
    const QStringList keywords{
        QStringLiteral("import"), QStringLiteral("pragma"), QStringLiteral("property"), QStringLiteral("readonly"),
        QStringLiteral("required"), QStringLiteral("default"), QStringLiteral("alias"), QStringLiteral("signal"),
        QStringLiteral("function"), QStringLiteral("component"), QStringLiteral("var"), QStringLiteral("let"),
        QStringLiteral("const"), QStringLiteral("if"), QStringLiteral("else"), QStringLiteral("for"),
        QStringLiteral("while"), QStringLiteral("do"), QStringLiteral("return"), QStringLiteral("switch"),
        QStringLiteral("case"), QStringLiteral("break"), QStringLiteral("continue"), QStringLiteral("new"),
        QStringLiteral("typeof"), QStringLiteral("instanceof"), QStringLiteral("in"), QStringLiteral("of"),
        QStringLiteral("try"), QStringLiteral("catch"), QStringLiteral("finally"), QStringLiteral("throw"),
        QStringLiteral("this"), QStringLiteral("as"), QStringLiteral("on")};
    m_rules.append({QRegularExpression(QStringLiteral("\\b(%1)\\b").arg(keywords.join(QLatin1Char('|')))), makeFormat("#ff7ab2", true)});
    // Types: Page, ListItem, Theme...
    m_rules.append({QRegularExpression(QStringLiteral("\\b[A-Z][A-Za-z0-9_]*\\b")), makeFormat("#5ac8fa")});
    // property names before ':' (text:, anchors.fill:)
    m_rules.append({QRegularExpression(QStringLiteral("^\\s*([a-z][\\w.]*)\\s*:")), makeFormat("#ffd60a"), 1});
    m_rules.append({QRegularExpression(QStringLiteral("\\b(id)\\s*:")), makeFormat("#ff9f0a", true), 1});
    m_rules.append({QRegularExpression(QStringLiteral("\\b(true|false|null|undefined|parent)\\b")), makeFormat("#bf5af2", true)});
    m_rules.append({QRegularExpression(QStringLiteral("\\b\\d+(\\.\\d+)?\\b")), makeFormat("#64d2ff")});
    m_rules.append({QRegularExpression(QStringLiteral("\\b(qsTr|qsTranslate|QT_TR_NOOP)\\b")), makeFormat("#30d158", true)});
    m_rules.append({QRegularExpression(QStringLiteral("\"(?:[^\"\\\\]|\\\\.)*\"|'(?:[^'\\\\]|\\\\.)*'|`[^`]*`")), makeFormat("#ff8170")});
    m_rules.append({QRegularExpression(QStringLiteral("//[^\\n]*")), m_comment});
}

void Highlighter::highlightBlock(const QString &text)
{
    for (const Rule &rule : std::as_const(m_rules)) {
        auto it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            const auto m = it.next();
            setFormat(int(m.capturedStart(rule.group)), int(m.capturedLength(rule.group)), rule.format);
        }
    }
    if (m_language != Qml)
        return;
    // /* block comments */ spanning lines
    setCurrentBlockState(0);
    int start = previousBlockState() == 1 ? 0 : int(text.indexOf(QLatin1String("/*")));
    while (start >= 0) {
        const int end = int(text.indexOf(QLatin1String("*/"), start + (previousBlockState() == 1 && start == 0 ? 0 : 2)));
        int length;
        if (end < 0) {
            setCurrentBlockState(1);
            length = int(text.length()) - start;
        } else {
            length = end - start + 2;
        }
        setFormat(start, length, m_comment);
        start = end < 0 ? -1 : int(text.indexOf(QLatin1String("/*"), start + length));
    }
}
