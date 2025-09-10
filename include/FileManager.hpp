#pragma once
#include <string>
#include <fstream>

class FileManager {
public:
    FileManager();
    ~FileManager();
    std::string copyContent(const std::string& filePath);
    void createFile(const std::string& directoryPath, const std::string& content);
};