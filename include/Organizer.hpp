#include <filesystem>
#include "FileManager.hpp"
#include <unordered_set>
#include "DirectoryManager.hpp"
#include "Parser.hpp"
#include <string>
#include "HtmlBuilder.hpp"
#pragma once

class Organizer {
private:
    std::string rootPath;
    std::unordered_map<std::string, size_t> absPathToIndex;
    FileManager fileManager;
    DirectoryManager directoryManager;
    Parser parser;
    std::vector<std::filesystem::path> htmlFilesPaths;
    std::vector<std::filesystem::path> cssFilesPaths;
    std::vector<FileData> htmlFiles;
    std::vector<FileData> cssFiles;
    std::vector<std::unique_ptr<HtmlNode>> htmlTrees;
    HtmlBuilder htmlBuilder;

    std::vector<std::string> findHrefs(const HtmlNode* node);
    std::string findTitle(const HtmlNode* node);
    std::unique_ptr<HtmlNode> cloneTree(const HtmlNode* originalNode);
    void updateHtmlLinks(HtmlNode* node, size_t currentFileIdx, const std::unordered_set<size_t>& visitedIndices);
public:
    Organizer();
    ~Organizer();
    void organize(const std::filesystem::path& path);
};