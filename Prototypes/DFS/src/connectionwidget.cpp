#include <QLabel>
#include <QVBoxLayout>

#include "connectionwidget.hpp"

namespace SHIZ {

	ConnectionWidget::ConnectionWidget(NetworkManager* manager, QWidget* parent)
		: networkManager(manager), QWidget(parent)
	{
		QVBoxLayout *layout = new QVBoxLayout(this);

		QLabel* apilabel = new QLabel("Enter the server address: ", this);
		layout->addWidget(apilabel);

		hostInput = new QLineEdit(this);
		hostInput->setPlaceholderText("127.0.0.1");
		hostInput->setText("127.0.0.1");
		layout->addWidget(hostInput);

		enterButton = new QPushButton("Enter", this);
		layout->addWidget(enterButton);
		connect(enterButton, &QPushButton::clicked, this, &ConnectionWidget::onEnterButtonClicked);

		resize(400,300);
		setLayout(layout);
	}

	void ConnectionWidget::onEnterButtonClicked() {
		if (!hostInput->text().isEmpty()) {
			bool success = networkManager->sendConnectionRequest(hostInput->text());

			if (success) {
				networkManager->setHost(hostInput->text());
				emit ConnectionSuccessful(hostInput->text());
			} else {
				qDebug() << "Failed to connect to server";
			}
		}
	}
}
