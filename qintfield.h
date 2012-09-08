/*
 * qintfield.h
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

#ifndef __QINTFIELD_H__
#define __QINTFIELD_H__

#include "qfield.h"

class QIntFieldPrivate;

class QIntField : public QField
{
    public:
        QIntField();
        QIntField(QModel *model, const QString &name);
        ~QIntField();

        operator int() const;
        QIntField &operator=(int value);

        _Q_F_ASSIGN(QIntField)

    private:
        QIntFieldPrivate *dptr() const;
};

#endif
