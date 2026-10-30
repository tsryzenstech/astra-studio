// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <QWidget>

class QComboBox;
class QLabel;
class QQuickWidget;
class QTimer;
class QToolButton;

// The live preview: the project's app on a phone-sized screen, with
// astraOS's real components (Astra.Controls) and a stand-in for the system
// services. It reloads a moment after every change.
class PreviewPane : public QWidget
{
    Q_OBJECT
public:
    explicit PreviewPane(QWidget *parent = nullptr);

    void setProject(const QString &dir);          // reads app.json (main, package, accent)
    void reloadSoon();                            // after an edit
    void reload();

Q_SIGNALS:
    void message(const QString &text, bool error); // QML warnings and console output

private:
    void applyLook();
    QQuickWidget *m_view;
    QComboBox *m_device;
    QToolButton *m_dark;
    QComboBox *m_textSize;
    QLabel *m_status;
    QTimer *m_debounce;
    QString m_project;
    QString m_main;
};
