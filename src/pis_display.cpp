#include "../include/pis_display.h"
#include <iomanip>

namespace pis {
namespace Color {
    const std::string RESET  = "\033[0m";
    const std::string BOLD   = "\033[1m";
    const std::string RED    = "\033[31m";
    const std::string GREEN  = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE   = "\033[34m";
    const std::string CYAN   = "\033[36m";
    const std::string WHITE  = "\033[37m";
}

std::string media::AnnouncementSource::process(const std::string& input) {
    return "[AUDIO: \"" + input + "\"]";
}
std::string media::AudioMixer::process(const std::string& input) {
    return "[MIX: chime + " + input + "]";
}
std::string media::AudioSink::process(const std::string& input) {
    return "[PLAYING: " + input + "] (audio output)";
}
void media::Pipeline::addElement(std::unique_ptr<PipelineElement> element) {
    elements_.push_back(std::move(element));
}
std::string media::Pipeline::run(const std::string& input) {
    std::string data = input;
    for (auto& e : elements_) data = e->process(data);
    return data;
}
void media::Pipeline::describe() const {
    std::cout << "  Pipeline: ";
    for (size_t i = 0; i < elements_.size(); ++i) {
        if (i > 0) std::cout << " -> ";
        std::cout << elements_[i]->name;
    }
    std::cout << std::endl;
}

DisplayModule::DisplayModule() {
    announcement_pipeline_.addElement(std::make_unique<media::AnnouncementSource>());
    announcement_pipeline_.addElement(std::make_unique<media::AudioMixer>());
    announcement_pipeline_.addElement(std::make_unique<media::AudioSink>());
}

void DisplayModule::initialize(const DisplayConfig& config) {
    config_ = config;
    std::cout << "[Display] Initialized (mode=" << (mode_ == DisplayMode::CONSOLE_COLOR ? "color" : "plain") << ")" << std::endl;
    announcement_pipeline_.describe();
}

void DisplayModule::setMode(DisplayMode mode) { mode_ = mode; }

std::string DisplayModule::colorize(const std::string& text, const std::string& color) const {
    return (mode_ == DisplayMode::CONSOLE_COLOR) ? color + text + Color::RESET : text;
}

std::string DisplayModule::headerBar(const std::string& title) const {
    return "+==========================================================+\n|  " + colorize(title, Color::BOLD + Color::CYAN) + "  |\n+==========================================================+";
}

void DisplayModule::onScheduleEvent(const ScheduleEvent& event) {
    std::string announcement = announcement_pipeline_.run(generateAnnouncement(event));
    last_announcement_ = announcement;
    last_display_ = event.message;
    std::cout << "\n" << headerBar(" PASSENGER INFORMATION SYSTEM ") << std::endl;
    std::cout << "| [" << event.typeString() << "] " << event.message << std::endl;
    int m = event.elapsed_sec / 60, s = event.elapsed_sec % 60;
    std::cout << "| Time: " << m << "m " << s << "s | Station: " << event.station_name_en << " / " << event.station_name_zh << std::endl;
    std::cout << "| Announcement: " << announcement << std::endl;
}

void DisplayModule::renderStatus(int elapsed_sec, int station_idx, const Route& route) {
    int m = elapsed_sec / 60, s = elapsed_sec % 60;
    std::cout << "\033[2J\033[H";
    std::cout << headerBar(" PIS LIVE STATUS ") << std::endl;
    std::cout << "| Elapsed: " << m << "m " << s << "s | " << route.name << " |" << std::endl;
}

std::string DisplayModule::generateAnnouncement(const ScheduleEvent& event) {
    switch (event.type) {
        case ScheduleEventType::DEPARTURE:   return "Now departing from " + event.station_name_en;
        case ScheduleEventType::APPROACHING: return "Approaching " + event.station_name_en;
        case ScheduleEventType::NEXT_STOP:   return "Next stop: " + event.station_name_en;
        case ScheduleEventType::ARRIVAL:     return "Arrived at " + event.station_name_en;
        case ScheduleEventType::TERMINUS:    return "Terminus: " + event.station_name_en + " - Thank you";
        default: return event.message;
    }
}
} // namespace pis
