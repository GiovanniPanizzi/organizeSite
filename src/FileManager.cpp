#include "../include/FileManager.hpp"
#include <iostream>
#include <sstream>

FileManager::FileManager() {
}

FileManager::~FileManager() {
}

std::string FileManager::copyContent(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file) {
        std::cerr << "Impossible to open file: " << filePath << "\n";
        return "";
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

