#ifndef PIS_DISPLAY_H
#define PIS_DISPLAY_H

#include "pis_schedule.h"
#include "pis_config.h"
#include <string>
#include <iostream>
#include <sstream>
#include <memory>
#include <vector>

namespace pis {

enum class DisplayMode {
    CONSOLE_COLOR,
    PLAIN_TEXT
};

namespace media {

struct PipelineElement {
    std::string name;
    PipelineElement() = default;
    explicit PipelineElement(const std::string& n) : name(n) {}
    virtual ~PipelineElement() = default;
    virtual std::string process(const std::string& input) = 0;
};

class AnnouncementSource : public PipelineElement {
public:
    AnnouncementSource() : PipelineElement("AnnouncementSource") {}
    std::string process(const std::string& input) override;
};

class AudioMixer : public PipelineElement {
public:
    AudioMixer() : PipelineElement("AudioMixer") {}
    std::string process(const std::string& input) override;
};

class AudioSink : public PipelineElement {
public:
    AudioSink() : PipelineElement("AudioSink") {}
    std::string process(const std::string& input) override;
};

class Pipeline {
public:
    void addElement(std::unique_ptr<PipelineElement> element);
    std::string run(const std::string& input);
    void describe() const;
private:
    std::vector<std::unique_ptr<PipelineElement>> elements_;
};

} // namespace media

class DisplayModule {
public:
    DisplayModule();
    void initialize(const DisplayConfig& config);
    void setMode(DisplayMode mode);
    void onScheduleEvent(const ScheduleEvent& event);
    void renderStatus(int elapsed_sec, int station_idx, const Route& route);
    std::string lastAnnouncement() const { return last_announcement_; }
    std::string lastDisplayText() const { return last_display_; }

private:
    DisplayConfig config_;
    DisplayMode mode_{DisplayMode::CONSOLE_COLOR};
    std::string last_announcement_;
    std::string last_display_;
    media::Pipeline announcement_pipeline_;
    std::string generateAnnouncement(const ScheduleEvent& event);
    std::string colorize(const std::string& text, const std::string& color) const;
    std::string headerBar(const std::string& title) const;
    std::string formatStationList(int current_idx, const Route& route) const;
};

} // namespace pis

#endif // PIS_DISPLAY_H
