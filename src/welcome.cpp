// SPDX-License-Identifier: Apache-2.0
#include "welcome.h"

#include <QDir>
#include <utility>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

Welcome::Welcome(QWidget *parent)
    : QWidget(parent)
    , m_recent(new QListWidget(this))
    , m_notice(new QLabel(this))
{
    auto *title = new QLabel(tr("Astra Studio"), this);
    title->setStyleSheet(QStringLiteral("font-size: 34px; font-weight: 700"));
    auto *subtitle = new QLabel(tr("Make apps for astraOS, with the components and the design language of the system."), this);
    subtitle->setStyleSheet(QStringLiteral("font-size: 15px; color: palette(placeholder-text)"));

    auto *newButton = new QPushButton(tr("New App..."), this);
    auto *openButton = new QPushButton(tr("Open..."), this);
    newButton->setDefault(true);
    for (QPushButton *b : {newButton, openButton}) {
        b->setMinimumHeight(40);
        b->setMinimumWidth(150);
    }
    auto *buttons = new QHBoxLayout;
    buttons->addWidget(newButton);
    buttons->addWidget(openButton);
    buttons->addStretch();

    auto *docs = new QHBoxLayout;
    for (const auto &[label, page] : {std::pair{tr("SDK guide"), QStringLiteral("sdk/README.md")},
                                      std::pair{tr("Design language"), QStringLiteral("DESIGN.md")},
                                      std::pair{tr("Reference"), QStringLiteral("sdk/REFERENCE.md")}}) {
        auto *b = new QPushButton(label, this);
        b->setFlat(true);
        b->setCursor(Qt::PointingHandCursor);
        connect(b, &QPushButton::clicked, this, [this, page = page] { Q_EMIT openDocs(page); });
        docs->addWidget(b);
    }
    docs->addStretch();

    m_notice->setWordWrap(true);
    m_notice->setStyleSheet(QStringLiteral("color: #ff9f0a"));
    m_notice->hide();

    auto *column = new QVBoxLayout;
    column->addStretch(1);
    column->addWidget(title);
    column->addWidget(subtitle);
    column->addSpacing(24);
    column->addLayout(buttons);
    column->addSpacing(8);
    column->addWidget(m_notice);
    column->addSpacing(24);
    column->addWidget(new QLabel(tr("Recent"), this));
    column->addWidget(m_recent, 2);
    column->addLayout(docs);
    column->addStretch(1);

    auto *outer = new QHBoxLayout(this);
    outer->addStretch(1);
    outer->addLayout(column, 3);
    outer->addStretch(1);

    connect(newButton, &QPushButton::clicked, this, &Welcome::newProject);
    connect(openButton, &QPushButton::clicked, this, &Welcome::openProject);
    connect(m_recent, &QListWidget::itemActivated, this, [this](QListWidgetItem *item) {
        Q_EMIT openRecent(item->data(Qt::UserRole).toString());
    });
}

void Welcome::setRecent(const QStringList &projects)
{
    m_recent->clear();
    for (const QString &dir : projects) {
        if (!QFileInfo::exists(dir + QStringLiteral("/app.json")))
            continue;
        auto *item = new QListWidgetItem(QStringLiteral("%1\n%2").arg(QFileInfo(dir).fileName(), QDir::toNativeSeparators(dir)), m_recent);
        item->setData(Qt::UserRole, dir);
    }
}

void Welcome::setNotice(const QString &text)
{
    m_notice->setText(text);
    m_notice->setVisible(!text.isEmpty());
}
