#pragma once
#include "structs.hpp"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <stack>
#include <unordered_set>

class Parser {
private:
    bool startsWithCI(const std::string& s, size_t i, const std::string& pattern);
    bool isBufferEmptyOrWhitespace(const std::string& s);
    void skipSpaces(const std::string& s, size_t& i);
    std::string parseTagName(const std::string& s, size_t& i);
    std::unordered_map<std::string, std::string> parseAttributes(const std::string& s, size_t& i);
public:
    Parser();
    ~Parser();
    std::unique_ptr<HtmlNode> parseHTML(const std::string& html);
};