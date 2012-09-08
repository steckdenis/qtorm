/*
 * qf.h
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

#ifndef __QF_H__
#define __QF_H__

#include "qwhere.h"
#include "qassign.h"

class QF
{
    public:
        QF(const QField &f);
        ~QF();

        QField field() const;

        // QWhere integration
        QWhere operator==(const QVariant &other) const;
        QWhere operator!=(const QVariant &other) const;
        QWhere operator<(const QVariant &other) const;
        QWhere operator>(const QVariant &other) const;
        QWhere operator<=(const QVariant &other) const;
        QWhere operator>=(const QVariant &other) const;

        QWhere operator==(const QField &other) const;
        QWhere operator!=(const QField &other) const;
        QWhere operator<(const QField &other) const;
        QWhere operator>(const QField &other) const;
        QWhere operator<=(const QField &other) const;
        QWhere operator>=(const QField &other) const;

        QWhere operator!() const;
        QWhere operator>>(const QVariantList &other) const;

        QWhere in(const QVariantList &other) const;
        QWhere like(const QString &pattern) const;
        QWhere divisibleBy(int divisor, int offset) const;

        // QAssign integration
        QAssign operator+(const QAssign &other);
        QAssign operator-(const QAssign &other);
        QAssign operator*(const QAssign &other);
        QAssign operator/(const QAssign &other);

    private:
        struct Private;
        Private *d;
};

#endif