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

            bool ACCESS_WRITE_SELF = false;
            bool ACCESS_READ_SELF = false;
            bool ACCESS_DELETE_SELF = false;

            bool ACCESS_WRITE_GROUP = false;
            bool ACCESS_READ_GROUP = false;
            bool ACCESS_DELETE_GROUP = false;

            bool ACCESS_WRITE_ANOTHER = false;
            bool ACCESS_READ_ANOTHER = false;
            bool ACCESS_DELETE_ANOTHER = false;


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

		signals:
			void showLoginWindow();

        public slots:
            void setRights();

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
