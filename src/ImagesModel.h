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

#ifndef IMAGESMODEL_H
#define IMAGESMODEL_H

// Local includes
#include "KGeoTag.h"

// KDE includes
#include <KColorScheme>

// Qt includes
#include <QAbstractListModel>
#include <QDateTime>

// Local classes
class ImageCache;

class ImagesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum DataRole {
        Path = Qt::UserRole,
        Changed
    };

    explicit ImagesModel(QObject *parent, ImageCache *imageCache, bool splitImagesList);
    virtual int rowCount(const QModelIndex &) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void addImage(const QString &path);
    void setChanged(const QString &path, bool changed);
    void setMatchType(const QString &path, int matchType);
    QVector<QString> changedImages() const;
    int matchType(const QString &path) const;

private: // Functions
    void emitDataChanged(const QString &path);

private: // Variables
    struct ImageData {
        QString fileName;
        bool changed;
        int matchType;
    };

    ImageCache *m_imageCache;
    bool m_splitImagesList;
    QVector<QString> m_paths;
    QHash<QString, ImageData> m_imageData;

    KColorScheme m_colorScheme;

};

#endif // IMAGESMODEL_H
