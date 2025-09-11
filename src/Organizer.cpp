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

void Organizer::organize(const std::filesystem::path& rootPath) {
    // 1) Find files (recursive) and read contents
    htmlFilesPaths = directoryManager.findFiles(rootPath, ".html");
    cssFilesPaths  = directoryManager.findFiles(rootPath, ".css");

    htmlFiles.clear();
    cssFiles.clear();
    htmlTrees.clear();

    // Map: normalized absolute path string -> index in htmlFiles/htmlFilesPaths/htmlTrees
    std::unordered_map<std::string, size_t> absPathToIndex;
    absPathToIndex.reserve(htmlFilesPaths.size());

    // Read file contents and populate htmlFiles vector preserving the same order as htmlFilesPaths.
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
            // store ownership of the tree; htmlTrees[i] corresponds to htmlFiles[i] / htmlFilesPaths[i]
            htmlTrees.push_back(std::move(tree));
        } else {
            // push a nullptr to keep indices aligned (optional)
            htmlTrees.push_back(nullptr);
        }
    }

    // 3) Prepare Organized root directory
    std::filesystem::path organizedRoot = rootPath / "Organized";
    directoryManager.createDirectory(rootPath, "Organized");

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

    // 4) Build an index map for fast file lookups by absolute path
    // (already built as absPathToIndex). For convenience, we also create a map from filename-like keys
    // (relative filename) to index — but prefer absolute matching to avoid collisions.
    std::unordered_map<std::string, size_t> filenameKeyToIndex;
    filenameKeyToIndex.reserve(htmlFilesPaths.size());
    for (size_t i = 0; i < htmlFilesPaths.size(); ++i) {
        // use filename (e.g. "foo.html") as a fallback key
        filenameKeyToIndex[htmlFilesPaths[i].filename().string()] = i;
    }

    // 5) For each index.html create site directory, copy index.html, then copy reachable html files
    int nameSuffix = 1;
    for (size_t i = 0; i < htmlFilesPaths.size(); ++i) {
        if (htmlFilesPaths[i].filename() != "index.html") continue; // only for index.html files

        // Find a title for the site: if no <title> found, fallback to SiteN
        std::string title = findTitle(htmlTrees[i].get());
        std::string finalDirName = title.empty() ? ("Site" + std::to_string(nameSuffix++)) : title;

        // compute site path and create site folder
        std::filesystem::path sitePath = organizedRoot / finalDirName;
        directoryManager.createDirectory(organizedRoot, finalDirName);

        // create the index.html file inside the site folder (use original content)
        fileManager.createFile((sitePath / "index.html").string(), htmlFiles[i].content);

        // create html subfolder (only once per site)
        directoryManager.createDirectory(sitePath, "html");
        std::filesystem::path htmlSubfolder = sitePath / "html";

        // BFS over HTML files reachable from this index
        std::vector<size_t> queue;
        std::unordered_set<size_t> visited;
        queue.push_back(i);
        visited.insert(i);

        // while queue not empty, pop front (use index-based queue)
        for (size_t qi = 0; qi < queue.size(); ++qi) {
            size_t curIdx = queue[qi];

            // Get hrefs from the parsed tree of current file (skip if no tree)
            if (!htmlTrees[curIdx]) continue;
            auto hrefs = findHrefs(htmlTrees[curIdx].get());

            // resolve each href and decide whether it maps to one of our htmlFilesPaths
            for (const auto& rawHref : hrefs) {
                // skip fragments, empty hrefs, and external URIs
                if (rawHref.empty()) continue;
                if (rawHref.front() == '#') continue;
                if (isExternalLink(rawHref)) continue;

                // normalize href (strip query/fragment)
                std::string href = normalizeHrefString(rawHref);

                // Resolve relative href against current file's parent directory
                std::filesystem::path resolved;
                if (href.empty()) continue;

                std::error_code ec;
                // If href looks absolute path (starts with '/'), treat as root-relative to rootPath
                if (!href.empty() && href.front() == '/') {
                    // treat as relative to the project root (rootPath)
                    resolved = std::filesystem::absolute(rootPath / href.substr(1), ec);
                } else {
                    // treat as relative to the current html file parent directory
                    std::filesystem::path parent = htmlFilesPaths[curIdx].parent_path();
                    resolved = std::filesystem::absolute(parent / href, ec);
                }

                if (ec) {
                    // If resolving failed, try fallback: treat as simple filename
                    auto itF = filenameKeyToIndex.find(href);
                    if (itF != filenameKeyToIndex.end()) {
                        size_t foundIdx = itF->second;
                        if (!visited.count(foundIdx)) {
                            visited.insert(foundIdx);
                            queue.push_back(foundIdx);
                        }
                    }
                    continue;
                }

                // Normalize lexical form for matching
                std::string resolvedKey = resolved.lexically_normal().string();

                // Try exact absolute path match first (best)
                auto itAbs = absPathToIndex.find(resolvedKey);
                if (itAbs != absPathToIndex.end()) {
                    size_t foundIdx = itAbs->second;
                    if (!visited.count(foundIdx)) {
                        visited.insert(foundIdx);
                        queue.push_back(foundIdx);
                    }
                    continue;
                }

                // Fallback: try matching by filename only (e.g. href "foo.html")
                auto itName = filenameKeyToIndex.find(std::filesystem::path(href).filename().string());
                if (itName != filenameKeyToIndex.end()) {
                    size_t foundIdx = itName->second;
                    if (!visited.count(foundIdx)) {
                        visited.insert(foundIdx);
                        queue.push_back(foundIdx);
                    }
                    continue;
                }

                // If nothing matched, we ignore that href (probably external or missing file)
            } // end for hrefs
        } // end BFS queue

        // Now 'visited' contains indices of reachable HTML files (including index itself)
        // Copy each reachable file into sitePath/html/
        for (size_t idx : visited) {
            // skip the index file if you want to keep it only at site root.
            if (idx == i) {
                // index is already written to sitePath/index.html
                continue;
            }

            // target filename: use original filename (preserve duplicates risk; you may rename to avoid collisions)
            std::string filename = htmlFilesPaths[idx].filename().string();
            std::filesystem::path outFile = htmlSubfolder / filename;

            // Write file content (we use FileData content stored earlier)
            fileManager.createFile(outFile.string(), htmlFiles[idx].content);
        }

        // finished with this index.html site
    } // end for each index.html
}