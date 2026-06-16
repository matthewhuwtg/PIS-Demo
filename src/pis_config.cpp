#include "../include/pis_config.h"
#include <cctype>
#include <stdexcept>
#include <iostream>

namespace pis {

void ConfigManager::skipWhitespace(const std::string& json, size_t& pos) const {
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) {
        ++pos;
    }
}

bool ConfigManager::expectChar(const std::string& json, size_t& pos, char c) {
    skipWhitespace(json, pos);
    if (pos < json.size() && json[pos] == c) {
        ++pos;
        return true;
    }
    return false;
}

std::string ConfigManager::extractString(const std::string& json, size_t& pos) {
    skipWhitespace(json, pos);
    if (pos >= json.size() || json[pos] != '"') {
        return "";
    }
    ++pos;
    std::string result;
    while (pos < json.size() && json[pos] != '"') {
        if (json[pos] == '\\' && pos + 1 < json.size()) {
            ++pos;
            switch (json[pos]) {
                case 'n': result += '\n'; break;
                case 't': result += '\t'; break;
                case '\\': result += '\\'; break;
                case '"': result += '"'; break;
                default: result += json[pos]; break;
            }
        } else {
            result += json[pos];
        }
        ++pos;
    }
    if (pos < json.size()) ++pos;
    return result;
}

double ConfigManager::extractNumber(const std::string& json, size_t& pos) {
    skipWhitespace(json, pos);
    size_t start = pos;
    if (pos < json.size() && json[pos] == '-') ++pos;
    while (pos < json.size() && std::isdigit(static_cast<unsigned char>(json[pos]))) ++pos;
    if (pos < json.size() && json[pos] == '.') {
        ++pos;
        while (pos < json.size() && std::isdigit(static_cast<unsigned char>(json[pos]))) ++pos;
    }
    std::string numStr = json.substr(start, pos - start);
    return std::stod(numStr);
}

bool ConfigManager::extractBool(const std::string& json, size_t& pos) {
    skipWhitespace(json, pos);
    if (json.substr(pos, 4) == "true") {
        pos += 4;
        return true;
    }
    if (json.substr(pos, 5) == "false") {
        pos += 5;
        return false;
    }
    return false;
}

std::string ConfigManager::trim(const std::string& s) const {
    size_t start = 0, end = s.size();
    while (start < end && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(start, end - start);
}

void ConfigManager::parseStation(const std::string& json, size_t& pos, Station& station) {
    expectChar(json, pos, '{');
    while (pos < json.size() && !expectChar(json, pos, '}')) {
        std::string key = extractString(json, pos);
        expectChar(json, pos, ':');
        if (key == "id") station.id = extractString(json, pos);
        else if (key == "name_en") station.name_en = extractString(json, pos);
        else if (key == "name_zh") station.name_zh = extractString(json, pos);
        else if (key == "arrival_sec") station.arrival_sec = static_cast<int>(extractNumber(json, pos));
        else { if (json[pos] == '"') extractString(json, pos); else extractNumber(json, pos); }
        expectChar(json, pos, ',');
    }
}

void ConfigManager::parseRoute(const std::string& json, size_t& pos, Route& route) {
    expectChar(json, pos, '{');
    while (pos < json.size() && !expectChar(json, pos, '}')) {
        std::string key = extractString(json, pos);
        expectChar(json, pos, ':');
        if (key == "id") route.id = extractString(json, pos);
        else if (key == "name") route.name = extractString(json, pos);
        else if (key == "direction") route.direction = extractString(json, pos);
        else if (key == "stations") {
            expectChar(json, pos, '[');
            while (pos < json.size() && !expectChar(json, pos, ']')) {
                Station station;
                parseStation(json, pos, station);
                route.stations.push_back(station);
                expectChar(json, pos, ',');
            }
        } else { if (json[pos] == '"') extractString(json, pos); else extractNumber(json, pos); }
        expectChar(json, pos, ',');
    }
}

void ConfigManager::parseObject(const std::string& json, size_t& pos, PISConfig& cfg) {
    expectChar(json, pos, '{');
    while (pos < json.size() && !expectChar(json, pos, '}')) {
        std::string key = extractString(json, pos);
        expectChar(json, pos, ':');
        if (key == "system") {
            expectChar(json, pos, '{');
            while (pos < json.size() && !expectChar(json, pos, '}')) {
                std::string sk = extractString(json, pos);
                expectChar(json, pos, ':');
                if (sk == "name") cfg.system.name = extractString(json, pos);
                else if (sk == "version") cfg.system.version = extractString(json, pos);
                else if (sk == "language") cfg.system.language = extractString(json, pos);
                else if (sk == "languages") {
                    expectChar(json, pos, '[');
                    while (pos < json.size() && !expectChar(json, pos, ']')) {
                        cfg.system.languages.push_back(extractString(json, pos));
                        expectChar(json, pos, ',');
                    }
                } else { extractString(json, pos); }
                expectChar(json, pos, ',');
            }
        } else if (key == "route") { parseRoute(json, pos, cfg.route); }
        else if (key == "display") {
            expectChar(json, pos, '{');
            while (pos < json.size() && !expectChar(json, pos, '}')) {
                std::string dk = extractString(json, pos);
                expectChar(json, pos, ':');
                if (dk == "refresh_interval_ms") cfg.display.refresh_interval_ms = static_cast<int>(extractNumber(json, pos));
                else if (dk == "color_scheme") cfg.display.color_scheme = extractString(json, pos);
                else if (dk == "show_next_stop") cfg.display.show_next_stop = extractBool(json, pos);
                else if (dk == "show_time") cfg.display.show_time = extractBool(json, pos);
                else { extractString(json, pos); }
                expectChar(json, pos, ',');
            }
        } else if (key == "ipc") {
            expectChar(json, pos, '{');
            while (pos < json.size() && !expectChar(json, pos, '}')) {
                std::string ik = extractString(json, pos);
                expectChar(json, pos, ':');
                if (ik == "socket_path") cfg.ipc.socket_path = extractString(json, pos);
                else if (ik == "enabled") cfg.ipc.enabled = extractBool(json, pos);
                else { extractString(json, pos); }
                expectChar(json, pos, ',');
            }
        } else if (key == "dbus") {
            expectChar(json, pos, '{');
            while (pos < json.size() && !expectChar(json, pos, '}')) {
                std::string dk = extractString(json, pos);
                expectChar(json, pos, ':');
                if (dk == "service_name") cfg.dbus.service_name = extractString(json, pos);
                else if (dk == "object_path") cfg.dbus.object_path = extractString(json, pos);
                else if (dk == "interface_name") cfg.dbus.interface_name = extractString(json, pos);
                else if (dk == "enabled") cfg.dbus.enabled = extractBool(json, pos);
                else { extractString(json, pos); }
                expectChar(json, pos, ',');
            }
        } else if (key == "media") {
            expectChar(json, pos, '{');
            while (pos < json.size() && !expectChar(json, pos, '}')) {
                std::string mk = extractString(json, pos);
                expectChar(json, pos, ':');
                if (mk == "announcement_enabled") cfg.media.announcement_enabled = extractBool(json, pos);
                else if (mk == "audio_backend") cfg.media.audio_backend = extractString(json, pos);
                else { extractString(json, pos); }
                expectChar(json, pos, ',');
            }
        } else {
            if (json[pos] == '{') { int d = 1; ++pos; while (pos < json.size() && d > 0) { if (json[pos] == '{') ++d; else if (json[pos] == '}') --d; ++pos; } }
            else extractString(json, pos);
        }
        expectChar(json, pos, ',');
    }
}

bool ConfigManager::load(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[Config] ERROR: Cannot open file: " << filepath << std::endl;
        return false;
    }
    std::stringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();
    try {
        size_t pos = 0;
        parseObject(content, pos, config_);
        loaded_ = true;
        std::cout << "[Config] Loaded config from: " << filepath << std::endl;
        std::cout << "[Config] System: " << config_.system.name
                  << " v" << config_.system.version
                  << " | Route: " << config_.route.name
                  << " (" << config_.route.stations.size() << " stations)" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[Config] ERROR parsing config: " << e.what() << std::endl;
        return false;
    }
}

} // namespace pis
