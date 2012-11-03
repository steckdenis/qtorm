/*
 * qwhere.h
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

#ifndef __QWHERE_H__
#define __QWHERE_H__

#include <QVariant>

class QField;
class QWherePrivate;

class QSqlDriver;

class QWhere
{
    public:
        enum Condition
        {
            Equal,
            NotEqual,
            Less,
            Greater,
            LessEqual,
            GreaterEqual,
            Null,
            In,
            And,
            Or,
            Not,
            Like
        };

    public:
        QWhere();
        QWhere(const QWhere &other);
        QWhere(QWherePrivate *d);
        virtual ~QWhere();

#if defined(__GXX_EXPERIMENTAL_CXX0X__)
        inline QWhere(QWhere &&other)
        {
            d = other.d; other.d = nullptr;
        }

        inline QWhere &operator=(QWhere &&other)
        {
            if (this != &other)
            {
                d = other.d;
                other.d = nullptr;
            }
            return *this;
        }
#endif

        bool isValid() const;

        QWhere operator!() const;
        QWhere operator&&(const QWhere &other) const;
        QWhere operator||(const QWhere &other) const;
        QWhere &operator=(const QWhere &other);

        static QString conditionStr(Condition cond);

    public:
        QString sql(QSqlDriver *driver) const;
        void bindValues(QVariantList &values) const;

    private:
        QWherePrivate *d;
};

class QFInWhere : public QWhere
{
    public:
        QFInWhere(const QField &left, const QVariantList &right);
};

class QFLikeWhere : public QWhere
{
    public:
        QFLikeWhere(const QField &left, const QString &right);
};

class QFDivWhere : public QWhere
{
    public:
        QFDivWhere(const QField &left, int divisor, int offset);
};

class QFFlagSetWhere : public QWhere
{
    public:
        QFFlagSetWhere(const QField &left, int flag);
};

class QFIWhere : public QWhere
{
    public:
        QFIWhere(const QField &left, const QVariant &right, Condition cond);
};

class QFFWhere : public QWhere
{
    public:
        QFFWhere(const QField &left, const QField &right, Condition cond);
};

class QWWWhere : public QWhere
{
    public:
        QWWWhere(const QWhere &left, const QWhere &right, Condition cond);
};

class QWWhere : public QWhere
{
    public:
        QWWhere(const QWhere &w, Condition cond);
};

class QFWhere : public QWhere
{
    public:
        QFWhere(const QField &f, Condition cond);
};

#endif
