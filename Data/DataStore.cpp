#include "DataStore.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

DataStore::DataStore(const std::string& filePath)
    : m_filePath(filePath)
    , m_format(detectFormat(filePath))
{
}

StoreFormat DataStore::detectFormat(const std::string& filePath) {
    // 확장자가 "json"이면 JSON, 그 외 TXT
    auto pos = filePath.rfind('.');
    if (pos != std::string::npos) {
        std::string ext = filePath.substr(pos + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == "json") return StoreFormat::JSON;
    }
    return StoreFormat::TXT;
}

void DataStore::load() {
    if (m_format == StoreFormat::JSON)
        loadJson();
    else
        loadTxt();
}

void DataStore::save() const {
    if (m_format == StoreFormat::JSON)
        saveJson();
    else
        saveTxt();
}

void DataStore::loadTxt() {
    m_data.clear();
    std::ifstream ifs(m_filePath);
    if (!ifs.is_open()) return;

    std::string line;
    while (std::getline(ifs, line)) {
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key   = line.substr(0, eq);
        std::string value = line.substr(eq + 1);
        m_data[key] = value;
    }
}

void DataStore::saveTxt() const {
    std::ofstream ofs(m_filePath);
    for (const auto& [k, v] : m_data) {
        ofs << k << '=' << v << '\n';
    }
}

void DataStore::loadJson() {
    m_data.clear();
    std::ifstream ifs(m_filePath);
    if (!ifs.is_open()) return;

    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string content = ss.str();

    // 단순 flat JSON 파싱 (key-value만)
    // 형식: { "key": "value", ... }
    std::size_t pos = 0;
    while (pos < content.size()) {
        // key 찾기
        auto kStart = content.find('"', pos);
        if (kStart == std::string::npos) break;
        auto kEnd = content.find('"', kStart + 1);
        if (kEnd == std::string::npos) break;
        std::string key = content.substr(kStart + 1, kEnd - kStart - 1);

        // ':' 찾기
        auto colon = content.find(':', kEnd + 1);
        if (colon == std::string::npos) break;

        // value 찾기
        auto vStart = content.find('"', colon + 1);
        if (vStart == std::string::npos) break;
        // value 문자열 (이스케이프 처리)
        std::string value;
        std::size_t i = vStart + 1;
        while (i < content.size() && content[i] != '"') {
            if (content[i] == '\\' && i + 1 < content.size()) {
                ++i;
                switch (content[i]) {
                    case '"':  value += '"';  break;
                    case '\\': value += '\\'; break;
                    case 'n':  value += '\n'; break;
                    case 'r':  value += '\r'; break;
                    case 't':  value += '\t'; break;
                    default:   value += content[i]; break;
                }
            } else {
                value += content[i];
            }
            ++i;
        }
        auto vEnd = i;

        m_data[key] = value;
        pos = vEnd + 1;
    }
}

void DataStore::saveJson() const {
    std::ofstream ofs(m_filePath);
    ofs << "{\n";
    bool first = true;
    for (const auto& [k, v] : m_data) {
        if (!first) ofs << ",\n";
        ofs << "  \"" << jsonEscape(k) << "\": \"" << jsonEscape(v) << "\"";
        first = false;
    }
    ofs << "\n}\n";
}

std::string DataStore::jsonEscape(const std::string& s) {
    std::string result;
    result.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '"':  result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n";  break;
            case '\r': result += "\\r";  break;
            case '\t': result += "\\t";  break;
            default:   result += c;      break;
        }
    }
    return result;
}

void DataStore::set(const std::string& key, const std::string& value) {
    m_data[key] = value;
}

std::string DataStore::get(const std::string& key, const std::string& defaultValue) const {
    auto it = m_data.find(key);
    if (it == m_data.end()) return defaultValue;
    return it->second;
}

bool DataStore::has(const std::string& key) const {
    return m_data.count(key) > 0;
}

void DataStore::remove(const std::string& key) {
    m_data.erase(key);
}

std::vector<std::string> DataStore::keys() const {
    std::vector<std::string> result;
    result.reserve(m_data.size());
    for (const auto& [k, v] : m_data) {
        result.push_back(k);
    }
    return result;
}

StoreFormat DataStore::format() const {
    return m_format;
}
