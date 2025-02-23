#ifndef LOUDS_H
#define LOUDS_H

#include "rank_counter.h"
#include "select_counter.h"

#include <cstdint>
#include <iostream>
#include <vector>
#include <list>

struct TreeNode
{
	TreeNode *parent;
	TreeNode *leftSon;
	TreeNode *rightBro;
};

/////////////////////////////////////////

template <class rankCounter, class selectCounter>
class LOUDS
{
public:
	LOUDS() {};
	~LOUDS() {};

	void treeToLOUDS(const TreeNode &root);
	uint32_t numOfBits();
	void printBits();

	int32_t select(uint8_t desired, uint32_t entryNum);
	uint32_t rank(uint8_t desired, uint32_t pos);

	int32_t firstChild(int32_t i);
	int32_t lastChild(int32_t i);
	int32_t childrenCount(int32_t i);
	int32_t parent(int32_t i);

	size_t size() { return m_louds.size() + _rankCounter.size() + _selectCounter.size(); };

	rankCounter _rankCounter;
	selectCounter _selectCounter;

	std::vector<uint8_t> m_louds;
};

#endif