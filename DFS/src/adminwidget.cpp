#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>

#include "adminwidget.hpp"
#include "macros.hpp"

namespace SHIZ {
	AdminWidget::AdminWidget(Logger* logger, NetworkManager* manager, QWidget* parent)
		: logger(logger), networkManager(manager), QWidget(parent)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);

		statusLabel = new QLabel(this);
		layout->addWidget(statusLabel);

		filterLineEdit = new QLineEdit(this);
		filterLineEdit->setPlaceholderText("Filter by username");
		layout->addWidget(filterLineEdit);
		connect(filterLineEdit, &QLineEdit::textChanged, this, &AdminWidget::onFilterTextChanged);

		userTableWidget = new QTableWidget(this);
		userTableWidget->setColumnCount(4);
		userTableWidget->setHorizontalHeaderLabels({"Username", "Is_admin", "Group", "Rights"});
		userTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
		userTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
		userTableWidget->setSortingEnabled(true);
		layout->addWidget(userTableWidget);
		connect(userTableWidget, &QTableWidget::cellChanged, this, &AdminWidget::onCellChanged);

		refreshButton = new QPushButton("Refresh User List", this);
		layout->addWidget(refreshButton);
		connect(refreshButton, &QPushButton::clicked, this, &AdminWidget::onRefreshButtonClicked);

		deleteButton = new QPushButton("Delete user", this);
		layout->addWidget(deleteButton);
		connect(deleteButton, &QPushButton::clicked, this, &AdminWidget::onDeleteButtonClicked);

		logoutButton = new QPushButton("Logout", this);
		layout->addWidget(logoutButton);
		connect(logoutButton, &QPushButton::clicked, this, &AdminWidget::onLogoutButtonClicked);

		connect(networkManager, &NetworkManager::deleteUserResult, this, &AdminWidget::onDeleteUserResult);
		connect(networkManager, &NetworkManager::statusMessage, this, &AdminWidget::onStatusMessageReceived);
		connect(networkManager, &NetworkManager::updateUserResult, this, &AdminWidget::onUpdateUserResult);
		connect(networkManager, &NetworkManager::userListResult, this, &AdminWidget::onUserListResult);
	}


	void AdminWidget::setCurrentLogin(const QString& login){
		currentLogin = login;
	}


	void AdminWidget::onDeleteUserResult(bool success) {
		if (success) {
			QMessageBox::information(this, "Delete", "User successfully deleted.");
			onRefreshButtonClicked();
		} else {
			QMessageBox::warning(this, "Delete", "The user could not be deleted.");
		}
	}

	void AdminWidget::onUpdateUserResult(bool success) {
		if (success) {
			QMessageBox::information(this, "Update", "User update successfully.");
			onRefreshButtonClicked();
		} else {
			QMessageBox::warning(this, "Update", "User update failed.");
		}
	}

	void AdminWidget::onUserListResult(const QStringList& users) {
		userTableWidget->setRowCount(users.size());
		userChange = false;
		for (int i = 0; i < users.size(); ++i) {
			QStringList userInfo = users[i].split("|");
			if (userInfo.size() == 4) {
				QTableWidgetItem* userNameItem = new QTableWidgetItem(userInfo[0]);
				QTableWidgetItem* ownerItem = new QTableWidgetItem(userInfo[1]);
				QTableWidgetItem* sizeItem = new QTableWidgetItem(userInfo[2]);
				QTableWidgetItem* dateItem = new QTableWidgetItem(userInfo[3]);

				userTableWidget->setItem(i, 0, userNameItem);
				userTableWidget->setItem(i, 1, ownerItem);
				userTableWidget->setItem(i, 2, sizeItem);
				userTableWidget->setItem(i, 3, dateItem);
			}
		}
		userChange = true;
	}

	void AdminWidget::onRefreshButtonClicked() {
		emit userListRequest();
	}


	void AdminWidget::onCellChanged(int row, int column) {
		QString userName = userTableWidget->item(row, 0)->text();
		QTableWidgetItem *item = userTableWidget->item(row, column);
		if (item && userChange) {
			QString value = item->text();
			if (column == 1) {
				emit updateUserRequest(userName, FIELD_USER_IS_ADMIN, value);
			} else if (column == 2) {
				emit updateUserRequest(userName, FIELD_USER_GROUP_ID, value);
			} else if (column == 3) {
				emit updateUserRequest(userName, FIELD_USER_RIGHTS, value);
			}
		}
	}

	void AdminWidget::onDeleteButtonClicked() {
		int selectedRow = userTableWidget->currentRow();
		if (selectedRow >= 0) {
			QString userName = userTableWidget->item(selectedRow, 0)->text();
			QString isAdmin = userTableWidget->item(selectedRow, 1)->text();
			if (isAdmin != QString("true")) {
				emit deleteUserRequest(userName);
			} else {
				QMessageBox::warning(this, "Delete", "It is an admin user.");
			}
		} else {
			QMessageBox::warning(this, "Delete", "No user selected.");
		}
	}

	void AdminWidget::onFilterTextChanged(const QString& text) {
		for (int i = 0; i < userTableWidget->rowCount(); ++i) {
			bool match = userTableWidget->item(i, 0)->text().contains(text, Qt::CaseInsensitive);
			userTableWidget->setRowHidden(i, !match);
		}
	}

	void AdminWidget::onLogoutButtonClicked() {
		userTableWidget->setRowCount(0);
		emit showLoginWindow();
	}

	void AdminWidget::onStatusMessageReceived(const QString& message) {
		statusLabel->setText(message);
	}
}
