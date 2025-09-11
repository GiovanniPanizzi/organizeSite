#include "../include/Parser.hpp"
#include <iostream>

Parser::Parser() {
}

Parser::~Parser() {
}

/* HELPER FUNCTIONS */

// helper: lowercase
static std::string toLower(const std::string& s) {
    std::string r; r.reserve(s.size());
    for (char c : s) r.push_back(static_cast<char>(std::tolower((unsigned char)c)));
    return r;
}

// helper: skip whitespace
static void skipSpaces(const std::string& s, size_t& i) {
    while (i < s.size() && std::isspace((unsigned char)s[i])) ++i;
}

// parse tag name starting at i (expects letter at s[i]) -> returns name and new index (first char after name)
static std::string parseTagName(const std::string& s, size_t& i) {
    size_t start = i;
    while (i < s.size() && (std::isalnum((unsigned char)s[i]) || s[i] == '-' || s[i] == ':' )) ++i;
    return s.substr(start, i - start);
}

// parse attributes between current pos (after tag name) and '>' or '/>'
// returns map and sets i to position of '>' (i points at '>' when returning)
static std::unordered_map<std::string, std::string> parseAttributes(const std::string& s, size_t& i) {
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
static bool startsWithCI(const std::string& s, size_t i, const std::string& pattern) {
    if (i + pattern.size() > s.size()) return false;
    for (size_t k = 0; k < pattern.size(); ++k) {
        if (std::tolower((unsigned char)s[i+k]) != std::tolower((unsigned char)pattern[k])) return false;
    }
    return true;
}



// check if buffer is empty or only whitespace
bool isBufferEmptyOrWhitespace(const std::string& s) {
    for (char c : s) {
        if (!std::isspace((unsigned char)c)) return false;
    }
    return true;
}

// main parser
std::unique_ptr<HtmlNode> Parser::parseHTML(const std::string& html) {
    size_t n = html.size();
    // root
    auto root = std::make_unique<HtmlNode>();
    root->tag = "root";

    std::vector<HtmlNode*> stack;
    stack.reserve(64);
    stack.push_back(root.get());

    std::string buffer; // accumula testo per #text, #comment, script content
    enum class State { CONTENT, TAG_OPEN, COMMENT, SCRIPT, SCRIPT_STRING_SINGLE, SCRIPT_STRING_DOUBLE };
    State state = State::CONTENT;

    for (size_t i = 0; i < n; ++i) {
        char c = html[i];

        if (state == State::CONTENT) {
            if (c == '<') {
                // comment?
                if (i + 3 < n && html.substr(i,4) == "<!--") {
                    // flush buffer as text if any
                    if (!buffer.empty() && !isBufferEmptyOrWhitespace(buffer)) {
                        auto txt = std::make_unique<HtmlNode>();
                        txt->tag = "#text";
                        txt->content = buffer;
                        stack.back()->children.push_back(std::move(txt));
                    }
                    buffer.clear();
                    // create comment node and push it
                    auto comment = std::make_unique<HtmlNode>();
                    comment->tag = "#comment";
                    stack.back()->children.push_back(std::move(comment));
                    stack.push_back(stack.back()->children.back().get());
                    state = State::COMMENT;
                    i += 3; // skip '!--'
                    continue;
                }
                // closing tag ?
                if (i + 1 < n && html[i+1] == '/') {
                    // flush buffer as text if any
                    if (!buffer.empty() && !isBufferEmptyOrWhitespace(buffer)) {
                        auto txt = std::make_unique<HtmlNode>();
                        txt->tag = "#text";
                        txt->content = buffer;
                        stack.back()->children.push_back(std::move(txt));
                    }
                    buffer.clear();
                    // parse closing tag name
                    size_t j = i + 2;
                    skipSpaces(html, j);
                    std::string closing = parseTagName(html, j);
                    // advance to next '>'
                    size_t gt = html.find('>', j);
                    if (gt == std::string::npos) {
                        std::cerr << "Unterminated closing tag at pos " << i << "\n";
                        return nullptr;
                    }
                    // attempt to pop until matching tag
                    bool matched = false;
                    if (stack.size() > 1 && stack.back()->tag == closing) {
                        stack.pop_back();
                        matched = true;
                    } else {
                        // try to find upwards (recovery)
                        for (int k = (int)stack.size()-1; k >= 1; --k) {
                            if (stack[k]->tag == closing) {
                                // pop until k
                                while (stack.size() > (size_t)k) stack.pop_back();
                                matched = true;
                                break;
                            }
                        }
                    }
                    if (!matched) {
                        std::cerr << "Mismatched closing tag: " << closing << " at position " << i << std::endl;
                        // skip it but continue
                    }
                    i = gt; // skip past '>'
                    continue;
                }
                // opening tag?
                if (i + 1 < n && std::isalpha((unsigned char)html[i+1])) {
                    // flush text buffer
                    if (!buffer.empty() && !isBufferEmptyOrWhitespace(buffer)) {
                        auto txt = std::make_unique<HtmlNode>();
                        txt->tag = "#text";
                        txt->content = buffer;
                        stack.back()->children.push_back(std::move(txt));
                    }
                    buffer.clear();
                    // parse tag name and attributes
                    size_t j = i + 1;
                    std::string tagName = parseTagName(html, j);
                    skipSpaces(html, j);
                    auto attrs = parseAttributes(html, j); // j points at '>' or '/'
                    // detect self-closing
                    bool selfClosing = false;
                    if (j < n && html[j] == '/') {
                        // expect '/>'
                        selfClosing = true;
                        // advance until '>'
                        while (j < n && html[j] != '>') ++j;
                    }
                    // ensure we end at '>'
                    if (j < n && html[j] == '>') {
                        // create node
                        auto node = std::make_unique<HtmlNode>();
                        node->tag = tagName;
                        node->attributes = std::move(attrs);
                        // push into tree
                        stack.back()->children.push_back(std::move(node));
                        HtmlNode* nodePtr = stack.back()->children.back().get();
                        if (!selfClosing) {
                            stack.push_back(nodePtr);
                        } else {
                            // self-closing: do not push
                        }
                        i = j; // move main loop to '>'
                        continue;
                    } else {
                        // malformed tag: skip
                        i = j;
                        continue;
                    }
                }

                // other '<' cases (like <!DOCTYPE>, <?xml ... >) - treat as text or skip
                buffer += c; // treat as literal '<'
                continue;
            } else {
                // normal text accumulation
                buffer += c;
                continue;
            }
        }
        else if (state == State::COMMENT) {
            // accumulate until -->
            if (i + 2 < n && html[i] == '-' && html[i+1] == '-' && html[i+2] == '>') {
                // assign buffer to comment node content
                if (!buffer.empty() && !isBufferEmptyOrWhitespace(buffer)) {
                    auto txt = std::make_unique<HtmlNode>();
                    txt->tag = "#text";
                    txt->content = buffer;
                    stack.back()->children.push_back(std::move(txt));
                }
                buffer.clear();
                // pop comment node
                stack.pop_back();
                state = State::CONTENT;
                i += 2;
                continue;
            } else {
                buffer += c;
                continue;
            }
        }
        else if (state == State::SCRIPT) {
            // we shouldn't reach here because we implement SCRIPT differently; keep for completeness
            buffer += c;
            continue;
        }
        // handle script specially: we detect <script ...> as a normal opening tag above and push node.
        // now if the top node is a script node (case-insensitive), we collect until matching </script>
        if (!stack.empty() && toLower(stack.back()->tag) == "script") {
            // we are inside script content; implement small JS-string-aware scanner
            // collect characters in buffer, but watch for quotes to avoid false </script> in strings
            // We'll run a tiny loop from current i forward until we find a real </script>
            bool inSingle = false, inDouble = false;
            for ( ; i < n; ++i) {
                char ch = html[i];
                buffer += ch;
                if (ch == '\\') {
                    // escape: include next char if any
                    if (i + 1 < n) { buffer += html[i+1]; ++i; }
                    continue;
                }
                if (!inSingle && !inDouble) {
                    // check if closing tag starts here (case-insensitive)
                    if (ch == '<' && i + 8 < n && startsWithCI(html, i, "</script")) {
                        // find '>' after </script
                        size_t gt = html.find('>', i+2);
                        if (gt != std::string::npos) {
                            // everything up to just before '<' belongs to script content (we already appended '<')
                            // store content (excluding the closing tag)
                            // remove the closing sequence from buffer: we appended '<', so buffer contains closing tag too.
                            // cut buffer so that closing tag is not part of content:
                            // simpler approach: we collected entire closing tag into buffer; we now separate:
                            // compute scriptContent by taking current buffer and removing suffix from '<' position
                            // to find position of '<' in buffer: scan backwards
                            size_t posOfLt = std::string::npos;
                            for (size_t k = buffer.size(); k-- > 0;) {
                                if (buffer[k] == '<') { posOfLt = k; break; }
                                if (k == 0) break;
                            }
                            std::string scriptContent;
                            if (posOfLt != std::string::npos) scriptContent = buffer.substr(0, posOfLt);
                            else scriptContent = buffer; // fallback

                            stack.back()->content = scriptContent;
                            buffer.clear();

                            // advance i to the end of the closing tag '>'
                            if (gt < n) i = gt;
                            // pop script node
                            stack.pop_back();
                            state = State::CONTENT;
                            break; // exit inner for loop, continue main loop
                        }
                    }
                }
                // quote handling
                if (!inSingle && ch == '"') inDouble = !inDouble;
                else if (!inDouble && ch == '\'') inSingle = !inSingle;
                // continue collecting
            } // end inner for
            continue;
        }
    } // main for

    // final flush of buffer as text node if any
    if (!buffer.empty() && !isBufferEmptyOrWhitespace(buffer)) {
        auto txt = std::make_unique<HtmlNode>();
        txt->tag = "#text";
        txt->content = buffer;
        stack.back()->children.push_back(std::move(txt));
    }
    buffer.clear();

    return root;
}