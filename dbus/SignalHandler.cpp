#include "SignalHandler.h"
#include <QCoreApplication>
#include <QDebug>
#include <cstdlib>
#include <sys/socket.h>
#include <unistd.h>

int SignalHandler::sigtermFd[2];
int SignalHandler::sigintFd[2];

SignalHandler::SignalHandler(QObject *parent)
	: QObject(parent)
{
	if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigtermFd)) {
		qFatal("Couldn't create TERM socketpair");
	}
	if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigintFd)) {
		qFatal("Couldn't create INT socketpair");
	}
	snTerm = new QSocketNotifier(sigtermFd[1], QSocketNotifier::Read, this);
	connect(snTerm, SIGNAL(activated(QSocketDescriptor)), this, SLOT(handleSigTerm()));
	snInt = new QSocketNotifier(sigintFd[1], QSocketNotifier::Read, this);
	connect(snInt, SIGNAL(activated(QSocketDescriptor)), this, SLOT(handleSigInt()));
}


SignalHandler::~SignalHandler() { }

void SignalHandler::termSignalHandler(int)
{
    char a = 1;
    if(::write(sigtermFd[0], &a, sizeof(a)) == -1) {
		std::abort();		
	}
}

void SignalHandler::intSignalHandler(int)
{
    char a = 1;
    if (::write(sigintFd[0], &a, sizeof(a)) == -1) {
		std::abort();		
	}
}

void SignalHandler::handleSigTerm()
{
    snTerm->setEnabled(false);
    char tmp;
    if (::read(sigtermFd[1], &tmp, sizeof(tmp)) == -1) {
		qDebug() << "[fatal] read() == -1";
	}

    // do Qt stuff
	QCoreApplication::quit();

    snTerm->setEnabled(true);
}

void SignalHandler::handleSigInt()
{
    snInt->setEnabled(false);
    char tmp;
    if (::read(sigintFd[1], &tmp, sizeof(tmp)) == -1) {
		qDebug() << "[fatal] read() == -1";
	}

	QCoreApplication::quit();

    snInt->setEnabled(true);
}
