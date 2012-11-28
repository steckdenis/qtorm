/*
 * qf.cpp
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

#include "qf.h"
#include "qfield.h"

struct QF::Private
{
    QField f;
};

QF::QF(const QField& f) : d(new Private)
{
    d->f = f;
}

QF::~QF()
{
    delete d;
}

QField QF::field() const
{
    return d->f;
}

QWhere QF::operator==(const QVariant &other) const
{
    return QFIWhere(d->f, other, QWhere::Equal);
}

QWhere QF::operator!=(const QVariant &other) const
{
    return QFIWhere(d->f, other, QWhere::NotEqual);
}

QWhere QF::operator<(const QVariant &other) const
{
    return QFIWhere(d->f, other, QWhere::Less);
}

QWhere QF::operator>(const QVariant &other) const
{
    return QFIWhere(d->f, other, QWhere::Greater);
}

QWhere QF::operator<=(const QVariant &other) const
{
    return QFIWhere(d->f, other, QWhere::LessEqual);
}

QWhere QF::operator>=(const QVariant &other) const
{
    return QFIWhere(d->f, other, QWhere::GreaterEqual);
}

QWhere QF::operator==(const QField &other) const
{
    return QFFWhere(d->f, other, QWhere::Equal);
}

QWhere QF::operator!=(const QField &other) const
{
    return QFFWhere(d->f, other, QWhere::NotEqual);
}

QWhere QF::operator<(const QField &other) const
{
    return QFFWhere(d->f, other, QWhere::Less);
}

QWhere QF::operator>(const QField &other) const
{
    return QFFWhere(d->f, other, QWhere::Greater);
}

QWhere QF::operator<=(const QField &other) const
{
    return QFFWhere(d->f, other, QWhere::LessEqual);
}

QWhere QF::operator>=(const QField &other) const
{
    return QFFWhere(d->f, other, QWhere::GreaterEqual);
}

QWhere QF::operator!() const
{
    return QFWhere(d->f, QWhere::Null);
}

QAssign QF::operator+(const QAssign &other)
{
    return QOpAssign(QAssign(*this), other, QAssign::Add);
}

QAssign QF::operator-(const QAssign &other)
{
    return QOpAssign(QAssign(*this), other, QAssign::Sub);
}

QAssign QF::operator*(const QAssign &other)
{
    return QOpAssign(QAssign(*this), other, QAssign::Mul);
}

QAssign QF::operator/(const QAssign &other)
{
    return QOpAssign(QAssign(*this), other, QAssign::Div);
}

QWhere QF::in(const QVariantList& other) const
{
    return QFInWhere(d->f, other);
}

QWhere QF::like(const QString& pattern) const
{
    return QFLikeWhere(d->f, pattern);
}

QWhere QF::divisibleBy(int divisor, int offset) const
{
    return QFDivWhere(d->f, divisor, offset);
}

QWhere QF::flagSet(int flag) const
{
    return QFFlagSetWhere(d->f, flag);
}

QWhere QF::isNull() const
{
    return QFNullWhere(d->f);
}
