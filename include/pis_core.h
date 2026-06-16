#ifndef PIS_CORE_H
#define PIS_CORE_H

#include "pis_config.h"
#include "pis_schedule.h"
#include "pis_display.h"
#include "pis_ipc.h"
#include "pis_dbus.h"
#include <memory>
#include <string>

namespace pis {

class PISEngine {
public:
    PISEngine();
    ~PISEngine();
    bool initialize(const std::string& config_path);
    void run(int duration_sec = 0);
    void stop();
    const PISConfig& getConfig() const { return config_mgr_.get(); }
    void printStatus() const;

private:
    ConfigManager config_mgr_;
    ScheduleEngine schedule_;
    DisplayModule display_;
    IpcServer ipc_server_;
    DBusInterface dbus_;
    void setupCallbacks();
};

} // namespace pis

#endif // PIS_CORE_H
