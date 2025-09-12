#pragma once
#include "structs.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <unordered_set>
#include <algorithm>
#include <cctype>

class HtmlBuilder {

private:
    // Recursive worker that serializes a node.
    std::string buildNode(const HtmlNode& node) const;

    // Escape text nodes: replace &, <, > with HTML entities.
    std::string escapeText(const std::string& s) const;

    // Escape attribute values: &, <, > and " are encoded.
    std::string escapeAttribute(const std::string& s) const;

    // Lower-case helper for case-insensitive checks (e.g. "SCRIPT" vs "script").
    std::string toLower(const std::string& s) const;

    // Return true if the tag is a void element in HTML5 (img, br, input, ...).
    bool isVoidTag(const std::string& lowerTag) const;

public:
    // Build a HTML string from the given HtmlNode tree.
    // The returned string contains the serialized HTML for the node and all its children.
    std::string buildHtml(const HtmlNode& node) const;
};