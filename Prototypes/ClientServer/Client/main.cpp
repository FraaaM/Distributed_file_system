#include <QCoreApplication>

#include "client.hpp"

int main(int argc, char *argv[]) {
	QCoreApplication a(argc, argv);

	FileClient client;
	std::filesystem::path filePath = std::filesystem::current_path() / "1.txt";
	QString qFilePath = QString::fromStdString(filePath.string());
	client.uploadFile(qFilePath);

	return a.exec();
}
