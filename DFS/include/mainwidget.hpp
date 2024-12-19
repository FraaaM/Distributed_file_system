#pragma once

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>

#include "networkmanager.hpp"

namespace SHIZ {
	class MainWidget : public QWidget{
		Q_OBJECT

		private:
			NetworkManager* networkManager;
			QString currentLogin;

			bool writeAccess = true;
			bool readAccess = true;
			bool deleteAccess = true;
			bool buttonLock = false;

			QLabel* statusLabel;
			QLineEdit* filterLineEdit;
			QTableWidget* fileTableWidget;
			QPushButton* refreshButton;
			QPushButton* uploadButton;
			QPushButton* downloadButton;
			QPushButton* deleteButton;
			QPushButton* logoutButton;

			Logger* logger;

		public:
			MainWidget(Logger* logger, NetworkManager* manager, QWidget* parent = nullptr);

			void setButtonLock(bool buttonLock);
			void setCurrentLogin(const QString& login);

		signals:
			void deleteFileRequest(const QString& fileName, const QString& userName);
			void downloadFileRequest(const QString& filePath, const QString& userName);
			void listFileRequest(const QString& userName);
			void showLoginWindow();
			void uploadFileRequest(const QString& filePath, const QString& userName);
			void userBanned();
			void userInfoRequest(const QString& userName);

		public slots:
			void onDeleteFileResult(const QString& result);
			void onDownloadFileResult(const QString& result);
			void onListFileResult(const QStringList& files);
			void onUploadFileResult(const QString& result);
			void onRefreshButtonClicked();

		private slots:
			void onDeleteButtonClicked();
			void onDownloadButtonClicked();
			void onFilterTextChanged(const QString& text);
			void onLogoutButtonClicked();
			void onStatusMessageReceived(const QString& message);
			void onUploadButtonClicked();
	};
}
