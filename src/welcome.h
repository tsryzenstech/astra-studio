// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <QWidget>

class QListWidget;

// What Astra Studio shows without a project: new, open, recent projects.
class Welcome : public QWidget
{
    Q_OBJECT
public:
    explicit Welcome(QWidget *parent = nullptr);
    void setRecent(const QStringList &projects);
    void setNotice(const QString &text);    // e.g. "Python 3 is needed"

Q_SIGNALS:
    void newProject();
    void openProject();
    void openRecent(const QString &dir);
    void openDocs(const QString &page);

private:
    QListWidget *m_recent;
    class QLabel *m_notice;
};
