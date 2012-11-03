/*
 * qassign.h
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

#ifndef __QASSIGN_H__
#define __QASSIGN_H__

#include <QVariant>

class QF;
class QAssignPrivate;
class QSqlDriver;

class QAssign
{
    public:
        enum Operation
        {
            Add,
            Sub,
            Mul,
            Div
        };

    public:
        QAssign();
        QAssign(const QAssign &other);
        QAssign &operator=(const QAssign &other);
        QAssign(const QVariant &value);
        QAssign(const QF &f);
        QAssign(QAssignPrivate *d);
        virtual ~QAssign();

#if defined(__GXX_EXPERIMENTAL_CXX0X__)
        inline QAssign(QAssign &&other)
        {
            d = other.d; other.d = nullptr;
        }

        inline QAssign &operator=(QAssign &&other)
        {
            if (this != &other)
            {
                d = other.d;
                other.d = nullptr;
            }
            return *this;
        }
#endif

        bool isValid() const;

        QAssign operator+(const QAssign &other);
        QAssign operator-(const QAssign &other);
        QAssign operator*(const QAssign &other);
        QAssign operator/(const QAssign &other);

        static QString operationStr(Operation op);

    public:
        QString sql(QSqlDriver *driver) const;
        void bindValues(QVariantList &values) const;

    private:
        QAssignPrivate *d;
};

class QOpAssign : public QAssign
{
    public:
        QOpAssign(const QAssign &left, const QAssign &right, Operation op);
        ~QOpAssign();
};

#endif