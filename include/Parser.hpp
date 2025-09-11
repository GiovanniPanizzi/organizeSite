#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <stack>

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

class Parser {
public:
    Parser();
    ~Parser();
    std::unique_ptr<HtmlNode> parseHTML(const std::string& htmlContent);
};