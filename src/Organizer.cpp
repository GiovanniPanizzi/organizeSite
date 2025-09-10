#include "../include/Organizer.hpp"
#include <iostream>

Organizer::Organizer() {
}

Organizer::~Organizer() {
}

void Organizer::organize(const std::filesystem::path& path) {
    htmlFilesPaths = directoryManager.findFiles(path, ".html");
    cssFilesPaths = directoryManager.findFiles(path, ".css");

    for (const auto& file : htmlFilesPaths) {
        std::string name = file.stem().string();      
        std::string ext  = file.extension().string(); 
        std::string content = fileManager.copyContent(file.string());

        htmlFiles.push_back(FileData{name, content, ext});
    }

    for (const auto& file : cssFilesPaths) {
        std::string name = file.stem().string();      
        std::string ext  = file.extension().string(); 
        std::string content = fileManager.copyContent(file.string());

        cssFiles.push_back(FileData{name, content, ext});
    }

    directoryManager.createDirectory(path, "Organized");
    directoryManager.createDirectory(path / "Organized", "HTML");
    directoryManager.createDirectory(path / "Organized", "CSS");

    for(size_t i = 0; i < htmlFiles.size(); ++i) {
        std::string fileName = htmlFiles[i].name + ".html";
        fileManager.createFile((path / "Organized" / "HTML" / fileName).string(), htmlFiles[i].content);
    }

    for(size_t i = 0; i < cssFiles.size(); ++i) {
        std::string fileName = cssFiles[i].name + ".css";
        fileManager.createFile((path / "Organized" / "CSS" / fileName).string(), cssFiles[i].content);
    }
}