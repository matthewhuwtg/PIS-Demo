#ifndef PIS_SCHEDULE_H
#define PIS_SCHEDULE_H

#include "pis_config.h"
#include <functional>
#include <chrono>
#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <vector>

namespace pis {

enum class ScheduleEventType {
    DEPARTURE,
    ARRIVAL,
    APPROACHING,
    NEXT_STOP,
    TERMINUS,
    CUSTOM
};

struct ScheduleEvent {
    ScheduleEventType type;
    std::string station_id;
    std::string station_name_en;
    std::string station_name_zh;
    int elapsed_sec;
    int station_index;
    std::string message;
    std::string typeString() const;
};

using ScheduleCallback = std::function<void(const ScheduleEvent&)>;

class ScheduleEngine {
public:
    ScheduleEngine();
    ~ScheduleEngine();
    void initialize(const Route& route);
    int onEvent(ScheduleCallback cb);
    void start();
    void stop();
    bool isRunning() const { return running_; }
    int currentStationIndex() const { return current_idx_; }
    int elapsedSeconds() const { return elapsed_sec_; }
    size_t eventCount() const { return events_.size(); }

private:
    Route route_;
    std::vector<ScheduleCallback> callbacks_;
    std::vector<ScheduleEvent> events_;
    std::unique_ptr<std::thread> worker_;
    std::atomic<bool> running_{false};
    std::atomic<int> current_idx_{-1};
    std::atomic<int> elapsed_sec_{0};
    void buildSchedule();
    void fireEvent(const ScheduleEvent& event);
};

} // namespace pis

#endif // PIS_SCHEDULE_H
