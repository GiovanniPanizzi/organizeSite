#include <filesystem>
#pragma once
class Organizer {
public:
    Organizer();
    ~Organizer();
    void organize(const std::filesystem::path& path);
};