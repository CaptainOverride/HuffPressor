#include "huffmanTree.h"
#include <queue>

HuffmanNode::HuffmanNode(unsigned char b, int freq)
    : byte(b), frequency(freq), left(nullptr), right(nullptr) {}

HuffmanNode::HuffmanNode(int freq, HuffmanNode* l, HuffmanNode* r)
    : byte(0), frequency(freq), left(l), right(r) {}

bool HuffmanNode::isLeaf() const {
    return !left && !right;
}

void HuffmanTree::build(const std::unordered_map<unsigned char, int>& freqMap) {
    auto cmp = [](HuffmanNode* a, HuffmanNode* b) {
        return a->frequency > b->frequency;
    };
    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, decltype(cmp)> pq(cmp);

    for (const auto& [byte, freq] : freqMap) {
        pq.push(new HuffmanNode(byte, freq));
    }

    while (pq.size() > 1) {
        HuffmanNode* left = pq.top(); pq.pop();
        HuffmanNode* right = pq.top(); pq.pop();
        HuffmanNode* parent = new HuffmanNode(left->frequency + right->frequency, left, right);
        pq.push(parent);
    }

    root = pq.empty() ? nullptr : pq.top();

    if (root) generateCodes();
}

void HuffmanTree::generateCodes(HuffmanNode* node, const std::string& currentCode) {
    if (!node) return;

    if (node->isLeaf()) {
        codes[node->byte] = currentCode;
        return;
    }

    generateCodes(node->left, currentCode + "0");
    generateCodes(node->right, currentCode + "1");
}

void HuffmanTree::generateCodes() {
    codes.clear();
    generateCodes(root, "");
}

const std::unordered_map<unsigned char, std::string>& HuffmanTree::getHuffmanCodes() const {
    return codes;
}

HuffmanNode* HuffmanTree::getRoot() const {
    return root;
}

HuffmanTree::~HuffmanTree() {
    destroyTree(root);
}

void HuffmanTree::destroyTree(HuffmanNode* node) {
    if (!node) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
}
