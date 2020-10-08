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

#ifndef IMAGESWIDGET_H
#define IMAGESWIDGET_H

// Qt includes
#include <QWidget>

// Local classes
class Settings;
class ImageCache;

// Qt classes
class QListWidget;
class QListWidgetItem;

class ImagesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImagesWidget(Settings *settings, ImageCache *imageCache, QWidget *parent = nullptr);

signals:
    void imageSelected(const QString &path) const;

public slots:
    void addImages();

private: // Variables
    Settings *m_settings;
    ImageCache *m_imageCache;
    QListWidget *m_images;

};

#endif // IMAGESWIDGET_H
