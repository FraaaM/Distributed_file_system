#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHeaderView>

#include "mainwidget.hpp"

namespace SHIZ{
	MainWidget::MainWidget(NetworkManager* manager, QWidget* parent):
		networkManager(manager), QWidget(parent)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);


		fileTableWidget = new QTableWidget(this);
		fileTableWidget->setColumnCount(4);
		fileTableWidget->setHorizontalHeaderLabels({"File Name", "Owner", "Size (bytes)", "Upload Date"});
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

		logoutButton = new QPushButton("Log Out", this);
        layout->addWidget(logoutButton);


		connect(refreshButton, &QPushButton::clicked, this, &MainWidget::onRefreshButtonClicked);
		connect(uploadButton, &QPushButton::clicked, this, &MainWidget::onUploadButtonClicked);
		connect(downloadButton, &QPushButton::clicked, this, &MainWidget::onDownloadButtonClicked);
        connect(logoutButton, &QPushButton::clicked, this, &MainWidget::onLogOutButtonClicked);
	}


	void MainWidget::setCurrentLogin(const QString& login){
		currentLogin = login;
	}


	void MainWidget::onDownloadButtonClicked(){
		int selectedRow = fileTableWidget->currentRow();
		if (selectedRow >= 0) {
			QString fileName = fileTableWidget->item(selectedRow, 0)->text();
			bool success = networkManager->downloadFile(fileName);
			if (success) {
				QMessageBox::information(this, "Download", "File downloaded successfully.");
			} else {
				QMessageBox::warning(this, "Download", "File download failed.");
			}
		} else {
			QMessageBox::warning(this, "Download", "No file selected.");
		}
	}

	void MainWidget::onRefreshButtonClicked(){
		QStringList files = networkManager->requestFileList();

		fileTableWidget->setRowCount(files.size());
		for (int i = 0; i < files.size(); ++i) {
			QStringList fileInfo = files[i].split("|");
			if (fileInfo.size() == 4) {
				QTableWidgetItem* fileNameItem = new QTableWidgetItem(fileInfo[0]);
				QTableWidgetItem* ownerItem = new QTableWidgetItem(fileInfo[1]);
				QTableWidgetItem* sizeItem = new QTableWidgetItem(fileInfo[2]);
				QTableWidgetItem* dateItem = new QTableWidgetItem(fileInfo[3]);

				fileTableWidget->setItem(i, 0, fileNameItem);
				fileTableWidget->setItem(i, 1, ownerItem);
				fileTableWidget->setItem(i, 2, sizeItem);
				fileTableWidget->setItem(i, 3, dateItem);
			}
		}
	}

	void MainWidget::onUploadButtonClicked(){
		QString filePath = QFileDialog::getOpenFileName(this, "Select a file to upload");
		if (!filePath.isEmpty()) {
			bool success = networkManager->uploadFile(filePath, currentLogin);
			if (success) {
				QMessageBox::information(this, "Upload", "File uploaded successfully.");
				onRefreshButtonClicked();
			} else {
				QMessageBox::warning(this, "Upload", "File upload failed.");
			}
		}
	}

    void MainWidget::onLogOutButtonClicked() {
        fileTableWidget->clear();
        emit showLoginWindow();
    }

}
