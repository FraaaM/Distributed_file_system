#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>

#include "connectionwidget.hpp"

namespace SHIZ {
	ConnectionWidget::ConnectionWidget(Logger* logger, NetworkManager* manager, QWidget* parent)
		: logger(logger), networkManager(manager), QWidget(parent)
	{
		QVBoxLayout *layout = new QVBoxLayout(this);

		QLabel* apilabel = new QLabel("Enter the server address: ", this);
		layout->addWidget(apilabel);

		hostInput = new QLineEdit(this);
		hostInput->setPlaceholderText("127.0.0.1");
		hostInput->setText("127.0.0.1");
		layout->addWidget(hostInput);

		portInput = new QLineEdit(this);
		portInput->setPlaceholderText("1234");
		portInput->setText("1234");
		layout->addWidget(portInput);

		enterButton = new QPushButton("Connect", this);
		layout->addWidget(enterButton);
		connect(enterButton, &QPushButton::clicked, this, &ConnectionWidget::onEnterButtonClicked);

		quitButton = new QPushButton("Quit", this);
		layout->addWidget(quitButton);
		connect(quitButton, &QPushButton::clicked, QCoreApplication::instance(), &QApplication::quit);

		resize(400,300);
		setLayout(layout);
	}

	void ConnectionWidget::onEnterButtonClicked() {
		QString host = hostInput->text();
		bool ok;
		quint16 port = portInput->text().toUShort(&ok);

		if (!host.isEmpty() && ok) {
			bool success = networkManager->connectToHost(host, port);

			if (success) {
				networkManager->setHostAndPort(host, port);
				emit ConnectionSuccessful(host, port);
			} else {
				logger->log("Failed to connect to server");
			}
		} else {
			logger->log("Invalid host or port");
		}
	}
}
