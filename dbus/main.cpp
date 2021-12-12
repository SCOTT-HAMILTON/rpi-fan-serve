#include <QCoreApplication>
#include <QDebug>
#include <signal.h>
#include "SignalHandler.h"
#include "dbus_server.h"
#include "../socket_locker.h"

// true on success, false on error
static bool setup_unix_signal_handlers()
{
	struct sigaction intSig, termSig;

	intSig.sa_handler = SignalHandler::intSignalHandler;
	sigemptyset(&intSig.sa_mask);
	intSig.sa_flags = 0;
	intSig.sa_flags |= SA_RESTART;

	if (sigaction(SIGINT, &intSig, 0)) {
		qDebug() << "[fatal] can't setup SIGINT handler.";
		return false;
	}

	termSig.sa_handler = SignalHandler::termSignalHandler;
	sigemptyset(&termSig.sa_mask);
	termSig.sa_flags = 0;
	termSig.sa_flags |= SA_RESTART;

	if (sigaction(SIGTERM, &termSig, 0)) {
		qDebug() << "[fatal] can't setup SIGTERM handler.";
		return false;
	}

	return true;
}

int main(int argc, char *argv[])
{

	SocketLocker<DBUS_LOCK_FILE> socketLocker;
	if (!socketLocker.checkAndTryLock()) {
		return 1;
	}
    QCoreApplication app(argc, argv);
	SignalHandler sigHandler;
	if (!setup_unix_signal_handlers()) {
		qDebug() << "[fatal] failed to set signal handlers.";
	}
	DbusServer dbusServer;
    auto rv = app.exec();
	qDebug() << "[log] clean stop !";	
	return rv;
}
