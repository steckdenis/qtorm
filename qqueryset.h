/*
 * qqueryset.h
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

#ifndef __QQUERYSET_H__
#define __QQUERYSET_H__

#include "qfield.h"
#include "qf.h"
#include "qforeignkey.h"

class QSqlDatabase;

class QModel;

class QQuerySet
{
    private:
        Q_DISABLE_COPY(QQuerySet)

    public:
        QQuerySet(QModel *model);
        ~QQuerySet();

        template<typename T>
        void addSelectRelated(const QForeignKey<T> &field);
        void addFilter(const QWhere &cond);
        void addOrderBy(const QField &field, bool asc);
        void setLimit(int count);
        void setOffset(int val);

        QString sql(bool for_remove = false);
        bool next();
        bool update(int *affectedRows = 0);
        void remove();
        void reset();

    private:
        QQuerySetPrivate *d;

        void addSelectRelated_p(const QField &field);
};

template<typename T>
void QQuerySet::addSelectRelated(const QForeignKey<T> &field)
{
    // Force the field to build its value
    field.checkValue();

    addSelectRelated_p(field);
}

#endif
