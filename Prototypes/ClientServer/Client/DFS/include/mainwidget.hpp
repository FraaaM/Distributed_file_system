#pragma once

#include <QTableWidget>
#include <QPushButton>

#include "networkmanager.hpp"

namespace SHIZ{
	class MainWidget : public QWidget{
		Q_OBJECT

		private:
			NetworkManager* networkManager;
			QString currentLogin;

			QTableWidget* fileTableWidget;
			QPushButton* refreshButton;
			QPushButton* uploadButton;
			QPushButton* downloadButton;
            QPushButton* logoutButton;

		public:
			MainWidget(NetworkManager* manager, QWidget* parent = nullptr);

			void setCurrentLogin(const QString& login);

		signals:
			void showLoginWindow();

		private slots:
			void onDownloadButtonClicked();
			void onRefreshButtonClicked();
			void onUploadButtonClicked();
            void onLogOutButtonClicked();
	};
}
