// SPDX-License-Identifier: Apache-2.0
#include "mainwindow.h"
#include "codeeditor.h"
#include "newprojectdialog.h"
#include "previewpane.h"
#include "welcome.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QSettings>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTabWidget>
#include <QTimer>
#include <QToolBar>
#include <QTreeView>
#include <QTreeWidget>
#include <QUrl>

MainWindow::MainWindow()
    : m_stack(new QStackedWidget(this))
    , m_welcome(new Welcome(this))
    , m_tree(new QTreeView(this))
    , m_files(new QFileSystemModel(this))
    , m_tabs(new QTabWidget(this))
    , m_preview(new PreviewPane(this))
    , m_problems(new QTreeWidget(this))
    , m_output(new QPlainTextEdit(this))
    , m_statusProblems(new QLabel(this))
    , m_checkTimer(new QTimer(this))
{
    setWindowTitle(QStringLiteral("Astra Studio"));
    resize(1500, 920);

    // Workspace: files | editors | preview
    m_files->setNameFilterDisables(false);
    m_files->setNameFilters({QStringLiteral("*.qml"), QStringLiteral("*.js"), QStringLiteral("*.json"),
                             QStringLiteral("*.svg"), QStringLiteral("*.png"), QStringLiteral("*.md"), QStringLiteral("*.txt")});
    m_tree->setModel(m_files);
    for (int c = 1; c < m_files->columnCount(); ++c)
        m_tree->hideColumn(c);
    m_tree->header()->hide();
    m_tabs->setTabsClosable(true);
    m_tabs->setDocumentMode(true);
    m_tabs->setMovable(true);

    auto *split = new QSplitter(this);
    split->addWidget(m_tree);
    split->addWidget(m_tabs);
    split->addWidget(m_preview);
    split->setStretchFactor(0, 0);
    split->setStretchFactor(1, 3);
    split->setStretchFactor(2, 2);
    split->setSizes({240, 760, 480});
    m_stack->addWidget(m_welcome);
    m_stack->addWidget(split);
    setCentralWidget(m_stack);

    // Problems and Output
    m_problems->setHeaderLabels({tr("Problem"), tr("File"), tr("Line")});
    m_problems->setRootIsDecorated(false);
    m_problems->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    auto *problemsDock = new QDockWidget(tr("Problems"), this);
    problemsDock->setObjectName(QStringLiteral("problems"));
    problemsDock->setWidget(m_problems);
    m_output->setReadOnly(true);
    m_output->setMaximumBlockCount(5000);
    QFont mono(QStringLiteral("Consolas"));
    mono.setPointSize(10);
    m_output->setFont(mono);
    auto *outputDock = new QDockWidget(tr("Output"), this);
    outputDock->setObjectName(QStringLiteral("output"));
    outputDock->setWidget(m_output);
    addDockWidget(Qt::BottomDockWidgetArea, problemsDock);
    addDockWidget(Qt::BottomDockWidgetArea, outputDock);
    tabifyDockWidget(problemsDock, outputDock);
    problemsDock->raise();
    statusBar()->addPermanentWidget(m_statusProblems);

    buildMenus();

    m_checkTimer->setSingleShot(true);
    m_checkTimer->setInterval(800);
    connect(m_checkTimer, &QTimer::timeout, this, &MainWindow::runCheck);
    connect(m_tree, &QTreeView::activated, this, [this](const QModelIndex &i) {
        if (!m_files->isDir(i))
            openFile(m_files->filePath(i));
    });
    connect(m_tabs, &QTabWidget::tabCloseRequested, this, [this](int i) {
        auto *e = qobject_cast<CodeEditor *>(m_tabs->widget(i));
        if (e && e->modified()) {
            const auto answer = QMessageBox::question(this, tr("Save changes?"), tr("Save the changes to %1?").arg(QFileInfo(e->path()).fileName()),
                                                      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            if (answer == QMessageBox::Cancel || (answer == QMessageBox::Save && !e->save()))
                return;
        }
        m_tabs->removeTab(i);
        if (e)
            e->deleteLater();
    });
    connect(m_problems, &QTreeWidget::itemActivated, this, [this](QTreeWidgetItem *item) {
        openFile(m_project + QLatin1Char('/') + item->text(1), item->text(2).toInt());
    });
    connect(&m_sdk, &Sdk::output, this, &MainWindow::appendOutput);
    connect(&m_sdk, &Sdk::finished, this, [this, outputDock](int code, const QStringList &args) {
        m_stop->setEnabled(false);
        appendOutput(code == 0 ? tr("Done.\n") : tr("Finished with errors (%1).\n").arg(code), code != 0);
        if (code != 0)
            outputDock->raise();
        if (!args.isEmpty() && args.first() == QLatin1String("new") && code == 0)
            openProject(args.value(1));
    });
    connect(m_preview, &PreviewPane::message, this, &MainWindow::appendOutput);
    connect(m_welcome, &Welcome::newProject, this, &MainWindow::newProject);
    connect(m_welcome, &Welcome::openProject, this, &MainWindow::chooseProject);
    connect(m_welcome, &Welcome::openRecent, this, &MainWindow::openProject);
    connect(m_welcome, &Welcome::openDocs, this, &MainWindow::openDocs);

    m_welcome->setRecent(recent());
    m_welcome->setNotice(m_sdk.problem());
    QSettings s;
    restoreState(s.value(QStringLiteral("window/state")).toByteArray());
    updateTitle();
}

void MainWindow::buildMenus()
{
    QMenu *file = menuBar()->addMenu(tr("&File"));
    file->addAction(tr("&New App..."), QKeySequence::New, this, &MainWindow::newProject);
    file->addAction(tr("&Open..."), QKeySequence::Open, this, &MainWindow::chooseProject);
    QAction *save = file->addAction(tr("&Save"), QKeySequence::Save, this, [this] {
        if (CodeEditor *e = currentEditor())
            e->save();
    });
    QAction *saveAllAction = file->addAction(tr("Save A&ll"), QKeySequence(QStringLiteral("Ctrl+Shift+S")), this, &MainWindow::saveAll);
    QAction *close = file->addAction(tr("&Close App"), this, &MainWindow::closeProject);
    file->addSeparator();
    file->addAction(tr("E&xit"), QKeySequence::Quit, this, &QWidget::close);

    QMenu *build = menuBar()->addMenu(tr("&Build"));
    QAction *check = build->addAction(tr("&Check Design"), QKeySequence(QStringLiteral("F7")), this, [this] { saveAll(); runCheck(); });
    QAction *pack = build->addAction(tr("&Build Package"), QKeySequence(QStringLiteral("Ctrl+B")), this,
                                     [this] { sdkCommand({QStringLiteral("build"), m_project}); });
    QAction *run = build->addAction(tr("&Run on Phone"), QKeySequence(QStringLiteral("F5")), this,
                                    [this] { sdkCommand({QStringLiteral("run"), m_project}); });
    m_stop = build->addAction(tr("&Stop"), QKeySequence(QStringLiteral("Shift+F5")), this, [this] { m_sdk.stop(); });
    m_stop->setEnabled(false);
    build->addSeparator();
    QAction *i18n = build->addAction(tr("Update &Translations"), this, [this] { sdkCommand({QStringLiteral("i18n"), m_project}); });

    QMenu *phone = menuBar()->addMenu(tr("&Phone"));
    phone->addAction(tr("&Information"), this, [this] { sdkCommand({QStringLiteral("devices")}, false); });
    phone->addAction(tr("Follow the &Log"), this, [this] { sdkCommand({QStringLiteral("logs")}, false); });
    phone->addAction(tr("&Screenshot"), this, [this] {
        const QString file = QFileDialog::getSaveFileName(this, tr("Screenshot"), QStringLiteral("screenshot.png"), tr("PNG images (*.png)"));
        if (!file.isEmpty())
            sdkCommand({QStringLiteral("screenshot"), file}, false);
    });
    phone->addAction(tr("&Install a Package..."), this, [this] {
        const QString file = QFileDialog::getOpenFileName(this, tr("Install"), m_project, tr("astraOS packages (*.apkg)"));
        if (!file.isEmpty())
            sdkCommand({QStringLiteral("install"), file}, false);
    });

    QMenu *view = menuBar()->addMenu(tr("&View"));
    view->addAction(tr("&Reload Preview"), QKeySequence(QStringLiteral("Ctrl+R")), m_preview, &PreviewPane::reload);

    QMenu *help = menuBar()->addMenu(tr("&Help"));
    help->addAction(tr("SDK &Guide"), this, [this] { openDocs(QStringLiteral("sdk/README.md")); });
    help->addAction(tr("&Design Language"), this, [this] { openDocs(QStringLiteral("DESIGN.md")); });
    help->addAction(tr("&Reference"), QKeySequence::HelpContents, this, [this] { openDocs(QStringLiteral("sdk/REFERENCE.md")); });
    help->addAction(tr("&About Astra Studio"), this, [this] {
        QMessageBox::about(this, tr("Astra Studio"),
                           tr("<h3>Astra Studio %1</h3><p>The IDE for astraOS apps.</p><p>SDK: %2<br>Python: %3</p>")
                               .arg(QStringLiteral(STUDIO_VERSION), QDir::toNativeSeparators(m_sdk.root()),
                                    m_sdk.python().isEmpty() ? tr("not found") : QDir::toNativeSeparators(m_sdk.python())));
    });

    QToolBar *bar = addToolBar(tr("Build"));
    bar->setObjectName(QStringLiteral("toolbar"));
    bar->setMovable(false);
    bar->addAction(check);
    bar->addAction(pack);
    bar->addAction(run);
    bar->addAction(m_stop);
    m_projectActions = {save, saveAllAction, close, check, pack, run, i18n};
    for (QAction *a : std::as_const(m_projectActions))
        a->setEnabled(false);
}

void MainWindow::newProject()
{
    if (!m_sdk.available()) {
        QMessageBox::warning(this, tr("Astra Studio"), m_sdk.problem());
        return;
    }
    NewProjectDialog dialog(&m_sdk, this);
    if (dialog.exec() != QDialog::Accepted)
        return;
    QDir().mkpath(QFileInfo(dialog.location()).absolutePath());
    sdkCommand({QStringLiteral("new"), dialog.location(), QStringLiteral("--template"), dialog.templateId(),
                QStringLiteral("--name"), dialog.name(), QStringLiteral("--package"), dialog.package(),
                QStringLiteral("--accent"), dialog.accent().name().toUpper()},
               false);
}

void MainWindow::chooseProject()
{
    const QString dir = QFileDialog::getExistingDirectory(this, tr("Open an astraOS App"), recent().value(0));
    if (dir.isEmpty())
        return;
    if (!QFileInfo::exists(dir + QStringLiteral("/app.json"))) {
        QMessageBox::warning(this, tr("Astra Studio"), tr("%1 is not an astraOS app (it has no app.json).").arg(QDir::toNativeSeparators(dir)));
        return;
    }
    openProject(dir);
}

void MainWindow::openProject(const QString &dir)
{
    if (!maybeSave())
        return;
    while (m_tabs->count())
        delete m_tabs->widget(0);
    m_project = QDir(dir).canonicalPath();
    m_tree->setRootIndex(m_files->setRootPath(m_project));
    m_stack->setCurrentIndex(1);
    for (QAction *a : std::as_const(m_projectActions))
        a->setEnabled(true);
    addRecent(m_project);
    QFile f(m_project + QStringLiteral("/app.json"));
    QString main = QStringLiteral("qml/Main.qml");
    if (f.open(QIODevice::ReadOnly))
        main = QJsonDocument::fromJson(f.readAll()).object().value(QLatin1String("main")).toString(main);
    openFile(m_project + QLatin1Char('/') + main);
    m_preview->setProject(m_project);
    runCheck();
    updateTitle();
}

void MainWindow::closeProject()
{
    if (!maybeSave())
        return;
    while (m_tabs->count())
        delete m_tabs->widget(0);
    m_project.clear();
    m_stack->setCurrentIndex(0);
    m_welcome->setRecent(recent());
    for (QAction *a : std::as_const(m_projectActions))
        a->setEnabled(false);
    m_problems->clear();
    m_statusProblems->clear();
    updateTitle();
}

CodeEditor *MainWindow::openFile(const QString &path, int line)
{
    const QString clean = QDir::cleanPath(path);
    for (int i = 0; i < m_tabs->count(); ++i)
        if (auto *e = qobject_cast<CodeEditor *>(m_tabs->widget(i)); e && e->path() == clean) {
            m_tabs->setCurrentIndex(i);
            if (line > 0)
                e->goToLine(line);
            return e;
        }
    const QString suffix = QFileInfo(clean).suffix().toLower();
    if (suffix == QLatin1String("png") || suffix == QLatin1String("svg")) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(clean));
        return nullptr;
    }
    auto *editor = new CodeEditor(clean, this);
    if (!editor->load()) {
        delete editor;
        return nullptr;
    }
    const int index = m_tabs->addTab(editor, QFileInfo(clean).fileName());
    m_tabs->setTabToolTip(index, QDir::toNativeSeparators(clean));
    m_tabs->setCurrentIndex(index);
    connect(editor->document(), &QTextDocument::modificationChanged, this, [this, editor](bool changed) {
        const int i = m_tabs->indexOf(editor);
        if (i >= 0)
            m_tabs->setTabText(i, QFileInfo(editor->path()).fileName() + (changed ? QStringLiteral(" *") : QString()));
    });
    // The preview follows the text as it is typed; the design check follows saves.
    connect(editor, &QPlainTextEdit::textChanged, this, [this, editor] {
        if (editor->path().endsWith(QLatin1String(".qml")) || editor->path().endsWith(QLatin1String(".js"))) {
            editor->save();
            m_preview->reloadSoon();
        }
    });
    connect(editor, &CodeEditor::saved, this, [this](const QString &p) {
        m_checkTimer->start();
        if (p.endsWith(QLatin1String("app.json")))
            m_preview->setProject(m_project);
    });
    if (line > 0)
        editor->goToLine(line);
    return editor;
}

CodeEditor *MainWindow::currentEditor() const
{
    return qobject_cast<CodeEditor *>(m_tabs->currentWidget());
}

bool MainWindow::saveAll()
{
    bool ok = true;
    for (int i = 0; i < m_tabs->count(); ++i)
        if (auto *e = qobject_cast<CodeEditor *>(m_tabs->widget(i)); e && e->modified())
            ok = e->save() && ok;
    return ok;
}

bool MainWindow::maybeSave()
{
    bool dirty = false;
    for (int i = 0; i < m_tabs->count(); ++i)
        if (auto *e = qobject_cast<CodeEditor *>(m_tabs->widget(i)); e && e->modified())
            dirty = true;
    if (!dirty)
        return true;
    const auto answer = QMessageBox::question(this, tr("Save changes?"), tr("Save the changes to the open files?"),
                                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (answer == QMessageBox::Cancel)
        return false;
    return answer == QMessageBox::Discard || saveAll();
}

void MainWindow::runCheck()
{
    if (m_project.isEmpty())
        return;
    QString error;
    const QVector<Problem> problems = m_sdk.check(m_project, &error);
    if (!error.isEmpty())
        appendOutput(error + QLatin1Char('\n'), true);
    showProblems(problems);
}

void MainWindow::showProblems(const QVector<Problem> &problems)
{
    m_problems->clear();
    int errors = 0;
    for (const Problem &p : problems) {
        auto *item = new QTreeWidgetItem(m_problems, {QStringLiteral("%1  %2").arg(p.code, p.message), p.file, QString::number(p.line)});
        item->setForeground(0, p.error ? QColor(QStringLiteral("#ff453a")) : QColor(QStringLiteral("#ff9f0a")));
        item->setToolTip(0, p.message);
        errors += p.error;
    }
    m_statusProblems->setText(problems.isEmpty() ? tr("Follows the design language")
                                                 : tr("%1 error(s), %2 warning(s)").arg(errors).arg(problems.size() - errors));
}

void MainWindow::sdkCommand(const QStringList &args, bool saveFirst)
{
    if (saveFirst)
        saveAll();
    m_stop->setEnabled(true);
    m_sdk.run(args, m_project.isEmpty() ? QDir::homePath() : m_project);
}

void MainWindow::appendOutput(const QString &text, bool error)
{
    QTextCursor c(m_output->document());
    c.movePosition(QTextCursor::End);
    QTextCharFormat f;
    f.setForeground(error ? QColor(QStringLiteral("#ff8170")) : m_output->palette().text().color());
    c.insertText(text.endsWith(QLatin1Char('\n')) ? text : text + QLatin1Char('\n'), f);
    m_output->verticalScrollBar()->setValue(m_output->verticalScrollBar()->maximum());
}

void MainWindow::openDocs(const QString &page)
{
    const QString dir = m_sdk.docsDir();
    if (!dir.isEmpty() && QFileInfo::exists(dir + QLatin1Char('/') + page))
        QDesktopServices::openUrl(QUrl::fromLocalFile(dir + QLatin1Char('/') + page));
    else
        QDesktopServices::openUrl(QUrl(QStringLiteral("https://github.com/tsryzenstech/astra/blob/main/docs/") + page));
}

QStringList MainWindow::recent() const
{
    return QSettings().value(QStringLiteral("recent")).toStringList();
}

void MainWindow::addRecent(const QString &dir)
{
    QStringList list = recent();
    list.removeAll(dir);
    list.prepend(dir);
    QSettings().setValue(QStringLiteral("recent"), list.mid(0, 10));
}

void MainWindow::updateTitle()
{
    setWindowTitle(m_project.isEmpty() ? QStringLiteral("Astra Studio")
                                       : QStringLiteral("%1 - Astra Studio").arg(QFileInfo(m_project).fileName()));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!maybeSave()) {
        event->ignore();
        return;
    }
    m_sdk.stop();
    QSettings().setValue(QStringLiteral("window/state"), saveState());
    event->accept();
}
