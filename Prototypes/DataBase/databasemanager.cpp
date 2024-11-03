#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>

#include "databasemanager.hpp"
#include "database.hpp"


DatabaseManager::DatabaseManager(QObject* parent) : QObject(parent) {
    QDir dir = QCoreApplication::applicationDirPath();
    QString dbPath = dir.absoluteFilePath("database.db");

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qDebug() << "Error: connection with database failed";
    }

    QSqlQuery query;
    QString createTableQuery = "CREATE TABLE IF NOT EXISTS " FILES_TABLE " ("
        ID " INTEGER PRIMARY KEY AUTOINCREMENT,"
        FILE_NAME " TEXT,"
        FILE_SIZE " INTEGER,"
        UPLOAD_DATE " TEXT,"
        FILE_PATH " TEXT,"
        FILE_IS_PRIVATE " BOOL)";
    if (!query.exec(createTableQuery)) {
        qDebug() << "Error creating table:" << query.lastError().text();
    }

    createTableQuery = "CREATE TABLE IF NOT EXISTS " USERS_TABLE " ("
        ID " INTEGER PRIMARY KEY AUTOINCREMENT,"
        LOGIN " TEXT UNIQUE,"
        PASSWORD " TEXT,"
        TEAM " INTEGER,"
        STATUS " TEXT,"
        RIGHT " TEXT)";
    if (!query.exec(createTableQuery)) {
        qDebug() << "Error creating table:";
    }

}

DatabaseManager::~DatabaseManager() {
    db.close();
}

bool DatabaseManager::execQuery(const QString& query) {
    QSqlQuery q;
    if (!q.exec(query)) {
        qDebug() << "Error executing query:" << q.lastError().text();
        return false;
    }
    return true;
}

QSqlQuery DatabaseManager::execPreparedQuery(const QString& query, const QVariantList& values) {
    QSqlQuery q;
    q.prepare(query);
    for (const auto& value : values) {
        q.addBindValue(value);
    }
    if (!q.exec()) {
        qDebug() << "Error executing prepared query:" << q.lastError().text();
    }
    return q;
}

