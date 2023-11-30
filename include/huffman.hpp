#pragma once

#include <string>
#include <vector>
#include <map>

struct Node {
	int data, freq;
	Node *l, *r;

	Node(int data, int freq, Node *l, Node *r);
};

class NodeCompare {
public:
	bool operator()(Node *a, Node *b);
};

void get_tree_info(Node *node, int depth, std::map<int, std::vector<int>> &info);
void cleanup(Node *node);
std::vector<int> huffman_encode(std::vector<int> &freq);