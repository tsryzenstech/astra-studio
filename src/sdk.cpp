// SPDX-License-Identifier: Apache-2.0
#include "sdk.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

Sdk::Sdk(QObject *parent)
    : QObject(parent)
{
    // ASTRA_SDK (a checkout's sdk/ while developing), else the bundled one.
    const QStringList candidates{qEnvironmentVariable("ASTRA_SDK"),
                                 QCoreApplication::applicationDirPath() + QStringLiteral("/sdk"),
                                 QCoreApplication::applicationDirPath() + QStringLiteral("/../astra/sdk")};
    for (const QString &c : candidates)
        if (!c.isEmpty() && QFileInfo::exists(c + QStringLiteral("/bin/astra"))) {
            m_root = QDir(c).canonicalPath();
            break;
        }
    // The Windows launcher "py -3" first, then python3 / python.
    if (const QString py = QStandardPaths::findExecutable(QStringLiteral("py")); !py.isEmpty()) {
        m_python = py;
        m_pythonArgs = {QStringLiteral("-3")};
    } else {
        for (const char *name : {"python3", "python"})
            if (const QString p = QStandardPaths::findExecutable(QString::fromLatin1(name)); !p.isEmpty()) {
                m_python = p;
                break;
            }
    }
}

QString Sdk::docsDir() const
{
    for (const QString &d : {m_root + QStringLiteral("/../docs"), QCoreApplication::applicationDirPath() + QStringLiteral("/docs")})
        if (QFileInfo::exists(d + QStringLiteral("/DESIGN.md")))
            return QDir(d).canonicalPath();
    return {};
}

QString Sdk::problem() const
{
    if (m_root.isEmpty())
        return tr("The astraOS SDK is missing: reinstall Astra Studio, or set ASTRA_SDK to an astraOS checkout's sdk folder.");
    if (m_python.isEmpty())
        return tr("Python 3 is needed to build apps: install it from python.org (tick \"Add to PATH\").");
    return {};
}

QVector<Sdk::Template> Sdk::templates() const
{
    QVector<Template> out;
    const QDir dir(m_root + QStringLiteral("/templates"));
    for (const QString &name : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
        QFile f(dir.filePath(name + QStringLiteral("/template.json")));
        if (!f.open(QIODevice::ReadOnly))
            continue;
        out.append({name, QJsonDocument::fromJson(f.readAll()).object().value(QLatin1String("description")).toString()});
    }
    return out;
}

QStringList Sdk::command(const QStringList &args) const
{
    return QStringList(m_pythonArgs) << m_root + QStringLiteral("/bin/astra") << args;
}

void Sdk::run(const QStringList &args, const QString &dir)
{
    stop();
    if (!available()) {
        Q_EMIT output(problem() + QLatin1Char('\n'), true);
        Q_EMIT finished(-1, args);
        return;
    }
    m_args = args;
    m_process = new QProcess(this);
    m_process->setWorkingDirectory(dir);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("PYTHONUNBUFFERED"), QStringLiteral("1"));
    env.insert(QStringLiteral("PYTHONIOENCODING"), QStringLiteral("utf-8"));
    m_process->setProcessEnvironment(env);
    QProcess *p = m_process;
    connect(p, &QProcess::readyReadStandardOutput, this, [this, p] {
        Q_EMIT output(QString::fromUtf8(p->readAllStandardOutput()), false);
    });
    connect(p, &QProcess::readyReadStandardError, this, [this, p] {
        Q_EMIT output(QString::fromUtf8(p->readAllStandardError()), true);
    });
    connect(p, &QProcess::finished, this, [this, p](int code) {
        if (p == m_process)
            m_process = nullptr;
        Q_EMIT finished(code, m_args);
        p->deleteLater();
    });
    Q_EMIT output(QStringLiteral("> astra %1\n").arg(args.join(QLatin1Char(' '))), false);
    p->start(m_python, command(args));
}

void Sdk::stop()
{
    if (!m_process)
        return;
    QProcess *p = std::exchange(m_process, nullptr);
    p->kill();
    p->waitForFinished(2000);
    p->deleteLater();
}

QVector<Problem> Sdk::check(const QString &projectDir, QString *error) const
{
    QVector<Problem> out;
    if (!available()) {
        if (error)
            *error = problem();
        return out;
    }
    QProcess p;
    p.setWorkingDirectory(projectDir);
    p.start(m_python, command({QStringLiteral("check"), QStringLiteral("--json"), projectDir}));
    if (!p.waitForFinished(20000)) {
        if (error)
            *error = tr("The design check did not answer.");
        return out;
    }
    const QJsonArray list = QJsonDocument::fromJson(p.readAllStandardOutput()).array();
    for (const QJsonValue &v : list) {
        const QJsonObject o = v.toObject();
        out.append({o.value(QLatin1String("file")).toString(), o.value(QLatin1String("line")).toInt(),
                    o.value(QLatin1String("severity")).toString() == QLatin1String("error"),
                    o.value(QLatin1String("code")).toString(), o.value(QLatin1String("message")).toString()});
    }
    return out;
}
