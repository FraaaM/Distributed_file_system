#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHeaderView>
#include <iostream>

#include "mainwidget.hpp"

namespace SHIZ{
	MainWidget::MainWidget(Logger* logger, NetworkManager* manager, QWidget* parent)
		: logger(logger), networkManager(manager), QWidget(parent)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);

		statusLabel = new QLabel(this);
		layout->addWidget(statusLabel);
		connect(networkManager, &NetworkManager::statusMessage, this, &MainWidget::onStatusMessageReceived);

		filterLineEdit = new QLineEdit(this);
		filterLineEdit->setPlaceholderText("Filter by file name");
		layout->addWidget(filterLineEdit);
		connect(filterLineEdit, &QLineEdit::textChanged, this, &MainWidget::onFilterTextChanged);

		fileTableWidget = new QTableWidget(this);
        fileTableWidget->setColumnCount(5);
        fileTableWidget->setHorizontalHeaderLabels({"File Name", "Owner", "Size (bytes)", "Upload Date", "Group"});
		fileTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
		fileTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
		fileTableWidget->setSortingEnabled(true);
		layout->addWidget(fileTableWidget);

		refreshButton = new QPushButton("Refresh File List", this);
		layout->addWidget(refreshButton);

		uploadButton = new QPushButton("Upload File", this);
		layout->addWidget(uploadButton);

		downloadButton = new QPushButton("Download File", this);
		layout->addWidget(downloadButton);

		deleteButton = new QPushButton("Delete File", this);
		layout->addWidget(deleteButton);

		logoutButton = new QPushButton("Logout", this);
		layout->addWidget(logoutButton);

		connect(refreshButton, &QPushButton::clicked, this, &MainWidget::onRefreshButtonClicked);
		connect(uploadButton, &QPushButton::clicked, this, &MainWidget::onUploadButtonClicked);
		connect(downloadButton, &QPushButton::clicked, this, &MainWidget::onDownloadButtonClicked);
		connect(deleteButton, &QPushButton::clicked, this, &MainWidget::onDeleteButtonClicked);
		connect(logoutButton, &QPushButton::clicked, this, &MainWidget::onLogoutButtonClicked);
	}


	void MainWidget::setCurrentLogin(const QString& login){
		currentLogin = login;
	}


	void MainWidget::onDeleteButtonClicked() {
		int selectedRow = fileTableWidget->currentRow();
		if (selectedRow >= 0) {
            QString fileName = fileTableWidget->item(selectedRow, 0)->text();
            QString groupOfFile = networkManager->getFileInfo(fileName);
            QString groupOfUser = networkManager->getUserInfo(currentLogin).split("|")[1];

            if((groupOfFile == groupOfUser && ACCESS_DELETE_GROUP) || (groupOfFile != groupOfUser && ACCESS_DELETE_ANOTHER)){
                bool success = networkManager->deleteFile(fileName);
                if (success) {
                    QMessageBox::information(this, "Delete", "File deleted successfully.");
                    onRefreshButtonClicked();
                } else {
                    QMessageBox::warning(this, "Delete", "File deletion failed.");
                }
            }else{
                QMessageBox::warning(this, "Delete", "You don't have right delete this file.");
            }

		} else {
			QMessageBox::warning(this, "Delete", "No file selected.");
		}
	}

	void MainWidget::onDownloadButtonClicked(){
		int selectedRow = fileTableWidget->currentRow();

		if (selectedRow >= 0) {
            QString fileName = fileTableWidget->item(selectedRow, 0)->text();
            QString groupOfFile = networkManager->getFileInfo(fileName);
            QString groupOfUser = networkManager->getUserInfo(currentLogin).split("|")[1];

            if((groupOfFile == groupOfUser && ACCESS_READ_GROUP) || (groupOfFile != groupOfUser && ACCESS_READ_ANOTHER)){
                QString directory = QFileDialog::getExistingDirectory(this, "Select Download Folder");

                if (!directory.isEmpty()) {
                    QString filePath = directory + "/" + fileName;
                    bool success = networkManager->downloadFile(filePath);
                    if (success) {
                        QMessageBox::information(this, "Download", "File downloaded successfully.");
                    } else {
                        QMessageBox::warning(this, "Download", "File download failed.");
                    }
                }
            }else{
                QMessageBox::warning(this, "Download", "You don't have right download this file.");
            }
		} else {
			QMessageBox::warning(this, "Download", "No file selected.");
		}
	}

	void MainWidget::onFilterTextChanged(const QString& text) {
		for (int i = 0; i < fileTableWidget->rowCount(); ++i) {
			bool match = fileTableWidget->item(i, 0)->text().contains(text, Qt::CaseInsensitive);
			fileTableWidget->setRowHidden(i, !match);
		}
	}

	void MainWidget::onLogoutButtonClicked() {
		fileTableWidget->setRowCount(0);

		emit showLoginWindow();
	}

	void MainWidget::onRefreshButtonClicked(){
        QStringList files = networkManager->requestFileList(currentLogin);

		fileTableWidget->setRowCount(files.size());
		for (int i = 0; i < files.size(); ++i) {
			QStringList fileInfo = files[i].split("|");
            if (fileInfo.size() == 5) {
				QTableWidgetItem* fileNameItem = new QTableWidgetItem(fileInfo[0]);
				QTableWidgetItem* ownerItem = new QTableWidgetItem(fileInfo[1]);
				QTableWidgetItem* sizeItem = new QTableWidgetItem(fileInfo[2]);
                QTableWidgetItem* dateItem = new QTableWidgetItem(fileInfo[3]);
                QTableWidgetItem* groupItem = new QTableWidgetItem(fileInfo[4]);

				fileTableWidget->setItem(i, 0, fileNameItem);
				fileTableWidget->setItem(i, 1, ownerItem);
				fileTableWidget->setItem(i, 2, sizeItem);
				fileTableWidget->setItem(i, 3, dateItem);
                fileTableWidget->setItem(i, 4, groupItem);
			}
		}
	}

	void MainWidget::onStatusMessageReceived(const QString& message) {
		statusLabel->setText(message);
	}

	void MainWidget::onUploadButtonClicked(){
        if(!ACCESS_WRITE_SELF){
            QMessageBox::warning(this, "Upload", "You don't have right upload files.");
            return;
        }
		QString filePath = QFileDialog::getOpenFileName(this, "Select a file to upload");
		if (filePath.isEmpty()) return;
		QString fileName = QFileInfo(filePath).fileName();

		onRefreshButtonClicked();
		bool fileExists = false;
		for (int i = 0; i < fileTableWidget->rowCount(); ++i) {
			if (fileTableWidget->item(i, 0)->text() == fileName) {
				fileExists = true;
				break;
			}
		}

		if (fileExists) {
			int response = QMessageBox::question(this, "File Exists",
												 "A file with this name already exists. Do you want to replace it?",
												 QMessageBox::Yes | QMessageBox::No);
			if (response == QMessageBox::No) {
				return;
			}
		}

		bool success = networkManager->uploadFile(filePath, currentLogin);
		if (success) {
			QMessageBox::information(this, "Upload", "File uploaded successfully.");
			onRefreshButtonClicked();
		} else {
			QMessageBox::warning(this, "Upload", "File upload failed.");
		}
	}
    void MainWidget::setRights(){
        std::string rights = networkManager->getUserInfo(currentLogin).split("|")[0].toStdString();

        for(int i = 0; i < 3; i++){
            int right = rights[i] - 48;

            if(right == 0){
                continue;
            }

            if(i == 0){
                ACCESS_READ_SELF = right & 4;
                ACCESS_WRITE_SELF = right & 2;
                ACCESS_DELETE_SELF = right & 1;
            }else if(i == 1){
                ACCESS_READ_GROUP = right & 4;
                ACCESS_WRITE_GROUP = right & 2;
                ACCESS_DELETE_GROUP = right & 1;
            }else if(i == 2){
                ACCESS_READ_ANOTHER = right & 4;
                ACCESS_WRITE_ANOTHER =  right & 2;
                ACCESS_DELETE_ANOTHER = right & 1;
            }
        }
    }
}
