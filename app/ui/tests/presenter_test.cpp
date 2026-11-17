#include "../presenter.hpp"
#include <cassert>
#include <iostream>
#include <limits>
using namespace perivolt::ui;
int main() {
    assert(present_discovery(false, {}).find("indisponível") != std::string::npos);
    assert(present_discovery(true, {}).find("Aguardando") != std::string::npos);
    assert(present_discovery(false, {}).find('?') == std::string::npos);
    DiscoverySummary discovery;
    assert(present_discovery(true, discovery).find("incompleta") != std::string::npos);
    discovery.os_error = -1;
    assert(present_discovery(true, discovery).find("Falha") != std::string::npos);
    discovery.os_error = 0; discovery.complete = true;
    assert(present_discovery(true, discovery).find("Nenhum") != std::string::npos);
    discovery.candidates = 1; discovery.family_endpoints = 2;
    const auto discovery_text = present_discovery(true, discovery);
    assert(discovery_text.find("1 candidato(s)") != std::string::npos);
    assert(discovery_text.find("não comprovadas") != std::string::npos);
    assert(discovery_text.find('%') == std::string::npos);
    Snapshot snapshot;
    assert(present(snapshot, false).rows.empty());
    assert(!present(snapshot, true).read_enabled);
    assert(!present(snapshot, true).read_checked);
    DeviceReading reading;
    reading.name = "SYNTHETIC TEST ONLY";
    reading.state.present = true;
    reading.state.device = "test-device";
    reading.state.component = "battery";
    snapshot.devices = {reading};
    auto row = present(snapshot, false).rows.at(0);
    assert(row.level == "Sem leitura");
    assert(row.age == "Idade desconhecida");
    assert(row.charging == "Carregamento desconhecido");
    snapshot.devices[0].state.level = {"reported_percent", 0, ""};
    assert(present(snapshot, false).rows[0].level == "0%");
    snapshot.devices[0].state.level.percent = 100;
    assert(present(snapshot, false).rows[0].level == "100%");
    snapshot.devices[0].state.level.percent = std::numeric_limits<double>::quiet_NaN();
    assert(present(snapshot, false).rows[0].level == "Sem leitura");
    snapshot.devices[0].state.level.percent = -1;
    assert(present(snapshot, false).rows[0].level == "Sem leitura");
    snapshot.devices[0].state.level.percent = 101;
    assert(present(snapshot, false).rows[0].level == "Sem leitura");
    snapshot.devices[0].state.level = {"qualitative", 0, "low"};
    assert(present(snapshot, false).rows[0].level == "low");
    snapshot.devices[0].state.level = {"reported_percent", 50, ""};
    snapshot.devices[0].state.level_freshness = "stale";
    snapshot.devices[0].age_ms = 42000;
    row = present(snapshot, false).rows[0];
    assert(row.level == "50%");
    assert(row.level_freshness == "stale");
    assert(row.age == "42 s");
    snapshot.devices[0].age_ms = -1;
    assert(present(snapshot, false).rows[0].age == "Idade desconhecida");
    snapshot.devices[0].origin = Origin::historical_import;
    assert(present(snapshot, false).rows.empty());
    snapshot.devices[0].origin = Origin::session_observation;
    snapshot.devices[0].state.present = false;
    assert(present(snapshot, false).rows.empty());
    snapshot.control.producer_connected = true;
    snapshot.control.capability_available = true;
    assert(present(snapshot, true).read_enabled);
    assert(!present(snapshot, false).read_enabled);
    snapshot.control.permission = Permission::denied;
    assert(!present(snapshot, true).read_enabled);
    snapshot.control.permission = Permission::granted;
    snapshot.control.consent = Consent::requested;
    assert(present(snapshot, true).read_enabled); // Pending consent must be cancellable.
    assert(present(snapshot, true).read_checked);
    snapshot.control.capability_available = false;
    snapshot.control.permission = Permission::denied;
    assert(present(snapshot, true).read_enabled); // OFF remains possible after capability loss.
    snapshot.control.capability_available = true;
    snapshot.control.permission = Permission::granted;
    snapshot.control.consent = Consent::granted;
    assert(present(snapshot, true).read_checked);
    HistoricalImport history;
    assert(!history.complete(0, {reading}));
    assert(history.readings().empty());
    auto old_request = history.begin();
    auto request = history.begin();
    reading.state.level = {"reported_percent", 50, "explicit test import"};
    assert(!history.complete(old_request, {reading}));
    assert(history.complete(request, {reading}));
    assert(history.readings()[0].origin == Origin::historical_import);
    assert(!history.readings()[0].age_ms);
    assert(present_reading(history.readings()[0]).level == "50%");
    assert(present_reading(history.readings()[0]).age == "Idade desconhecida");
    assert(!history.complete(request, {}));
    for (const bool supported : {false, true}) {
        for (const auto summary : {DiscoverySummary{}, DiscoverySummary{true, -1, 0, 0},
                                  DiscoverySummary{true, 0, 0, 0}, DiscoverySummary{true, 0, 1, 2}})
            assert(present_discovery(supported, summary).find('?') == std::string::npos);
    }
    snapshot.devices = {reading};
    snapshot.devices[0].state.level_freshness = "fresh";
    snapshot.control.active = true;
    assert(present(snapshot, true).status == "Monitorando");
    snapshot.devices[0].state.level_freshness = "stale";
    assert(present(snapshot, true).status == "Leitura pontual");
    std::cout << "presenter: percentages/range, unknown, qualitative, stale, age, removal, history/request IDs, consent, monitoring PASS\n";
}
