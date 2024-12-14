#pragma once

#include <QDateTime>
#include <QFile>

namespace SHIZ{
	class Logger : public QObject {
		Q_OBJECT

		private:
			QFile logFile;

		public:
			Logger(QObject* parent = nullptr);
			~Logger();

			void log(const QString& message);
	};
}
