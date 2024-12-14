#pragma once

#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

#include "logger.hpp"
#include "networkmanager.hpp"

namespace SHIZ{
    class AdminWidget : public QWidget{
        Q_OBJECT

    private:
        NetworkManager* networkManager;
        QString currentLogin;

        bool userChange = true;

        QLabel* statusLabel;
        QLineEdit* filterLineEdit;
        QTableWidget* userTableWidget;
        QPushButton* refreshButton;
        QPushButton* deleteButton;
        QPushButton* logoutButton;

        Logger* logger;

    public:
        AdminWidget(Logger* logger, NetworkManager* manager, QWidget* parent = nullptr);

        void setCurrentLogin(const QString& login);

    signals:
        void showLoginWindow();

	private slots:
		void onCellChanged(int row, int column);
        void onDeleteButtonClicked();
        void onFilterTextChanged(const QString& text);
        void onLogoutButtonClicked();
        void onRefreshButtonClicked();
		void onStatusMessageReceived(const QString& message);
    };
}
