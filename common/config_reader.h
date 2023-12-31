// ConfigReader.h

#ifndef TALON_CONFIG_READER_H
#define TALON_CONFIG_READER_H

#include <unordered_map>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

class ConfigReader {
public:
    ConfigReader(const std::string &filename) {
        std::ifstream file(filename);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                parseLine(line);
            }
            file.close();
        }
    }

    std::unordered_map<std::string, std::string> getMap() const {
        return configMap;
    }

private:
    std::unordered_map<std::string, std::string> configMap;

    void parseLine(const std::string &line) {
        // 去除行首行尾的空格
        std::string trimmedLine = trim(line);

        // 忽略注释行和空行
        if (trimmedLine.empty() || trimmedLine[0] == '#') {
            return;
        }

        // 查找 '=' 的位置
        std::size_t pos = trimmedLine.find('=');
        if (pos != std::string::npos) {
            std::string key = trim(trimmedLine.substr(0, pos));
            std::string value = trim(trimmedLine.substr(pos + 1));
            configMap[key] = value;
        }
    }

    static std::string trim(const std::string &str) {
        std::size_t first = str.find_first_not_of(' ');
        if (std::string::npos == first) {
            return "";
        }
        std::size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }
};

#endif // CONFIG_READER_H
