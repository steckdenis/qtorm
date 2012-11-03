#include "qtormdatabase.h"

static bool per_thread_database = false;
__thread QSqlDatabase *thread_database = NULL;

QSqlDatabase QtOrmDatabase::threadDatabase()
{
    // One database per thread, to avoid conflicts between threads and thread-non-safety of QtSql
    if (per_thread_database && thread_database != NULL)
    {
        return *thread_database;
    }
    else
    {
        return QSqlDatabase::database();
    }
}

void QtOrmDatabase::setPerThreadDatabase(bool enable)
{
    per_thread_database = enable;
}

void QtOrmDatabase::setThreadDatabase(QSqlDatabase db)
{
    if (thread_database)
        delete thread_database;

    thread_database = new QSqlDatabase(db);
}

bool QtOrmDatabase::threadHasDatabase()
{
    return (!per_thread_database || thread_database != NULL);
}
