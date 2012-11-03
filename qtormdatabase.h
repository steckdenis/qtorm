#ifndef __QTORMDATABASE_H__
#define __QTORMDATABASE_H__

#include <QSqlDatabase>

class QtOrmDatabase
{
    public:
        static QSqlDatabase threadDatabase();

        static void setPerThreadDatabase(bool enable);
        static bool threadHasDatabase();
        static void setThreadDatabase(QSqlDatabase db);
};

#endif