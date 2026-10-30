// SPDX-License-Identifier: Apache-2.0
#include "previewpane.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QFile>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWidget>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

namespace {
// Logical sizes (dp) of phones astraOS runs on.
struct Device { const char *name; int width, height; };
const Device kDevices[] = {
    {"Redmi Note 10 Pro (393 × 873)", 393, 873},
    {"Compact (360 × 760)", 360, 760},
    {"Large (430 × 932)", 430, 932},
};
}

PreviewPane::PreviewPane(QWidget *parent)
    : QWidget(parent)
    , m_view(new QQuickWidget(this))
    , m_device(new QComboBox(this))
    , m_dark(new QToolButton(this))
    , m_textSize(new QComboBox(this))
    , m_status(new QLabel(this))
    , m_debounce(new QTimer(this))
{
    auto *bar = new QHBoxLayout;
    bar->setContentsMargins(8, 6, 8, 6);
    for (const Device &d : kDevices)
        m_device->addItem(QString::fromUtf8(d.name));
    m_dark->setText(tr("Dark"));
    m_dark->setCheckable(true);
    m_dark->setChecked(true);
    m_dark->setToolTip(tr("Light or dark mode, like Settings > Display on the phone"));
    m_textSize->addItems({tr("Text 100%"), tr("Text 85%"), tr("Text 115%"), tr("Text 130%")});
    m_textSize->setToolTip(tr("The user's text size (Settings > Display > Text Size)"));
    auto *refresh = new QToolButton(this);
    refresh->setText(tr("Reload"));
    bar->addWidget(m_device);
    bar->addWidget(m_dark);
    bar->addWidget(m_textSize);
    bar->addStretch();
    bar->addWidget(refresh);

    m_view->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_view->engine()->addImportPath(QCoreApplication::applicationDirPath() + QStringLiteral("/qml"));
    connect(m_view->engine(), &QQmlEngine::warnings, this, [this](const QList<QQmlError> &warnings) {
        for (const QQmlError &w : warnings)
            Q_EMIT message(w.toString(), true);
    });
    m_view->setSource(QUrl(QStringLiteral("qrc:/studio/PreviewHost.qml")));
    for (const QQmlError &e : m_view->errors())
        Q_EMIT message(e.toString(), true);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addLayout(bar);
    layout->addWidget(m_view, 1);
    m_status->setContentsMargins(8, 4, 8, 4);
    layout->addWidget(m_status);

    m_debounce->setSingleShot(true);
    m_debounce->setInterval(600);
    connect(m_debounce, &QTimer::timeout, this, &PreviewPane::reload);
    connect(refresh, &QToolButton::clicked, this, &PreviewPane::reload);
    connect(m_dark, &QToolButton::toggled, this, &PreviewPane::applyLook);
    connect(m_textSize, &QComboBox::currentIndexChanged, this, &PreviewPane::applyLook);
    connect(m_device, &QComboBox::currentIndexChanged, this, [this](int i) {
        if (QQuickItem *root = m_view->rootObject()) {
            root->setProperty("screenWidth", kDevices[i].width);
            root->setProperty("screenHeight", kDevices[i].height);
        }
    });
}

void PreviewPane::applyLook()
{
    QObject *settings = m_view->engine()->singletonInstance<QObject *>("Astra.Core", "Settings");
    if (!settings)
        return;
    static const double scales[] = {1.0, 0.85, 1.15, 1.3};
    QMetaObject::invokeMethod(settings, "setValue", Q_ARG(QVariant, QStringLiteral("theme.mode")),
                              Q_ARG(QVariant, m_dark->isChecked() ? QStringLiteral("dark") : QStringLiteral("light")));
    QMetaObject::invokeMethod(settings, "setValue", Q_ARG(QVariant, QStringLiteral("display.font_scale")),
                              Q_ARG(QVariant, scales[qBound(0, m_textSize->currentIndex(), 3)]));
    m_dark->setText(m_dark->isChecked() ? tr("Dark") : tr("Light"));
}

void PreviewPane::setProject(const QString &dir)
{
    m_project = dir;
    QFile f(dir + QStringLiteral("/app.json"));
    QJsonObject manifest;
    if (f.open(QIODevice::ReadOnly))
        manifest = QJsonDocument::fromJson(f.readAll()).object();
    m_main = dir + QLatin1Char('/') + manifest.value(QLatin1String("main")).toString(QStringLiteral("qml/Main.qml"));
    if (QObject *app = m_view->engine()->singletonInstance<QObject *>("Astra.Core", "Application")) {
        app->setProperty("packageName", manifest.value(QLatin1String("package")).toString());
        const QColor accent(manifest.value(QLatin1String("accent")).toString());
        app->setProperty("accent", accent.isValid() ? accent : QColor(Qt::transparent));
    }
    applyLook();
    reload();
}

void PreviewPane::reloadSoon()
{
    m_debounce->start();
}

void PreviewPane::reload()
{
    QQuickItem *root = m_view->rootObject();
    if (!root || m_main.isEmpty())
        return;
    m_view->engine()->clearComponentCache();
    root->setProperty("source", QUrl::fromLocalFile(m_main));
    QMetaObject::invokeMethod(root, "reload");
    const QString error = root->property("error").toString();
    m_status->setText(error.isEmpty() ? tr("Preview of %1").arg(QFile(m_main).fileName().section(QLatin1Char('/'), -1))
                                      : tr("The preview has errors (see Output)"));
    if (!error.isEmpty())
        Q_EMIT message(error, true);
}
