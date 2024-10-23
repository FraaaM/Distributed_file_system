#pragma once

#include <QListWidget>
#include <QPushButton>

#include "networkmanager.hpp"

namespace SHIZ{
	class MainWidget : public QWidget{
		Q_OBJECT

		private:
			NetworkManager* networkManager;
			QString currentLogin;

			QListWidget* fileListWidget;
			QPushButton* refreshButton;
			QPushButton* uploadButton;
			QPushButton* downloadButton;

		public:
			MainWidget(NetworkManager* manager, QWidget* parent = nullptr);

			void setCurrentLogin(const QString& login);

		signals:
			void showLoginWindow();

		private slots:
			void onDownloadButtonClicked();
			void onRefreshButtonClicked();
			void onUploadButtonClicked();
	};
}
