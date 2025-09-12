#pragma once
#include <string>
#include <vector>
#include <unordered_map>

struct HtmlNode {
    std::string tag;
    std::string content;
    std::vector<std::unique_ptr<HtmlNode>> children;
    std::unordered_map<std::string, std::string> attributes;
    size_t lineNumber;
};

struct CssRule {
    std::string style;
    std::string mediaQuery;
};

struct FileData {
    std::string name;
    std::string content;
    std::string extension;
};