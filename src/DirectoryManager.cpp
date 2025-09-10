#include "../include/DirectoryManager.hpp"
#include <iostream>

DirectoryManager::DirectoryManager() {
}

DirectoryManager::~DirectoryManager() {
}

std::vector<std::filesystem::path> DirectoryManager::findFiles(const std::filesystem::path& path, const std::string& extension) {
    std::vector<std::filesystem::path> foundFiles;
    if(std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
        for(const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
            if(entry.is_regular_file() && entry.path().extension() == extension) {
                foundFiles.push_back(entry.path());
            }
        }
    }
    return foundFiles;
}

void DirectoryManager::createDirectory(const std::filesystem::path& path, const std::string& directoryName) {
    std::filesystem::path newDir = path / directoryName;
    if(!std::filesystem::exists(newDir)) {
        std::filesystem::create_directory(newDir);
    }
    else{
        std::cout << "Directory already exists: " << newDir << std::endl;
    }
}