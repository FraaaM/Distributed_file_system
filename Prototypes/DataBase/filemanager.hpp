#pragma once

#include <QWidget>
#include <QSqlDatabase>
#include <QTableView>
#include <QPushButton>
#include <QLineEdit>
#include <QSqlQueryModel>

#include "databasemanager.hpp"

class FileManager : public QWidget {
	Q_OBJECT

	private:
        DatabaseManager* db;
        QPushButton* addButton;
        QPushButton* removeButton;
        QPushButton* refreshDataButton;
        QPushButton* downloadButton;
        QTableView* tableView;
        QSqlQueryModel* model;
        QLineEdit* findPanel;


	public:
        FileManager(DatabaseManager* manager, QWidget *parent = nullptr);

	private slots:
        void addFile();
        void downloadFile();
        void findFiles();
        void removeFile();

	private:
        void setupUI();
        void updateTable();
};
