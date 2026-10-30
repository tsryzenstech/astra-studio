// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <QColor>
#include <QDialog>

class QLineEdit;
class QListWidget;
class QPushButton;
class Sdk;

// File > New Project: a name, a package, a template from the SDK, the
// app's accent colour and where to put it. Runs `astra new`.
class NewProjectDialog : public QDialog
{
    Q_OBJECT
public:
    NewProjectDialog(Sdk *sdk, QWidget *parent = nullptr);

    QString name() const;
    QString package() const;
    QString templateId() const;
    QString location() const;     // the project folder to create
    QColor accent() const { return m_accent; }

private:
    void updatePackage();
    void validate();
    QLineEdit *m_name;
    QLineEdit *m_package;
    QLineEdit *m_folder;
    QListWidget *m_templates;
    QPushButton *m_accentButton;
    QPushButton *m_create;
    QColor m_accent{QStringLiteral("#0A84FF")};
    bool m_packageEdited = false;
};
