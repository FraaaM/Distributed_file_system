#pragma once

#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QObject>

class FileServer : public QTcpServer {
	Q_OBJECT

	private:
		QTcpSocket *clientSocket;
		QFile file;

	public:
		explicit FileServer(QObject *parent = nullptr);
		void startServer(qint16 port);

	protected:
		void incomingConnection(qintptr socketDescriptor) override;

	private slots:
		void handleClientConnection();
		void onClientDisconnected();
};
