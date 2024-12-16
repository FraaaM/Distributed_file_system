#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHeaderView>

#include "clientmacros.hpp"
#include "mainwidget.hpp"

namespace SHIZ {
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
		connect(refreshButton, &QPushButton::clicked, this, &MainWidget::onRefreshButtonClicked);

		uploadButton = new QPushButton("Upload File", this);
		layout->addWidget(uploadButton);
		connect(uploadButton, &QPushButton::clicked, this, &MainWidget::onUploadButtonClicked);

		downloadButton = new QPushButton("Download File", this);
		layout->addWidget(downloadButton);
		connect(downloadButton, &QPushButton::clicked, this, &MainWidget::onDownloadButtonClicked);

		deleteButton = new QPushButton("Delete File", this);
		layout->addWidget(deleteButton);
		connect(deleteButton, &QPushButton::clicked, this, &MainWidget::onDeleteButtonClicked);

		logoutButton = new QPushButton("Logout", this);
		layout->addWidget(logoutButton);
		connect(logoutButton, &QPushButton::clicked, this, &MainWidget::onLogoutButtonClicked);

		connect(networkManager, &NetworkManager::deleteFileResult, this, &MainWidget::onDeleteFileResult);
		connect(networkManager, &NetworkManager::downloadFileResult, this, &MainWidget::onDownloadFileResult);
		connect(networkManager, &NetworkManager::listFileResult, this, &MainWidget::onListFileResult);
		connect(networkManager, &NetworkManager::uploadFileResult, this, &MainWidget::onUploadFileResult);
		connect(networkManager, &NetworkManager::userInfoResult, this, &MainWidget::onUserInfoResult);
	}


	void MainWidget::setButtonLock(bool buttonLock) {
		refreshButton->setEnabled(!buttonLock);
		uploadButton->setEnabled(!buttonLock);
		downloadButton->setEnabled(!buttonLock);
		deleteButton->setEnabled(!buttonLock);
		logoutButton->setEnabled(!buttonLock);
	}

	void MainWidget::setCurrentLogin(const QString& login){
		currentLogin = login;
	}


	void MainWidget::setRights() {
		emit userInfoRequest(currentLogin);
	}


	void MainWidget::onDeleteFileResult(bool success) {
		setButtonLock(false);
		if (success) {
			QMessageBox::information(this, "Delete", "File deleted successfully.");
			onRefreshButtonClicked();
		} else {
			QMessageBox::warning(this, "Delete", "File deletion failed.");
		}
	}

	void MainWidget::onDownloadFileResult(bool success) {
		setButtonLock(false);
		if (success) {
			QMessageBox::information(this, "Downloading", "The file downloaded successfully.");
		} else {
			QMessageBox::warning(this, "Download", "The file could not be downloaded.");
		}
	}

	void MainWidget::onListFileResult(const QStringList& files) {
		if(files.size() > 0 && files[0] == RESPONSE_USER_DOES_NOT_EXIST){
			emit userBanned();
		}
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

	void MainWidget::onUploadFileResult(bool success) {
		setButtonLock(false);
		if (success) {
			QMessageBox::information(this, "Upload", "File uploaded successfully.");
			onRefreshButtonClicked();
		} else {
			QMessageBox::warning(this, "Upload", "File upload failed.");
		}
	}

	void MainWidget::onUserInfoResult(const QString &userInfo){
		QString rights = userInfo.split("|")[0];

		readAccess = rights.contains(RIGHT_TO_READ);
		writeAccess = rights.contains(RIGHT_TO_WRITE);
		deleteAccess = rights.contains(RIGHT_TO_DELETE);
	}

	void MainWidget::onRefreshButtonClicked(){
		setRights();
		emit listFileRequest(currentLogin);
	}


	void MainWidget::onDeleteButtonClicked() {
		onRefreshButtonClicked();
		setRights();

		int selectedRow = fileTableWidget->currentRow();
		if (selectedRow >= 0) {
			QString fileName = fileTableWidget->item(selectedRow, 0)->text();

			if(deleteAccess){
				setButtonLock(true);
				emit deleteFileRequest(fileName);
			} else {
				QMessageBox::warning(this, "Delete", "You don't have right delete this file.");
			}
		} else {
			QMessageBox::warning(this, "Delete", "No file selected.");
		}
	}


	void MainWidget::onDownloadButtonClicked(){
		onRefreshButtonClicked();
		setRights();

		int selectedRow = fileTableWidget->currentRow();
		if (selectedRow >= 0) {
			QString fileName = fileTableWidget->item(selectedRow, 0)->text();

			if(readAccess){
				QString directory = QFileDialog::getExistingDirectory(this, "Select Download Folder");

				if (!directory.isEmpty()) {
					QString filePath = directory + "/" + fileName;
					setButtonLock(true);
					emit downloadFileRequest(filePath);
				}
			} else {
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

	void MainWidget::onStatusMessageReceived(const QString& message) {
		statusLabel->setText(message);
	}

	void MainWidget::onUploadButtonClicked(){
		onRefreshButtonClicked();
		setRights();

		if(!writeAccess){
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

		setButtonLock(true);
		emit uploadFileRequest(filePath, currentLogin);
	}
}
