// SPDX-License-Identifier: Apache-2.0
//
// Astra Studio - the IDE for astraOS apps: projects from the SDK's
// templates, an editor, a live preview with astraOS's real components, the
// design language check, packaging and running on a phone.

#include "mainwindow.h"

#include <QApplication>
#include <QDir>
#include <QFontDatabase>
#include <QIcon>
#include <QPalette>
#include <QStyleFactory>

namespace {
// astraOS's dark palette for the IDE itself.
void applyDarkTheme(QApplication &app)
{
    app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QPalette p;
    const QColor window(0x1c, 0x1c, 0x20), base(0x15, 0x15, 0x17), text(0xf2, 0xf2, 0xf7), accent(0x0a, 0x84, 0xff);
    p.setColor(QPalette::Window, window);
    p.setColor(QPalette::WindowText, text);
    p.setColor(QPalette::Base, base);
    p.setColor(QPalette::AlternateBase, window);
    p.setColor(QPalette::Text, text);
    p.setColor(QPalette::Button, QColor(0x2a, 0x2a, 0x30));
    p.setColor(QPalette::ButtonText, text);
    p.setColor(QPalette::Highlight, accent);
    p.setColor(QPalette::HighlightedText, Qt::white);
    p.setColor(QPalette::ToolTipBase, window);
    p.setColor(QPalette::ToolTipText, text);
    p.setColor(QPalette::PlaceholderText, QColor(0x8e, 0x8e, 0x93));
    p.setColor(QPalette::Link, accent);
    p.setColor(QPalette::Disabled, QPalette::Text, QColor(0x63, 0x63, 0x66));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(0x63, 0x63, 0x66));
    app.setPalette(p);
}
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("astraOS"));
    QApplication::setApplicationName(QStringLiteral("Astra Studio"));
    QApplication::setApplicationVersion(QStringLiteral(STUDIO_VERSION));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/studio/astra-studio.svg")));
    applyDarkTheme(app);

    // astraOS's font, for the IDE and for the preview (Theme.fontFamily).
    const QDir fonts(QCoreApplication::applicationDirPath() + QStringLiteral("/fonts"));
    for (const QString &f : fonts.entryList({QStringLiteral("*.ttf")}, QDir::Files))
        QFontDatabase::addApplicationFont(fonts.filePath(f));
    QFont ui(QStringLiteral("Poppins"));
    ui.setPointSize(9);
    app.setFont(ui);

    MainWindow window;
    window.show();
    if (argc > 1)
        window.openProject(QString::fromLocal8Bit(argv[1]));
    return app.exec();
}
