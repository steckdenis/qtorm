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
#include "qtormdatabase.h"

#include <QtSql>
#include <QtDebug>
#include <QVector>
#include <QSet>
#include <QPair>
#include <QString>

class QForeignKeyPrivate;

class QQuerySetPrivate
{
    public:
        QQuerySetPrivate(QModel *model, const QSqlDatabase &db);
        ~QQuerySetPrivate();

        void addSelectRelated(const QField &field);
        void addFilter(const QWhere &cond);
        void addOrderBy(const QField &field, bool asc);
        void addField(const QField &field);
        void addFields(QModel *model);
        void excludeField(const QField &field);
        void setLimit(int count);
        void setOffset(int val);

        bool next();
        bool update(int *affectedRows);

        void build(bool for_remove);
        void exec();
        QString sql() const;
        void reset();

    private:
        struct Join
        {
            QModel *model;
            QForeignKeyPrivate *parent_foreignkey;
            bool accepts_null;
        };

        bool buildJoins(QVector<QQuerySetPrivate::Join> &joins, bool useSelectedFields);
        QVector<Join> buildSelectedFields(bool for_remove);
        QString buildSelect();
        QString buildFrom(const QVector<Join> &joins, bool for_remove);
        QString buildWhere(bool for_remove);
        QString buildOrderBy();
        QString buildLimit();

    private:
        QSqlDriver *_driver;
        QModel *_model;
        int _limit, _offset;
        bool _built, _executed;

        QVector<QField> _selected_fields;
        QSet<QField> _excluded_fields;
        QSet<QModel *> _selected_models;
        QVector<QField> _select_related;
        QVector<QWhere> _filter;
        QVector<QPair<QField, bool> > _order_by;

        QSqlQuery _query;
};

/*
 * Private
 */

QQuerySetPrivate::QQuerySetPrivate(QModel *model, const QSqlDatabase &db)
: _driver(db.driver()),
  _model(model),
  _limit(0),
  _offset(0),
  _built(false),
  _executed(false),
  _query(db)
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

void QQuerySetPrivate::addField(const QField &field)
{
    _selected_fields.append(field);
    _selected_models.insert(field.model());
}

void QQuerySetPrivate::addFields(QModel *model)
{
    int i, count = model->fieldsCount();

    for (i=0; i<count; ++i)
    {
        addField(model->field(i));
    }
}

void QQuerySetPrivate::excludeField(const QField &field)
{
    _excluded_fields.insert(field);
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


bool QQuerySetPrivate::buildJoins(QVector<Join> &joins, bool useSelectedFields)
{
    // Model to explore
    Join &join = joins.last();

    // Allocate a table number for this model
    join.model->setTableNumber(joins.count());

    // If one of the requested fields is in this model, we are useful
    bool useful_join = _selected_models.contains(join.model);

    // Explore the foreign keys of this model
    QVector<QForeignKeyPrivate *> subkeys;
    Join new_join;

    join.model->getForeignKeys(subkeys);

    for (int i=0; i<subkeys.count(); ++i)
    {
        QForeignKeyPrivate *foreign_key = subkeys.at(i);
        QModel *target_model = foreign_key->value();

        // Ignore a field in the exclude list
        if (_excluded_fields.contains(QField(foreign_key, true)))
            continue;

        if (target_model)
        {
            // Create a new join
            new_join.model = target_model;
            new_join.parent_foreignkey = foreign_key;
            new_join.accepts_null = (foreign_key->acceptsNull() || join.accepts_null);

            // Add the join to the list of joins to explore
            joins.append(new_join);

            // Explore the joint. If we restrict ourself to the fields in
            // _selected_fields, don't join with a table that has no field in
            // our list.
            if (useSelectedFields)
            {
                if (!buildJoins(joins, useSelectedFields))
                {
                    // No field used in this join, remove it from the list
                    joins.resize(joins.size() - 1);
                }
                else
                {
                    useful_join = true;
                }
            }
            else
            {
                // Inconditionnally build joins
                buildJoins(joins, useSelectedFields);
            }
        }
    }

    return useful_join;
}

QVector<QQuerySetPrivate::Join> QQuerySetPrivate::buildSelectedFields(bool for_remove)
{
    // First join we always have
    QVector<Join> joins;
    Join start_join;

    start_join.model = _model;
    start_join.parent_foreignkey = NULL;
    start_join.accepts_null = false;

    joins.append(start_join);

    // Explore the model to build joins, but not when we remove as not all databases
    // support that.
    if (!for_remove)
    {
        buildJoins(joins, _selected_fields.count() != 0);

        // If we use a user-supplied _selected_fields list, we are done
        if (_selected_fields.count() != 0)
            return joins;
    }

    // Add the fields of every join to the list of the fields
    for (int i=0; i<joins.count(); ++i)
    {
        const Join &join = joins.at(i);
        int field_count = join.model->fieldsCount();

        for (int j=0; j<field_count; ++j)
        {
            const QField &field = join.model->field(j);

            if (!_excluded_fields.contains(field))
            {
                _selected_fields.append(field);
            }
        }
    }

    // Return the joins so other methods can use them
    return joins;
}

QString QQuerySetPrivate::buildSelect()
{
    QString rs;

    // Select all the selected fields
    for (int i=0; i<_selected_fields.count(); ++i)
    {
        const QField &field = _selected_fields.at(i);

        if (i != 0)
            rs += QLatin1String(", ");

        rs += _driver->escapeIdentifier(field.fieldName(), QSqlDriver::FieldName);
    }

    return rs;
}

QString QQuerySetPrivate::buildFrom(const QVector<Join> &joins, bool for_remove)
{
    QString rs;

    // Select from every table listed in joins
    for (int i=0; i<joins.count(); ++i)
    {
        const Join &join = joins.at(i);
        QModel *table = join.model;

        if (i == 0)
        {
            // Just the table, without alias if we build a DELETE FROM
            if (for_remove)
            {
                rs = _driver->escapeIdentifier(table->tableName(), QSqlDriver::TableName);
            }
            else
            {
                rs += QString("%0 AS T%1")
                    .arg(_driver->escapeIdentifier(table->tableName(), QSqlDriver::TableName))
                    .arg(table->tableNumber());
            }
        }
        else
        {
            Q_ASSERT(join.parent_foreignkey != NULL && "Only the first join (the main table in fact) can have a NULL parent foreign key.");

            rs += QString(" %0 JOIN %1 AS T%2 ON %3 = %4")
                .arg(join.accepts_null ? "LEFT" : "INNER")
                .arg(_driver->escapeIdentifier(table->tableName(), QSqlDriver::TableName))
                .arg(table->tableNumber())
                .arg(table->pk().fieldName())
                .arg(QField(join.parent_foreignkey, true).fieldName());
        }
    }

    return rs;
}

QString QQuerySetPrivate::buildWhere(bool for_remove)
{
    QString rs;

    for (int i=0; i<_filter.count(); ++i)
    {
        if (i == 0)
            rs = QLatin1String(" WHERE ");
        else
            rs += QLatin1String(" AND ");

        QString part = _filter.at(i).sql(_driver);

        // If we delete, remove all allusions to T0
        if (for_remove)
        {
            part.remove(QString("%1.")
                        .arg(_driver->escapeIdentifier("T0", QSqlDriver::TableName)));
        }

        rs.append(part);
    }

    return rs;
}

QString QQuerySetPrivate::buildOrderBy()
{
    QString rs;

    // Build the ORDER BY part
    for (int i=0; i<_order_by.count(); ++i)
    {
        if (i == 0)
            rs = QLatin1String(" ORDER BY ");
        else
            rs += QLatin1String(", ");

        rs += _driver->escapeIdentifier(_order_by.at(i).first.fieldName(), QSqlDriver::FieldName);
        rs += _order_by.at(i).second ? " ASC" : " DESC";
    }

    return rs;
}

QString QQuerySetPrivate::buildLimit()
{
    QString rs;

    // Build the LIMIT/OFFSET part
    if (_limit)
        rs = QString(" LIMIT %0").arg(_limit);
    if (_offset)
        rs += QString(" OFFSET %0").arg(_offset);

    return rs;
}

void QQuerySetPrivate::build(bool for_remove)
{
    if (_built)
        return;

    _built = true;

    // Joins used throughout
    QVector<QQuerySetPrivate::Join> joins = buildSelectedFields(for_remove);


    // Build the query
    QString q;

    if (for_remove)
    {
        q = QString("DELETE FROM %2%3;")
            .arg(buildFrom(joins, true))
            .arg(buildWhere(true));
    }
    else
    {
        q = QString("SELECT %1 FROM %2%3%4%5")
            .arg(buildSelect())
            .arg(buildFrom(joins, false))
            .arg(buildWhere(false))
            .arg(buildOrderBy())
            .arg(buildLimit());
    }

    // Prepare the query
    _query.finish();

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
                fields_part += assign.sql(_driver);
                assign.bindValues(values);
            }

            first = false;
        }
    }

    if (fields_part.isEmpty())
        return true;

    // Whole SQL
    QString sql = QString("UPDATE %0 AS T0 SET %1%2;")
        .arg(_driver->escapeIdentifier(_model->tableName(), QSqlDriver::TableName))
        .arg(fields_part)
        .arg(buildWhere(false));

    // Bind values for where
    for (int i=0; i<_filter.count(); ++i)
    {
        _filter.at(i).bindValues(values);
    }

    // Prepare and run the query
    _query.finish();
    _query.prepare(sql);

    for (int i=0; i<values.count(); ++i)
    {
        _query.addBindValue(values.at(i));
    }

    if (!_query.exec())
    {
        qDebug() << _query.lastError();
        return false;
    }

    if (affectedRows)
        *affectedRows = _query.numRowsAffected();

    return true;
}

void QQuerySetPrivate::reset()
{
    _built = false;
    _executed = false;

    _selected_fields.clear();
    _excluded_fields.clear();
    _select_related.clear();
    _filter.clear();
    _order_by.clear();
    _query.finish();
}

/*
 * QuerySet
 */


QQuerySet::QQuerySet(QModel *model)
: d(new QQuerySetPrivate(model, QtOrmDatabase::threadDatabase()))
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

void QQuerySet::addField(const QField &field)
{
    d->addField(field);
}

void QQuerySet::addFields(QModel *model)
{
    d->addFields(model);
}

void QQuerySet::excludeField(const QField &field)
{
    d->excludeField(field);
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

bool QQuerySet::update(int *affectedRows)
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
