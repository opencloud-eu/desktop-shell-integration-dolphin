/******************************************************************************
 *   Copyright (C) 2024 by Klaas Freitag <kfreitag@owncloud.com>              *
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

#include <KAbstractFileItemActionPlugin>
#include <KCoreDirLister>
#include <KFileItemListProperties>

class QAction;
class QWidget;

class OpenCloudDolphinPluginAction : public KAbstractFileItemActionPlugin
{
    Q_OBJECT
public:
    explicit OpenCloudDolphinPluginAction(QObject *parent, const QVariantList &args);

    QList<QAction *> actions(const KFileItemListProperties &fileItemInfos, QWidget *parentWidget) override;

private:
    QList<QAction *> legacyActions(const KFileItemListProperties &fileItemInfos, QWidget *parentWidget);
};
