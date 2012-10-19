/*
 * qforeignkey.h
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

#ifndef __QFOREIGNKEY_H__
#define __QFOREIGNKEY_H__

#include "qfield.h"
#include "qforeignkey_p.h"

class QQuerySet;

template<typename T>
class QForeignKey : public QField
{
    friend class QQuerySet;

    public:
        QForeignKey();
        QForeignKey(QModel *model, const QString &name);
        ~QForeignKey();

        QForeignKey &operator=(const QVariant &value);
        T *value() const;
        T *operator->() const;

        void setDelegate(T *model);    /*!< @brief Define a model that will contain the target data of this foreign key, instead of a default one instanced as needed */

        _Q_F_ASSIGN(QForeignKey)

    private:
        T *checkValue() const;

    private:
        QForeignKeyPrivate *dptr() const;
};

template<> template<typename T>
Q_DECLARE_TYPEINFO_BODY(QForeignKey<T>, Q_MOVABLE_TYPE);

template<typename T>
QForeignKey<T>::QForeignKey() : QField(NULL)
{
}

template<typename T>
QForeignKey<T>::QForeignKey(QModel *model, const QString &name)
: QField(new QForeignKeyPrivate(model, name))
{
}

template<typename T>
QForeignKey<T>::~QForeignKey()
{
}

template<typename T>
QForeignKey<T> &QForeignKey<T>::operator=(const QVariant &value)
{
    dptr()->setValue(value);
    return *this;
}

template<typename T>
void QForeignKey<T>::setDelegate(T *model)
{
    dptr()->setValue(model);

    // Don't delete this model
    dptr()->setDeleteValue(false);
}

template<typename T>
T *QForeignKey<T>::checkValue() const
{
    T *rs = (T *)dptr()->value();

    if (!rs)
    {
        // Instantiate a child model (currently unbound)
        rs = new T;
        dptr()->setValue(rs);

        // We can delete this submodel (we come here after a setDelegate(NULL))
        dptr()->setDeleteValue(true);
    }

    return rs;
}

template<typename T>
T *QForeignKey<T>::value() const
{
    T *rs = checkValue();

    if ((rs->pk().isNull() && !dptr()->data().isNull()) ||
        (rs->pk().data() != dptr()->data()))
    {
        // Need to fill the cache
        dptr()->fillCache();
    }

    return rs;
}

template<typename T>
T *QForeignKey<T>::operator->() const
{
    return value();
}

template<typename T>
QForeignKeyPrivate *QForeignKey<T>::dptr() const
{
    return (QForeignKeyPrivate *)d;
}

#endif
