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
#pragma once

#include <QObject>
#include <QBasicTimer>
#include <QLocalSocket>
#include <QRegularExpression>
#include <QPixmap>
#include "openclouddolphinpluginhelper_export.h"

class OPENCLOUDDOLPHINPLUGINHELPER_EXPORT OpenCloudDolphinPluginHelper : public QObject {
    Q_OBJECT
public:
    static OpenCloudDolphinPluginHelper *instance();

    bool isConnected() const;
    void sendCommand(const QByteArray&);
    void sendGetClientIconCommand(int size);

    QVector<QString> paths() const { return _paths; }

    QString contextMenuTitle() const
    {
        return _strings.value("CONTEXT_MENU_TITLE", QStringLiteral("OpenCloud"));
    }
    QString shareActionTitle() const
    {
        return _strings.value("SHARE_MENU_TITLE", QStringLiteral("Share..."));
    }
    QPixmap clientIcon() const
    {
        return _clientIcon;
    }

    QString copyPrivateLinkTitle() const { return _strings["COPY_PRIVATE_LINK_MENU_TITLE"]; }
    QString emailPrivateLinkTitle() const { return _strings["EMAIL_PRIVATE_LINK_MENU_TITLE"]; }

    QByteArray version() { return _version; }

Q_SIGNALS:
    void commandReceived(const QByteArray &cmd);

protected:
    void timerEvent(QTimerEvent*) override;

private:
    OpenCloudDolphinPluginHelper();
    void slotConnected();
    void slotReadyRead();
    void tryConnect();
    QLocalSocket _socket;
    QByteArray _line;
    QVector<QString> _paths;
    QBasicTimer _connectTimer;

    QMap<QByteArray, QString> _strings;
    QByteArray _version;
    QPixmap _clientIcon;
    int _msgId = 1;
};
