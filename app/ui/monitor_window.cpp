#include "monitor_window.hpp"
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QGroupBox>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSystemTrayIcon>
#include <QTableWidget>
#include <QThread>
#include <QVBoxLayout>

namespace perivolt::ui {
namespace {
QString q(const std::string& text) { return QString::fromUtf8(text.data(), static_cast<int>(text.size())); }
QLabel* label(const QString& text, QWidget* parent) {
    auto* item = new QLabel(text, parent);
    item->setTextFormat(Qt::PlainText);
    item->setWordWrap(true);
    return item;
}
QTableWidget* table(QWidget* parent) {
    auto* result = new QTableWidget(0, 8, parent);
    result->setHorizontalHeaderLabels({"Dispositivo", "Componente", "Bateria", "Frescor do nível",
                                      "Idade", "Carregamento", "Conexão", "Fonte"});
    result->setEditTriggers(QAbstractItemView::NoEditTriggers);
    result->setSelectionBehavior(QAbstractItemView::SelectRows);
    result->setSelectionMode(QAbstractItemView::SingleSelection);
    result->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    result->horizontalHeader()->setStretchLastSection(true);
    result->verticalHeader()->hide();
    result->setMinimumHeight(140);
    return result;
}
QString freshness(const std::string& value) {
    if (value == "fresh") return "Atual";
    if (value == "stale") return "Antigo (stale)";
    if (value == "unknown") return "Desconhecido";
    return q(value);
}
void populate(QTableWidget* table, const std::vector<DeviceRow>& rows, bool historical) {
    // Preserve keyboard selection by canonical identity, never by display name.
    QString selected_device, selected_component;
    if (table->currentRow() >= 0 && table->item(table->currentRow(), 0)) {
        selected_device = table->item(table->currentRow(), 0)->data(Qt::UserRole).toString();
        selected_component = table->item(table->currentRow(), 1)->text();
    }
    const QSignalBlocker blocker(table);
    table->setRowCount(static_cast<int>(rows.size()));
    int index = 0;
    for (const auto& row : rows) {
        const QStringList values = {q(row.name), q(row.component), q(row.level),
            historical ? QStringLiteral("Histórico — sem atualização") : freshness(row.level_freshness),
            q(row.age), q(row.charging) + " · " + freshness(row.charging_freshness),
            q(row.connection) + " · " + freshness(row.connection_freshness), q(row.source)};
        for (int column = 0; column < values.size(); ++column) {
            auto* cell = new QTableWidgetItem(values[column]);
            cell->setToolTip("Diagnóstico: " + q(row.diagnostic) + "\nFrescor de carga: "
                + freshness(row.charging_freshness) + "\nFrescor de conexão: " + freshness(row.connection_freshness));
            table->setItem(index, column, cell);
        }
        table->item(index, 0)->setData(Qt::UserRole, q(row.device));
        if (selected_device == q(row.device) && selected_component == q(row.component))
            table->setCurrentCell(index, 0);
        ++index;
    }
}
}
MonitorWindow::MonitorWindow(QWidget* parent) : QWidget(parent) {
    setWindowTitle("PeriVolt — Monitor de bateria");
    resize(1000, 720);
    setStyleSheet("QWidget { background: #18221e; color: #f2f7f4; } "
        "QGroupBox { border: 1px solid #4c6556; border-radius: 6px; margin-top: 12px; padding: 14px; } "
        "QGroupBox::title { subcontrol-origin: margin; left: 12px; } "
        "QPushButton { background: #17633f; padding: 9px; border: 1px solid #78d5a0; border-radius: 4px; } "
        "QPushButton:disabled { background: #37433c; color: #afbab3; border-color: #58655d; } "
        "QPushButton:focus, QCheckBox:focus { border: 2px solid #94f7b9; } "
        "QHeaderView::section { background: #283c30; padding: 6px; } "
        "QTableWidget { alternate-background-color: #24352b; selection-background-color: #17633f; }");
    auto* layout = new QVBoxLayout(this);
    auto* title = label("PeriVolt", this);
    auto font = title->font(); font.setPointSize(24); font.setBold(true); title->setFont(font);
    layout->addWidget(title);
    status_ = label({}, this); layout->addWidget(status_);
    empty_ = label("Nenhum dispositivo detectado", this); layout->addWidget(empty_);
    devices_ = table(this); devices_->setAccessibleName("Dispositivos da sessão"); layout->addWidget(devices_);

    auto* preferences = new QGroupBox("Preferências", this);
    auto* settings = new QVBoxLayout(preferences);
    read_ = new QCheckBox("&Autorizar leitura dos dispositivos", preferences);
    settings->addWidget(read_);
    read_explanation_ = label({}, preferences); settings->addWidget(read_explanation_);
    close_to_tray_ = new QCheckBox("Ao fechar, manter na &bandeja", preferences);
    close_to_tray_->setChecked(false); settings->addWidget(close_to_tray_);
    tray_status_ = label({}, preferences); settings->addWidget(tray_status_);
    layout->addWidget(preferences);

    auto* history = new QGroupBox("Diagnóstico histórico — leitura pontual", this);
    auto* history_layout = new QVBoxLayout(history);
    history_status_ = label({}, history); history_layout->addWidget(history_status_);
    import_ = new QPushButton("&Importar diagnóstico…", history); history_layout->addWidget(import_);
    historical_ = table(history); historical_->setAccessibleName("Diagnósticos históricos, sem alertas");
    history_layout->addWidget(historical_); layout->addWidget(history);

    QPixmap image(32, 32); image.fill(Qt::transparent);
    { QPainter painter(&image); painter.setBrush(QColor("#45c780")); painter.setPen(Qt::NoPen);
      painter.drawRoundedRect(2, 2, 28, 28, 7, 7); painter.setPen(QColor("#18221e"));
      painter.drawText(image.rect(), Qt::AlignCenter, "P"); }
    setWindowIcon(QIcon(image));
    tray_ = new QSystemTrayIcon(windowIcon(), this);
    tray_->setToolTip("PeriVolt");
    auto* menu = new QMenu(this);
    auto* open = menu->addAction("Abrir PeriVolt");
    auto* prefs = menu->addAction("Preferências");
    menu->addSeparator();
    auto* quit = menu->addAction("Sair");
    tray_->setContextMenu(menu);
    connect(open, &QAction::triggered, this, [this] { showWindow(); });
    connect(prefs, &QAction::triggered, this, [this] { showWindow(); read_->setFocus(); });
    connect(quit, &QAction::triggered, qApp, &QApplication::quit);
    connect(tray_, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) showWindow();
    });
    if (QSystemTrayIcon::isSystemTrayAvailable()) tray_->show();
    connect(read_, &QCheckBox::clicked, this, [this](bool requested) {
        const auto view = present(snapshot_, static_cast<bool>(intents_.request_read));
        if (!view.read_enabled || read_pending_) { render(); return; }
        read_pending_ = true; render();
        intents_.request_read(requested); // No optimistic consent/state mutation.
    });
    connect(import_, &QPushButton::clicked, this, [this] {
        if (!intents_.import_diagnostic) return;
        const auto path = QFileDialog::getOpenFileName(this, "Selecionar diagnóstico histórico", {}, "Todos os arquivos (*)");
        if (path.isEmpty()) return;
        const auto request = history_.begin();
        import_message_ = "Importação solicitada; aguardando validação do arquivo.";
        render();
        intents_.import_diagnostic(request, path.toUtf8().toStdString());
    });
    render();
}
void MonitorWindow::setIntents(Intents intents) {
    Q_ASSERT(QThread::currentThread() == thread());
    intents_ = std::move(intents); render();
}
void MonitorWindow::setSnapshot(Snapshot snapshot) {
    Q_ASSERT(QThread::currentThread() == thread());
    snapshot_ = std::move(snapshot); read_pending_ = false; render();
}
bool MonitorWindow::completeHistoricalImport(std::uint64_t request, std::vector<DeviceReading> readings,
                                           const std::string& error) {
    Q_ASSERT(QThread::currentThread() == thread());
    if (!error.empty()) readings.clear();
    if (!history_.complete(request, std::move(readings))) return false;
    import_message_ = error.empty() ? "Arquivo importado. Dados históricos não geram alertas nem renovam leituras."
                                   : "Falha na importação: " + error;
    render(); return true;
}
void MonitorWindow::render() {
    const auto view = present(snapshot_, static_cast<bool>(intents_.request_read));
    status_->setText(q(view.status));
    tray_->setToolTip("PeriVolt — " + q(view.status));
    empty_->setVisible(view.rows.empty()); devices_->setVisible(!view.rows.empty());
    populate(devices_, view.rows, false);
    const QSignalBlocker blocker(read_);
    read_->setChecked(view.read_checked);
    read_->setEnabled(view.read_enabled && !read_pending_);
    const auto permission = snapshot_.control.permission == Permission::granted ? "concedida"
        : snapshot_.control.permission == Permission::denied ? "negada" : "desconhecida";
    read_explanation_->setText((read_pending_ ? QStringLiteral("Solicitação enviada; aguardando confirmação do produtor.")
        : q(view.read_explanation)) + "\nPermissão do sistema: " + permission);
    const bool tray_available = QSystemTrayIcon::isSystemTrayAvailable();
    close_to_tray_->setEnabled(tray_available);
    tray_status_->setText(tray_available ? "Janela disponível pelo menu da bandeja."
        : "Bandeja indisponível neste ambiente; use esta janela. Fechar encerra o aplicativo.");
    import_->setEnabled(static_cast<bool>(intents_.import_diagnostic));
    history_status_->setText(import_message_.empty() ? (intents_.import_diagnostic
        ? "Nenhum diagnóstico importado. Escolha um arquivo explicitamente."
        : "Importação indisponível: serviço de importação não conectado.") : q(import_message_));
    std::vector<DeviceRow> rows;
    for (const auto& reading : history_.readings()) rows.push_back(present_reading(reading));
    populate(historical_, rows, true); historical_->setVisible(!rows.empty());
}
void MonitorWindow::showWindow() { showNormal(); raise(); activateWindow(); }
void MonitorWindow::closeEvent(QCloseEvent* event) {
    if (close_to_tray_->isChecked() && QSystemTrayIcon::isSystemTrayAvailable() && tray_->isVisible()) {
        hide(); event->ignore();
    } else QWidget::closeEvent(event);
}
}
