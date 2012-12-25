/*
 * qstringfield.cpp
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

#include "qstringfield.h"
#include "qfield_p.h"

class QStringFieldPrivate : public QFieldPrivate
{
    public:
        QStringFieldPrivate(QModel *model, const QString &name);
        ~QStringFieldPrivate();

        void setMaxLength(unsigned int length);
        void setValue(const QString &value);

        void fromData(const QVariant &data);
        QVariant data() const;
        QString sqlDescription() const;

    private:
        QString _data;
        unsigned int _max_length;
};

QStringFieldPrivate::QStringFieldPrivate(QModel *model, const QString &name)
 : QFieldPrivate(model, name), _max_length(200)
{

}

QStringFieldPrivate::~QStringFieldPrivate()
{

}

void QStringFieldPrivate::setMaxLength(unsigned int length)
{
    _max_length = length;
}

void QStringFieldPrivate::setValue(const QString &value)
{
    setNull(false); // The field is not null anymore
    setModified(true);
    _data = value;
}

void QStringFieldPrivate::fromData(const QVariant &data)
{
    setNull(data.isNull());
    setModified(false);
    _data = data.toString();
}

QVariant QStringFieldPrivate::data() const
{
    if (isNull())
        return QVariant();
    else
        return QVariant(_data);
}

QString QStringFieldPrivate::sqlDescription() const
{
    QString rs = QString("VARCHAR(%1)").arg(_max_length);

    rs += commonSqlDescription();

    return rs;
}

/*
 * QStringField
 */

QStringField::QStringField()
 : QField(NULL)
{
}

QStringField::QStringField(QModel *model, const QString &name)
 : QField(new QStringFieldPrivate(model, name))
{
}

QStringField::~QStringField()
{
}

QStringField::operator QString() const
{
    return dptr()->data().toString();
}

void QStringField::setMaxLength(unsigned int length)
{
    dptr()->setMaxLength(length);
}

QStringField &QStringField::operator=(const QString &value)
{
    dptr()->setValue(value);
    return *this;
}

QStringFieldPrivate *QStringField::dptr() const
{
    return (QStringFieldPrivate *)d;
}
