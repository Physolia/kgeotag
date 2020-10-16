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
#include "Settings.h"

// Qt includes
#include <QDebug>
#include <QSize>

namespace
{

// Main

const QLatin1String c_main("main/");
const QString c_main_windowGeometry = c_main + QLatin1String("window_geometry");
const QString c_main_windowState   = c_main + QLatin1String("window_state");
const QString c_main_lastOpenPath = c_main + QLatin1String("last_open_path");

// Map

const QLatin1String c_map("map/");
const QString c_map_centerLon = c_map + QLatin1String("center_lon");
const QString c_map_centerLat = c_map + QLatin1String("center_lat");
const QString c_map_zoom = c_map + QLatin1String("zoom");

const QLatin1String c_floatersVisibility("floaters_visibility/");

// Tracks

const QLatin1String c_track("track/");
const QString c_track_color = c_track + QLatin1String("color");
const QString c_track_width = c_track + QLatin1String("width");
const QString c_track_style = c_track + QLatin1String("style");
const QVector<Qt::PenStyle> c_track_styleEnum {
    Qt::SolidLine,
    Qt::DashLine,
    Qt::DotLine,
    Qt::DashDotLine,
    Qt::DashDotDotLine
};
const QVector<QString> c_track_styleString {
    QStringLiteral("solid"),
    QStringLiteral("dashes"),
    QStringLiteral("dots"),
    QStringLiteral("dash_dot"),
    QStringLiteral("dash_dot_dot")
};

// Images

const QLatin1String c_images("images/");
const QString c_images_thumnailSize = c_images + QLatin1String("thumbnail_size");
const QString c_images_previewSize = c_images + QLatin1String("preview_size");

// Assignment

const QLatin1String c_assignment("assignment/");
const QString c_assignment_exactMatchTolerance
    = c_assignment + QLatin1String("exact_match_tolerance");
const QString c_assignment_maximumInterpolationInterval
    = c_assignment + QLatin1String("maximum_interpolation_interval");
const QString c_assignment_maximumInterpolationDistance
    = c_assignment + QLatin1String("maximum_interpolation_distance");

// Saving

const QLatin1String c_save("save/");
const QString c_save_createBackups = c_save + QLatin1String("create_backups");

}

Settings::Settings(QObject *parent)
    : QSettings(QStringLiteral("kgeotag"), QStringLiteral("kgeotag"), parent)
{
}

void Settings::saveMainWindowGeometry(const QByteArray &data)
{
    setValue(c_main_windowGeometry, data);
}

QByteArray Settings::mainWindowGeometry() const
{
    return value(c_main_windowGeometry, QByteArray()).toByteArray();
}

void Settings::saveMainWindowState(const QByteArray &data)
{
    setValue(c_main_windowState, data);
}

QByteArray Settings::mainWindowState() const
{
    return value(c_main_windowState, QByteArray()).toByteArray();
}

void Settings::saveFloatersVisibility(const QHash<QString, bool> &data)
{
    const auto keys = data.keys();
    for (const auto &key : keys) {
        setValue(c_floatersVisibility + key, data.value(key));
    }
}

QHash<QString, bool> Settings::floatersVisibility()
{
    QHash<QString, bool> data;

    beginGroup(c_floatersVisibility);
    const auto keys = allKeys();
    endGroup();

    for (const auto &key : keys) {
        data.insert(key, value(c_floatersVisibility + key).toBool());
    }

    return data;
}

void Settings::saveMapCenter(const KGeoTag::Coordinates &coordinates)
{
    setValue(c_map_centerLon, coordinates.lon);
    setValue(c_map_centerLat, coordinates.lat);
}

KGeoTag::Coordinates Settings::mapCenter() const
{
    return KGeoTag::Coordinates { value(c_map_centerLon, 0).toDouble(),
                                  value(c_map_centerLat, 0).toDouble() };
}

void Settings::saveZoom(int zoom)
{
    setValue(c_map_zoom, zoom);
}

int Settings::zoom() const
{
    return value(c_map_zoom, 1520).toInt();
}

void Settings::saveLastOpenPath(const QString &path)
{
    setValue(c_main_lastOpenPath, path);
}

QString Settings::lastOpenPath() const
{
    return value(c_main_lastOpenPath, QString()).toString();
}

void Settings::saveThumbnailSize(int size)
{
    setValue(c_images_thumnailSize, size);
}

QSize Settings::thumbnailSize() const
{
    const int size = value(c_images_thumnailSize, 32).toInt();
    return QSize(size, size);
}

void Settings::savePreviewSize(int size)
{
    setValue(c_images_previewSize, size);
}

QSize Settings::previewSize() const
{
    const int size = value(c_images_previewSize, 400).toInt();
    return QSize(size, size);
}

void Settings::saveExactMatchTolerance(int seconds)
{
    setValue(c_assignment_exactMatchTolerance, seconds);
}

int Settings::exactMatchTolerance() const
{
    return value(c_assignment_exactMatchTolerance, 10).toInt();
}

void Settings::saveMaximumInterpolationInterval(int seconds)
{
    setValue(c_assignment_maximumInterpolationInterval, seconds);
}

int Settings::maximumInterpolationInterval() const
{
    return value(c_assignment_maximumInterpolationInterval, -1).toInt();
}

void Settings::saveMaximumInterpolationDistance(int meters)
{
    setValue(c_assignment_maximumInterpolationDistance, meters);
}

int Settings::maximumInterpolationDistance() const
{
    return value(c_assignment_maximumInterpolationDistance, -1).toInt();
}

void Settings::saveTrackColor(const QColor &color)
{
    setValue(c_track_color, color);
}

QColor Settings::trackColor() const
{
    QColor color = value(c_track_color).value<QColor>();
    if (! color.isValid()) {
        color = QColor(255, 0, 255, 150);
    }

    return color;
}

void Settings::saveTrackStyle(Qt::PenStyle style)
{
    setValue(c_track_style, c_track_styleString.at(c_track_styleEnum.indexOf(style)));
}

Qt::PenStyle Settings::trackStyle() const
{
    QString styleString = value(c_track_style).toString();
    if (c_track_styleString.contains(styleString)) {
        return c_track_styleEnum.at(c_track_styleString.indexOf(styleString));
    } else {
        return Qt::DotLine;
    }
}

void Settings::saveTrackWidth(int width)
{
    setValue(c_track_width, width);
}

int Settings::trackWidth() const
{
    return value(c_track_width, 3).toInt();
}

void Settings::saveCreateBackups(bool state)
{
    setValue(c_save_createBackups, state);
}

bool Settings::createBackups() const
{
    return value(c_save_createBackups, true).toBool();
}
