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

			void setCurrentLogin(const QString& login);

		private:
			void setRights();

		signals:
			void showLoginWindow();
            void userBanned();

        private slots:
			void onDeleteButtonClicked();
			void onDownloadButtonClicked();
			void onFilterTextChanged(const QString& text);
			void onLogoutButtonClicked();
			void onRefreshButtonClicked();
			void onStatusMessageReceived(const QString& message);
			void onUploadButtonClicked();
	};
}
