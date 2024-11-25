#include "logger.hpp"

namespace SHIZ{
	Logger::Logger(QObject* parent): QObject(parent), logFile("log.txt") {
		if (!logFile.open(QIODevice::Append | QIODevice::Text)) {
			qDebug() << "Failed to open log file for writing.";
		}
	}

	Logger::~Logger() {
		if (logFile.isOpen()) {
			logFile.close();
		}
	}

	void Logger::log(const QString& message) {
		if (!logFile.isOpen()) return;

		QTextStream out(&logFile);
		QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
		out << "[" << timestamp << "] " << message << "\n";
		logFile.flush();
	}
}
