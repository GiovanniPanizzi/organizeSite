#include "../include/DirectoryManager.hpp"

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