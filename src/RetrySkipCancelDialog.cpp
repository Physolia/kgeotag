/* Copyright (C) 2020 Tobias Leupold <tobias.leupold@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e. V. (or its successor approved
   by the membership of KDE e. V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

// Local includes
#include "RetrySkipCancelDialog.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QAbstractButton>

RetrySkipCancelDialog::RetrySkipCancelDialog(QWidget *parent, const QString &title,
                                             const QString &text, bool isSingleFile)
    : QMessageBox(parent)
{
    setIcon(QMessageBox::Warning);
    setWindowTitle(title);

    if (isSingleFile) {
        setStandardButtons(QMessageBox::Retry | QMessageBox::Abort);
    } else {
        setStandardButtons(QMessageBox::Retry | QMessageBox::Discard | QMessageBox::Abort);
        button(QMessageBox::Discard)->setText(i18n("Skip current image"));
    }

    setText(text);
}

int RetrySkipCancelDialog::exec()
{
    const auto reply = QMessageBox::exec();
    if (reply == QMessageBox::Discard) {
        return Reply::Skip;
    } else if (reply == QMessageBox::Abort) {
        return Reply::Abort;
    } else {
        return Reply::Retry;
    }
}
