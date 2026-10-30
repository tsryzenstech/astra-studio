// SPDX-License-Identifier: Apache-2.0
#include "newprojectdialog.h"
#include "sdk.h"

#include <QColorDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QVBoxLayout>

NewProjectDialog::NewProjectDialog(Sdk *sdk, QWidget *parent)
    : QDialog(parent)
    , m_name(new QLineEdit(this))
    , m_package(new QLineEdit(this))
    , m_folder(new QLineEdit(this))
    , m_templates(new QListWidget(this))
    , m_accentButton(new QPushButton(this))
{
    setWindowTitle(tr("New astraOS App"));
    setMinimumWidth(560);

    for (const Sdk::Template &t : sdk->templates()) {
        auto *item = new QListWidgetItem(QStringLiteral("%1\n%2").arg(t.id, t.description), m_templates);
        item->setData(Qt::UserRole, t.id);
    }
    m_templates->setCurrentRow(0);
    m_templates->setSpacing(4);

    m_name->setPlaceholderText(tr("My App"));
    m_package->setPlaceholderText(QStringLiteral("com.example.myapp"));
    m_folder->setText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QStringLiteral("/AstraProjects"));
    auto *browse = new QPushButton(tr("Browse..."), this);
    auto *folderRow = new QHBoxLayout;
    folderRow->addWidget(m_folder, 1);
    folderRow->addWidget(browse);
    m_accentButton->setFixedWidth(120);

    auto *form = new QFormLayout;
    form->addRow(tr("Name"), m_name);
    form->addRow(tr("Package"), m_package);
    form->addRow(tr("Accent"), m_accentButton);
    form->addRow(tr("Location"), folderRow);

    auto *hint = new QLabel(tr("Apps are built from astraOS's components and follow the system's design language: "
                               "the accent is the one colour of their own."), this);
    hint->setWordWrap(true);
    hint->setStyleSheet(QStringLiteral("color: palette(placeholder-text)"));

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    m_create = buttons->addButton(tr("Create"), QDialogButtonBox::AcceptRole);
    m_create->setDefault(true);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Template"), this));
    layout->addWidget(m_templates, 1);
    layout->addLayout(form);
    layout->addWidget(hint);
    layout->addWidget(buttons);

    auto paintAccent = [this] {
        m_accentButton->setText(m_accent.name().toUpper());
        m_accentButton->setStyleSheet(QStringLiteral("background: %1; color: white; border-radius: 6px; padding: 6px")
                                          .arg(m_accent.name()));
    };
    paintAccent();
    connect(m_accentButton, &QPushButton::clicked, this, [this, paintAccent] {
        const QColor c = QColorDialog::getColor(m_accent, this, tr("Accent"));
        if (c.isValid()) {
            m_accent = c;
            paintAccent();
        }
    });
    connect(browse, &QPushButton::clicked, this, [this] {
        const QString d = QFileDialog::getExistingDirectory(this, tr("Location"), m_folder->text());
        if (!d.isEmpty())
            m_folder->setText(d);
    });
    connect(m_name, &QLineEdit::textChanged, this, &NewProjectDialog::updatePackage);
    connect(m_package, &QLineEdit::textEdited, this, [this] { m_packageEdited = true; validate(); });
    connect(m_folder, &QLineEdit::textChanged, this, &NewProjectDialog::validate);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    validate();
}

void NewProjectDialog::updatePackage()
{
    if (!m_packageEdited) {
        QString slug = m_name->text().toLower();
        slug.remove(QRegularExpression(QStringLiteral("[^a-z0-9]")));
        m_package->setText(slug.isEmpty() ? QString() : QStringLiteral("com.example.") + slug);
    }
    validate();
}

void NewProjectDialog::validate()
{
    static const QRegularExpression pkg(QStringLiteral("^[a-z][a-z0-9_]*(\\.[a-z][a-z0-9_]*)+$"));
    const bool ok = !m_name->text().trimmed().isEmpty() && pkg.match(m_package->text()).hasMatch() &&
                    !m_folder->text().trimmed().isEmpty() && !QDir(location()).exists();
    m_create->setEnabled(ok);
}

QString NewProjectDialog::name() const { return m_name->text().trimmed(); }
QString NewProjectDialog::package() const { return m_package->text().trimmed(); }

QString NewProjectDialog::templateId() const
{
    return m_templates->currentItem() ? m_templates->currentItem()->data(Qt::UserRole).toString() : QStringLiteral("app");
}

QString NewProjectDialog::location() const
{
    QString folder = name();
    folder.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_-]+")), QStringLiteral("-"));
    return QDir(m_folder->text().trimmed()).filePath(folder);
}
