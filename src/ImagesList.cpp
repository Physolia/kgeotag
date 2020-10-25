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
#include "ImagesList.h"
#include "SharedObjects.h"
#include "Settings.h"
#include "ImageCache.h"
#include "ImageItem.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QIcon>
#include <QDebug>
#include <QApplication>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QFileInfo>
#include <QMenu>
#include <QAction>
#include <QKeyEvent>
#include <QInputDialog>

// C++ includes
#include <algorithm>
#include <utility>
#include <functional>

ImagesList::ImagesList(ImagesList::Type type, SharedObjects *sharedObjects,
                       const QHash<QString, KGeoTag::Coordinates> *bookmarks, QWidget *parent)
    : QListWidget(parent),
      m_type(type),
      m_settings(sharedObjects->settings()),
      m_imageCache(sharedObjects->imageCache()),
      m_elevationEngine(sharedObjects->elevationEngine()),
      m_bookmarks(bookmarks)
{
    connect(m_elevationEngine, &ElevationEngine::elevationProcessed,
            this, &ImagesList::elevationProcessed);

    setSortingEnabled(true);
    setIconSize(m_imageCache->thumbnailSize());

    connect(this, &QListWidget::currentItemChanged, this, &ImagesList::imageHighlighted);
    connect(this, &QListWidget::itemClicked, this, &ImagesList::processItemClicked);

    m_contextMenu = new QMenu(this);

    auto *assignToMapCenterAction = m_contextMenu->addAction(i18n("Assign to map center"));
    connect(assignToMapCenterAction, &QAction::triggered,
            [this]
            {
                emit assignToMapCenter(dynamic_cast<ImageItem *>(currentItem())->path());
            });

    m_bookmarksMenu = m_contextMenu->addMenu(i18n("Assign to bookmark"));
    updateBookmarks();

    if (m_type == Type::Assigned) {
        m_contextMenu->addSeparator();

        m_lookupElevation = m_contextMenu->addAction(i18n("Lookup elevation"));
        connect(m_lookupElevation, &QAction::triggered,
                [this]
                {
                    lookupElevation(dynamic_cast<ImageItem *>(currentItem())->path());
                });

        m_setElevation = m_contextMenu->addAction(i18n("Set elevation manually"));
        connect(m_setElevation, &QAction::triggered, this, &ImagesList::setElevation);
    }

    m_contextMenu->addSeparator();

    m_removeCoordinates = m_contextMenu->addAction(i18n("Remove coordinates"));
    connect(m_removeCoordinates, &QAction::triggered,
            [this]
            {
                emit removeCoordinates(dynamic_cast<ImageItem *>(currentItem())->path());
            });

    m_discardChanges = m_contextMenu->addAction(i18n("Discard changes"));
    connect(m_discardChanges, &QAction::triggered,
            [this]
            {
                emit discardChanges(dynamic_cast<ImageItem *>(currentItem())->path());
            });

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QListWidget::customContextMenuRequested, this, &ImagesList::showContextMenu);
}

void ImagesList::keyPressEvent(QKeyEvent *event)
{
    if (m_type == Type::Assigned) {
        const auto key = event->key();
        if (key == Qt::Key_Up || key == Qt::Key_Down
            || key == Qt::Key_PageUp || key == Qt::Key_PageDown) {

            m_itemBeforeKeyPress = currentItem();
        }
    }

    QListWidget::keyPressEvent(event);
}

void ImagesList::keyReleaseEvent(QKeyEvent *event)
{
    if (m_type == Type::Assigned) {
        const auto *item = currentItem();
        if (m_itemBeforeKeyPress != nullptr && m_itemBeforeKeyPress != item) {
            emit centerImage(dynamic_cast<const ImageItem *>(item)->path());
        }
        m_itemBeforeKeyPress = nullptr;
    }

    QListWidget::keyReleaseEvent(event);
}

void ImagesList::imageHighlighted(QListWidgetItem *item) const
{
    if (item == nullptr) {
        return;
    }

    emit imageSelected(dynamic_cast<ImageItem *>(item)->path());
}

void ImagesList::processItemClicked(QListWidgetItem *item)
{
    const auto path = dynamic_cast<ImageItem *>(item)->path();
    emit imageSelected(path);

    if (m_type == Type::Assigned) {
        emit centerImage(path);
    }
}

void ImagesList::addOrUpdateImage(const QString &path)
{
    bool itemFound = false;
    ImageItem *imageItem;

    const QFileInfo info(path);
    const QString fileName = info.fileName();

    // Check for an existing entry we want to update
    for (int i = 0; i < count(); i++) {
        if (dynamic_cast<ImageItem *>(item(i))->path() == path) {
            imageItem = dynamic_cast<ImageItem *>(item(i));
            itemFound = true;
            break;
        }
    }

    if (! itemFound) {
        // We need a new item
        imageItem = new ImageItem(QIcon(QPixmap::fromImage(m_imageCache->thumbnail(path))),
                                  fileName, path);
    }

    imageItem->setChanged(m_imageCache->changed(path));
    imageItem->setMatchType(m_imageCache->matchType(path));

    if (! itemFound) {
        addItem(imageItem);
    }

    if (currentItem() == imageItem) {
        emit imageSelected(path);
    }
}

QVector<QString> ImagesList::allImages() const
{
    QVector<QString> paths;
    for (int i = 0; i < count(); i++) {
        paths.append(dynamic_cast<ImageItem *>(item(i))->path());
    }
    return paths;
}

void ImagesList::removeImage(const QString &path)
{
    for (int i = 0; i < count(); i++) {
        if (dynamic_cast<ImageItem *>(item(i))->path() == path) {
            const auto *item = takeItem(i);
            delete item;
            clearSelection();
            return;
        }
    }
}

void ImagesList::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragStartPosition = event->pos();
    }

    QListWidget::mousePressEvent(event);
}

void ImagesList::mouseMoveEvent(QMouseEvent *event)
{
    const auto *item = currentItem();

    if (! (event->buttons() & Qt::LeftButton)
        || item == nullptr
        || (event->pos() - m_dragStartPosition).manhattanLength()
           < QApplication::startDragDistance()) {

        return;
    }

    const auto path = dynamic_cast<const ImageItem *>(item)->path();
    emit imageSelected(path);

    auto *drag = new QDrag(this);
    drag->setPixmap(item->icon().pixmap(iconSize()));

    QMimeData *mimeData = new QMimeData;
    mimeData->setText(path);
    drag->setMimeData(mimeData);
    drag->exec(Qt::MoveAction);
}

void ImagesList::showContextMenu(const QPoint &point)
{
    const auto *item = currentItem();
    if (item == nullptr) {
        return;
    }

    if (m_lookupElevation != nullptr) {
        m_lookupElevation->setEnabled(m_settings->lookupElevation());
    }

    const QString &path = dynamic_cast<const ImageItem *>(item)->path();
    m_removeCoordinates->setEnabled(m_imageCache->coordinates(path).isSet);
    m_discardChanges->setEnabled(m_imageCache->changed(path));

    m_contextMenu->exec(mapToGlobal(point));
}

void ImagesList::lookupElevation(const QString &path)
{
    QApplication::setOverrideCursor(Qt::BusyCursor);
    m_elevationEngine->request(ElevationEngine::Target::Image, path,
                               m_imageCache->coordinates(path));
}

void ImagesList::setElevation()
{
    const auto path = dynamic_cast<ImageItem *>(currentItem())->path();

    bool okay = false;
    auto elevation = QInputDialog::getDouble(this, i18n("Set elevation"),
                                             i18n("Elevation for \"%1\" (m)", path), 0, -12000,
                                             8900, 1, &okay);
    if (! okay) {
        return;
    }

    elevationProcessed(ElevationEngine::Target::Image, path, elevation);
}

void ImagesList::elevationProcessed(ElevationEngine::Target target, const QString &path,
                                    double elevation)
{
    if (target != ElevationEngine::Target::Image) {
        return;
    }

    QApplication::restoreOverrideCursor();

    auto coordinates = m_imageCache->coordinates(path);
    coordinates.alt = elevation;
    m_imageCache->setCoordinates(path, coordinates);

    emit checkUpdatePreview(path);
}

void ImagesList::updateBookmarks()
{
    m_bookmarksMenu->clear();
    auto bookmarks = m_bookmarks->keys();

    if (bookmarks.count() == 0) {
        m_bookmarksMenu->addAction(i18n("(No bookmarks defined)"));
        return;
    }

    std::sort(bookmarks.begin(), bookmarks.end());
    for (const auto &label : std::as_const(bookmarks)) {
        auto *entry = m_bookmarksMenu->addAction(label);
        entry->setData(label);
        connect(entry, &QAction::triggered,
                this, std::bind(&ImagesList::emitAssignTo, this, entry));
    }
}

void ImagesList::emitAssignTo(QAction *action)
{
    emit assignTo(dynamic_cast<ImageItem *>(currentItem())->path(),
                  m_bookmarks->value(action->data().toString()));
}
