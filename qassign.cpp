/*
 * qassign.cpp
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

#include "qassign.h"
#include "qfield.h"
#include "qf.h"

#include <QtDebug>
#include <QSqlDriver>

class QAssignPrivate
{
    public:
        QAssignPrivate();
        virtual ~QAssignPrivate();

        QString fieldName(const QField &field, QSqlDriver *driver) const;

        void ref();
        bool deref();

        virtual QString sql(QSqlDriver *driver) const = 0;
        virtual void bindValues(QVariantList &values) const = 0;

    private:
        unsigned int _refcount;
};

class QFAssignPrivate : public QAssignPrivate
{
    public:
        QFAssignPrivate(const QField &f);
        ~QFAssignPrivate();

        QString sql(QSqlDriver *driver) const;
        void bindValues(QVariantList &values) const;

    private:
        QField _f;
};

class QIAssignPrivate : public QAssignPrivate
{
    public:
        QIAssignPrivate(const QVariant &value);
        ~QIAssignPrivate();

        QString sql(QSqlDriver *driver) const;
        void bindValues(QVariantList &values) const;

    private:
        QVariant _value;
};

class QOpAssignPrivate : public QAssignPrivate
{
    public:
        QOpAssignPrivate(const QAssign &left, const QAssign &right, QAssign::Operation op);
        ~QOpAssignPrivate();

        QString sql(QSqlDriver *driver) const;
        void bindValues(QVariantList &values) const;

    private:
        QAssign _left;
        QAssign _right;
        QAssign::Operation _op;
};

QAssignPrivate::QAssignPrivate() : _refcount(1)
{
}

QAssignPrivate::~QAssignPrivate()
{
}

QString QAssignPrivate::fieldName(const QField& field, QSqlDriver *driver) const
{
    return driver->escapeIdentifier(field.fieldName(), QSqlDriver::FieldName);
}

void QAssignPrivate::ref()
{
    _refcount++;
}

bool QAssignPrivate::deref()
{
    _refcount--;

    return (_refcount != 0);
}

QAssign::QAssign() : d(NULL)
{
}

QAssign::QAssign(const QAssign& other) : d(other.d)
{
    if (d)
        d->ref();
}

QAssign::QAssign(QAssignPrivate* d) : d(d)
{
}

QAssign::QAssign(const QVariant& value) : d(new QIAssignPrivate(value))
{
}

QAssign::QAssign(const QF& f) : d(new QFAssignPrivate(f.field()))
{
}

QAssign &QAssign::operator=(const QAssign &other)
{
    if (d && d->deref())
        delete d;

    d = other.d;

    if (d)
        d->ref();

    return *this;
}

QAssign::~QAssign()
{
    if (d && !d->deref())
        delete d;
}

bool QAssign::isValid() const
{
    return (d != NULL);
}

QString QAssign::operationStr(QAssign::Operation op)
{
    switch (op)
    {
        case Add:
            return "+";
        case Sub:
            return "-";
        case Mul:
            return "*";
        case Div:
            return "/";
    }

    return QString();
}

void QAssign::bindValues(QVariantList& values) const
{
    d->bindValues(values);
}

QString QAssign::sql(QSqlDriver *driver) const
{
    return d->sql(driver);
}

QAssign QAssign::operator+(const QAssign& other)
{
    return QOpAssign(*this, other, Add);
}

QAssign QAssign::operator-(const QAssign& other)
{
    return QOpAssign(*this, other, Sub);
}

QAssign QAssign::operator*(const QAssign& other)
{
    return QOpAssign(*this, other, Mul);
}

QAssign QAssign::operator/(const QAssign& other)
{
    return QOpAssign(*this, other, Div);
}

/*
 * QFAssignPrivate
 */
QFAssignPrivate::QFAssignPrivate(const QField& f) : QAssignPrivate(), _f(f)
{
}

QFAssignPrivate::~QFAssignPrivate()
{
}

void QFAssignPrivate::bindValues(QVariantList& values) const
{
    (void) values;
    return;
}

QString QFAssignPrivate::sql(QSqlDriver *driver) const
{
    return fieldName(_f, driver);
}

/*
 * QIAssignPrivate
 */
QIAssignPrivate::QIAssignPrivate(const QVariant& value) : QAssignPrivate(), _value(value)
{
}

QIAssignPrivate::~QIAssignPrivate()
{
}

void QIAssignPrivate::bindValues(QVariantList& values) const
{
    values.append(_value);
}

QString QIAssignPrivate::sql(QSqlDriver *driver) const
{
    (void) driver;

    return QString("?");
}

/*
 * QOpAssign
 */
QOpAssignPrivate::QOpAssignPrivate(const QAssign &left, const QAssign &right, QAssign::Operation op)
: QAssignPrivate(), _left(left), _right(right), _op(op)
{
}

QOpAssignPrivate::~QOpAssignPrivate()
{
}

QString QOpAssignPrivate::sql(QSqlDriver *driver) const
{
    return QString("(%0) %1 (%2)")
        .arg(_left.sql(driver))
        .arg(QAssign::operationStr(_op))
        .arg(_right.sql(driver));
}

void QOpAssignPrivate::bindValues(QVariantList &values) const
{
    _left.bindValues(values);
    _right.bindValues(values);
}

QOpAssign::QOpAssign(const QAssign& left, const QAssign& right, QAssign::Operation op)
: QAssign(new QOpAssignPrivate(left, right, op))
{
}

QOpAssign::~QOpAssign()
{
}