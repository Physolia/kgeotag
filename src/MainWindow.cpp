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
#include "MainWindow.h"
#include "Settings.h"
#include "ImageCache.h"
#include "DragableImagesList.h"
#include "PreviewWidget.h"
#include "MapWidget.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QMenuBar>
#include <QAction>
#include <QDockWidget>
#include <QGuiApplication>
#include <QScreen>
#include <QApplication>
#include <QDebug>
#include <QFileDialog>

MainWindow::MainWindow() : QMainWindow()
{
    m_settings = new Settings(this);
    m_imageCache = new ImageCache(this, m_settings);

    // Menu setup

    auto *fileMenu = menuBar()->addMenu(i18n("File"));

    auto *addImagesAction = fileMenu->addAction(i18n("Add images"));

    fileMenu->addSeparator();

    auto *quitAction = fileMenu->addAction(i18n("Quit"));
    connect(quitAction, &QAction::triggered, this, &QWidget::close);

    // Dock setup

    setDockNestingEnabled(true);

    m_unassignedImages = new DragableImagesList(m_imageCache);
    auto *unassignedImagesDock = createDockWidget(i18n("Unassigned images"), m_unassignedImages,
                                        QStringLiteral("unassignedImagesDock"));
    connect(addImagesAction, &QAction::triggered, this, &MainWindow::addImages);

    m_previewWidget = new PreviewWidget(m_imageCache);
    auto *previewDock = createDockWidget(i18n("Preview"), m_previewWidget,
                                         QStringLiteral("previewDock"));
    connect(m_unassignedImages, &ImagesList::imageSelected,
            m_previewWidget, &PreviewWidget::setImage);

    m_mapWidget = new MapWidget(m_settings, m_imageCache);
    createDockWidget(i18n("Map"), m_mapWidget, QStringLiteral("mapDock"));

    // Size initialization/restoration
    if (! restoreGeometry(m_settings->mainWindowGeometry())) {
        const QRect availableGeometry = QGuiApplication::primaryScreen()->availableGeometry();
        if (availableGeometry.width() > 1024 && availableGeometry.height() > 765) {
            resize(QSize(1024, 765));
        } else {
            setWindowState(Qt::WindowMaximized);
        }
    }

    // Initialize/Restore the dock widget arrangement
    if (! restoreState(m_settings->mainWindowState())) {
        splitDockWidget(unassignedImagesDock, previewDock, Qt::Vertical);
    }

    // Restore the map's settings
    m_mapWidget->restoreSettings();
}

QDockWidget *MainWindow::createDockWidget(const QString &title, QWidget *widget,
                                          const QString &objectName)
{
    auto *dock = new QDockWidget(title, this);
    dock->setObjectName(objectName);
    dock->setContextMenuPolicy(Qt::PreventContextMenu);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    dock->setWidget(widget);
    addDockWidget(Qt::TopDockWidgetArea, dock);
    return dock;
}

void MainWindow::closeEvent(QCloseEvent *)
{
    m_settings->saveMainWindowGeometry(saveGeometry());
    m_settings->saveMainWindowState(saveState());

    m_mapWidget->saveSettings();

    QApplication::quit();
}

void MainWindow::addImages()
{
    const auto files = QFileDialog::getOpenFileNames(this,
                           i18n("Please select the images to add"),
                           m_settings->lastImagesOpenPath(),
                           i18n("JPEG Images (*.jpg *.jpeg)"));
    if (files.isEmpty()) {
        return;
    }

    const QFileInfo info(files.at(0));
    m_settings->saveLastImagesOpenPath(info.dir().absolutePath());

    QApplication::setOverrideCursor(Qt::WaitCursor);
    for (const auto &path : files) {
        const QFileInfo info(path);
        const QString canonicalPath = info.canonicalFilePath();
        if (m_imageCache->addImage(canonicalPath)) {
            m_unassignedImages->addImage(info.fileName(), canonicalPath);
        }
    }
    QApplication::restoreOverrideCursor();
}
