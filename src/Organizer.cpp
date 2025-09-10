#include "../include/Organizer.hpp"
#include <iostream>

Organizer::Organizer() {
}

Organizer::~Organizer() {
}

void Organizer::organize(const std::filesystem::path& path) {
    std::cout << "Organizing files in: " << path << std::endl;
}