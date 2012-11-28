/*
 * qwhere.cpp
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

#include "qwhere.h"
#include "qfield.h"

#include <QtDebug>
#include <QSqlDriver>

/*
 * QWhere
 */

class QWherePrivate
{
    public:
        QWherePrivate(QWhere::Condition cond);
        virtual ~QWherePrivate();

        QWhere::Condition condition() const;
        QString fieldName(const QField &field, QSqlDriver *driver) const;

        void ref();
        bool deref();

        virtual QString sql(QSqlDriver *driver) const = 0;
        virtual void bindValues(QVariantList &values) const = 0;

    private:
        QWhere::Condition _cond;
        unsigned int _refcount;
};

QWherePrivate::QWherePrivate(QWhere::Condition cond) : _cond(cond), _refcount(1)
{
}

QWherePrivate::~QWherePrivate()
{
}

QWhere::Condition QWherePrivate::condition() const
{
    return _cond;
}

void QWherePrivate::ref()
{
    _refcount++;
}

bool QWherePrivate::deref()
{
    _refcount--;

    return (_refcount != 0);
}

QString QWherePrivate::fieldName(const QField &field, QSqlDriver *driver) const
{
    return driver->escapeIdentifier(field.fieldName(), QSqlDriver::FieldName);
}

QWhere::QWhere() : d(NULL)
{
}

QWhere::QWhere(const QWhere &other) : d(other.d)
{
    d->ref();
}

QWhere::QWhere(QWherePrivate *d) : d(d)
{
}

QWhere::~QWhere()
{
    if (d && !d->deref())
        delete d;
}

bool QWhere::isValid() const
{
    return (d != NULL);
}

QWhere QWhere::operator!() const
{
    return QWWhere(*this, Not);
}

QWhere QWhere::operator&&(const QWhere &other) const
{
    return QWWWhere(*this, other, And);
}

QWhere QWhere::operator||(const QWhere &other) const
{
    return QWWWhere(*this, other, Or);
}

QWhere &QWhere::operator=(const QWhere &other)
{
    if (d && !d->deref())
        delete d;

    d = other.d;

    if (d)
        d->ref();

    return *this;
}

QString QWhere::conditionStr(QWhere::Condition cond)
{
    switch (cond)
    {
        case Equal:
            return QLatin1String(" = ");
        case NotEqual:
            return QLatin1String(" != ");
        case Less:
            return QLatin1String(" < ");
        case Greater:
            return QLatin1String(" > ");
        case LessEqual:
            return QLatin1String(" <= ");
        case GreaterEqual:
            return QLatin1String(" >= ");
        case Null:
            return QLatin1String(" IS NULL");
        case In:
            return QLatin1String(" IN ");
        case And:
            return QLatin1String(" AND ");
        case Or:
            return QLatin1String(" OR ");
        case Not:
            return QLatin1String("NOT ");
        case Like:
            return QLatin1String(" LIKE ");
    }

    return QString();
}

QString QWhere::sql(QSqlDriver *driver) const
{
    return d->sql(driver);
}

void QWhere::bindValues(QVariantList &values) const
{
    d->bindValues(values);
}

/*
 * QFInWhere
 */

class QFInWherePrivate : public QWherePrivate
{
    public:
        QFInWherePrivate(const QField &left, const QVariantList &right);
        ~QFInWherePrivate();

        QString sql(QSqlDriver *driver) const;
        void bindValues(QVariantList &values) const;

    private:
        QField _f;
        QVariantList _list;
};

QFInWherePrivate::QFInWherePrivate(const QField &left, const QVariantList &right)
: QWherePrivate(QWhere::In), _f(left), _list(right)
{
}

QFInWherePrivate::~QFInWherePrivate()
{
}

QString QFInWherePrivate::sql(QSqlDriver *driver) const
{
    QString rs(fieldName(_f, driver));

    rs += QWhere::conditionStr(QWhere::In) + QLatin1String("(");

    for (int i=0; i<_list.count(); ++i)
    {
        if (i != 0)
            rs += QLatin1String(", ");

        rs += '?';
    }

    rs += QLatin1String(")");

    return rs;
}

void QFInWherePrivate::bindValues(QVariantList &values) const
{
    values << _list;
}

QFInWhere::QFInWhere(const QField &left, const QVariantList &right)
: QWhere(new QFInWherePrivate(left, right))
{
}

/*
 * QFLikeWhere
 */
class QFLikeWherePrivate : public QWherePrivate
{
    public:
        QFLikeWherePrivate(const QField &left, const QString &right);
        ~QFLikeWherePrivate();

        QString sql(QSqlDriver *driver) const;
        void bindValues(QVariantList &values) const;

    private:
        QField _f;
        QString _pattern;
};

QFLikeWherePrivate::QFLikeWherePrivate(const QField &left, const QString &right)
: QWherePrivate(QWhere::Like), _f(left), _pattern(right)
{
}

QFLikeWherePrivate::~QFLikeWherePrivate()
{
}

QString QFLikeWherePrivate::sql(QSqlDriver *driver) const
{
    QString rs(fieldName(_f, driver));

    rs += QWhere::conditionStr(QWhere::Like) + QLatin1String("?");

    return rs;
}

void QFLikeWherePrivate::bindValues(QVariantList &values) const
{
    values.append(_pattern);
}

QFLikeWhere::QFLikeWhere(const QField &left, const QString &right)
: QWhere(new QFLikeWherePrivate(left, right))
{
}

/*
 * QFDivWhere
 */

class QFDivWherePrivate : public QWherePrivate
{
    public:
        QFDivWherePrivate(const QField &left, int divisor, int offset);
        ~QFDivWherePrivate();

        QString sql(QSqlDriver *driver) const;
        void bindValues(QVariantList &values) const;

    private:
        QField _f;
        int _divisor;
        int _offset;
};

QFDivWherePrivate::QFDivWherePrivate(const QField &left, int divisor, int offset)
: QWherePrivate(QWhere::Like), _f(left), _divisor(divisor), _offset(offset)
{
}

QFDivWherePrivate::~QFDivWherePrivate()
{
}

QString QFDivWherePrivate::sql(QSqlDriver *driver) const
{
    return QString("((%1 + ?) % ? = 0)").arg(fieldName(_f, driver));
}

void QFDivWherePrivate::bindValues(QVariantList &values) const
{
    values.append(_offset);
    values.append(_divisor);
}

QFDivWhere::QFDivWhere(const QField &left, int divisor, int offset)
: QWhere(new QFDivWherePrivate(left, divisor, offset))
{
}

/*
 * QFFlagSetWhere
 */
class QFFlagSetWherePrivate : public QWherePrivate
{
    public:
        QFFlagSetWherePrivate(const QField &left, int flag);
        ~QFFlagSetWherePrivate();

        QString sql(QSqlDriver *driver) const;
        void bindValues(QVariantList &values) const;

    private:
        QField _f;
        int _flag;
};

QFFlagSetWherePrivate::QFFlagSetWherePrivate(const QField &left, int flag)
: QWherePrivate(QWhere::Like), _f(left), _flag(flag)
{
}

QFFlagSetWherePrivate::~QFFlagSetWherePrivate()
{
}

QString QFFlagSetWherePrivate::sql(QSqlDriver *driver) const
{
    return QString("((%1 & ?) != 0)").arg(fieldName(_f, driver));
}

void QFFlagSetWherePrivate::bindValues(QVariantList &values) const
{
    values.append(_flag);
}

QFFlagSetWhere::QFFlagSetWhere(const QField &left, int flag)
: QWhere(new QFFlagSetWherePrivate(left, flag))
{
}

/*
 * QFNullWhere
 */
class QFNullWherePrivate : public QWherePrivate
{
    public:
        QFNullWherePrivate(const QField &left);
        ~QFNullWherePrivate();

        QString sql(QSqlDriver *driver) const;
        void bindValues(QVariantList &values) const;

    private:
        QField _f;
};

QFNullWherePrivate::QFNullWherePrivate(const QField &left)
: QWherePrivate(QWhere::Like), _f(left)
{
}

QFNullWherePrivate::~QFNullWherePrivate()
{
}

QString QFNullWherePrivate::sql(QSqlDriver *driver) const
{
    return QString("%1 IS NULL").arg(fieldName(_f, driver));
}

void QFNullWherePrivate::bindValues(QVariantList &values) const
{
    (void) values;
}

QFNullWhere::QFNullWhere(const QField &left)
: QWhere(new QFNullWherePrivate(left))
{
}

/*
 * QFIWhere
 */

class QFIWherePrivate : public QWherePrivate
{
    public:
        QFIWherePrivate(const QField &left, const QVariant &right, QWhere::Condition cond);
        ~QFIWherePrivate();

        QString sql(QSqlDriver *driver) const;
        void bindValues(QVariantList &values) const;

    private:
        QField _f;
        QVariant _value;
};

QFIWherePrivate::QFIWherePrivate(const QField &left, const QVariant &right, QWhere::Condition cond)
: QWherePrivate(cond), _f(left), _value(right)
{
}

QFIWherePrivate::~QFIWherePrivate()
{
}

QString QFIWherePrivate::sql(QSqlDriver *driver) const
{
    QString rs(fieldName(_f, driver));

    rs += QWhere::conditionStr(condition());
    rs += QLatin1String("?");

    return rs;
}

void QFIWherePrivate::bindValues(QVariantList &values) const
{
    values.append(_value);
}

QFIWhere::QFIWhere(const QField &left, const QVariant &right, Condition cond)
: QWhere(new QFIWherePrivate(left, right, cond))
{
}

/*
 * QFFWhere
 */

class QFFWherePrivate : public QWherePrivate
{
    public:
        QFFWherePrivate(const QField &left, const QField &right, QWhere::Condition cond);
        ~QFFWherePrivate();

        QString sql(QSqlDriver *driver) const;
        void bindValues(QVariantList &values) const;

    private:
        QField _left;
        QField _right;
};

QFFWherePrivate::QFFWherePrivate(const QField &left, const QField &right, QWhere::Condition cond)
: QWherePrivate(cond), _left(left), _right(right)
{
}

QFFWherePrivate::~QFFWherePrivate()
{
}

QString QFFWherePrivate::sql(QSqlDriver *driver) const
{
    QString rs(fieldName(_left, driver));

    rs += QWhere::conditionStr(condition());
    rs += fieldName(_right, driver);

    return rs;
}

void QFFWherePrivate::bindValues(QVariantList &values) const
{
    (void) values;
    return;
}

QFFWhere::QFFWhere(const QField &left, const QField &right, Condition cond)
: QWhere(new QFFWherePrivate(left, right, cond))
{
}

/*
 * QWWWhere
 */

class QWWWherePrivate : public QWherePrivate
{
    public:
        QWWWherePrivate(const QWhere &left, const QWhere &right, QWhere::Condition cond);
        ~QWWWherePrivate();

        QString sql(QSqlDriver *driver) const;
        void bindValues(QVariantList &values) const;

    private:
        QWhere _left;
        QWhere _right;
};

QWWWherePrivate::QWWWherePrivate(const QWhere &left, const QWhere &right, QWhere::Condition cond)
: QWherePrivate(cond), _left(left), _right(right)
{
}

QWWWherePrivate::~QWWWherePrivate()
{
}

QString QWWWherePrivate::sql(QSqlDriver *driver) const
{
    QString rs('(');

    rs += _left.sql(driver);
    rs += ')';
    rs += QWhere::conditionStr(condition());
    rs += '(';
    rs += _right.sql(driver);
    rs += ')';

    return rs;
}

void QWWWherePrivate::bindValues(QVariantList &values) const
{
    _left.bindValues(values);
    _right.bindValues(values);
}

QWWWhere::QWWWhere(const QWhere &left, const QWhere &right, Condition cond)
: QWhere(new QWWWherePrivate(left, right, cond))
{
}

/*
 * QWWhere
 */

class QWWherePrivate : public QWherePrivate
{
    public:
        QWWherePrivate(const QWhere &w, QWhere::Condition cond);
        ~QWWherePrivate();

        QString sql(QSqlDriver *driver) const;
        void bindValues(QVariantList &values) const;

    private:
        QWhere _w;
};

QWWherePrivate::QWWherePrivate(const QWhere &w, QWhere::Condition cond)
: QWherePrivate(cond), _w(w)
{
}

QWWherePrivate::~QWWherePrivate()
{
}

QString QWWherePrivate::sql(QSqlDriver *driver) const
{
    QString rs(QWhere::conditionStr(condition()));

    rs += '(';
    rs += _w.sql(driver);
    rs += ')';

    return rs;
}

void QWWherePrivate::bindValues(QVariantList &values) const
{
    _w.bindValues(values);
}

QWWhere::QWWhere(const QWhere &w, Condition cond)
: QWhere(new QWWherePrivate(w, cond))
{
}

/*
 * QFWhere
 */

class QFWherePrivate : public QWherePrivate
{
    public:
        QFWherePrivate(const QField &f, QWhere::Condition cond);
        ~QFWherePrivate();

        QString sql(QSqlDriver *driver) const;
        void bindValues(QVariantList &values) const;

    private:
        QField _f;
};

QFWherePrivate::QFWherePrivate(const QField &f, QWhere::Condition cond)
: QWherePrivate(cond), _f(f)
{
}

QFWherePrivate::~QFWherePrivate()
{
}

QString QFWherePrivate::sql(QSqlDriver *driver) const
{
    QString rs(fieldName(_f, driver));

    rs += QWhere::conditionStr(condition());

    return rs;
}

void QFWherePrivate::bindValues(QVariantList &values) const
{
    (void) values;
    return;
}

QFWhere::QFWhere(const QField &f, Condition cond)
: QWhere(new QFWherePrivate(f, cond))
{
}
