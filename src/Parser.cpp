#include "../include/Parser.hpp"
#include <iostream>

Parser::Parser() {
}

Parser::~Parser() {
}

/* HELPER FUNCTIONS */

// helper: lowercase
std::string toLower(const std::string& s) {
    std::string r; r.reserve(s.size());
    for (char c : s) r.push_back(static_cast<char>(std::tolower((unsigned char)c)));
    return r;
}

// helper: skip whitespace
void Parser::skipSpaces(const std::string& s, size_t& i) {
    while (i < s.size() && std::isspace((unsigned char)s[i])) ++i;
}

// parse tag name starting at i (expects letter at s[i]) -> returns name and new index (first char after name)
std::string Parser::parseTagName(const std::string& s, size_t& i) {
    size_t start = i;
    while (i < s.size() && (std::isalnum((unsigned char)s[i]) || s[i] == '-' || s[i] == ':' )) i++;
    return s.substr(start, i - start);
}

// parse attributes between current pos (after tag name) and '>' or '/>'
// returns map and sets i to position of '>' (i points at '>' when returning)
std::unordered_map<std::string, std::string> Parser::parseAttributes(const std::string& s, size_t& i) {
    std::unordered_map<std::string, std::string> attrs;
    skipSpaces(s, i);
    while (i < s.size() && s[i] != '>' && !(s[i] == '/' && i + 1 < s.size() && s[i+1] == '>')) {
        // parse attr name
        size_t start = i;
        while (i < s.size() && (std::isalnum((unsigned char)s[i]) || s[i] == '-' || s[i] == ':' )) ++i;
        std::string name = s.substr(start, i - start);
        skipSpaces(s, i);
        std::string value;
        if (i < s.size() && s[i] == '=') {
            ++i;
            skipSpaces(s, i);
            if (i < s.size() && (s[i] == '"' || s[i] == '\'')) {
                char q = s[i++];
                size_t vstart = i;
                while (i < s.size() && s[i] != q) {
                    if (s[i] == '\\' && i + 1 < s.size()) i += 2; // skip escapes
                    else ++i;
                }
                value = s.substr(vstart, i - vstart);
                if (i < s.size()) ++i; // skip closing quote
            } else {
                // unquoted value
                size_t vstart = i;
                while (i < s.size() && !std::isspace((unsigned char)s[i]) && s[i] != '>' ) ++i;
                value = s.substr(vstart, i - vstart);
            }
        }
        if (!name.empty()) {
            attrs[name] = value;
        } else {
            // skip unexpected char
            ++i;
        }
        skipSpaces(s, i);
    }
    // here i points at '>' or at '/' of '/>'
    return attrs;
}

// sees if at position i the substring s starts with pattern (case-insensitive)
bool Parser::startsWithCI(const std::string& s, size_t i, const std::string& pattern) {
    if (i + pattern.size() > s.size()) return false;
    for (size_t k = 0; k < pattern.size(); ++k) {
        if (std::tolower((unsigned char)s[i+k]) != std::tolower((unsigned char)pattern[k])) return false;
    }
    return true;
}

// check if buffer is empty or only whitespace
bool Parser::isBufferEmptyOrWhitespace(const std::string& s) {
    for (char c : s) {
        if (!std::isspace((unsigned char)c)) return false;
    }
    return true;
}


/*PUBLIC*/

// main parser
std::unique_ptr<HtmlNode> Parser::parseHTML(const std::string& html) {
    size_t n = html.size();
    auto root = std::make_unique<HtmlNode>();
    root->tag = "root";

    // Set di elementi "void" in HTML5
    const std::unordered_set<std::string> voidElements = {
        "area", "base", "br", "col", "embed", "hr", "img", "input", "link",
        "meta", "param", "source", "track", "wbr"
    };

    std::vector<HtmlNode*> stack;
    stack.reserve(64);
    stack.push_back(root.get());

    std::string buffer;
    enum class State {
        CONTENT,
        TAG_OPEN,
        COMMENT,
        SCRIPT,
        SCRIPT_STRING_SINGLE,
        SCRIPT_STRING_DOUBLE
    };
    State state = State::CONTENT;

    for (size_t i = 0; i < n; i++) {
        char c = html[i];

        switch (state) {
            case State::CONTENT: {
                if (c == '<') {
                    if (!buffer.empty() && !isBufferEmptyOrWhitespace(buffer)) {
                        auto txt = std::make_unique<HtmlNode>();
                        txt->tag = "#text";
                        txt->content = buffer;
                        stack.back()->children.push_back(std::move(txt));
                    }
                    buffer.clear();
                    state = State::TAG_OPEN;
                    continue;
                }
                buffer += c;
                break;
            }

            case State::TAG_OPEN: {
                if (c == '!') {
                    if (i + 2 < n && html.substr(i, 3) == "!--") {
                        auto comment = std::make_unique<HtmlNode>();
                        comment->tag = "#comment";
                        stack.back()->children.push_back(std::move(comment));
                        stack.push_back(stack.back()->children.back().get());
                        i += 2;
                        state = State::COMMENT;
                    } else {
                        // Tratta <!DOCTYPE> e simili come testo (o ignorali)
                        buffer += '<';
                        buffer += c;
                        state = State::CONTENT;
                    }
                } else if (c == '/') {
                    size_t j = i + 1;
                    skipSpaces(html, j);
                    std::string closing = parseTagName(html, j);
                    size_t gt = html.find('>', j);
                    if (gt == std::string::npos) {
                        std::cerr << "Mancanza di '>' nel tag di chiusura a pos " << i << "\n";
                        return nullptr;
                    }
                    if (stack.size() > 1 && stack.back()->tag == toLower(closing)) {
                        stack.pop_back();
                    } else {
                        std::cerr << "Tag di chiusura non corrispondente: " << closing << " a pos " << i << "\n";
                    }
                    i = gt;
                    state = State::CONTENT;
                } else if (isalpha(static_cast<unsigned char>(c))) {
                    size_t j = i;
                    std::string tagName = parseTagName(html, j);
                    auto attrs = parseAttributes(html, j);
                    bool selfClosing = ((j < n && html[j] == '/') || (voidElements.count(toLower(tagName)) > 0));
                    if (selfClosing) while (j < n && html[j] != '>') j++;
                    
                    if (j < n && html[j] == '>') {
                        auto node = std::make_unique<HtmlNode>();
                        node->tag = toLower(tagName);
                        node->attributes = std::move(attrs);

                        if (node->tag == "html" && stack.back()->tag == "root") {
                             root->children.push_back(std::move(node));
                             stack.back() = root->children.back().get();
                        } else {
                             stack.back()->children.push_back(std::move(node));
                             stack.push_back(stack.back()->children.back().get());
                        }

                        if (selfClosing) stack.pop_back();

                        if (toLower(tagName) == "script") {
                            state = State::SCRIPT;
                        } else {
                            state = State::CONTENT;
                        }
                        i = j;
                    } else {
                        std::cerr << "Tag malformato a pos " << i << "\n";
                        i = j;
                        state = State::CONTENT;
                    }
                } else {
                    buffer += '<';
                    buffer += c;
                    state = State::CONTENT;
                }
                break;
            }

            case State::COMMENT: {
                if (i + 2 < n && html.substr(i, 3) == "-->") {
                    stack.back()->content = buffer;
                    buffer.clear();
                    stack.pop_back();
                    i += 2;
                    state = State::CONTENT;
                } else {
                    buffer += c;
                }
                break;
            }

            case State::SCRIPT: {
                if (i + 1 < n && html.substr(i, 2) == "/*") {
                    buffer += "/*";
                    i++;
                } else if (i + 7 < n && html.substr(i, 8) == "</script") {
                    // Chiusura del tag <script>
                    stack.back()->content = buffer;
                    buffer.clear();
                    stack.pop_back();
                    i += 7; // Avanza il cursore per saltare il tag di chiusura
                    state = State::CONTENT;
                } else if (c == '\'') {
                    buffer += c;
                    state = State::SCRIPT_STRING_SINGLE;
                } else if (c == '"') {
                    buffer += c;
                    state = State::SCRIPT_STRING_DOUBLE;
                } else {
                    buffer += c;
                }
                break;
            }

            case State::SCRIPT_STRING_SINGLE: {
                buffer += c;
                if (c == '\'' && (i == 0 || html[i-1] != '\\')) {
                    state = State::SCRIPT;
                }
                break;
            }

            case State::SCRIPT_STRING_DOUBLE: {
                buffer += c;
                if (c == '"' && (i == 0 || html[i-1] != '\\')) {
                    state = State::SCRIPT;
                }
                break;
            }
        }
    }

    if (!buffer.empty() && !isBufferEmptyOrWhitespace(buffer)) {
        auto txt = std::make_unique<HtmlNode>();
        txt->tag = "#text";
        txt->content = buffer;
        stack.back()->children.push_back(std::move(txt));
    }
    buffer.clear();

    if (root->children.size() == 1 && root->children[0]->tag == "html") {
        return std::move(root->children[0]);
    }

    return root;
}