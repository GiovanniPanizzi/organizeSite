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

void FileManager::createFile(const std::string& filePath, const std::string& content) {
    std::fstream file(filePath, std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return;
    }
    file << content;
    file.close();
}

