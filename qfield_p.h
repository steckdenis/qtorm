/*
 * qfield_p.h
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

#ifndef __QFIELDPRIVATE_H__
#define __QFIELDPRIVATE_H__

#include <QString>
#include <QVariant>

#include "qassign.h"

class QFieldPrivate
{
    public:
        QFieldPrivate(QModel *model, const QString &name);
        virtual ~QFieldPrivate();

        QString name() const;
        QModel *model() const;
        bool isNull() const;
        void setNull(bool isnull);
        void setModified(bool modified);
        bool isModified() const;
        void setAcceptsNull(bool null);
        bool acceptsNull() const;
        void setAutoIncrement(bool autoincrement);
        bool autoIncrement() const;
        void setPrimaryKey(bool primarykey);
        bool primaryKey() const;
        void setAssignation(const QAssign &assignation);
        QAssign assignation() const;

        virtual void fromData(const QVariant &data) = 0;
        virtual QVariant data() const = 0;
        virtual QString sqlDescription() const = 0;

        virtual bool isForeignKey() const;

        void ref();
        bool deref();

    protected:
        QString commonSqlDescription() const;

    protected:
        QModel *_model;
        QString _name;
        unsigned int _refcount;
        bool _isnull, _accepts_null, _auto_increment, _primary_key, _modified;
        QAssign _assignation;
};

#endif
