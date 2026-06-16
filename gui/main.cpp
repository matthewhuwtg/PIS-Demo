#include <QApplication>
#include "pis_window.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    PISWindow window;
    window.show();
    return app.exec();
}
