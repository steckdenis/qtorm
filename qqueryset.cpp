/*
 * qqueryset.cpp
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

#include "qqueryset.h"
#include "qmodel.h"
#include "qfield.h"
#include "qf.h"

#include <QtSql>
#include <QtDebug>
#include <QVector>
#include <QPair>
#include <QString>

class QForeignKeyPrivate;

class QQuerySetPrivate
{
    public:
        QQuerySetPrivate(QModel *model);
        ~QQuerySetPrivate();

        void addSelectRelated(const QField &field);
        void addFilter(const QWhere &cond);
        void addOrderBy(const QField &field, bool asc);
        void setLimit(int count);
        void setOffset(int val);

        bool next();
        bool update(int *affectedRows);

        void build(bool for_remove);
        void exec();
        QString sql() const;
        void reset();

    private:
        void exploreModel(QModel *model, QForeignKeyPrivate *referrer);

    private:
        QModel *_model;
        int _limit, _offset;
        bool _built, _executed;

        QVector<QField> _selected_fields;
        QVector<QField> _select_related;
        QVector<QWhere> _filter;
        QVector<QPair<QField, bool> > _order_by;
        QVector<QPair<QModel *, QForeignKeyPrivate *> > _joins;

        QSqlQuery _query;
};

/*
 * Private
 */

QQuerySetPrivate::QQuerySetPrivate(QModel *model)
: _model(model), _limit(0), _offset(0), _built(false), _executed(false)
{
}

QQuerySetPrivate::~QQuerySetPrivate()
{
}

void QQuerySetPrivate::addSelectRelated(const QField &field)
{
    _select_related.append(field);
}

void QQuerySetPrivate::addFilter(const QWhere &cond)
{
    _filter.append(cond);
}

void QQuerySetPrivate::addOrderBy(const QField &field, bool asc)
{
    _order_by.append(qMakePair(field, asc));
}

void QQuerySetPrivate::setLimit(int count)
{
     _limit = count;
}

void QQuerySetPrivate::setOffset(int val)
{
    _offset = val;
}

QString QQuerySetPrivate::sql() const
{
    return _query.lastQuery();
}

void QQuerySetPrivate::build(bool for_remove)
{
    if (_built)
        return;

    _built = true;

    // Explore the model to find all its foreign keys that are instantiated
    exploreModel(_model, NULL);

    QSqlDriver *driver = QSqlDatabase::database().driver();

    // Build the select and join parts of the query
    QString select_part;
    QString from_part;

    for (int i=0; i<_joins.count(); ++i)
    {
        QModel *model = _joins.at(i).first;
        QForeignKeyPrivate *referrer = _joins.at(i).second;

        // Select the model's fields
        if (!for_remove)
        {
            for (int j=0; j<model->fieldsCount(); ++j)
            {
                if (i != 0 or j != 0)
                    select_part += QLatin1String(", ");

                select_part += model->field(j).fieldName();
                _selected_fields.append(model->field(j));
            }
        }

        // Join with the model
        if (for_remove)
        {
            // When we delete, don't use "FROM .. AS .." syntax
            from_part = driver->escapeIdentifier(model->tableName(), QSqlDriver::TableName);
        }
        else if (!referrer)
        {
            // No referrer, main model
            from_part += QString("%0 AS T%1")
                .arg(driver->escapeIdentifier(model->tableName(), QSqlDriver::TableName))
                .arg(model->tableNumber());
        }
        else
        {
            // INNER/LEFT JOIN with an ON clause
            QString joinType = referrer->acceptsNull() ? "LEFT" : "INNER";

            from_part += QString(" %0 JOIN %1 AS T%2 ON %3 = %4")
                .arg(joinType)
                .arg(driver->escapeIdentifier(model->tableName(), QSqlDriver::TableName))
                .arg(model->tableNumber())
                .arg(model->pk().fieldName())
                .arg(QField(referrer).fieldName());
        }
    }

    // Build the WHERE part
    QString where_part;

    for (int i=0; i<_filter.count(); ++i)
    {
        if (i == 0)
            where_part = QLatin1String(" WHERE ");
        else
            where_part += QLatin1String(" AND ");

        QString part = _filter.at(i).sql();

        // If we delete, remove all allusions to T0
        if (for_remove)
        {
            part.remove(QString("%1.")
                        .arg(driver->escapeIdentifier("T0", QSqlDriver::TableName)));
        }

        where_part.append(part);
    }

    // ORDER BY and LIMIT/OFFSET
    QString order_by_part;
    QString limit_part;

    if (!for_remove)
    {
        // Build the ORDER BY part
        for (int i=0; i<_order_by.count(); ++i)
        {
            if (i == 0)
                order_by_part = QLatin1String(" ORDER BY ");
            else
                order_by_part += QLatin1String(", ");

            order_by_part += _order_by.at(i).first.fieldName();
            order_by_part += _order_by.at(i).second ? " ASC" : " DESC";
        }

        // Build the LIMIT/OFFSET part
        if (_limit)
            limit_part = QString(" LIMIT %0").arg(_limit);
        if (_offset)
            limit_part += QString(" OFFSET %0").arg(_offset);
    }

    // Prepare the query
    QString q;

    if (for_remove)
    {
        q = QString("DELETE FROM %2%3;")
            .arg(from_part, where_part);
    }
    else
    {
        q = QString("SELECT %1 FROM %2%3%4%5;")
            .arg(select_part, from_part, where_part, order_by_part, limit_part);
    }

    if (!_query.prepare(q))
    {
        qDebug() << "Cannot prepare the query \"" << _query.lastQuery() << "\" :" << _query.lastError();
    }
}

void QQuerySetPrivate::exec()
{
    if (_executed)
        return;

    _executed = true;

    // Bind the values
    QVariantList values;

    for (int i=0; i<_filter.count(); ++i)
    {
        _filter.at(i).bindValues(values);
    }

    for (int i=0; i<values.count(); ++i)
    {
        _query.addBindValue(values.at(i));
    }

    if (!_query.exec())
    {
        qDebug() << "Cannot execute the query \"" << _query.lastQuery() << "\" :" << _query.lastError();
    }
}

bool QQuerySetPrivate::next()
{
    if (!_query.next())
        return false;

    // Get a row from the query and populate the model with it
    for (int i=0; i<_selected_fields.count(); ++i)
    {
        _selected_fields[i].fromData(_query.value(i));
    }

    return true;
}

bool QQuerySetPrivate::update(int *affectedRows)
{
    QSqlDriver *driver = QSqlDatabase::database().driver();

    // Build the list of fields to update
    QString fields_part;
    QVariantList values;
    bool first = true;

    for (int i=0; i<_model->fieldsCount(); ++i)
    {
        const QField &f = _model->field(i);

        if (f.isModified())
        {
            if (!first)
                fields_part += QLatin1String(", ");

            fields_part += f.fieldName();
            const QAssign &assign = f.assignation();

            if (!assign.isValid())
            {
                // No assignation, just an immediate value
                fields_part += QLatin1String(" = ?");
                values.append(f.data());
            }
            else
            {
                // An assignation, append its SQL
                fields_part += QLatin1String(" = ");
                fields_part += assign.sql();
                assign.bindValues(values);
            }
            first = false;
        }
    }

    if (fields_part.isEmpty())
        return true;

    // Build the WHERE part
    QString where_part;

    for (int i=0; i<_filter.count(); ++i)
    {
        if (i == 0)
            where_part = QLatin1String(" WHERE ");
        else
            where_part += QLatin1String(" AND ");

        where_part.append(_filter.at(i).sql());
        _filter.at(i).bindValues(values);
    }

    // Whole SQL
    QString sql = QString("UPDATE %0 AS T0 SET %1%2;")
        .arg(driver->escapeIdentifier(_model->tableName(), QSqlDriver::TableName))
        .arg(fields_part)
        .arg(where_part);

    // Prepare and run the query
    QSqlQuery query;
    query.prepare(sql);

    for (int i=0; i<values.count(); ++i)
    {
        query.addBindValue(values.at(i));
    }

    if (!query.exec())
    {
        qDebug() << query.lastError();
        return false;
    }

    if (affectedRows)
        *affectedRows = query.numRowsAffected();

    return true;
}

void QQuerySetPrivate::reset()
{
    _built = false;
    _executed = false;

    _selected_fields.clear();
    _select_related.clear();
    _filter.clear();
    _order_by.clear();
    _joins.clear();
    _query.finish();
}

void QQuerySetPrivate::exploreModel(QModel *model, QForeignKeyPrivate *referrer)
{
    // getForeignKeys(QVector<QForeignKeyPrivate *> foreignKeys)
    // Allocate a table name for this model
    int tableNumber = _joins.count();

    model->setTableNumber(tableNumber);

    // Add the new table into the list
    _joins.append(qMakePair(model, referrer));

    // Explore the sub foreign keys
    QVector<QForeignKeyPrivate *> subkeys;

    model->getForeignKeys(subkeys);

    for (int i=0; i<subkeys.count(); ++i)
    {
        QForeignKeyPrivate *key = subkeys.at(i);
        QModel *submodel = key->value();

        if (submodel)
        {
            // Select-related or involved in a condition
            exploreModel(submodel, key);
        }
    }
}

/*
 * QuerySet
 */


QQuerySet::QQuerySet(QModel *model)
: d(new QQuerySetPrivate(model))
{
}

QQuerySet::~QQuerySet()
{
    delete d;
}

void QQuerySet::addSelectRelated_p(const QField &field)
{
    d->addSelectRelated(field);
}

void QQuerySet::addFilter(const QWhere &cond)
{
    d->addFilter(cond);
}

void QQuerySet::addOrderBy(const QField &field, bool asc)
{
    d->addOrderBy(field, asc);
}

void QQuerySet::setLimit(int count)
{
    d->setLimit(count);
}

void QQuerySet::setOffset(int val)
{
    d->setOffset(val);
}

QString QQuerySet::sql(bool for_remove)
{
    d->build(for_remove);
    return d->sql();
}

bool QQuerySet::next()
{
    d->build(false);
    d->exec();
    return d->next();
}

int QQuerySet::update(int *affectedRows)
{
    return d->update(affectedRows);
}

void QQuerySet::remove()
{
    d->build(true);
    d->exec();
}

void QQuerySet::reset()
{
    d->reset();
}
