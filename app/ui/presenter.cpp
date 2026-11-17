#include "presenter.hpp"
#include <cmath>
#include <iomanip>
#include <sstream>

namespace perivolt::ui {
std::uint64_t HistoricalImport::begin() {
    pending_ = ++sequence_;
    return pending_;
}
bool HistoricalImport::complete(std::uint64_t request, std::vector<DeviceReading> readings) {
    if (!pending_ || request != pending_) return false;
    pending_ = 0;
    for (auto& reading : readings) reading.origin = Origin::historical_import;
    readings_ = std::move(readings);
    return true;
}
DeviceRow present_reading(const DeviceReading& reading) {
    const auto& state = reading.state;
    DeviceRow row;
    row.device = state.device;
    row.component = state.component;
    row.name = reading.name.empty() ? "Dispositivo sem nome" : reading.name;
    row.level = "Sem leitura";
    if (state.level.kind == "reported_percent" && std::isfinite(state.level.percent)
        && state.level.percent >= 0 && state.level.percent <= 100) {
        std::ostringstream value;
        value << std::setprecision(6) << state.level.percent << '%';
        row.level = value.str();
    } else if (state.level.kind == "qualitative" && !state.level.detail.empty()) {
        row.level = state.level.detail;
    }
    row.diagnostic = state.level.detail;
    row.age = reading.age_ms && *reading.age_ms >= 0
        ? std::to_string(*reading.age_ms / 1000) + " s" : "Idade desconhecida";
    row.charging = state.charging == "unknown" ? "Carregamento desconhecido" : state.charging;
    row.connection = state.connection == "unknown" ? "Conexão desconhecida" : state.connection;
    row.level_freshness = state.level_freshness;
    row.charging_freshness = state.charging_freshness;
    row.connection_freshness = state.connection_freshness;
    row.source = reading.source.empty() ? "Fonte desconhecida" : reading.source;
    return row;
}
View present(const Snapshot& snapshot, bool read_intent_connected) {
    View view;
    for (const auto& reading : snapshot.devices) {
        if (reading.state.present && reading.origin == Origin::session_observation)
            view.rows.push_back(present_reading(reading));
    }
    const auto& control = snapshot.control;
    view.read_checked = control.consent == Consent::granted;
    view.read_enabled = control.producer_connected && read_intent_connected
        && control.capability_available && control.permission != Permission::denied
        && control.consent != Consent::requested;
    if (!control.producer_connected || !read_intent_connected)
        view.read_explanation = "Leitura indisponível: produtor não conectado.";
    else if (!control.capability_available)
        view.read_explanation = "Leitura indisponível nesta fonte.";
    else if (control.permission == Permission::denied)
        view.read_explanation = "Permissão do sistema negada.";
    else if (control.consent == Consent::requested)
        view.read_explanation = "Solicitação pendente; aguardando confirmação do produtor.";
    else if (control.consent == Consent::denied)
        view.read_explanation = "Consentimento negado; leitura desativada.";
    else view.read_explanation = "Alterações solicitam autorização; não iniciam leitura pela interface.";
    // An active producer alone does not prove a current battery observation.
    bool current = false;
    for (const auto& row : view.rows) current = current || row.level_freshness == "fresh";
    if (control.consent != Consent::granted) view.status = "Desativado";
    else if (control.active && control.producer_connected && control.capability_available
             && control.permission == Permission::granted && current) view.status = "Monitorando";
    else view.status = view.rows.empty() ? "Sem leitura" : "Leitura pontual";
    return view;
}
}
