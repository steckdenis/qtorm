/*
 * qfield.cpp
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
#include "qfield_p.h"
#include "qmodel.h"

#include <assert.h>

#include <QtSql>

/*
 * QFieldPrivate
 */

QFieldPrivate::QFieldPrivate(QModel *model, const QString &name)
 : _model(model),
   _name(name),
   _refcount(1),
   _isnull(true),
   _accepts_null(false),
   _auto_increment(false),
   _primary_key(false),
   _modified(false)
{
}

QFieldPrivate::~QFieldPrivate()
{
}

QString QFieldPrivate::name() const
{
    return _name;
}

QModel *QFieldPrivate::model() const
{
    return _model;
}

bool QFieldPrivate::isForeignKey() const
{
    return false;
}

bool QFieldPrivate::isNull() const
{
    return _isnull;
}

void QFieldPrivate::setNull(bool isnull)
{
    _isnull = isnull;
}

void QFieldPrivate::setModified(bool modified)
{
    _modified = modified;
}

bool QFieldPrivate::isModified() const
{
    return _modified;
}

void QFieldPrivate::setAcceptsNull(bool null)
{
    _accepts_null = null;
}

bool QFieldPrivate::acceptsNull() const
{
    return _accepts_null;
}

void QFieldPrivate::setAutoIncrement(bool autoincrement)
{
    _auto_increment = autoincrement;
}

bool QFieldPrivate::autoIncrement() const
{
    return _auto_increment;
}

void QFieldPrivate::setPrimaryKey(bool primarykey)
{
    _primary_key = primarykey;
}

bool QFieldPrivate::primaryKey() const
{
    return _primary_key;
}

void QFieldPrivate::setAssignation(const QAssign &assignation)
{
    _assignation = assignation;
}

QAssign QFieldPrivate::assignation() const
{
    return _assignation;
}

QString QFieldPrivate::commonSqlDescription() const
{
    QString rs;

    if (autoIncrement())
        rs += QLatin1String(" AUTO_INCREMENT");

    if (acceptsNull())
        rs += QLatin1String(" NULL");
    else
        rs += QLatin1String(" NOT NULL");

    if (primaryKey())
        rs += QLatin1String(" PRIMARY KEY");

    return rs;
}

void QFieldPrivate::ref()
{
    _refcount++;
}

bool QFieldPrivate::deref()
{
    _refcount--;

    return (_refcount != 0);
}

/*
 * QField
 */

QField::QField() : d(NULL)
{
}

QField::QField(QFieldPrivate *dptr) : d(dptr)
{
}

QField::QField(const QField &other) : d(other.d)
{
    if (d)
        d->ref();
}

QField::~QField()
{
    if (d && !d->deref())
        delete d;
}

QField &QField::operator=(const QField &other)
{
    if (d && !d->deref())
        delete d;

    d = other.d;

    if (d)
        d->ref();

    return *this;
}

bool QField::isValid() const
{
    return (d != NULL);
}

QString QField::name() const
{
    return d->name();
}

QModel *QField::model() const
{
    return d->model();
}

void QField::setNull(bool null)
{
    d->setNull(null);
}

bool QField::isNull() const
{
    return d->isNull();
}

void QField::setModified(bool modified)
{
    d->setModified(modified);
}

bool QField::isModified() const
{
    return d->isModified();
}

void QField::setAcceptsNull(bool null)
{
    d->setAcceptsNull(null);
}

bool QField::acceptsNull() const
{
    return d->acceptsNull();
}

void QField::setAutoIncrement(bool autoincrement)
{
    d->setAutoIncrement(autoincrement);
}

bool QField::autoIncrement() const
{
    return d->autoIncrement();
}

void QField::setPrimaryKey(bool primarykey)
{
    d->setPrimaryKey(primarykey);
}

bool QField::primaryKey() const
{
    return d->primaryKey();
}

void QField::fromData(const QVariant &data)
{
    d->fromData(data);
}

QVariant QField::data() const
{
    return d->data();
}

QString QField::sqlDescription() const
{
    return d->sqlDescription();
}

QString QField::fieldName() const
{
    QSqlDriver *driver = QSqlDatabase::database().driver();
    int tableNumber = d->model()->tableNumber();
    QString fName = QString("T%0.%1").arg(tableNumber).arg(d->name());

    return driver->escapeIdentifier(fName, QSqlDriver::FieldName);
}

void QField::setAssignation(const QAssign &assignation)
{
    setModified(true);
    d->setAssignation(assignation);
}

QAssign QField::assignation() const
{
    return d->assignation();
}