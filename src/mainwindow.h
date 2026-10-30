// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <QMainWindow>

#include "sdk.h"

class CodeEditor;
class PreviewPane;
class QFileSystemModel;
class QLabel;
class QPlainTextEdit;
class QStackedWidget;
class QTabWidget;
class QTimer;
class QTreeView;
class QTreeWidget;
class Welcome;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();

    void openProject(const QString &dir);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void buildMenus();
    void newProject();
    void chooseProject();
    void closeProject();
    CodeEditor *openFile(const QString &path, int line = 0);
    CodeEditor *currentEditor() const;
    bool saveAll();
    bool maybeSave();
    void runCheck();
    void showProblems(const QVector<Problem> &problems);
    void sdkCommand(const QStringList &args, bool saveFirst = true);
    void appendOutput(const QString &text, bool error);
    void openDocs(const QString &page);
    void addRecent(const QString &dir);
    QStringList recent() const;
    void updateTitle();

    Sdk m_sdk;
    QString m_project;
    QStackedWidget *m_stack;
    Welcome *m_welcome;
    QTreeView *m_tree;
    QFileSystemModel *m_files;
    QTabWidget *m_tabs;
    PreviewPane *m_preview;
    QTreeWidget *m_problems;
    QPlainTextEdit *m_output;
    QLabel *m_statusProblems;
    QTimer *m_checkTimer;
    QList<QAction *> m_projectActions;
    QAction *m_stop = nullptr;
};
