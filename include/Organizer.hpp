#include <filesystem>
#include "FileManager.hpp"
#include "DirectoryManager.hpp"
#include <string>
#pragma once

class Organizer {
private:
    FileManager fileManager;
    DirectoryManager directoryManager;
    std::vector<std::filesystem::path> htmlFiles;
    std::vector<std::filesystem::path> cssFiles;
public:
    Organizer();
    ~Organizer();

    void organize(const std::filesystem::path& path);
};