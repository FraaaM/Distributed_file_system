#include <QApplication>

#include "logger.hpp"
#include "followerwindow.hpp"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

	SHIZ::Logger logger;
	logger.log("Application started.");

	SHIZ::FollowerWindow followerWindow(&logger);
	followerWindow.show();

	int result = app.exec();
	logger.log("Application exited.");
	return result;
}
