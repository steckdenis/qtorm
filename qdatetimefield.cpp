/*
 * qdatetimefield.cpp
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

#include "qdatetimefield.h"
#include "qfield_p.h"

class QDateTimeFieldPrivate : public QFieldPrivate
{
    public:
        QDateTimeFieldPrivate(QModel *model, const QString &name);
        ~QDateTimeFieldPrivate();

        void setValue(const QDateTime &value);

        void fromData(const QVariant &data);
        QVariant data() const;
        QString sqlDescription() const;

    private:
        QDateTime _datetime;
};

QDateTimeFieldPrivate::QDateTimeFieldPrivate(QModel *model, const QString &name)
 : QFieldPrivate(model, name)
{

}

QDateTimeFieldPrivate::~QDateTimeFieldPrivate()
{

}

void QDateTimeFieldPrivate::setValue(const QDateTime &value)
{
    setNull(false); // The field is not null anymore
    setModified(true);
    _datetime = value;
}

void QDateTimeFieldPrivate::fromData(const QVariant &data)
{
    setNull(data.isNull());
    setModified(false);
    _datetime = data.toDateTime();
}

QVariant QDateTimeFieldPrivate::data() const
{
    if (isNull())
        return QVariant(QVariant::DateTime);
    else
        return QVariant(_datetime);
}

QString QDateTimeFieldPrivate::sqlDescription() const
{
    QString rs = QLatin1String("DATETIME");

    rs += commonSqlDescription();

    return rs;
}

/*
 * QDateTimeField
 */

QDateTimeField::QDateTimeField()
 : QField(NULL)
{
}

QDateTimeField::QDateTimeField(QModel *model, const QString &name)
 : QField(new QDateTimeFieldPrivate(model, name))
{
}

QDateTimeField::~QDateTimeField()
{
}

QDateTimeField::operator QDateTime() const
{
    return dptr()->data().toDateTime();
}

QDateTimeField &QDateTimeField::operator=(const QDateTime &value)
{
    dptr()->setValue(value);
    return *this;
}

QDateTimeFieldPrivate *QDateTimeField::dptr() const
{
    return (QDateTimeFieldPrivate *)d;
}
