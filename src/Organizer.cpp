#include "../include/Organizer.hpp"
#include <iostream>

Organizer::Organizer() {
}

Organizer::~Organizer() {
}

void printTree(const HtmlNode* node, const std::string& prefix = "", bool isLast = true) {
    if (!node) return;

    std::cout << prefix;
    std::cout << (isLast ? "└─" : "├─");
    std::cout << (node->tag.empty() ? "[TEXT]" : node->tag) << "\n";

    std::string newPrefix = prefix + (isLast ? "  " : "│ ");

    for (size_t i = 0; i < node->children.size(); ++i) {
        printTree(node->children[i].get(), newPrefix, i == node->children.size() - 1);
    }
}

std::vector<std::string> Organizer::findHrefs(const HtmlNode* node) {
    std::vector<std::string> hrefs;
    if (!node) return hrefs;

    if (node->tag == "a") {
        auto it = node->attributes.find("href");
        if (it != node->attributes.end()) {
            hrefs.push_back(it->second);
        }
    }

    for (const auto& child : node->children) {
        auto childHrefs = findHrefs(child.get());
        hrefs.insert(hrefs.end(), childHrefs.begin(), childHrefs.end());
    }

    return hrefs;
}

std::string Organizer::findTitle(const HtmlNode* node) {
    if (!node) return "";

    if (node->tag == "title") {
        for (const auto& child : node->children) {
            if (child->tag == "#text") {
                return child->content;
            }
        }
    }

    for (const auto& child : node->children) {
        std::string title = findTitle(child.get());
        if (!title.empty()) {
            return title;
        }
    }

    return "";
}

// Funzione di utilità per trovare un attributo in un nodo
static std::string findAttribute(HtmlNode* node, const std::string& attributeName) {
    auto it = node->attributes.find(attributeName);
    if (it != node->attributes.end()) {
        return it->second;
    }
    return "";
}

// Funzione ricorsiva per la copia profonda di un albero HTML
std::unique_ptr<HtmlNode> Organizer::cloneTree(const HtmlNode* originalNode) {
    // Helper lambda: check whether a link is external (http, //, mailto, javascript:, etc.)
    if (!originalNode) {
        return nullptr;
    }

    auto newNode = std::make_unique<HtmlNode>();
    newNode->tag = originalNode->tag;
    newNode->content = originalNode->content;
    newNode->attributes = originalNode->attributes;

    for (const auto& child : originalNode->children) {
        newNode->children.push_back(cloneTree(child.get()));
    }

    return newNode;
}

// Funzione helper per verificare se un link è esterno
static bool isExternalLink(const std::string& href) {
    // Controlla se il link inizia con un protocollo
    return href.rfind("http://", 0) == 0 ||
           href.rfind("https://", 0) == 0 ||
           href.rfind("ftp://", 0) == 0 ||
           href.rfind("mailto:", 0) == 0 ||
           href.rfind("tel:", 0) == 0;
}

// Funzione di utilità per risolvere un percorso di file
std::string resolvePath(const std::filesystem::path& currentPath, const std::string& href, const std::filesystem::path& rootPath) {
    std::filesystem::path resolved;
    std::error_code ec;

    if (href.front() == '/') {
        resolved = std::filesystem::absolute(rootPath / href.substr(1), ec);
    } else {
        resolved = std::filesystem::absolute(currentPath.parent_path() / href, ec);
    }

    if (ec) {
        return "";
    }
    return resolved.lexically_normal().string();
}

// Funzione ricorsiva per aggiornare i link in un albero HTML
void Organizer::updateHtmlLinks(HtmlNode* node, size_t currentFileIdx, const std::unordered_set<size_t>& visitedIndices) {
    if (!node) {
        return;
    }

    std::string originalHref = findAttribute(node, "href");
    std::string originalSrc = findAttribute(node, "src");

    // Process href attribute
    if (!originalHref.empty()) {
        if (!isExternalLink(originalHref) && originalHref.front() != '#') {
            std::string resolvedKey = resolvePath(htmlFilesPaths[currentFileIdx], originalHref, rootPath);
            if (resolvedKey.empty()) return;

            auto itAbs = absPathToIndex.find(resolvedKey);

            if (itAbs != absPathToIndex.end() && visitedIndices.count(itAbs->second)) {
                std::string targetFilename = htmlFilesPaths[itAbs->second].filename().string();
                std::string currentFilename = htmlFilesPaths[currentFileIdx].filename().string();
                
                if (targetFilename == "index.html" && currentFilename != "index.html") {
                    node->attributes["href"] = "../index.html"; // Riferimento corretto per l'index
                } else if (currentFilename == "index.html") {
                    node->attributes["href"] = "html/" + targetFilename;
                } else {
                    node->attributes["href"] = targetFilename;
                }
            }
        }
    }

    // Process src attribute
    if (!originalSrc.empty()) {
        if (!isExternalLink(originalSrc) && originalSrc.front() != '#') {
            std::string resolvedKey = resolvePath(htmlFilesPaths[currentFileIdx], originalSrc, rootPath);
            if (resolvedKey.empty()) return;

            auto itAbs = absPathToIndex.find(resolvedKey);

            if (itAbs != absPathToIndex.end() && visitedIndices.count(itAbs->second)) {
                std::string targetFilename = htmlFilesPaths[itAbs->second].filename().string();
                
                // Assuming all assets go into a new 'assets' directory
                node->attributes["src"] = "assets/" + targetFilename;
            }
        }
    }

    // Process children recursively
    for (const auto& child : node->children) {
        updateHtmlLinks(child.get(), currentFileIdx, visitedIndices);
    }
}
/* API */

void Organizer::organize(const std::filesystem::path& rootPath) {

    this -> rootPath = rootPath;
    htmlFilesPaths = directoryManager.findFiles(rootPath, ".html");
    cssFilesPaths  = directoryManager.findFiles(rootPath, ".css");

    // Clear previous data
    htmlFiles.clear();
    cssFiles.clear();
    htmlTrees.clear();

    // Mapping file absolute path to their index in htmlFilesPaths
    absPathToIndex.reserve(htmlFilesPaths.size());

    // Read file contents and populate htmlFiles vector preserving the same order as htmlFilesPaths
    for (size_t i = 0; i < htmlFilesPaths.size(); ++i) {
        const auto& p = htmlFilesPaths[i];

        // Normalize to absolute path string to use as key for lookups later
        std::filesystem::path abs = std::filesystem::absolute(p);
        std::string absKey = abs.lexically_normal().string();
        absPathToIndex[absKey] = i;

        std::string name = p.stem().string();
        std::string ext  = p.extension().string();
        std::string content = fileManager.copyContent(p.string());

        // Keep FileData aligned by index with htmlFilesPaths.
        htmlFiles.push_back(FileData{name, content, ext});
    }

    // 2) Parse each HTML into a tree and save the tree aligned by index
    for (const auto& file : htmlFiles) {
        auto tree = parser.parseHTML(file.content);
        if (tree) {
            htmlTrees.push_back(std::move(tree));
        } else {
            htmlTrees.push_back(nullptr);
            std::cerr << "Warning: Failed to parse HTML file: " << file.name << file.extension << "\n";
        }
    }

    printTree(htmlTrees[0].get());
    // 3) Prepare Organized root directory
    std::filesystem::path organizedRoot = rootPath / "Organized";
    directoryManager.createDirectory(rootPath, "Organized");

    /* HELPERS */

    // Helper lambda: check whether a link is external (http, //, mailto, javascript:, etc.)
    auto isExternalLink = [](const std::string& href) -> bool {
        if (href.empty()) return true;
        // common schemes that are not local files
        const std::vector<std::string> schemes = {"http://", "https://", "//", "mailto:", "javascript:"};
        for (const auto& s : schemes) {
            if (href.rfind(s, 0) == 0) return true; // starts with scheme
        }
        return false;
    };

    // Helper lambda: normalize a href string by stripping fragment and query (optional)
    auto normalizeHrefString = [](const std::string& href) -> std::string {
        // remove fragment (#...) and query (?...) for file matching
        auto posHash = href.find('#');
        auto posQ = href.find('?');
        size_t cut = std::string::npos;
        if (posHash != std::string::npos && posQ != std::string::npos) cut = std::min(posHash, posQ);
        else if (posHash != std::string::npos) cut = posHash;
        else if (posQ != std::string::npos) cut = posQ;
        if (cut != std::string::npos) return href.substr(0, cut);
        return href;
    };

    /* END HELPERS */

    /* THIS IS USEFUL IN CASE SOME FILES ARE NOT PROPERLY LINKED */
    std::unordered_map<std::string, size_t> filenameKeyToIndex;
    filenameKeyToIndex.reserve(htmlFilesPaths.size());
    for (size_t i = 0; i < htmlFilesPaths.size(); i++) {
        filenameKeyToIndex[htmlFilesPaths[i].filename().string()] = i;
    }

    // 5) For each index.html create site directory, copy index.html, then copy reachable html files in html subdir
    int nameSuffix = 1;
    for (size_t i = 0; i < htmlFilesPaths.size(); i++) {
        if (htmlFilesPaths[i].filename() != "index.html") continue;
        if (!htmlTrees[i]) {
            std::cerr << "Skipping index.html at " << htmlFilesPaths[i] << " due to parse error.\n";
            continue;
        }
        std::string title = findTitle(htmlTrees[i].get());

        if(title.empty()) title = "Site";
        std::string siteDirName = title;

        // Ensure unique directory name
        std::filesystem::path siteDirPath = organizedRoot / siteDirName;
        while (std::filesystem::exists(siteDirPath)) {
            siteDirName = title + "_" + std::to_string(nameSuffix++);
            siteDirPath = organizedRoot / siteDirName;
        }

        directoryManager.createDirectory(organizedRoot, siteDirName);
        directoryManager.createDirectory(siteDirPath, "html");

        // Find all reachable HTML files from this index.html
        std::unordered_set<size_t> reachableIndices;
        std::vector<std::string> hrefs = findHrefs(htmlTrees[i].get());

        // Transform hrefs into indices
        std::queue<size_t> queue;
        std::unordered_set<size_t> visited;

        queue.push(i);
        visited.insert(i);

        while (!queue.empty()) {
            size_t curIdx = queue.front();
            queue.pop();

            if (!htmlTrees[curIdx]) continue;
            auto hrefs = findHrefs(htmlTrees[curIdx].get());

            for (const auto& rawHref : hrefs) {
                if (isExternalLink(rawHref)) continue;
                std::string href = normalizeHrefString(rawHref);
                if (href.empty()) continue;

                std::filesystem::path resolved;
                std::error_code ec;

                if (href.front() == '/') {
                    resolved = std::filesystem::absolute(rootPath / href.substr(1), ec);
                } else {
                    resolved = std::filesystem::absolute(htmlFilesPaths[curIdx].parent_path() / href, ec);
                }

                if (ec) {
                    //std::cerr << "Warning: Failed to resolve path '" << href << "' from " << htmlFilesPaths[curIdx] << "\n";
                    continue;
                }

                std::string resolvedKey = resolved.lexically_normal().string();
                auto itAbs = absPathToIndex.find(resolvedKey);
                if (itAbs != absPathToIndex.end()) {
                    size_t foundIdx = itAbs->second;
                    if (visited.find(foundIdx) == visited.end()) {
                        visited.insert(foundIdx);
                        queue.push(foundIdx);
                    }
                    continue;
                }

                auto itName = filenameKeyToIndex.find(std::filesystem::path(href).filename().string());
                if (itName != filenameKeyToIndex.end()) {
                    size_t foundIdx = itName->second;
                    if (visited.find(foundIdx) == visited.end()) {
                        visited.insert(foundIdx);
                        queue.push(foundIdx);
                    }
                    continue;
                }

                //std::cerr << "Warning: Could not resolve link '" << href << "' in file " << htmlFilesPaths[curIdx] << "\n";
            }
        }

        reachableIndices = visited;

        // modifie index.html and copy to site directory
        /* 
            HANDLE CSS AND HTML LINKS
        */

        // Rebuild string from html tree
        std::unique_ptr<HtmlNode> modifiedIndexTree = cloneTree(htmlTrees[i].get());

        // Update links in the cloned tree
        updateHtmlLinks(modifiedIndexTree.get(), i, reachableIndices);

        // Write modified index.html
        std::string rebuiltIndexContent = htmlBuilder.buildHtml(*modifiedIndexTree);
        
        // Put index.html in site directory
        fileManager.createFile(siteDirPath / "index.html", rebuiltIndexContent);

        // Copy reachable HTML files into html subdirectory
        for (const auto& idx : reachableIndices) {
            if (idx == i) continue; // skip index.html, already handled

            if (!htmlTrees[idx]) {
                std::cerr << "Skipping HTML file at " << htmlFilesPaths[idx] << " due to parse error.\n";
                continue;
            }

            // Clone and update links
            std::unique_ptr<HtmlNode> modifiedTree = cloneTree(htmlTrees[idx].get());
            updateHtmlLinks(modifiedTree.get(), idx, reachableIndices);

            // Rebuild content
            std::string rebuiltContent = htmlBuilder.buildHtml(*modifiedTree);

            // Write to html subdirectory
            fileManager.createFile(siteDirPath / "html" / htmlFilesPaths[idx].filename(), rebuiltContent);
        }
    }
}