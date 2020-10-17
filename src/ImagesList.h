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

#ifndef IMAGESLIST_H
#define IMAGESLIST_H

// Local includes
#include "KGeoTag.h"

// Qt includes
#include <QListWidget>

// Local classes
class Settings;
class ImageCache;

// Qt classes
class QMouseEvent;
class QMenu;
class QAction;

class ImagesList : public QListWidget
{
    Q_OBJECT

public:
    enum Type {
        UnAssigned,
        Assigned
    };

    explicit ImagesList(Type type, Settings *settings, ImageCache *imageCache,
                        QWidget *parent = nullptr);
    void addOrUpdateImage(const QString &path);
    QVector<QString> allImages() const;
    void removeImage(const QString &path);

signals:
    void imageSelected(const QString &path, bool center = true) const;
    void lookupElevation(const QString &path) const;
    void removeCoordinates(const QString &path) const;
    void discardChanges(const QString &path) const;

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void imageHighlighted(QListWidgetItem *item, QListWidgetItem *) const;
    void showContextMenu(const QPoint &point);

private: // Variables
    Type m_type;
    Settings *m_settings;
    ImageCache *m_imageCache;
    QPoint m_dragStartPosition;
    QMenu *m_contextMenu;
    QAction *m_lookupElevation = nullptr;
    QAction *m_removeCoordinates;
    QAction *m_discardChanges;

};

#endif // IMAGESLIST_H
