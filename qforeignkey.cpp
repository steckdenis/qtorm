/*
 * qforeignkey.cpp
 * This file is part of QtORM
 *
 * Copyright (C) 2012 - Denis Steckelmacher <steckdenis@yahoo.fr>
 *
 * QtORM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * QtORM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Logram; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "qfield.h"
#include "qforeignkey_p.h"
#include "qmodel.h"
#include "qqueryset.h"

#include <QtDebug>

QForeignKeyPrivate::QForeignKeyPrivate(QModel *model, const QString &name)
 : QFieldPrivate(model, name),
   _value(NULL),
   _id(QVariant(QVariant::Int)),
   _delete_value(true)
{
}

QForeignKeyPrivate::~QForeignKeyPrivate()
{
    deleteValue();
}

void QForeignKeyPrivate::deleteValue()
{
    if (_delete_value && _value)
        delete _value;
}

bool QForeignKeyPrivate::isForeignKey() const
{
    return true;
}

QModel *QForeignKeyPrivate::value()
{
    return _value;
}

void QForeignKeyPrivate::setDeleteValue(bool enable)
{
    _delete_value = enable;
}

void QForeignKeyPrivate::fillCache() const
{
    if (_id.isNull())
        return;

    // Fill the value model with data from the database
    QQuerySet query(_value);

    query.addFilter(QF(_value->pk()) == _id);
    query.next();

    _value->resetModified();
}

void QForeignKeyPrivate::setValue(QModel *value)
{
    setNull(false); // The field is not null anymore
    setModified(true);

    deleteValue();

    // Update our current child model ID
    _value = value;
    if (!value->pk().data().isNull())
        _id = value->pk().data();
}

void QForeignKeyPrivate::setValue(const QVariant &data)
{
    _id = data;
    setNull(data.isNull());
    setModified(true);

    deleteValue();

    _value = NULL;
}

void QForeignKeyPrivate::fromData(const QVariant &data)
{
    _id = data;
    setNull(data.isNull());
    setModified(false);
}

QVariant QForeignKeyPrivate::data() const
{
    return _id;
}

QString QForeignKeyPrivate::sqlDescription() const
{
    QString rs = QLatin1String("INTEGER");

    rs += commonSqlDescription();

    return rs;
}
