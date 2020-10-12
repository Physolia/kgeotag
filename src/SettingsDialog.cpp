/* Copyright (c) 2020 Tobias Leupold <tobias.leupold@gmx.de>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// Local includes
#include "SettingsDialog.h"
#include "Settings.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QDebug>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>

SettingsDialog::SettingsDialog(Settings *settings, QWidget *parent)
    : QDialog(parent), m_settings(settings)
{
    auto *layout = new QVBoxLayout(this);

    auto *trackBox = new QGroupBox(i18n("GPX track rendering"));
    auto *trackBoxLayout = new QGridLayout(trackBox);
    layout->addWidget(trackBox);

    trackBoxLayout->addWidget(new QLabel(i18n("Color:")), 0, 0);
    m_trackColor = new QPushButton;
    m_trackColor->setFlat(true);
    m_trackColor->setStyleSheet(QStringLiteral("border: none; background-color: %1;").arg(
                                               m_settings->trackColor().name()));
    trackBoxLayout->addWidget(m_trackColor, 0, 1);

    trackBoxLayout->addWidget(new QLabel(i18n("Line width:")), 1, 0);
    m_trackWidth = new QSpinBox;
    m_trackWidth->setMinimum(1);
    m_trackWidth->setMaximum(50);
    trackBoxLayout->addWidget(m_trackWidth, 1, 1);

    trackBoxLayout->addWidget(new QLabel(i18n("Line style:")), 2, 0);
    m_trackStyle = new QComboBox;
    m_trackStyle->addItem(i18n("Solid"), static_cast<int>(Qt::SolidLine));
    m_trackStyle->addItem(i18n("Dash"), static_cast<int>(Qt::DashLine));
    m_trackStyle->addItem(i18n("Dot"), static_cast<int>(Qt::DotLine));
    m_trackStyle->addItem(i18n("Dash-Dot"), static_cast<int>(Qt::DashDotLine));
    m_trackStyle->addItem(i18n("Dash-Dot-Dot"), static_cast<int>(Qt::DashDotDotLine));
    trackBoxLayout->addWidget(m_trackStyle, 2, 1);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);
}

void SettingsDialog::accept()
{
    QDialog::accept();
}
