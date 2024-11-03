#pragma once
#include <QString>
#include <QSqlQuery>
#include <QVariantList>

class DatabaseManager : public QObject {
    Q_OBJECT
    private:
        QSqlDatabase db;
    public:
        DatabaseManager(QObject* parent = nullptr);
        ~DatabaseManager();

        bool execQuery(const QString& query);
        QSqlQuery execPreparedQuery(const QString& query, const QVariantList& values);

};

