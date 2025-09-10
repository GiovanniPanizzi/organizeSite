#include <filesystem>
#include "FileManager.hpp"
#include "DirectoryManager.hpp"
#include <string>
#pragma once

class Organizer {
private:
    FileManager fileManager;
    DirectoryManager directoryManager;
    std::vector<std::filesystem::path> htmlFilesPaths;
    std::vector<std::filesystem::path> cssFilesPaths;
    std::vector<std::string> htmlFiles;
    std::vector<std::string> cssFiles;
public:
    Organizer();
    ~Organizer();

    void organize(const std::filesystem::path& path);
};