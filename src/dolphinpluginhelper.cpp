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

#include "dolphinpluginhelper.h"

#include <iostream>


#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QStandardPaths>
#include <QTimerEvent>
#include <QtNetwork/QLocalSocket>
#include <QTimer>

Q_LOGGING_CATEGORY(lcPluginHelper, "opencloud.dolphin", QtInfoMsg)

using namespace Qt::StringLiterals;

namespace {
const int CacheMaxEntries{1000};
}

OpenCloudDolphinPluginHelper* OpenCloudDolphinPluginHelper::instance()
{
    static OpenCloudDolphinPluginHelper self;
    return &self;
}

OpenCloudDolphinPluginHelper::OpenCloudDolphinPluginHelper()
{
    QObject::connect(&_socket, &QLocalSocket::connected, this, &OpenCloudDolphinPluginHelper::slotConnected);
    connect(&_socket, &QLocalSocket::readyRead, this, &OpenCloudDolphinPluginHelper::slotReadyRead);

    _connectTimer = new QTimer(this);

    connect(_connectTimer, &QTimer::timeout, [this]() {
        tryConnect();

        // clear the cache, just to refresh it
        m_status.clear();
    });
    _connectTimer->start(std::chrono::seconds(45));

    tryConnect();
}

bool OpenCloudDolphinPluginHelper::isConnected() const
{
    return _socket.state() == QLocalSocket::ConnectedState;
}

bool OpenCloudDolphinPluginHelper::sendCommand(const QByteArray& data)
{
    if (isConnected()) {
        _socket.write(data);
        _socket.flush();
        return true;
    }
    return false;
}

void OpenCloudDolphinPluginHelper::sendGetClientIconCommand(int size)
{
    const QByteArray cmd{"V2/GET_CLIENT_ICON:"};
    const QByteArray newLine{"\n"};
    const QJsonObject args { { QStringLiteral("size"), size } };
    const QJsonObject obj { { QStringLiteral("id"), QString::number(_msgId++) }, { QStringLiteral("arguments"), args } };
    const auto json = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    sendCommand(cmd + json + newLine);
}

void OpenCloudDolphinPluginHelper::slotConnected()
{
    sendCommand("VERSION:\n");
    sendCommand("GET_STRINGS:\n");
}

void OpenCloudDolphinPluginHelper::tryConnect()
{
    if (_socket.state() != QLocalSocket::UnconnectedState) {
        return;
    }

    QString socketPath = QStandardPaths::locate(QStandardPaths::RuntimeLocation,
                                                u"OpenCloud"_s,
                                                QStandardPaths::LocateDirectory);
    if(socketPath.isEmpty()) {
        std::cerr << "Socket path is not avialable";
        return;
    }

    socketPath.append(u"/socket"_s);
    if (! QFile::exists(socketPath)) {
        std::cerr << "Socket " << socketPath.toStdString() << " is not available ";
        return;
    }
    _socket.connectToServer(socketPath);
}

void OpenCloudDolphinPluginHelper::slotReadyRead()
{
    while (_socket.bytesAvailable()) {
        _line += _socket.readLine();
        if (!_line.endsWith("\n")) {
            continue;
        }
        QByteArray line;
        qSwap(line, _line);
        line.chop(1);
        if (line.isEmpty())
            continue;
        const int firstColon = line.indexOf(':');
        if (firstColon == -1) {
            continue;
        }
        // get the command (at begin of line, before first ':')
        const QByteArray command = line.left(firstColon);
        // rest of line contains the information
        const QByteArray info = line.mid(firstColon + 1);

        if (command == "REGISTER_PATH"_ba) {
            const QString file = QString::fromUtf8(info);
            _paths.append(file);
            continue;
        } else if (command == "STRING"_ba) {
            auto args = QString::fromUtf8(info).split(QLatin1Char(':'));
            if (args.size() >= 2) {
                _strings[args[0].toUtf8()] = args.mid(1).join(QLatin1Char(':'));
            }
            continue;
        } else if (command == "VERSION"_ba) {
            auto args = info.split(':');
            if (args.size() >= 2) {
                auto version = args.value(1);
                _version = version;
            }
            if (!_version.startsWith("1.")) {
                // Incompatible version, disconnect forever
                _connectTimer->stop();
                _socket.disconnectFromServer();
                return;
            }
        } else if (command == "V2/GET_CLIENT_ICON_RESULT"_ba) {
            QJsonParseError error;
            auto json = QJsonDocument::fromJson(info, &error).object();
            if (error.error != QJsonParseError::NoError) {
                qCWarning(lcPluginHelper) << "Error while parsing result: " << error.error;
                continue;
            }

            auto jsonArgs = json.value(QStringLiteral("arguments")).toObject();
            if (jsonArgs.isEmpty()) {
                auto jsonErr = json.value(QStringLiteral("error")).toObject();
                qCWarning(lcPluginHelper) << "Error getting client icon: " << jsonErr;
                continue;
            }

            const QByteArray pngBase64 = jsonArgs.value(QStringLiteral("png")).toString().toUtf8();
            QByteArray png = QByteArray::fromBase64(pngBase64);

            QPixmap pixmap;
            bool isLoaded = pixmap.loadFromData(png, "PNG");
            if (isLoaded) {
                _clientIcon = pixmap;
            }
        }

        Q_EMIT commandReceived(line);
    }
}

void OpenCloudDolphinPluginHelper::putInStatusCache(const QByteArray& file, const QByteArray& status)
{
    // Do not let the cache grow infinite...
    if (m_status.count() < CacheMaxEntries) {
        m_status.insertOrAssign(file, status);
    }
}
