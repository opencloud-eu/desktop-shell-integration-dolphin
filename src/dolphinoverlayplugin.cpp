/******************************************************************************
 *   Copyright (C) 2014 by Olivier Goffart <ogoffart@woboq.com                *
 *                                                                            *
 *   This program is free software; you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License as published by     *
 *   the Free Software Foundation; either version 2 of the License, or        *
 *   (at your option) any later version.                                      *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program; if not, write to the                            *
 *   Free Software Foundation, Inc.,                                          *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA               *
 ******************************************************************************/

#include <KOverlayIconPlugin>
#include <KPluginFactory>
#include <QLocalSocket>
#include <KFileItem>
#include <QDir>

#include <algorithm>
#include <iostream>

#include "dolphinpluginhelper.h"

using namespace Qt::StringLiterals;

class OpenCloudDolphinPlugin : public KOverlayIconPlugin
{
    Q_PLUGIN_METADATA(IID "eu.opencloud.ovarlayiconplugin")
    Q_OBJECT

public:
    enum class Icon {
        Cloud,
        DarkGreenCheckMark,
        LightGreenCheckMark,
        Sync,
        Ignore,
        Share,
        Error
    };

    OpenCloudDolphinPlugin() {
        auto helper = OpenCloudDolphinPluginHelper::instance();
        QObject::connect(helper, &OpenCloudDolphinPluginHelper::commandReceived,
                         this, &OpenCloudDolphinPlugin::slotCommandReceived);
    }

    QStringList getOverlays(const QUrl& url) override {
        auto helper = OpenCloudDolphinPluginHelper::instance();

        if (!helper->isConnected()) {
            std::cerr << "helper is not connected!" << std::endl;
            return QStringList();
        }
        if (!url.isLocalFile()) {
            return QStringList();
        }

        const QDir localPath(url.toLocalFile());
        const QString cleanLocalPath{localPath.canonicalPath()};

        // check if the file to get Overlays for is actually part of a synced dir
        const QStringList syncPaths = helper->paths();
        if (std::ranges::find_if(syncPaths, [cleanLocalPath](const QString& syncPath) {
                                 return cleanLocalPath.startsWith(syncPath);
            }) == syncPaths.cend() ) {
            return QStringList();
        };

        if (helper->sendCommand("RETRIEVE_FILE_STATUS:"_ba + cleanLocalPath.toUtf8())) {
            auto stat = helper->statusFromCache(cleanLocalPath.toUtf8());

            if (!stat.isEmpty()) {
                // return from cache for now
                return overlaysForString(stat);
            }
        }
        return QStringList();
    }

private:
    QString overlayIcon(Icon i) const {
        switch(i) {
        case Icon::Cloud:
            return u"OpenCloud_cloud"_s;
        case Icon::DarkGreenCheckMark:
            return u"OpenCloud_ok"_s;
        case Icon::LightGreenCheckMark:
            return u"OpenCloud_lightok"_s;
        case Icon::Sync:
            return u"OpenCloud_sync"_s;
        case Icon::Ignore:
            return u"OpenCloud_warn"_s;
        case Icon::Share:
            return u"OpenCloud_share"_s;
        case Icon::Error:
            return u"OpenCloud_error"_s;
        }
        Q_UNREACHABLE();
    }
    /*
     * A typical status string looks like
     *   "OK+VIRT+AL" -> error free file that is virtual and marked as always local
     *
     * The following icons are needed:
     * - A cloud for virtual files
     * - A dark green checkmark for files that are locally and marked as always locally
     * - A light green checkmark for files that are locally but can be freed
     * - A sync icon: For files with an ongoing sync or new files
     * - A ignore icon: For warnings and excluded files
     * - A error icon: For files in error state
     * - A share icon: For files that additionally show that they're shared.
     *
     * that is maximum compatibility with MS Cloud API as described here:
     * https://support.microsoft.com/en-us/office/what-do-the-onedrive-icons-mean-11143026-8000-44f8-aaa9-67c985aa49b3#id0ebh=windows#ID0EDRBBHBH
     *
     */
    QStringList overlaysForString(const QByteArray &status) {
        QStringList r;
        if (status.startsWith("NOP"_ba))
            return r;

        if (status.startsWith("OK"_ba)) { // File is ok. Check if it is virtual
            if (status.contains("+VIRT"_ba)) { // virtual marker
                // the cloud
                r.append(overlayIcon(Icon::Cloud));
            } else {
                // not virutal
                if (status.contains("+AL"_ba)) { // always-local marker
                    // dark green checkmark - marked as available online
                    r.append(overlayIcon(Icon::DarkGreenCheckMark));
                } else {
                    // light green checkmark
                    r.append(overlayIcon(Icon::LightGreenCheckMark));
                }
            }
        } else if (status.startsWith("SYNC"_ba) || status.startsWith("NEW"_ba)) {
            // status that indicates syncing
            r.append(overlayIcon(Icon::Sync));
        } else if (status.startsWith("IGNORE"_ba) || status.startsWith("WARN"_ba)) {
            r.append(overlayIcon(Icon::Ignore));
        } else if (status.startsWith("ERROR"_ba)) { // HARD ERROR
            r.append(overlayIcon(Icon::Error));
        }

        // Shared flag comes additionally
        if (status.contains("+SWM"_ba)) {
            r.append(overlayIcon(Icon::Share));
        }

        return r;
    }

    void slotCommandReceived(const QByteArray &line) {
        QList<QByteArray> tokens = line.split(':');
        if (tokens.count() != 3)
            return;
        if (tokens[0] != "STATUS"_ba && tokens[0] != "BROADCAST"_ba)
            return;
        if (tokens[2].isEmpty())
            return;

        auto helper = OpenCloudDolphinPluginHelper::instance();
        const QByteArray name = tokens[2];
        const QByteArray status = tokens[1];
        // check if the status was in the cache before, and return if nothing has
        // changed.
        const auto cacheStatus = helper->statusFromCache(name);
        if (cacheStatus == status) {
            return;
        }

        // ...otherwise remember the status in the cache
        helper->putInStatusCache(name, status);
        Q_EMIT overlaysChanged(QUrl::fromLocalFile(QString::fromUtf8(name)), overlaysForString(status));
    }
};

#include "dolphinoverlayplugin.moc"
