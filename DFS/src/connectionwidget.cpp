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

		connectButton = new QPushButton("Connect", this);
		layout->addWidget(connectButton);
		connect(connectButton, &QPushButton::clicked, this, &ConnectionWidget::onConnectButtonClicked);

		quitButton = new QPushButton("Quit", this);
		layout->addWidget(quitButton);
		connect(quitButton, &QPushButton::clicked, QCoreApplication::instance(), &QApplication::quit);

		connect(networkManager, &NetworkManager::connectResult, this, &ConnectionWidget::onConnectResult);

		resize(400,300);
		setLayout(layout);
	}


	void ConnectionWidget::onConnectResult(bool success) {
		if (success) {
			emit connectSuccessful(hostInput->text(), portInput->text().toUShort());
		}
	}


	void ConnectionWidget::onConnectButtonClicked() {
		QString host = hostInput->text();
		bool ok;
		quint16 port = portInput->text().toUShort(&ok);

		if (!host.isEmpty() && ok) {
			emit connectRequest(host, port);
		} else {
			logger->log("Invalid host or port.");
		}
	}
}
