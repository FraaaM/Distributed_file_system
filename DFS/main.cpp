#include <QApplication>

#include "window.hpp"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	SHIZ::Window window;
	window.show();
	return app.exec();
}
