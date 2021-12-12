#include <QObject>
#include <QSocketNotifier>

class SignalHandler : public QObject {
Q_OBJECT
public:
	SignalHandler(QObject *parent = nullptr);
	~SignalHandler();

	// Unix signal handlers.
	static void termSignalHandler(int unused);
	static void intSignalHandler(int unused);

public slots:
	// Qt signal handlers.
	void handleSigTerm();
	void handleSigInt();

private:
	static int sigtermFd[2];
	static int sigintFd[2];

	QSocketNotifier *snInt;
	QSocketNotifier *snTerm;
};
