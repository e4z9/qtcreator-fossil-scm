/****************************************************************************
**
** Copyright (c) 2016 Artur Shepilko
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "fossilcommitwidget.h"
#include "branchinfo.h"

#include <texteditor/texteditorsettings.h>
#include <texteditor/fontsettings.h>
#include <texteditor/texteditorconstants.h>

#include <utils/completingtextedit.h>
#include <utils/qtcassert.h>

#include <QSyntaxHighlighter>
#include <QTextEdit>

#include <QRegularExpression>
#include <QDir>

namespace Fossil {
namespace Internal {

// Retrieve the comment char format from the text editor.
static QTextCharFormat commentFormat()
{
    const TextEditor::FontSettings settings = TextEditor::TextEditorSettings::instance()->fontSettings();
    return settings.toTextCharFormat(TextEditor::C_COMMENT);
}

// Highlighter for Fossil submit messages.
// Marks up [ticket-id] fields in the message.
class FossilSubmitHighlighter : QSyntaxHighlighter
{
public:
    explicit FossilSubmitHighlighter(Utils::CompletingTextEdit *parent);
    void highlightBlock(const QString &text) final;

private:
    const QTextCharFormat m_commentFormat;
    const QRegularExpression m_keywordPattern;
};

FossilSubmitHighlighter::FossilSubmitHighlighter(Utils::CompletingTextEdit *parent) : QSyntaxHighlighter(parent),
    m_commentFormat(commentFormat()),
    m_keywordPattern("\\[([0-9a-f]{5,40})\\]")
{
    QTC_CHECK(m_keywordPattern.isValid());
}

void FossilSubmitHighlighter::highlightBlock(const QString &text)
{
    // Fossil commit message allows listing of [ticket-id],
    // where ticket-id is a partial SHA1.
    // Match the ticket-ids and highlight them for convenience.

    // Format keywords
    QRegularExpressionMatchIterator i = m_keywordPattern.globalMatch(text);
    while (i.hasNext()) {
        const QRegularExpressionMatch keywordMatch = i.next();
        QTextCharFormat charFormat = format(0);
        charFormat.setFontItalic(true);
        setFormat(keywordMatch.capturedStart(0), keywordMatch.capturedLength(0), charFormat);
    }
}


FossilCommitWidget::FossilCommitWidget() : m_commitPanel(new QWidget)
{
    m_commitPanelUi.setupUi(m_commitPanel);
    insertTopWidget(m_commitPanel);
    new FossilSubmitHighlighter(descriptionEdit());
    m_branchValidator = new QRegularExpressionValidator(QRegularExpression("[^\\n]*"), this);

    connect(m_commitPanelUi.branchLineEdit, &QLineEdit::textChanged,
            this, &FossilCommitWidget::branchChanged);
}

void FossilCommitWidget::setFields(const QString &repoPath, const BranchInfo &branch,
                                   const QStringList &tags, const QString &userName)
{
    m_commitPanelUi.localRootLineEdit->setText(QDir::toNativeSeparators(repoPath));
    m_commitPanelUi.currentBranchLineEdit->setText(branch.name());
    const QString tagsText = tags.join(", ");
    m_commitPanelUi.currentTagsLineEdit->setText(tagsText);
    m_commitPanelUi.authorLineEdit->setText(userName);

    branchChanged();
}

QString FossilCommitWidget::newBranch() const
{
    const QString branchName = m_commitPanelUi.branchLineEdit->text().trimmed();
    return branchName;
}

QStringList FossilCommitWidget::tags() const
{
    QString tagsText = m_commitPanelUi.tagsLineEdit->text().trimmed();
    if (tagsText.isEmpty())
        return QStringList();

    tagsText.replace(',', ' ');
    const QStringList tags = tagsText.split(' ', QString::SkipEmptyParts);
    return tags;
}

QString FossilCommitWidget::committer() const
{
    const QString author = m_commitPanelUi.authorLineEdit->text();
    if (author.isEmpty())
        return QString();

    const QString user = author;
    return user;
}

bool FossilCommitWidget::isPrivateOptionEnabled() const
{
    return m_commitPanelUi.isPrivateCheckBox->isChecked();
}

bool FossilCommitWidget::canSubmit() const
{
    QString message = cleanupDescription(descriptionText()).trimmed();

    if (m_commitPanelUi.invalidBranchLabel->isVisible() || message.isEmpty())
        return false;

    return VcsBase::SubmitEditorWidget::canSubmit();
}

void FossilCommitWidget::branchChanged()
{
    m_commitPanelUi.invalidBranchLabel->setVisible(!isValidBranch());

    updateSubmitAction();
}

bool FossilCommitWidget::isValidBranch() const
{
    int pos = m_commitPanelUi.branchLineEdit->cursorPosition();
    QString text = m_commitPanelUi.branchLineEdit->text();
    return m_branchValidator->validate(text, pos) == QValidator::Acceptable;
}

} // namespace Internal
} // namespace Fossil