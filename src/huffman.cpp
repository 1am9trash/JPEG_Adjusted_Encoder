#include <iostream>
#include <queue>

#include "huffman.hpp"

Node::Node(int data, int freq, Node *l=nullptr, Node *r=nullptr){
	this->data = data;
	this->freq = freq;
	this->l = l;
	this->r = r;
}

bool NodeCompare::operator()(Node *a, Node *b) {
	return (a->freq > b->freq);
}

void get_tree_info(Node *node, int depth, std::map<int, std::vector<int>> &info) {
	if (node != nullptr) {
		if (node->data != -1) {
			if (info.count(depth) == 0) {
				info[depth] = std::vector<int>(0);
			}
			info[std::max(depth, 1)].push_back(node->data);
		}
		get_tree_info(node->l, depth + 1, info);
		get_tree_info(node->r, depth + 1, info);
	}
}

void cleanup(Node *node){
	if (node){
		cleanup(node->l);
		cleanup(node->r);

		delete node;
	}
}

std::vector<int> huffman_encode(std::vector<int> &freq) {
	Node *l, *r, *top;
	std::priority_queue<Node *, std::vector<Node *>, NodeCompare> q;

	for (int i = 0; i < freq.size(); i++) {
		if (freq[i] == 0) {
			continue;
		}
        q.push(new Node(i, freq[i]));
    }

	if (q.size() == 0) {
		return std::vector<int>(16, 0);
	}

	while (q.size() > 1) {
		l = q.top();
		q.pop();

		r = q.top();
		q.pop();

		top = new Node(-1, l->freq + r->freq, l, r);
		q.push(top);
	}

	std::map<int, std::vector<int>> huffman_info;
	get_tree_info(q.top(), 0, huffman_info);

	std::vector<int> huffman_table(16, 0);
	for (auto &i: huffman_info) {
		huffman_table[std::min(i.first, 15)] += i.second.size();
		for (int j = 0; j < i.second.size(); j++) {
			huffman_table.push_back(i.second[j]);
		}
	}

	cleanup(q.top());

	return huffman_table;
}