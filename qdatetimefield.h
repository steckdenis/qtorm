/*
 * qdatetimefield.h
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

#ifndef __QDATETIMEFIELD_H__
#define __QDATETIMEFIELD_H__

#include "qfield.h"
#include <QDateTime>

class QDateTimeFieldPrivate;

class QDateTimeField : public QField
{
    public:
        QDateTimeField();
        QDateTimeField(QModel *model, const QString &name);
        ~QDateTimeField();

        operator QDateTime() const;
        QDateTimeField &operator=(const QDateTime &value);

        _Q_F_ASSIGN(QDateTimeField)

    private:
        QDateTimeFieldPrivate *dptr() const;
};

Q_DECLARE_TYPEINFO(QDateTimeField, Q_MOVABLE_TYPE);

#endif
