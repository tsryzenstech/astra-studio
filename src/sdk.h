// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QVector>

// One finding of the design language check (astra check --json).
struct Problem {
    QString file;
    int line = 0;
    bool error = false;
    QString code;
    QString message;
};

// The astraOS SDK next to Astra Studio (sdk/: the astra command, templates,
// design check, apkgtool, adu), run with Python 3.
class Sdk : public QObject
{
    Q_OBJECT
public:
    explicit Sdk(QObject *parent = nullptr);

    QString root() const { return m_root; }         // .../sdk
    QString docsDir() const;                        // .../docs
    QString python() const { return m_python; }
    bool available() const { return !m_python.isEmpty() && !m_root.isEmpty(); }
    QString problem() const;                        // why it is not available

    struct Template { QString id, description; };
    QVector<Template> templates() const;

    // Runs `astra <args>` in dir; output and the end come as signals.
    // Only one command at a time: a new one stops the previous.
    void run(const QStringList &args, const QString &dir);
    void stop();
    bool running() const { return m_process && m_process->state() != QProcess::NotRunning; }

    // `astra check --json`, synchronous (it takes a fraction of a second).
    QVector<Problem> check(const QString &projectDir, QString *error = nullptr) const;

Q_SIGNALS:
    void output(const QString &text, bool error);
    void finished(int exitCode, const QStringList &args);

private:
    QStringList command(const QStringList &args) const;
    QString m_root;
    QString m_python;
    QStringList m_pythonArgs;
    QProcess *m_process = nullptr;
    QStringList m_args;
};
