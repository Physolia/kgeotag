/* SPDX-FileCopyrightText: 2021 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef GEODATAMODEL_H
#define GEODATAMODEL_H

// Local includes
#include "Coordinates.h"

// Qt includes
#include <QAbstractListModel>
#include <QHash>
#include <QDateTime>

class GeoDataModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit GeoDataModel(QObject *parent);

    int rowCount(const QModelIndex & = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void addTrack(const QString &path, const QVector<QVector<QDateTime>> &times,
                  const QVector<QVector<Coordinates>> &segments);
    bool contains(const QString &path);

private: // Variables
    QVector<QString> m_loadedFiles;
    QVector<QVector<QDateTime>> m_dateTimes;
    QVector<QHash<QDateTime, Coordinates>> m_trackPoints;

};

#endif // GEODATAMODEL_H
