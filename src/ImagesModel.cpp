/*
    SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>

    SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

// Local includes
#include "ImagesModel.h"
#include "KGeoTag.h"
#include "Coordinates.h"

// KDE includes
#include <KLocalizedString>
#include <KExiv2/KExiv2>

// Qt includes
#include <QFileInfo>
#include <QFont>

// C++ includes
#include <utility>

ImagesModel::ImagesModel(QObject *parent, bool splitImagesList, int thumbnailSize, int previewSize)
    : QAbstractListModel(parent),
      m_splitImagesList(splitImagesList),
      m_thumbnailSize(QSize(thumbnailSize, thumbnailSize)),
      m_previewSize(QSize(previewSize, previewSize))
{
}

int ImagesModel::rowCount(const QModelIndex &) const
{
    return m_paths.count();
}

QVariant ImagesModel::data(const QModelIndex &index, int role) const
{
    if (! index.isValid() || index.row() > m_paths.count()) {
        return QVariant();
    }

    const auto &path = m_paths.at(index.row());
    const auto &data = m_imageData[path];

    if (role == Qt::DisplayRole) {
        const QString associatedMarker = (! m_splitImagesList && data.coordinates.isSet())
            ? i18nc("Marker for an associated file", "\u2713\u2009")
            : QString();
        const QString changedmarker = data.coordinates != data.originalCoordinates
            ? i18nc("Marker for a changed file", "\u2009*")
            : QString();
        return i18nc("Pattern for a display filename with a \"changed\" and a \"associated\" "
                     "marker. The first option is the \"associated\" marker, the second one the "
                     "filename and the third one the \"changed\" marker.",
                     "%1%2%3",
                     associatedMarker, data.fileName, changedmarker);

    } else if (role == Qt::DecorationRole) {
        return data.thumbnail;

    } else if (role == Qt::ForegroundRole) {
        switch (data.matchType) {
        case KGeoTag::NotMatched:
            return m_colorScheme.foreground();
        case KGeoTag::ExactMatch:
            return m_colorScheme.foreground(KColorScheme::PositiveText);
        case KGeoTag::InterpolatedMatch:
            return m_colorScheme.foreground(KColorScheme::NeutralText);
        case KGeoTag::ManuallySet:
            return m_colorScheme.foreground(KColorScheme::LinkText);
        }

    } else if (! m_splitImagesList && role == Qt::FontRole) {
        QFont font;
        if (data.coordinates.isSet()) {
            font.setBold(true);
        }
        return font;

    } else if (role == KGeoTag::PathRole) {
        return path;

    } else if (role == KGeoTag::DateRole) {
        return data.date;

    } else if (role == KGeoTag::CoordinatesRole) {
        QVariant coordinates;
        coordinates.setValue(data.coordinates);
        return coordinates;

    } else if (role == KGeoTag::ThumbnailRole) {
        return data.thumbnail;

    } else if (role == KGeoTag::PreviewRole) {
        return data.preview;

    } else if (role == KGeoTag::MatchTypeRole) {
        QVariant matchType;
        matchType.setValue(data.matchType);
        return matchType;

    } else if (role == KGeoTag::ChangedRole) {
        return data.originalCoordinates != data.coordinates;

    }

    return QVariant();
}

ImagesModel::LoadResult ImagesModel::addImage(const QString &path)
{
    // Check if we already have the image
    if (m_paths.contains(path)) {
        return LoadResult::AlreadyLoaded;
    }

    // Read the image
    QImage image = QImage(path);
    if (image.isNull()) {
        return LoadResult::LoadingImageFailed;
    }

    // Read the exif data
    auto exif = KExiv2Iface::KExiv2();
    exif.setUseXMPSidecar4Reading(true);
    if (! exif.load(path)) {
        return LoadResult::LoadingMetadataFailed;
    }

    // Prepare the images's data struct
    ImageData data;

    // Add the filename
    const QFileInfo info(path);
    data.fileName = info.fileName();

    // Read the date
    data.date = exif.getImageDateTime();

    // If no date could be read from the metadata, fall back to file properties
    if (! data.date.isValid()) {
        // First try to get the file's initial creation date
        data.date = info.birthTime();

        // If that fails, fall back to the file's mtime
        if (! data.date.isValid()) {
            data.date = info.lastModified();
        }
    }

    // Try to read gps information
    double altitude;
    double latitude;
    double longitude;
    if (exif.getGPSInfo(altitude, latitude, longitude)) {
        data.originalCoordinates = Coordinates(longitude, latitude, altitude, true);
        data.coordinates = Coordinates(longitude, latitude, altitude, true);
    }

    // Fix the image's orientation
    exif.rotateExifQImage(image, exif.getImageOrientation());

    // Create a smaller thumbnail
    data.thumbnail = QPixmap::fromImage(image.scaled(m_thumbnailSize, Qt::KeepAspectRatio,
                                                     Qt::SmoothTransformation));

    // Create a bigger preview (to be scaled according to the view size)
    data.preview = image.scaled(m_previewSize, Qt::KeepAspectRatio);

    // Find the correct row for the new image (sorted by date)
    int row = 0;
    for (const QString &path : m_paths) {
        if (m_imageData.value(path).date > data.date) {
            break;
        }
        row++;
    }

    // Add the image

    beginInsertRows(QModelIndex(), row, row);
    m_paths.insert(row, path);
    m_imageData.insert(path, data);
    endInsertRows();

    const auto modelIndex = index(row, 0);
    emit dataChanged(modelIndex, modelIndex, { Qt::DisplayRole });

    return LoadResult::LoadingSucceeded;
}

void ImagesModel::emitDataChanged(const QString &path)
{
    const auto modelIndex = indexFor(path);
    emit dataChanged(modelIndex, modelIndex, { Qt::DisplayRole });
}

const QVector<QString> &ImagesModel::allImages() const
{
    return m_paths;
}

QVector<QString> ImagesModel::changedImages() const
{
    QVector<QString> changed;
    for (const auto &path : std::as_const(m_paths)) {
        const auto &data = m_imageData[path];
        if (data.coordinates != data.originalCoordinates) {
            changed.append(path);
        }
    }
    return changed;
}

QDateTime ImagesModel::date(const QString &path) const
{
    return m_imageData.value(path).date;
}

bool ImagesModel::contains(const QString &path) const
{
    return m_paths.contains(path);
}

Coordinates ImagesModel::coordinates(const QString &path) const
{
    return m_imageData.value(path).coordinates;
}

void ImagesModel::setCoordinates(const QString &path, const Coordinates &coordinates,
                                 KGeoTag::MatchType matchType)
{
    m_imageData[path].matchType = matchType;
    m_imageData[path].coordinates = coordinates;
    emitDataChanged(path);
}

void ImagesModel::setElevation(const QString &path, double elevation)
{
    m_imageData[path].coordinates.setAlt(elevation);
}

void ImagesModel::resetChanges(const QString &path)
{
    auto &data = m_imageData[path];
    data.coordinates = data.originalCoordinates;
    data.matchType = KGeoTag::NotMatched;
    emitDataChanged(path);
}

QModelIndex ImagesModel::indexFor(const QString &path) const
{
    return index(m_paths.indexOf(path), 0);
}

void ImagesModel::setSaved(const QString &path)
{
    auto &data = m_imageData[path];
    data.originalCoordinates = data.coordinates;
}

KGeoTag::MatchType ImagesModel::matchType(const QString &path) const
{
    return m_imageData.value(path).matchType;
}
