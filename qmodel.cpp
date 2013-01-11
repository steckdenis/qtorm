/*
 * qmodel.cpp
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

#include "qmodel.h"
#include "qfield_p.h"
#include "qtormdatabase.h"

#include <QVector>
#include <QtSql>
#include <QtDebug>

struct QModel::Private
{
    Private()
     : tableNumber(0)
    {
    }

    QString db_table;
    int tableNumber;

    QVector<QField> fields;
    QField primaryKey;
};

QModel::QModel(const QString &tableName)
: d(new QModel::Private())
{
    d->db_table = tableName;
}

QModel::~QModel()
{
    delete d;
}

void QModel::init()
{
    // Explore the field and set/create the primary key
    for (int i=0; i<d->fields.size(); ++i)
    {
        if (d->fields.at(i).primaryKey())
        {
            d->primaryKey = d->fields.at(i);
            break;
        }
    }

    if (!d->primaryKey.isValid())
    {
        // Create a primary key field
        d->primaryKey = intField("id");
        d->primaryKey.setAutoIncrement(true);
        d->primaryKey.setPrimaryKey(true);

        // We want id to be the first field
        d->fields.prepend(d->primaryKey);
        d->fields.remove(d->fields.size()-1);
    }
}

void QModel::addField(const QField &field)
{
    d->fields.append(field);
}

QField &QModel::pk() const
{
    return d->primaryKey;
}

QString QModel::tableName() const
{
    return d->db_table;
}

QStringField QModel::stringField(const QString &name)
{
    QStringField rs(this, name);

    addField(rs);

    return rs;
}

QIntField QModel::intField(const QString &name)
{
    QIntField rs(this, name);

    addField(rs);

    return rs;
}

QDoubleField QModel::doubleField(const QString &name)
{
    QDoubleField rs(this, name);

    addField(rs);

    return rs;
}

QDateTimeField QModel::dateTimeField(const QString& name)
{
    QDateTimeField rs(this, name);

    addField(rs);

    return rs;
}

void QModel::save(bool forceInsert)
{
    QSqlDriver *driver = QtOrmDatabase::threadDatabase().driver();
    QSqlQuery query(QtOrmDatabase::threadDatabase());

    if (forceInsert || pk().isNull())
    {
        // Build the fields list and placeholder lists, skip the primary key
        QString field_list;
        QString placeholders;
        bool first = true;

        for (int i=0; i<d->fields.size(); ++i)
        {
            if (d->fields.at(i).primaryKey() && d->fields.at(i).isNull())
                continue;

            if (!first)
            {
                field_list += QLatin1String(", ");
                placeholders += QLatin1String(", ");
            }

            field_list += driver->escapeIdentifier(d->fields.at(i).name(), QSqlDriver::FieldName);
            placeholders += QLatin1String("?");
            first = false;
        }

        // INSERT query
        QString sql = QString("INSERT INTO %1 (%2) VALUES (%3);")
            .arg(driver->escapeIdentifier(d->db_table, QSqlDriver::TableName))
            .arg(field_list)
            .arg(placeholders);

        query.prepare(sql);

        for (int i=0; i<d->fields.size(); ++i)
            if (!(d->fields.at(i).primaryKey() && d->fields.at(i).isNull()))
                query.addBindValue(d->fields.at(i).data());

        if (!query.exec())
        {
            qDebug() << "Could not save object :" << query.lastError();
        }

        // Set the id
        pk().fromData(query.lastInsertId());
    }
    else
    {
        // Only update an existing field
        QString values;
        bool first = true;

        for (int i=0; i<d->fields.size(); ++i)
        {
            // Ne pas mettre à jour les champs non modifiés
            if (!d->fields.at(i).isModified())
                continue;

            if (!first)
                values += QLatin1String(", ");

            values += driver->escapeIdentifier(d->fields.at(i).name(), QSqlDriver::FieldName);
            values += QLatin1String("=?");
            first = false;
        }

        // UPDATE query
        QString sql = QString("UPDATE %1 SET %2 WHERE %3=?;")
            .arg(driver->escapeIdentifier(d->db_table, QSqlDriver::TableName))
            .arg(values)
            .arg(driver->escapeIdentifier(pk().name(), QSqlDriver::FieldName));

        query.prepare(sql);

        for (int i=0; i<d->fields.size(); ++i)
        {
            if (d->fields.at(i).isModified())
                query.addBindValue(d->fields.at(i).data());
        }

        query.addBindValue(pk().data());

        if (!query.exec())
        {
            qDebug() << "Could not update object :" << query.lastError();
        }
    }
}

void QModel::remove()
{
    QSqlDriver *driver = QtOrmDatabase::threadDatabase().driver();
    QSqlQuery query(QtOrmDatabase::threadDatabase());

    // DELETE the current object, and set pk() to NULL
    QString sql = QString("DELETE FROM %1 WHERE %2=?;")
        .arg(driver->escapeIdentifier(d->db_table, QSqlDriver::TableName))
        .arg(driver->escapeIdentifier(pk().name(), QSqlDriver::FieldName));

    query.prepare(sql);
    query.addBindValue(pk().data());

    if (!query.exec())
    {
        qDebug() << "Could not delete object :" << query.lastError();
    }

    pk().setNull(true);
}

QString QModel::createTableSql() const
{
    QSqlDriver *driver = QSqlDatabase::database().driver();

    // Build the fields list
    QString field_list;

    for (int i=0; i<d->fields.size(); ++i)
    {
        field_list += QLatin1String("    ");
        field_list += driver->escapeIdentifier(d->fields.at(i).name(), QSqlDriver::FieldName);
        field_list += QChar(' ');
        field_list += d->fields.at(i).sqlDescription();

        if (i != d->fields.size() - 1)
            field_list += QLatin1String(",\n");
    }

    // CREATE TABLE statement
    return QString("CREATE TABLE %1 (\n%2\n);")
        .arg(driver->escapeIdentifier(d->db_table, QSqlDriver::TableName))
        .arg(field_list);
}

void QModel::getForeignKeys(QVector<QForeignKeyPrivate *> &foreignKeys) const
{
    for (int i=0; i<d->fields.size(); ++i)
    {
        if (d->fields.at(i).d->isForeignKey())
        {
            foreignKeys.append((QForeignKeyPrivate *)d->fields.at(i).d);
        }
    }
}

void QModel::resetModified()
{
    for (int i=0; i<d->fields.size(); ++i)
    {
        d->fields[i].setModified(false);
    }
}

void QModel::setTableNumber(int tableNumber)
{
    d->tableNumber = tableNumber;
}

int QModel::tableNumber() const
{
    return d->tableNumber;
}

int QModel::fieldsCount() const
{
    return d->fields.count();
}

const QField &QModel::field(int i) const
{
    return d->fields.at(i);
}
