#pragma once

#include <QWidget>
#include <QSqlDatabase>
#include <QTableView>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QSqlQueryModel>
#include <QFileDialog>
#include <QFile>
#include <QDateTime>
#include<QLineEdit>

class FileManager : public QWidget {
	Q_OBJECT

	private:
		QSqlDatabase db;
          QPushButton* addButton;
          QPushButton* removeButton;
		QPushButton* downloadButton;
		QTableView* tableView;
		QSqlQueryModel* model;
          QLineEdit* findPanel;

	public:
		explicit FileManager(QWidget *parent = nullptr);
		~FileManager();

	private slots:
          void addFile();
          void downloadFile();
          void findFiles();
          void removeFile();

	private:
		void setupUI();
		void setupDatabase();
		void updateTable();
};
