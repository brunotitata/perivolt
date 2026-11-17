#include "monitor_window.hpp"
#include <QApplication>
int main(int argc, char** argv) {
    QApplication app(argc, argv);
    app.setApplicationName("PeriVolt");
    perivolt::ui::MonitorWindow window;
    // No producer or import handler attached: empty list, consent OFF, controls unavailable.
    window.show();
    return app.exec();
}
