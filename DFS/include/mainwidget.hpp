#pragma once

#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

#include "networkmanager.hpp"

namespace SHIZ{
	class MainWidget : public QWidget{
		Q_OBJECT

		private:
			NetworkManager* networkManager;
			QString currentLogin;

			bool writeAccess = false;
			bool readAccess = false;
			bool deleteAccess = false;
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

		private:
			void setRights();

		signals:
			void deleteFileRequest(const QString& fileName);
			void downloadFileRequest(const QString& filePath);
			void listFileRequest(const QString& userName);
			void showLoginWindow();
			void uploadFileRequest(const QString& filePath, const QString& userName);
			void userBanned();
			void userInfoRequest(const QString& userName);

		public slots:
			void onDeleteFileResult(bool success);
			void onDownloadFileResult(bool success);
			void onListFileResult(const QStringList& files);
			void onUploadFileResult(bool success);
			void onUserInfoResult(const QString& userInfo);
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
