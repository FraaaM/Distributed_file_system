#pragma once

#include <QLabel>
#include <QPushButton>
#include <QWidget>

namespace SHIZ{
	class MainWidget : public QWidget{
		Q_OBJECT

		private:
			QLabel *chatArea;
			QPushButton *sendMessageButton;

		public:
			MainWidget(QWidget* parent = nullptr);

		signals:
			void showLoginWindow();
	};
}
