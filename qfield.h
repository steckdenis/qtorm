/*
 * qfield.h
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

#ifndef __QFIELD_H__
#define __QFIELD_H__

#include <QString>
#include <QVariant>

#include "qf.h"

class QModel;
class QFieldPrivate;
class QWherePrivate;
class QAssignPrivate;
class QQuerySetPrivate;
class QForeignKeyPrivate;

#define _Q_F_ASSIGN(T) T &operator=(const QAssign &a) { setAssignation(a); return *this; }

class QField
{
    friend class QModel;
    friend class QWherePrivate;
    friend class QAssignPrivate;
    friend class QQuerySetPrivate;
    friend class QForeignKeyPrivate;

    public:
        QField();
        QField(const QField &other);
        QField &operator=(const QField &other);
        ~QField();

        inline bool operator==(const QField &other) const { return d == other.d; }

#if defined(__GXX_EXPERIMENTAL_CXX0X__)
        inline QField(QField &&other)
        {
            d = other.d; other.d = nullptr;
        }

        inline QField &operator=(QField &&other)
        {
            if (this != &other)
            {
                d = other.d;
                other.d = nullptr;
            }
            return *this;
        }
#endif

        QString name() const;
        bool isNull() const;
        bool isValid() const;
        bool isModified() const;

        // Parameters
        void setAcceptsNull(bool null);
        bool acceptsNull() const;
        void setAutoIncrement(bool autoincrement);
        bool autoIncrement() const;
        void setPrimaryKey(bool primarykey);
        bool primaryKey() const;

        // Accessor
        QVariant data() const;

        // QAssign integration
        void setAssignation(const QAssign &assignation);

    private:
        void setNull(bool null);
        void setModified(bool modified);

        void fromData(const QVariant &data);
        QModel *model() const;

        QString sqlDescription() const;
        QString fieldName() const;
        QAssign assignation() const;

    protected:
        QField(QFieldPrivate *dptr, bool reference = false);
        QFieldPrivate *d;

    friend uint qHash(const QField &field);
};

Q_DECLARE_TYPEINFO(QField, Q_MOVABLE_TYPE);

uint qHash(const QField &field);

#endif
