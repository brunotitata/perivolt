#include "monitor_window.hpp"
#include "../application/monitor.hpp"
#include <QApplication>
#include <QTimer>
#include <QUuid>
#include <chrono>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    app.setApplicationName("PeriVolt");
    perivolt::application::Monitor monitor;
    perivolt::ui::MonitorWindow window;
    // One process-local epoch; a GUI tick requests projection, never a new observation.
    const auto epoch = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
#ifdef __APPLE__
    constexpr bool native_discovery = true;
#else
    constexpr bool native_discovery = false;
#endif
    const auto render = [&] {
        const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        window.setSnapshot(monitor.snapshot(now, epoch));
        std::optional<perivolt::ui::DiscoverySummary> discovery;
        if (const auto sample = monitor.metadata()) {
            discovery = perivolt::ui::DiscoverySummary{sample->enumeration_complete,
                sample->os_error, sample->candidate_count, sample->family_endpoint_count};
        }
        window.setDiscoveryStatus(perivolt::ui::present_discovery(native_discovery, discovery));
    };
    perivolt::ui::Intents intents;
    intents.request_read = [&](bool enabled) {
        (void)monitor.request_read(enabled);
        if (enabled) monitor.refresh_metadata();
        render(); // Confirm the actual application state, never optimistic consent.
    };
    window.setIntents(std::move(intents));
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &window, [&] {
        monitor.shutdown(); // Invalidate outstanding tokens before UI teardown.
    });
    QTimer timer;
    timer.setInterval(250);
    QObject::connect(&timer, &QTimer::timeout, &window, render);
    monitor.refresh_metadata(); // Registry-only worker; no hardware I/O on GUI thread.
    render();
    timer.start();
    window.show();
    return app.exec();
}
