#ifndef PIS_CONFIG_H
#define PIS_CONFIG_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <sstream>

namespace pis {

struct Station {
    std::string id;
    std::string name_en;
    std::string name_zh;
    int arrival_sec;
};

struct Route {
    std::string id;
    std::string name;
    std::string direction;
    std::vector<Station> stations;
};

struct DisplayConfig {
    int refresh_interval_ms = 1000;
    std::string color_scheme = "dark";
    bool show_next_stop = true;
    bool show_time = true;
};

struct IpcConfig {
    std::string socket_path = "/tmp/pis_demo.sock";
    bool enabled = true;
};

struct DBusConfig {
    std::string service_name = "com.pis.demo";
    std::string object_path = "/com/pis/demo";
    std::string interface_name = "com.pis.demo.Interface";
    bool enabled = true;
};

struct MediaConfig {
    bool announcement_enabled = true;
    std::string audio_backend = "gstreamer";
};

struct SystemConfig {
    std::string name;
    std::string version;
    std::string language = "bilingual";
    std::vector<std::string> languages = {"en", "zh"};
};

struct PISConfig {
    SystemConfig system;
    Route route;
    DisplayConfig display;
    IpcConfig ipc;
    DBusConfig dbus;
    MediaConfig media;
};

class ConfigManager {
public:
    bool load(const std::string& filepath);
    const PISConfig& get() const { return config_; }
    PISConfig& get() { return config_; }
    bool isLoaded() const { return loaded_; }

private:
    PISConfig config_;
    bool loaded_ = false;
    std::string trim(const std::string& s) const;
    std::string extractString(const std::string& json, size_t& pos);
    double extractNumber(const std::string& json, size_t& pos);
    bool extractBool(const std::string& json, size_t& pos);
    void skipWhitespace(const std::string& json, size_t& pos) const;
    bool expectChar(const std::string& json, size_t& pos, char c);
    void parseObject(const std::string& json, size_t& pos, PISConfig& cfg);
    void parseRoute(const std::string& json, size_t& pos, Route& route);
    void parseStation(const std::string& json, size_t& pos, Station& station);
};

} // namespace pis

#endif // PIS_CONFIG_H
