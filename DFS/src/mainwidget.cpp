#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>

#include "mainwidget.hpp"

namespace SHIZ{
	MainWidget::MainWidget(NetworkManager* manager, QWidget* parent):
		networkManager(manager), QWidget(parent)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);


		fileListWidget = new QListWidget(this);
		layout->addWidget(fileListWidget);

		refreshButton = new QPushButton("Refresh File List", this);
		layout->addWidget(refreshButton);

		uploadButton = new QPushButton("Upload File", this);
		layout->addWidget(uploadButton);

		downloadButton = new QPushButton("Download File", this);
		layout->addWidget(downloadButton);


		connect(refreshButton, &QPushButton::clicked, this, &MainWidget::onRefreshButtonClicked);
		connect(uploadButton, &QPushButton::clicked, this, &MainWidget::onUploadButtonClicked);
		connect(downloadButton, &QPushButton::clicked, this, &MainWidget::onDownloadButtonClicked);
	}


	void MainWidget::setCurrentLogin(const QString& login){
		currentLogin = login;
	}


	void MainWidget::onDownloadButtonClicked(){
		QListWidgetItem* selectedItem = fileListWidget->currentItem();
		if (selectedItem) {
			QString fileName = selectedItem->text().split(" | ").first();
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
		fileListWidget->clear();
		fileListWidget->addItems(files);
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
}
