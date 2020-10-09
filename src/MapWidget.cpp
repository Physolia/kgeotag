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
#include "MapWidget.h"
#include "Settings.h"
#include "ImageCache.h"
#include "Coordinates.h"

// Marble includes
#include <marble/GeoPainter.h>
#include <marble/AbstractFloatItem.h>
#include <marble/MarbleModel.h>

// Qt includes
#include <QDebug>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDropEvent>
#include <QXmlStreamReader>

MapWidget::MapWidget(Settings *settings, ImageCache *imageCache, QWidget *parent)
    : Marble::MarbleWidget(parent), m_settings(settings), m_imageCache(imageCache)
{
    setAcceptDrops(true);

    setProjection(Marble::Mercator);
    setMapThemeId(QStringLiteral("earth/openstreetmap/openstreetmap.dgml"));

    m_trackPen.setColor(QColor(255, 0, 255, 150));
    m_trackPen.setWidth(3);
    m_trackPen.setStyle(Qt::DotLine);
    m_trackPen.setCapStyle(Qt::RoundCap);
    m_trackPen.setJoinStyle(Qt::RoundJoin);
}

void MapWidget::addGpx(const QString &path)
{

    QFile gpxFile(path);
    gpxFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xml(&gpxFile);

    Marble::GeoDataLineString lineString;
    double lon;
    double lat;
    QDateTime time;

    while (! xml.atEnd() && ! xml.hasError()) {
        const QXmlStreamReader::TokenType token = xml.readNext();
        const QStringRef name = xml.name();

        if (token == QXmlStreamReader::StartElement) {
            if (name == QStringLiteral("trkpt")) {
                QXmlStreamAttributes attributes = xml.attributes();
                lon = attributes.value(QStringLiteral("lon")).toDouble();
                lat = attributes.value(QStringLiteral("lat")).toDouble();
                const Marble::GeoDataCoordinates coordinates
                    = Marble::GeoDataCoordinates(lon, lat, 0.0, Marble::GeoDataCoordinates::Degree);
                lineString.append(coordinates);

            } else if (name == QStringLiteral("time")) {
                xml.readNext();
                time = QDateTime::fromString(xml.text().toString(), Qt::ISODate);
            }

        } else if (token == QXmlStreamReader::EndElement) {
            if (name == QStringLiteral("trkseg") && ! lineString.isEmpty()) {
                m_tracks.append(lineString);
                lineString.clear();

            } else if (name == QStringLiteral("time")) {
                m_points[time] = Coordinates::Data { lon, lat };
            }
        }
    }

}

void MapWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(QStringLiteral("text/plain"))
        && m_imageCache->contains(event->mimeData()->text())) {

        event->acceptProposedAction();
    }
}

void MapWidget::dropEvent(QDropEvent *event)
{
    const auto dropPosition = event->pos();

    qreal lon;
    qreal lat;
    if (! geoCoordinates(dropPosition.x(), dropPosition.y(), lon, lat,
                         Marble::GeoDataCoordinates::Degree)) {
        return;
    }

    const QString path = event->mimeData()->text();
    addImage(path, lon, lat);
    reloadMap();

    m_imageCache->setCoordinates(path, lon, lat);
    emit imageAssigned(path);

    event->acceptProposedAction();
}

void MapWidget::addImage(const QString &path, double lon, double lat)
{
    m_images[path] = Marble::GeoDataCoordinates(lon, lat, 0.0, Marble::GeoDataCoordinates::Degree);
}

void MapWidget::customPaint(Marble::GeoPainter *painter)
{
    const auto images = m_images.keys();
    for (const auto image : images) {
        painter->drawPixmap(m_images.value(image),
                            QPixmap::fromImage(m_imageCache->thumbnail(image)));
    }

    painter->setPen(m_trackPen);
    for (const auto &lineString : m_tracks) {
        painter->drawPolyline(lineString);
    }
}

void MapWidget::saveSettings()
{
    // Save the floaters visibility

    QHash<QString, bool> visibility;

    const auto floatItemsList = floatItems();
    for (const auto &item : floatItemsList) {
        visibility.insert(item->name(), item->visible());
    }

    m_settings->saveFloatersVisibility(visibility);

    // Save the current center point
    const auto center = focusPoint();
    m_settings->saveMapCenter(Coordinates::Data {
                                  center.longitude(Marble::GeoDataCoordinates::Degree),
                                  center.latitude(Marble::GeoDataCoordinates::Degree) });

    // Save the zoom level
    m_settings->saveZoom(zoom());
}

void MapWidget::restoreSettings()
{
    // Restore the floaters visiblility
    const auto floatersVisiblility = m_settings->floatersVisibility();
    const auto floatItemsList = floatItems();
    for (const auto &item : floatItemsList) {
        const auto name = item->name();
        if (floatersVisiblility.contains(name)) {
            item->setVisible(floatersVisiblility.value(name));
        }
    }

    // Restore map's last center point
    const auto [ lon, lat, isSet ] = m_settings->mapCenter();
    centerOn(lon, lat);

    // Restore the last zoom level
    setZoom(m_settings->zoom());
}

void MapWidget::centerImage(const QString &path)
{
    const auto coordinates = m_imageCache->coordinates(path);
    centerOn(coordinates.lon, coordinates.lat);
}
