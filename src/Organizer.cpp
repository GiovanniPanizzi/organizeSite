#include "../include/Organizer.hpp"
#include <iostream>

Organizer::Organizer() {
}

Organizer::~Organizer() {
}

void Organizer::organize(const std::filesystem::path& path) {
    htmlFiles = directoryManager.findFiles(path, ".html");
    cssFiles = directoryManager.findFiles(path, ".css");

    for(const auto& file : htmlFiles) {
        std::cout << "Found HTML file: " << file << std::endl;
    }

    for(const auto& file : cssFiles) {
        std::cout << "Found CSS file: " << file << std::endl;
    }
}