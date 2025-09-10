#include "../include/Organizer.hpp"
#include <iostream>

Organizer::Organizer() {
}

Organizer::~Organizer() {
}

void Organizer::organize(const std::filesystem::path& path) {
    htmlFilesPaths = directoryManager.findFiles(path, ".html");
    cssFilesPaths = directoryManager.findFiles(path, ".css");

    for(const auto& file : htmlFilesPaths) {
        htmlFiles.push_back(fileManager.copyContent(file));
    }

    for(const auto& file : cssFilesPaths) {
        cssFiles.push_back(fileManager.copyContent(file));
    }

    std::cout << htmlFiles[0] << std::endl;
}