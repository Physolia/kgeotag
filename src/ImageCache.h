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

#ifndef IMAGECACHE_H
#define IMAGECACHE_H

// Qt includes
#include <QObject>
#include <QHash>
#include <QImage>

class ImageCache : public QObject
{
    Q_OBJECT

public:
    explicit ImageCache(QObject *parent);
    bool addImage(const QString &path);
    QImage thumbnail(const QString &path) const;
    QImage preview(const QString &path) const;

private: // Variables
    QHash<QString, QImage> m_thumbnails;
    QHash<QString, QImage> m_previews;

};

#endif // IMAGECACHE_H
