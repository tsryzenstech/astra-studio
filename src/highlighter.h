// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QVector>

// QML / JavaScript and JSON colouring, in astraOS's dark palette.
class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    enum Language { Qml, Json, Plain };
    Highlighter(Language language, QTextDocument *document);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct Rule {
        QRegularExpression pattern;
        QTextCharFormat format;
        int group = 0;
    };
    QVector<Rule> m_rules;
    QTextCharFormat m_comment;
    Language m_language;
};
