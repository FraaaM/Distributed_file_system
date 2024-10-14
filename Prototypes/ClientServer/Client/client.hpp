#pragma once

#include <QTcpSocket>
#include <QFile>
#include <QObject>

class FileClient : public QObject {
	Q_OBJECT

	private:
		QTcpSocket *socket;
		QFile file;
		qint64 bytesSent;

	public:
		explicit FileClient(QObject *parent = nullptr);
		void connectToServer(const QString &host, qint16 port);
		void uploadFile(const QString &filePath);

	private slots:
		void onConnected();
		void onBytesWritten(qint64 bytes);
};
