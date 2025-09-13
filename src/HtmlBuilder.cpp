#include "../include/HtmlBuilder.hpp"

// Build public API: just forward to recursive worker
std::string HtmlBuilder::buildHtml(const HtmlNode& node) const {
    if (node.tag == "root") {
        return buildNode(*node.children.front());
    }
    return buildNode(node);
}

// Recursive serializer
std::string HtmlBuilder::buildNode(const HtmlNode& node) const {
    // Text nodes: escape and return content
    if (node.tag == "#text") {
        return escapeText(node.content);
    }

    // Comment nodes: emit as <!-- ... -->
    if (node.tag == "#comment") {
        return std::string("<!--") + node.content + "-->";
    }

    // Normalize tag name for checks (case-insensitive)
    std::string tagLower = toLower(node.tag);

    // Build attributes string
    std::ostringstream attrs;
    for (const auto& kv : node.attributes) {
        if (kv.first.empty()) continue; // defensive: skip empty attribute names
        attrs << ' ' << kv.first << "=\"" << escapeAttribute(kv.second) << '"';
    }

    // For <script> and <style> keep raw content (do not HTML-escape)
    if (tagLower == "script" || tagLower == "style") {
        std::ostringstream out;
        out << '<' << node.tag << attrs.str() << '>';
        out << node.content; // raw script/style content
        out << "</" << node.tag << '>';
        return out.str();
    }

    // Handle void tags: if it's a void tag and has no children/content, emit self-closing form
    if (isVoidTag(tagLower) && node.children.empty() && node.content.empty()) {
        std::ostringstream out;
        out << '<' << node.tag << attrs.str() << " />";
        return out.str();
    }

    // Normal element: open tag, recursively append children, then close tag
    std::ostringstream out;
    out << '<' << node.tag << attrs.str() << '>';

    // Note: for regular elements we do not use node.content; content is reserved for #text/#comment/script
    for (const auto& child : node.children) {
        out << buildNode(*child);
    }

    out << "</" << node.tag << '>';
    return out.str();
}

// Escape &, <, > in text nodes
std::string HtmlBuilder::escapeText(const std::string& s) const {
    std::string out;
    out.reserve(s.size());
    for (unsigned char ch : s) {
        switch (ch) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;";  break;
            case '>': out += "&gt;";  break;
            default:  out.push_back(static_cast<char>(ch)); break;
        }
    }
    return out;
}

// Escape attribute values (also escape double quote)
std::string HtmlBuilder::escapeAttribute(const std::string& s) const {
    std::string out;
    out.reserve(s.size());
    for (unsigned char ch : s) {
        switch (ch) {
            case '&': out += "&amp;";  break;
            case '<': out += "&lt;";   break;
            case '>': out += "&gt;";   break;
            case '"': out += "&quot;"; break;
            default:  out.push_back(static_cast<char>(ch)); break;
        }
    }
    return out;
}

// Lower-case helper (ASCII-aware)
std::string HtmlBuilder::toLower(const std::string& s) const {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(),
                   [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return r;
}

// Check if the tag is a void tag in HTML5
bool HtmlBuilder::isVoidTag(const std::string& lowerTag) const {
    // static local ensures the set is constructed once per program lifetime
    static const std::unordered_set<std::string> voidTags = {
        "area","base","br","col","embed","hr","img","input","link","meta","param","source","track","wbr"
    };
    return voidTags.find(lowerTag) != voidTags.end();
}
   