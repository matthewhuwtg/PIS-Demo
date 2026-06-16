#include "../include/pis_core.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace pis {

PISEngine::PISEngine() = default;
PISEngine::~PISEngine() { stop(); }

bool PISEngine::initialize(const std::string& config_path) {
    std::cout << "\n========== PIS Engine Initializing ==========\n" << std::endl;
    if (!config_mgr_.load(config_path)) { std::cerr << "[Engine] Config load failed!" << std::endl; return false; }
    const auto& cfg = config_mgr_.get();
    display_.initialize(cfg.display);
    schedule_.initialize(cfg.route);
    if (cfg.ipc.enabled && !ipc_server_.initialize(cfg.ipc)) std::cerr << "[Engine] IPC init failed" << std::endl;
    if (cfg.dbus.enabled && !dbus_.initialize(cfg.dbus)) std::cerr << "[Engine] D-Bus init failed" << std::endl;
    setupCallbacks();
    std::cout << "\n========== PIS Engine Ready ==========\n" << std::endl;
    return true;
}

void PISEngine::setupCallbacks() {
    schedule_.onEvent([this](const ScheduleEvent& e) { display_.onScheduleEvent(e); });
    if (config_mgr_.get().ipc.enabled) schedule_.onEvent([this](const ScheduleEvent& e) { ipc_server_.broadcast(e); });
    if (config_mgr_.get().dbus.enabled) schedule_.onEvent([this](const ScheduleEvent& e) { dbus_.emitEvent(e); });
}

void PISEngine::run(int duration_sec) {
    const auto& cfg = config_mgr_.get();
    std::cout << "\n========== PIS Demo Running ==========" << std::endl;
    std::cout << "Route: " << cfg.route.name << " (" << cfg.route.stations.size() << " stations)" << std::endl;
    ipc_server_.start();
    dbus_.start();
    schedule_.start();
    if (duration_sec > 0) {
        for (int i = 0; i < duration_sec * 2; ++i) {
            if (!schedule_.isRunning()) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        stop();
    } else {
        while (schedule_.isRunning()) std::this_thread::sleep_for(std::chrono::milliseconds(200));
        stop();
    }
}

void PISEngine::stop() {
    schedule_.stop();
    ipc_server_.stop();
    dbus_.stop();
    std::cout << "\n========== PIS Demo Completed ==========\n" << std::endl;
}

void PISEngine::printStatus() const {
    const auto& cfg = config_mgr_.get();
    std::cout << "\n--- PIS Status ---" << std::endl;
    std::cout << "  System: " << cfg.system.name << " v" << cfg.system.version << std::endl;
    std::cout << "  Route: " << cfg.route.name << " (" << cfg.route.stations.size() << " stations)" << std::endl;
    std::cout << "  IPC: " << (cfg.ipc.enabled ? "enabled" : "disabled") << std::endl;
    std::cout << "  D-Bus: " << (cfg.dbus.enabled ? "enabled" : "disabled") << " [" << dbus_.backend() << "]" << std::endl;
    std::cout << "------------------\n" << std::endl;
}

} // namespace pis
