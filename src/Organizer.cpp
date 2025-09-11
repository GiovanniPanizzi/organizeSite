#include "../include/Organizer.hpp"
#include <iostream>

Organizer::Organizer() {
}

Organizer::~Organizer() {
}

void printTree(const HtmlNode* node, const std::string& prefix = "", bool isLast = true) {
    if (!node) return;

    // Stampa il nodo corrente con simboli di albero
    std::cout << prefix;
    std::cout << (isLast ? "└─" : "├─");
    std::cout << (node->tag.empty() ? "[TEXT]" : node->tag) << "\n";

    // Calcola il nuovo prefisso per i figli
    std::string newPrefix = prefix + (isLast ? "  " : "│ ");

    // Itera sui figli
    for (size_t i = 0; i < node->children.size(); ++i) {
        printTree(node->children[i].get(), newPrefix, i == node->children.size() - 1);
    }
}

void Organizer::organize(const std::filesystem::path& path) {
    // Find files and copy content
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

    // Parse HTML files (example: parse first HTML file)
    if(!htmlFiles.empty())
    printTree(parser.parseHTML(htmlFiles[0].content).get());

    // Create directories
    /*directoryManager.createDirectory(path, "Organized");
    directoryManager.createDirectory(path / "Organized", "HTML");
    directoryManager.createDirectory(path / "Organized", "CSS");

    for(size_t i = 0; i < htmlFiles.size(); ++i) {
        std::string fileName = htmlFiles[i].name + ".html";
        fileManager.createFile((path / "Organized" / "HTML" / fileName).string(), htmlFiles[i].content);
    }

    for(size_t i = 0; i < cssFiles.size(); ++i) {
        std::string fileName = cssFiles[i].name + ".css";
        fileManager.createFile((path / "Organized" / "CSS" / fileName).string(), cssFiles[i].content);
    }*/
}