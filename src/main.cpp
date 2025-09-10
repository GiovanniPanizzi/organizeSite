#include <iostream>
#include <filesystem>
#include "../include/Organizer.hpp"

Organizer organizer;

int main(int argc, char** argv) {
    std::filesystem::path path = std::filesystem::current_path();
    if(argc > 1) {
        path = path / argv[1];
    }
    if(!std::filesystem::exists(path)) {
        std::cerr << "Path does not exist: " << path << std::endl;
        return 1;
    }
    organizer.organize(path);
    return 0;
}