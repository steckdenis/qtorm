/*
 * qmodel.h
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

#ifndef __QMODEL_H__
#define __QMODEL_H__

#include <QSqlDatabase>
#include <QString>

#include "qstringfield.h"
#include "qintfield.h"
#include "qforeignkey.h"
#include "qdoublefield.h"
#include "qdatetimefield.h"

class QQuerySetPrivate;
class QForeignKeyPrivate;

class QModel
{
    friend class QQuerySetPrivate;
    friend class QField;

    private:
        Q_DISABLE_COPY(QModel)

    public:
        QModel(const QString &tableName);
        virtual ~QModel();

        QField &pk() const;
        QString tableName() const;

        void clearBatch();
        void addInBatch();
        void saveBatch();

        void setTableName(const QString &tableName);
        void save(bool forceInsert=false);
        void remove();
        void resetModified();
        QString createTableSql() const;

    protected:
        void init();

        QStringField stringField(const QString &name);
        QIntField intField(const QString &name);
        QDoubleField doubleField(const QString &name);
        QDateTimeField dateTimeField(const QString &name);
        template<typename T>
        QForeignKey<T> foreignKey(const QString &name);

    private:
        void getForeignKeys(QVector<QForeignKeyPrivate *> &foreignKeys) const;
        void setTableNumber(int tableNumber);
        int tableNumber() const;

        int fieldsCount() const;
        const QField &field(int i) const;

    private:
        struct Private;
        Private *d;

        void addField(const QField &field);
};

template<typename T>
QForeignKey<T> QModel::foreignKey(const QString &name)
{
    QForeignKey<T> rs(this, name);

    addField(rs);

    return rs;
}

#endif
