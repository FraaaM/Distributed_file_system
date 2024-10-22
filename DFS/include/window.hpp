#pragma once

#include <QTcpSocket>
#include <QStackedWidget>
#include <QWidget>

#include "mainwidget.hpp"
#include "loginwidget.hpp"
#include "registrationwidget.hpp"

namespace SHIZ{
	class Window: public QWidget{
		Q_OBJECT

		private:
			QTcpSocket *tcpSocket;
			QStackedWidget *stackedWidget;

			MainWidget *chatWidget;
			LoginWidget *loginWidget;
			RegistrationWidget *registrationWidget;

		public:
			Window(QWidget* parent = nullptr);

		private slots:
			void onSwitchToChatWindow();
			void onSwitchToLoginWindow();
			void onSwitchToRegistrationWindow();
	};
}
