/*
 * qintfield.cpp
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

#include "qintfield.h"
#include "qfield_p.h"

class QIntFieldPrivate : public QFieldPrivate
{
    public:
        QIntFieldPrivate(QModel *model, const QString &name);
        ~QIntFieldPrivate();

        void setValue(int value);

        void fromData(const QVariant &data);
        QVariant data() const;
        QString sqlDescription() const;

    private:
        int _value;
};

QIntFieldPrivate::QIntFieldPrivate(QModel *model, const QString &name)
 : QFieldPrivate(model, name), _value(0)
{

}

QIntFieldPrivate::~QIntFieldPrivate()
{

}

void QIntFieldPrivate::setValue(int value)
{
    setNull(false); // The field is not null anymore
    setModified(true);
    _value = value;
}

void QIntFieldPrivate::fromData(const QVariant &data)
{
    setNull(data.isNull());
    setModified(false);
    _value = data.toInt();
}

QVariant QIntFieldPrivate::data() const
{
    if (isNull())
        return QVariant();
    else
        return QVariant(_value);
}

QString QIntFieldPrivate::sqlDescription() const
{
    QString rs = QLatin1String("INTEGER");

    rs += commonSqlDescription();

    return rs;
}

/*
 * QIntField
 */

QIntField::QIntField()
 : QField(NULL)
{
}

QIntField::QIntField(QModel *model, const QString &name)
 : QField(new QIntFieldPrivate(model, name))
{
}

QIntField::~QIntField()
{
}

QIntField::operator int() const
{
    return dptr()->data().toInt();
}

QIntField &QIntField::operator=(int value)
{
    dptr()->setValue(value);
    return *this;
}

QIntFieldPrivate *QIntField::dptr() const
{
    return (QIntFieldPrivate *)d;
}
