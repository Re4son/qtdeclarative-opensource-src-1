/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
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

#include "qqmlaccessors_p.h"

#include "qqmldata_p.h"
#include "qqmlnotifier_p.h"

QT_BEGIN_NAMESPACE

struct AccessorProperties {
    AccessorProperties();

    QReadWriteLock lock;
    QHash<const QMetaObject *, QQmlAccessorProperties::Properties> properties;
};

Q_GLOBAL_STATIC(AccessorProperties, accessorProperties)

static void buildNameMask(QQmlAccessorProperties::Properties &properties)
{
    quint32 mask = 0;

    for (int ii = 0; ii < properties.count; ++ii) {
        Q_ASSERT(strlen(properties.properties[ii].name) == properties.properties[ii].nameLength);
        Q_ASSERT(properties.properties[ii].nameLength > 0);

        mask |= (1 << qMin(31U, properties.properties[ii].nameLength - 1));
    }

    properties.nameMask = mask;
}

AccessorProperties::AccessorProperties()
{
}

QQmlAccessorProperties::Properties::Properties(Property *properties, int count)
: count(count), properties(properties)
{
    buildNameMask(*this);
}

QQmlAccessorProperties::Properties
QQmlAccessorProperties::properties(const QMetaObject *mo)
{
    AccessorProperties *This = accessorProperties();

    QReadLocker lock(&This->lock);
    return This->properties.value(mo);
}

void QQmlAccessorProperties::registerProperties(const QMetaObject *mo, int count,
                                                        Property *props)
{
    Q_ASSERT(count > 0);

    Properties properties(props, count);

    AccessorProperties *This = accessorProperties();

    QWriteLocker lock(&This->lock);

    Q_ASSERT(!This->properties.contains(mo) || This->properties.value(mo) == properties);

    This->properties.insert(mo, properties);
}

QT_END_NAMESPACE
