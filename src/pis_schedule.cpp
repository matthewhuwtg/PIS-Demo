#include "../include/pis_schedule.h"
#include <algorithm>
#include <iostream>

namespace pis {

std::string ScheduleEvent::typeString() const {
    switch (type) {
        case ScheduleEventType::DEPARTURE:    return "DEPARTURE";
        case ScheduleEventType::ARRIVAL:      return "ARRIVAL";
        case ScheduleEventType::APPROACHING:  return "APPROACHING";
        case ScheduleEventType::NEXT_STOP:    return "NEXT_STOP";
        case ScheduleEventType::TERMINUS:     return "TERMINUS";
        case ScheduleEventType::CUSTOM:       return "CUSTOM";
    }
    return "UNKNOWN";
}

ScheduleEngine::ScheduleEngine() = default;
ScheduleEngine::~ScheduleEngine() { stop(); }

void ScheduleEngine::initialize(const Route& route) {
    route_ = route;
    buildSchedule();
    std::cout << "[Schedule] Initialized: " << route_.name
              << " (" << route_.stations.size() << " stations, "
              << events_.size() << " events)" << std::endl;
}

int ScheduleEngine::onEvent(ScheduleCallback cb) {
    callbacks_.push_back(std::move(cb));
    return static_cast<int>(callbacks_.size()) - 1;
}

void ScheduleEngine::buildSchedule() {
    events_.clear();
    const auto& stations = route_.stations;
    if (stations.empty()) return;
    {
        ScheduleEvent ev;
        ev.type = ScheduleEventType::DEPARTURE;
        ev.station_id = stations[0].id;
        ev.station_name_en = stations[0].name_en;
        ev.station_name_zh = stations[0].name_zh;
        ev.elapsed_sec = 0;
        ev.station_index = 0;
        ev.message = "Train departing from " + stations[0].name_en;
        events_.push_back(ev);
    }
    for (size_t i = 1; i < stations.size(); ++i) {
        const auto& prev = stations[i - 1];
        const auto& curr = stations[i];
        int travel_time = curr.arrival_sec - prev.arrival_sec;
        if (travel_time > 10) {
            ScheduleEvent ev;
            ev.type = ScheduleEventType::APPROACHING;
            ev.station_id = curr.id;
            ev.station_name_en = curr.name_en;
            ev.station_name_zh = curr.name_zh;
            ev.elapsed_sec = prev.arrival_sec + travel_time / 2;
            ev.station_index = static_cast<int>(i);
            ev.message = "Approaching " + curr.name_en;
            events_.push_back(ev);
        }
        {
            ScheduleEvent ev;
            ev.type = ScheduleEventType::NEXT_STOP;
            ev.station_id = curr.id;
            ev.station_name_en = curr.name_en;
            ev.station_name_zh = curr.name_zh;
            ev.elapsed_sec = curr.arrival_sec - std::min(5, travel_time / 4);
            ev.station_index = static_cast<int>(i);
            ev.message = "Next stop: " + curr.name_en;
            events_.push_back(ev);
        }
        {
            ScheduleEvent ev;
            ev.type = ScheduleEventType::ARRIVAL;
            ev.station_id = curr.id;
            ev.station_name_en = curr.name_en;
            ev.station_name_zh = curr.name_zh;
            ev.elapsed_sec = curr.arrival_sec;
            ev.station_index = static_cast<int>(i);
            ev.message = "Arriving at " + curr.name_en;
            events_.push_back(ev);
        }
    }
    {
        const auto& last = stations.back();
        ScheduleEvent ev;
        ev.type = ScheduleEventType::TERMINUS;
        ev.station_id = last.id;
        ev.station_name_en = last.name_en;
        ev.station_name_zh = last.name_zh;
        ev.elapsed_sec = last.arrival_sec;
        ev.station_index = static_cast<int>(stations.size()) - 1;
        ev.message = "Terminus: " + last.name_en + " - End of line";
        events_.push_back(ev);
    }
    std::stable_sort(events_.begin(), events_.end(),
        [](const ScheduleEvent& a, const ScheduleEvent& b) {
            if (a.elapsed_sec != b.elapsed_sec) return a.elapsed_sec < b.elapsed_sec;
            return static_cast<int>(a.type) < static_cast<int>(b.type);
        });
}

void ScheduleEngine::fireEvent(const ScheduleEvent& event) {
    for (auto& cb : callbacks_) { if (cb) cb(event); }
}

void ScheduleEngine::start() {
    if (running_) return;
    if (events_.empty()) { std::cerr << "[Schedule] No events!" << std::endl; return; }
    running_ = true;
    current_idx_ = -1;
    elapsed_sec_ = 0;
    worker_ = std::make_unique<std::thread>([this]() {
        size_t event_idx = 0;
        auto start_time = std::chrono::steady_clock::now();
        while (running_ && event_idx < events_.size()) {
            auto now = std::chrono::steady_clock::now();
            int elapsed = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count());
            elapsed_sec_ = elapsed;
            while (event_idx < events_.size() && events_[event_idx].elapsed_sec <= elapsed) {
                current_idx_ = events_[event_idx].station_index;
                fireEvent(events_[event_idx]);
                ++event_idx;
            }
            if (event_idx >= events_.size()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                continue;
            }
            int next_time = events_[event_idx].elapsed_sec;
            int sleep_ms = std::min(200, (next_time - elapsed) * 1000);
            if (sleep_ms > 0) std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
        }
        running_ = false;
    });
}

void ScheduleEngine::stop() {
    running_ = false;
    if (worker_ && worker_->joinable()) worker_->join();
    std::cout << "[Schedule] Stopped. Elapsed: " << elapsed_sec_ << "s" << std::endl;
}

} // namespace pis
