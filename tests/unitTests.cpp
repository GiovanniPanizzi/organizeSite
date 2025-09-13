#include "../include/Organizer.hpp"
#include <iostream>

void printTag(HtmlNode* node, int indent = 0) {
    if (!node) return;
    for (int i = 0; i < indent; i++) std::cout << "  ";
    std::cout << "Tag: " << node->tag;
    if (!node->attributes.empty()) {
        std::cout << " (Attrs: ";
        for (const auto& attr : node->attributes) {
            std::cout << attr.first << "=\"" << attr.second << "\" ";
        }
        std::cout << ")";
    }
    std::cout << "\n";
    for (const auto& child : node->children) {
        printTag(child.get(), indent + 1);
    }
}

int main() {
    Organizer testingOrganizer;
    Parser testingParser;
    HtmlBuilder testingHtmlBuilder;

    /* PARSER */
    std::unique_ptr<HtmlNode> root = testingParser.parseHTML("<!DOCTYPE html><html lang=\"it\"><head></head><body><header></header><main id=\"main\"></main><footer></footer></body></html>");
    assert(root.get() != nullptr);
    assert(root.get()->tag == "root");
    std::cout << "Test 1: Il nodo radice e' corretto (#root). Passato.\n";

    // 3. Asserzione sul primo figlio di root (il nodo <html>)
    assert(root.get()->children.size() == 1);
    HtmlNode* htmlNode = root.get()->children[0].get();
    assert(htmlNode->tag == "html");
    std::cout << "Test 2: Il primo figlio di #root e' il tag <html>. Passato.\n";

    // 4. Asserzione sull'attributo lang del tag html
    assert(htmlNode->attributes.at("lang") == "it");
    std::cout << "Test 3: L'attributo 'lang' e' corretto. Passato.\n";

    // 5. Asserzione sui nodi figli diretti di html
    // Ci aspettiamo due figli: <head> e <body>
    assert(htmlNode->children.size() == 2);
    assert(htmlNode->children[0]->tag == "head");
    assert(htmlNode->children[1]->tag == "body");
    std::cout << "Test 4: I figli di 'html' (<head> e <body>) sono corretti. Passato.\n";

    // 6. Asserzioni sui figli del tag body
    HtmlNode* bodyNode = htmlNode->children[1].get();
    assert(bodyNode->children.size() == 3);
    assert(bodyNode->children[0]->tag == "header");
    assert(bodyNode->children[1]->tag == "main");
    assert(bodyNode->children[2]->tag == "footer");
    std::cout << "Test 5: I figli di 'body' (<header>, <main>, <footer>) sono corretti. Passato.\n";

    // 7. Asserzione sull'attributo 'id' del tag main
    HtmlNode* mainNode = bodyNode->children[1].get();
    assert(mainNode->attributes.at("id") == "main");
    std::cout << "Test 6: L'attributo 'id' del tag 'main' e' corretto. Passato.\n";

    std::cout << "\nTutti i test sono passati! Il parser funziona come previsto.\n";

    // Per il debug, puoi decommentare la linea seguente per vedere l'albero
    // printTag(root.get());

    return 0;





    return 0;
}