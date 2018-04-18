/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qsgcontextplugin_p.h"
#include <QtQuick/private/qsgcontext_p.h>
#include <QtGui/qguiapplication.h>
#include <QtCore/private/qfactoryloader_p.h>
#include <QtCore/qlibraryinfo.h>

QT_BEGIN_NAMESPACE

QSGContextPlugin::QSGContextPlugin(QObject *parent)
    : QObject(parent)
{
}

QSGContextPlugin::~QSGContextPlugin()
{
}

#ifndef QT_NO_LIBRARY
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QSGContextFactoryInterface_iid, QLatin1String("/scenegraph")))
#endif

struct QSGAdaptionPluginData
{
    QSGAdaptionPluginData()
        : tried(false)
        , factory(0)
    {
    }

    ~QSGAdaptionPluginData()
    {
    }

    bool tried;
    QSGContextFactoryInterface *factory;
    QString deviceName;
};

Q_GLOBAL_STATIC(QSGAdaptionPluginData, qsg_adaptation_data)

QSGAdaptionPluginData *contextFactory()
{
    QSGAdaptionPluginData *plugin = qsg_adaptation_data();
    if (!plugin->tried) {

        plugin->tried = true;
        const QStringList args = QGuiApplication::arguments();
        QString device;
        for (int index = 0; index < args.count(); ++index) {
            if (args.at(index).startsWith(QLatin1String("--device="))) {
                device = args.at(index).mid(9);
                break;
            }
        }
        if (device.isEmpty())
            device = QString::fromLocal8Bit(qgetenv("QMLSCENE_DEVICE"));

#ifndef QT_NO_LIBRARY
        if (!device.isEmpty()) {
            const int index = loader()->indexOf(device);
            if (index != -1)
                plugin->factory = qobject_cast<QSGContextFactoryInterface*>(loader()->instance(index));
            plugin->deviceName = device;
#ifndef QT_NO_DEBUG
            if (!plugin->factory) {
                qWarning("Could not create scene graph context for device '%s'"
                         " - check that plugins are installed correctly in %s",
                         qPrintable(device),
                         qPrintable(QLibraryInfo::location(QLibraryInfo::PluginsPath)));
            }
#endif
        }

#endif // QT_NO_LIBRARY
    }
    return plugin;
}



/*!
    \fn QSGContext *QSGContext::createDefaultContext()

    Creates a default scene graph context for the current hardware.
    This may load a device-specific plugin.
*/
QSGContext *QSGContext::createDefaultContext()
{
    QSGAdaptionPluginData *plugin = contextFactory();
    if (plugin->factory)
        return plugin->factory->create(plugin->deviceName);
    return new QSGContext();
}



/*!
    Calls into the scene graph adaptation if available and creates a texture
    factory. The primary purpose of this function is to reimplement hardware
    specific asynchronous texture frameskip-less uploads that can happen on
    the image providers thread.
 */

QQuickTextureFactory *QSGContext::createTextureFactoryFromImage(const QImage &image)
{
    QSGAdaptionPluginData *plugin = contextFactory();
    if (plugin->factory)
        return plugin->factory->createTextureFactoryFromImage(image);
    return 0;
}


/*!
    Calls into the scene graph adaptation if available and creates a hardware
    specific window manager.
 */

QSGRenderLoop *QSGContext::createWindowManager()
{
    QSGAdaptionPluginData *plugin = contextFactory();
    if (plugin->factory)
        return plugin->factory->createWindowManager();
    return 0;
}





QT_END_NAMESPACE