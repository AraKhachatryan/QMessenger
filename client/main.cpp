#include "messenger.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QSharedPointer<messenger> start_messenger = QSharedPointer<messenger>::create();

    QObject::connect(start_messenger.data(), &messenger::close_all_windows, &app, &QApplication::closeAllWindows);

    return app.exec();
}
