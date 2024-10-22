#include <QVBoxLayout>

#include "mainwidget.hpp"

namespace SHIZ{
	MainWidget::MainWidget(QWidget* parent): QWidget(parent){
		QVBoxLayout *layout = new QVBoxLayout(this);

		QLabel *label = new QLabel("Hi!", this);

		layout->addWidget(label);
	}
}
