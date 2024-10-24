#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include <QString>
#include <iostream>
#include <QSqlQueryModel>

#include "filemanager.hpp"

FileManager::FileManager(QWidget *parent) : QWidget(parent) {
	setupUI();
	setupDatabase();
	updateTable();
}

FileManager::~FileManager() {
	db.close();
}

void FileManager::setupUI() {
	addButton = new QPushButton("Add File", this);
	connect(addButton, &QPushButton::clicked, this, &FileManager::addFile);

	downloadButton = new QPushButton("Download File", this);
	connect(downloadButton, &QPushButton::clicked, this, &FileManager::downloadFile);

     removeButton = new QPushButton("Remove File", this);
     connect(removeButton, &QPushButton::clicked, this, &FileManager::removeFile);

	tableView = new QTableView(this);
	model = new QSqlQueryModel(this);
	tableView->setModel(model);

     findPanel = new QLineEdit(this);
     findPanel->setPlaceholderText("Введите название файла");
     // QSortFilterSqlQueryModel *model1 = new QSortFilterSqlQueryModel(model);
     connect(findPanel, &QLineEdit::textChanged, this, &FileManager::findFiles);
     // connect(findPanel, SIGNAL (textChanged(QString)), model1, SLOT (filter(QString)));




	QVBoxLayout *layout = new QVBoxLayout(this);

     layout->addWidget(findPanel);
	layout->addWidget(addButton);
     layout->addWidget(removeButton);
	layout->addWidget(downloadButton);
	layout->addWidget(tableView);

	setLayout(layout);
	setWindowTitle("File Manager");
	resize(500, 400);
}

void FileManager::setupDatabase() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("files.db");

	if (!db.open()) {
		qDebug() << "Error: connection with database failed";
		return;
	}

	QSqlQuery query;
	QString createTableQuery = "CREATE TABLE IF NOT EXISTS files ("
							   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
							   "file_name TEXT,"
							   "file_size INTEGER,"
							   "upload_date TEXT,"
							   "file_data BLOB)";
	if (!query.exec(createTableQuery)) {
		qDebug() << "Error creating table:" << query.lastError().text();
	}
}

void FileManager::addFile() {
	QString filePath = QFileDialog::getOpenFileName(this, "Select File", "", "All Files (*)");
	if (filePath.isEmpty()) {
		QMessageBox::warning(this, "File Selection", "No file selected.");
		return;
	}

	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly)) {
		QMessageBox::warning(this, "File Error", "Could not open file.");
		return;
	}

	QByteArray fileData = file.readAll();
	QString fileName = QFileInfo(filePath).fileName();
	qint64 fileSize = file.size();
	QString uploadDate = QDateTime::currentDateTime().toString(Qt::ISODate);

	file.close();

    QSqlQuery query;
     query.prepare("INSERT INTO files (file_name, file_size, upload_date, file_data) "
				  "VALUES (:file_name, :file_size, :upload_date, :file_data)");
	query.bindValue(":file_name", fileName);
	query.bindValue(":file_size", fileSize);
	query.bindValue(":upload_date", uploadDate);
	query.bindValue(":file_data", fileData);

	if (!query.exec()) {
		qDebug() << "Error inserting into table:" << query.lastError().text();
	} else {
		updateTable();
	}
}

void FileManager::downloadFile() {
	QModelIndex index = tableView->currentIndex();
	if (!index.isValid()) {
		QMessageBox::warning(this, "Selection Error", "Please select a file to download.");
		return;
	}

	int row = index.row();
	int fileId = model->data(model->index(row, 0)).toInt();

	QSqlQuery query;
	query.prepare("SELECT file_name, file_data FROM files WHERE id = :id");
	query.bindValue(":id", fileId);

	if (!query.exec()) {
		qDebug() << "Error retrieving file:" << query.lastError().text();
		return;
	}

	if (query.next()) {
		QString fileName = query.value(0).toString();
		QByteArray fileData = query.value(1).toByteArray();

		QString savePath = QFileDialog::getSaveFileName(this, "Save File", fileName);
		if (savePath.isEmpty()) {
			return;
		}

		QFile file(savePath);
		if (!file.open(QIODevice::WriteOnly)) {
			QMessageBox::warning(this, "File Error", "Could not save file.");
			return;
		}

		file.write(fileData);
		file.close();

		QMessageBox::information(this, "Success", "File downloaded successfully.");
	}
}

void FileManager::removeFile(){
    QModelIndex index = tableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Selection Error", "Please select a file to remove.");
        return;
    }

    int row = index.row();
    int fileId = model->data(model->index(row, 0)).toInt();

    QSqlQuery query;
    query.prepare("DELETE FROM files WHERE id = :id");
    query.bindValue(":id", fileId);

    if (!query.exec()) {
        qDebug() << "Error remove of file:" << query.lastError().text();
        return;
    }else{
        updateTable();
    }

    QMessageBox::information(this, "Success", "File removed successfully.");
}
void FileManager::findFiles(){
    QSqlQuery query;

    QString fileName = findPanel->text();
    if(fileName != ""){
        query.prepare("SELECT id, file_name, file_size, upload_date FROM files WHERE file_name='name.txt' ");


        if (!query.exec()) {
            qDebug() << "Error find of file:" << query.lastError().text();
            return;
        }
        // query.bindValue(":file_name", fileName);
        //fileName.toStdString().data()
        // query.addBindValue("n");
        // std::cout << fileName<< "\n";
        // model->setQuery("SELECT id, file_name, file_size, upload_date FROM files WHERE file_name='name.txt'");
        // model->setQuery("SELECT id, file_name, file_size, upload_date FROM files");
        // model->setQuery(query);
    }else{
        model->setQuery("SELECT id, file_name, file_size, upload_date FROM files");
    }
    // fileName.data()
    // query.bindValue(":file","name");

    // model->setQuery("SELECT id, file_name, file_size, upload_date FROM files WHERE file_name ");
}

void FileManager::updateTable() {
	model->setQuery("SELECT id, file_name, file_size, upload_date FROM files");
}
