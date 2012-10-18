/*
 * qforeignkey_p.h
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

#ifndef __QFOREIGNKEYPRIVATE_H__
#define __QFOREIGNKEYPRIVATE_H__

#include "qfield_p.h"

class QModel;

class QForeignKeyPrivate : public QFieldPrivate
{
    public:
        QForeignKeyPrivate(QModel *model, const QString &name);
        ~QForeignKeyPrivate();

        void setValue(QModel *value);
        void setValue(const QVariant &value);
        QModel *value();
        void setDeleteValue(bool enable);
        void fillCache() const;

        void fromData(const QVariant &data);
        QVariant data() const;
        QString sqlDescription() const;

        bool isForeignKey() const;
        void foreignInit();

    private:
        void deleteValue();

    private:
        QModel *_value;
        QVariant _id;
        bool _delete_value;
};

#endif
