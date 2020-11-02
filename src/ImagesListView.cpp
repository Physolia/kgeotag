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
#include "ImagesListView.h"
#include "SharedObjects.h"
#include "ImagesModel.h"
#include "ImageCache.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QMouseEvent>
#include <QApplication>
#include <QMimeData>
#include <QDrag>
#include <QDebug>
#include <QKeyEvent>
#include <QMenu>

// C++ includes
#include <functional>

ImagesListView::ImagesListView(SharedObjects *sharedObjects, QWidget *parent)
    : QListView(parent),
      m_imageCache(sharedObjects->imageCache()),
      m_bookmarks(sharedObjects->bookmarks())
{
    setModel(sharedObjects->imagesModel());
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &QAbstractItemView::clicked,
            [this](const QModelIndex &index)
            {
                emit imageSelected(index.data(ImagesModel::Path).toString());
            });

    connect(this, &ImagesListView::imageSelected, this, &ImagesListView::checkCenterImage);

    // Context menu

    m_contextMenu = new QMenu(this);

    auto *searchMatchesMenu = m_contextMenu->addMenu(i18n("Automatic matching"));
    auto *searchExactMatchesAction = searchMatchesMenu->addAction(i18n("Exact match search"));

    auto *searchInterpolatedMatchesAction
        = searchMatchesMenu->addAction(i18n("Interpolated match search"));

    m_bookmarksMenu = m_contextMenu->addMenu(i18n("Assign to bookmark"));
    updateBookmarks();

    m_contextMenu->addSeparator();

    auto *assignToMapCenterAction = m_contextMenu->addAction(i18n("Assign to map center"));

    m_assignManually = m_contextMenu->addAction(i18n("Set coordinates manually"));

    m_editCoordinates = m_contextMenu->addAction(i18n("Edit coordinates"));

    m_lookupElevation = m_contextMenu->addAction(i18n("Lookup elevation"));

    m_contextMenu->addSeparator();

    m_removeCoordinates = m_contextMenu->addAction(i18n("Remove coordinates"));

    m_contextMenu->addSeparator();

    m_discardChanges = m_contextMenu->addAction(i18n("Discard changes"));

    connect(this, &QListView::customContextMenuRequested, this, &ImagesListView::showContextMenu);
}

void ImagesListView::updateBookmarks()
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
    }
}

void ImagesListView::mousePressEvent(QMouseEvent *event)
{
    const auto pos = event->pos();
    const auto selectedIndex = indexAt(pos);
    if (event->button() == Qt::LeftButton && selectedIndex.isValid()) {
        m_dragStarted = true;
        m_dragStartPosition = event->pos();
        emit imageSelected(selectedIndex.data(ImagesModel::Path).toString());
    } else {
        m_dragStarted = false;
    }

    QListView::mousePressEvent(event);
}

void ImagesListView::mouseMoveEvent(QMouseEvent *event)
{
    // Enable selecting more images by dragging when the shift key is pressed
    if ((event->buttons() & Qt::LeftButton) && event->modifiers() == Qt::ShiftModifier) {
        QListView::mouseMoveEvent(event);
        return;
    }

    if (! (event->buttons() & Qt::LeftButton)
        || ! m_dragStarted
        || (event->pos() - m_dragStartPosition).manhattanLength()
           < QApplication::startDragDistance()) {

        return;
    }

    const auto path = currentIndex().data(ImagesModel::Path).toString();

    auto *drag = new QDrag(this);
    const auto paths = selectedPaths();

    if (paths.count() == 1) {
        drag->setPixmap(QPixmap::fromImage(m_imageCache->thumbnail(path)));
    }

    QMimeData *mimeData = new QMimeData;
    QList<QUrl> urls;
    for (const auto &path : paths) {
        urls.append(QUrl::fromLocalFile(path));
    }
    mimeData->setUrls(urls);

    drag->setMimeData(mimeData);
    drag->exec(Qt::MoveAction);
}

QVector<QString> ImagesListView::selectedPaths() const
{
    QVector<QString> paths;
    const auto selected = selectedIndexes();
    for (const auto &index : selected) {
        paths.append(index.data(ImagesModel::Path).toString());
    }
    return paths;
}

void ImagesListView::checkCenterImage(const QString &path) const
{
    if (m_imageCache->coordinates(path) != KGeoTag::NoCoordinates) {
        emit centerImage(path);
    }
}

void ImagesListView::keyPressEvent(QKeyEvent *event)
{
    QListView::keyPressEvent(event);

    const auto key = event->key();
    if (! (key == Qt::Key_Up || key == Qt::Key_Down
           || key == Qt::Key_PageUp || key == Qt::Key_PageDown)) {

        return;
    }

    const auto index = currentIndex();
    if (index.isValid()) {
        emit imageSelected(index.data(ImagesModel::Path).toString());
    }
}

void ImagesListView::showContextMenu(const QPoint &point)
{
    const auto paths = selectedPaths();
    const int allPaths = paths.count();
    if (allPaths == 0) {
        return;
    }

    int hasCoordinates = 0;
    int changed = 0;
    for (const auto &path : paths) {
        if (m_imageCache->coordinates(path) != KGeoTag::NoCoordinates) {
            hasCoordinates++;
        }
        if (m_imageCache->changed(path)) {
            changed++;
        }
    }

    m_assignManually->setVisible(hasCoordinates == 0);
    m_editCoordinates->setVisible(hasCoordinates > 0);
    m_lookupElevation->setVisible(hasCoordinates == allPaths);
    m_removeCoordinates->setVisible(hasCoordinates > 0);
    m_discardChanges->setVisible(changed > 0);

    m_contextMenu->exec(mapToGlobal(point));
}
