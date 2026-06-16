#include "../include/pis_dbus.h"
#include <sstream>
#include <chrono>

namespace pis {

DBusInterface::DBusInterface() = default;
DBusInterface::~DBusInterface() { stop(); }

std::string DBusInterface::backend() const {
#ifdef HAS_DBUS
    return "real libdbus-1";
#else
    return "mock (no libdbus-1 detected)";
#endif
}

bool DBusInterface::initialize(const DBusConfig& config) {
    config_ = config;
    if (!config_.enabled) { std::cout << "[D-Bus] Disabled" << std::endl; return true; }
    std::cout << "[D-Bus] Backend: " << backend() << std::endl;
#ifdef HAS_DBUS
    return setupRealDBus();
#else
    std::cout << "[D-Bus] Mock implementation (install libdbus-1-dev for real D-Bus)" << std::endl;
    return true;
#endif
}

void DBusInterface::start() { if (config_.enabled) { running_ = true; std::cout << "[D-Bus] Started" << std::endl; } }
void DBusInterface::stop() {
    running_ = false;
#ifdef HAS_DBUS
    dbus_conn_ = nullptr;
#endif
    std::cout << "[D-Bus] Stopped" << std::endl;
}

void DBusInterface::emitEvent(const ScheduleEvent& event) {
    if (!running_ || !config_.enabled) return;
#ifdef HAS_DBUS
    emitRealSignal(event);
#else
    emitMockSignal(event);
#endif
}

#ifndef HAS_DBUS
void DBusInterface::emitMockSignal(const ScheduleEvent& event) {
    mock_signal_count_++;
    auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    std::cout << "[D-Bus:MOCK] Signal #" << mock_signal_count_ << " | " << event.typeString() << " | " << event.station_name_en << " | ts=" << ts << std::endl;
}
#endif

#ifdef HAS_DBUS
#include <dbus/dbus.h>
bool DBusInterface::setupRealDBus() {
    DBusError err; dbus_error_init(&err);
    dbus_conn_ = dbus_bus_get_private(DBUS_BUS_SESSION, &err);
    if (!dbus_conn_ || dbus_error_is_set(&err)) { std::cerr << "[D-Bus] " << err.message << std::endl; dbus_error_free(&err); return false; }
    dbus_error_free(&err);
    int ret = dbus_bus_request_name(static_cast<DBusConnection*>(dbus_conn_), config_.service_name.c_str(), DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    if (dbus_error_is_set(&err)) { std::cerr << "[D-Bus] " << err.message << std::endl; dbus_error_free(&err); return false; }
    return ret == DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER;
}

void DBusInterface::emitRealSignal(const ScheduleEvent& event) {
    auto* conn = static_cast<DBusConnection*>(dbus_conn_);
    if (!conn) return;
    DBusMessage* msg = dbus_message_new_signal(config_.object_path.c_str(), config_.interface_name.c_str(), "PISEvent");
    if (!msg) return;
    DBusMessageIter args; dbus_message_iter_init_append(msg, &args);
    const char* t = event.typeString().c_str();
    const char* s = event.station_name_en.c_str();
    const char* m = event.message.c_str();
    int32_t e = event.elapsed_sec;
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &t);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &s);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &m);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &e);
    dbus_connection_send(conn, msg, nullptr);
    dbus_connection_flush(conn);
    dbus_message_unref(msg);
}
#endif

} // namespace pis
