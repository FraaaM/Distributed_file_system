#pragma once

#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>

#include "networkmanager.hpp"

namespace SHIZ{
	class MainWidget : public QWidget{
		Q_OBJECT

		private:
			NetworkManager* networkManager;
			QString currentLogin;

			QLineEdit* filterLineEdit;
			QTableWidget* fileTableWidget;
			QPushButton* refreshButton;
			QPushButton* uploadButton;
			QPushButton* downloadButton;
			QPushButton* deleteButton;
			QPushButton* logoutButton;

		public:
			MainWidget(NetworkManager* manager, QWidget* parent = nullptr);

			void setCurrentLogin(const QString& login);

		signals:
			void showLoginWindow();

		private slots:
			void onDeleteButtonClicked();
			void onDownloadButtonClicked();
			void onFilterTextChanged(const QString& text);
			void onLogoutButtonClicked();
			void onRefreshButtonClicked();
			void onUploadButtonClicked();
	};
}
