#include <filesystem>
#include "FileManager.hpp"
#include "DirectoryManager.hpp"
#include "Parser.hpp"
#include <string>
#pragma once

struct FileData {
    std::string name;
    std::string content;
    std::string extension;
};

class Organizer {
private:
    FileManager fileManager;
    DirectoryManager directoryManager;
    Parser parser;
    std::vector<std::filesystem::path> htmlFilesPaths;
    std::vector<std::filesystem::path> cssFilesPaths;
    std::vector<FileData> htmlFiles;
    std::vector<FileData> cssFiles;
public:
    Organizer();
    ~Organizer();

    void organize(const std::filesystem::path& path);
};