/*
 * qdoublefield.cpp
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

#include "qdoublefield.h"
#include "qfield_p.h"

class QDoubleFieldPrivate : public QFieldPrivate
{
    public:
        QDoubleFieldPrivate(QModel *model, const QString &name);
        ~QDoubleFieldPrivate();

        void setValue(double value);

        void fromData(const QVariant &data);
        QVariant data() const;
        QString sqlDescription() const;

    private:
        double _value;
};

QDoubleFieldPrivate::QDoubleFieldPrivate(QModel *model, const QString &name)
 : QFieldPrivate(model, name), _value(0)
{

}

QDoubleFieldPrivate::~QDoubleFieldPrivate()
{

}

void QDoubleFieldPrivate::setValue(double value)
{
    setNull(false); // The field is not null anymore
    setModified(true);
    _value = value;
}

void QDoubleFieldPrivate::fromData(const QVariant &data)
{
    setNull(data.isNull());
    setModified(false);
    _value = data.toDouble();
}

QVariant QDoubleFieldPrivate::data() const
{
    if (isNull())
        return QVariant();
    else
        return QVariant(_value);
}

QString QDoubleFieldPrivate::sqlDescription() const
{
    QString rs = QLatin1String("DOUBLE");

    rs += commonSqlDescription();

    return rs;
}

/*
 * QDoubleField
 */

QDoubleField::QDoubleField()
 : QField(NULL)
{
}

QDoubleField::QDoubleField(QModel *model, const QString &name)
 : QField(new QDoubleFieldPrivate(model, name))
{
}

QDoubleField::~QDoubleField()
{
}

QDoubleField::operator double() const
{
    return dptr()->data().toDouble();
}

QDoubleField &QDoubleField::operator=(double value)
{
    dptr()->setValue(value);
    return *this;
}

QDoubleFieldPrivate *QDoubleField::dptr() const
{
    return (QDoubleFieldPrivate *)d;
}
