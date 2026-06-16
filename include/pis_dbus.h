#ifndef PIS_DBUS_H
#define PIS_DBUS_H

#include "pis_schedule.h"
#include "pis_config.h"
#include <string>
#include <memory>
#include <atomic>
#include <iostream>

namespace pis {

class DBusInterface {
public:
    DBusInterface();
    ~DBusInterface();
    bool initialize(const DBusConfig& config);
    void start();
    void stop();
    void emitEvent(const ScheduleEvent& event);
    bool isRunning() const { return running_; }
    std::string backend() const;

private:
    DBusConfig config_;
    std::atomic<bool> running_{false};
#ifdef HAS_DBUS
    void* dbus_conn_{nullptr};
    uint32_t serial_{0};
    bool setupRealDBus();
    void emitRealSignal(const ScheduleEvent& event);
#else
    int mock_signal_count_{0};
    void emitMockSignal(const ScheduleEvent& event);
#endif
};

} // namespace pis

#endif // PIS_DBUS_H
