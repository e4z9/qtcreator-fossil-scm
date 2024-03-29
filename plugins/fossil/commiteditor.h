/****************************************************************************
**
** Copyright (c) 2018 Artur Shepilko
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

#pragma once

#include <vcsbase/vcsbaseclient.h>
#include <vcsbase/vcsbasesubmiteditor.h>

namespace VcsBase { class SubmitFileModel; }

namespace Fossil {
namespace Internal {

class BranchInfo;
class FossilCommitWidget;

class CommitEditor : public VcsBase::VcsBaseSubmitEditor
{
    Q_OBJECT

public:
    CommitEditor();

    void setFields(const Utils::FilePath &repositoryRoot, const BranchInfo &branch,
                   const QStringList &tags, const QString &userName,
                   const QList<VcsBase::VcsBaseClient::StatusItem> &repoStatus);

    FossilCommitWidget *commitWidget();

private:
    VcsBase::SubmitFileModel *m_fileModel = nullptr;
};

} // namespace Internal
} // namespace Fossil
