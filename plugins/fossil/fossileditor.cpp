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

#include "fossileditor.h"
#include "annotationhighlighter.h"
#include "constants.h"
#include "fossilplugin.h"
#include "fossilclient.h"

#include <coreplugin/editormanager/editormanager.h>
#include <utils/qtcassert.h>
#include <utils/synchronousprocess.h>
#include <vcsbase/diffandloghighlighter.h>

#include <QRegularExpression>
#include <QRegExp>
#include <QString>
#include <QTextCursor>
#include <QTextBlock>
#include <QDir>
#include <QFileInfo>

namespace Fossil {
namespace Internal {

class FossilEditorWidgetPrivate
{
public:
    FossilEditorWidgetPrivate() :
        m_exactChangesetId(Constants::CHANGESET_ID_EXACT),
        m_firstChangesetId(QString("\n") + Constants::CHANGESET_ID + " "),
        m_nextChangesetId(m_firstChangesetId),
        m_configurationWidget(nullptr)
    {
        QTC_ASSERT(m_exactChangesetId.isValid(), return);
        QTC_ASSERT(m_firstChangesetId.isValid(), return);
        QTC_ASSERT(m_nextChangesetId.isValid(), return);
    }


    const QRegularExpression m_exactChangesetId;
    const QRegularExpression m_firstChangesetId;
    const QRegularExpression m_nextChangesetId;

    VcsBase::VcsBaseEditorConfig *m_configurationWidget;
};

FossilEditorWidget::FossilEditorWidget() :
    d(new FossilEditorWidgetPrivate)
{
    setAnnotateRevisionTextFormat(tr("&Annotate %1"));
    setAnnotatePreviousRevisionTextFormat(tr("Annotate &Parent Revision %1"));

    const QRegExp exactDiffFileIdPattern(Constants::DIFFFILE_ID_EXACT);
    QTC_ASSERT(exactDiffFileIdPattern.isValid(), return);
    setDiffFilePattern(exactDiffFileIdPattern);

    const QRegExp logChangePattern("^.*\\[([0-9a-f]{5,40})\\]");
    QTC_ASSERT(logChangePattern.isValid(), return);
    setLogEntryPattern(logChangePattern);
}

FossilEditorWidget::~FossilEditorWidget()
{
    delete d;
}

bool FossilEditorWidget::setConfigurationWidget(VcsBase::VcsBaseEditorConfig *w)
{
    if (configurationAdded())
        return false;

    d->m_configurationWidget = w;
    setConfigurationAdded();
    return true;
}

VcsBase::VcsBaseEditorConfig *FossilEditorWidget::configurationWidget() const
{
    return d->m_configurationWidget;
}

QSet<QString> FossilEditorWidget::annotationChanges() const
{

    const QString txt = toPlainText();
    if (txt.isEmpty())
        return QSet<QString>();

    // extract changeset id at the beginning of each annotated line:
    // <changeid> ...:

    QSet<QString> changes;

    QRegularExpressionMatch firstChangesetIdMatch = d->m_firstChangesetId.match(txt);
    if (firstChangesetIdMatch.hasMatch()) {
        QString changeId = firstChangesetIdMatch.captured(1);
        changes.insert(changeId);

        QRegularExpressionMatchIterator i = d->m_nextChangesetId.globalMatch(txt);
        while (i.hasNext()) {
            const QRegularExpressionMatch nextChangesetIdMatch = i.next();
            changeId = nextChangesetIdMatch.captured(1);
            changes.insert(changeId);
        }
    }
    return changes;
}

QString FossilEditorWidget::changeUnderCursor(const QTextCursor &cursorIn) const
{
    QTextCursor cursor = cursorIn;
    cursor.select(QTextCursor::WordUnderCursor);
    if (cursor.hasSelection()) {
        const QString change = cursor.selectedText();
        QRegularExpressionMatch exactChangesetIdMatch = d->m_exactChangesetId.match(change);
        if (exactChangesetIdMatch.hasMatch())
            return change;
    }
    return QString();
}


VcsBase::BaseAnnotationHighlighter *FossilEditorWidget::createAnnotationHighlighter(const QSet<QString> &changes) const
{
    return new FossilAnnotationHighlighter(changes);
}

} // namespace Internal
} // namespace Fossil