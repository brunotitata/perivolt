#pragma once
#include "presenter.hpp"
#include <QWidget>
#include <functional>
class QLabel;
class QCheckBox;
class QPushButton;
class QTableWidget;
class QSystemTrayIcon;
class QCloseEvent;
namespace perivolt::ui {
struct Intents {
    std::function<void(bool)> request_read;
    // Recipient parses the chosen file. Completion must return the request ID.
    std::function<void(std::uint64_t, const std::string&)> import_diagnostic;
};
class MonitorWindow final : public QWidget {
public:
    explicit MonitorWindow(QWidget* parent = nullptr);
    void setIntents(Intents intents);
    // GUI thread only. Producers should queue delivery with invokeMethod/context.
    void setSnapshot(Snapshot snapshot);
    bool completeHistoricalImport(std::uint64_t request, std::vector<DeviceReading> readings,
                                  const std::string& error = {});
protected:
    void closeEvent(QCloseEvent* event) override;
private:
    void render();
    void showWindow();
    Snapshot snapshot_;
    HistoricalImport history_;
    Intents intents_;
    bool read_pending_ = false;
    std::string import_message_;
    QLabel *status_, *empty_, *read_explanation_, *history_status_, *tray_status_;
    QTableWidget *devices_, *historical_;
    QCheckBox *read_, *close_to_tray_;
    QPushButton* import_;
    QSystemTrayIcon* tray_;
};
}
