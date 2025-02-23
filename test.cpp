#include <iostream>
#include <vector>
#include <cstdint>
#include <gtest/gtest.h>

#include "louds.h"
#include <list>

void randTreeGen(std::vector<TreeNode> &Tree, int numV)
{
	Tree.assign(numV, {nullptr, nullptr, nullptr});
	for (size_t i = 1; i < numV; i++)
	{
		uint32_t rnd = 0;
		bool sonBro = false;

		if (i > 1)
		{
			while (rnd == 0)
			{
				rnd = uint32_t(rand());
				rnd = rnd % (i);
			}

			sonBro = rand() % 2;
		}

		TreeNode *cur = &(Tree[rnd]);

		if (sonBro)
		{
			while (cur->rightBro != nullptr)
				cur = cur->rightBro;

			cur->rightBro = &(Tree[i]);
			cur->rightBro->parent = cur->parent;
		}
		else
		{
			while (cur->leftSon != nullptr)
				cur = cur->leftSon;

			cur->leftSon = &(Tree[i]);
			cur->leftSon->parent = cur;
		}
	}
}

void randTreeGen2(std::vector<TreeNode> &Tree, int numV)
{
	Tree.assign(numV, {nullptr, nullptr, nullptr});

	size_t counterOfGeneratedNodes = 0;

	std::list<size_t> q;

	q.push_back(0);

	counterOfGeneratedNodes++;

	while ((q.empty() == false) && (counterOfGeneratedNodes < numV))
	{
		auto curVertId = q.front();

		q.pop_front();

		size_t degree = 1 + (rand() % 4);

		for (size_t i = 1; (i <= degree) && (counterOfGeneratedNodes < numV); i++)
		{
			Tree[counterOfGeneratedNodes].parent = &(Tree[curVertId]);
			if (i == 1)
				Tree[curVertId].leftSon = &(Tree[counterOfGeneratedNodes]);

			if ((i != degree) && (counterOfGeneratedNodes + 1 < Tree.size()))
				Tree[counterOfGeneratedNodes].rightBro = &(Tree[counterOfGeneratedNodes + 1]);

			q.push_back(counterOfGeneratedNodes);

			counterOfGeneratedNodes++;
		}
	}
}

void nDegreeTreeGen(std::vector<TreeNode> &Tree, size_t degree, int numV)
{
	Tree.assign(numV, {nullptr, nullptr, nullptr});

	size_t counterOfGeneratedNodes = 0;

	std::list<size_t> q;

	q.push_back(0);

	counterOfGeneratedNodes++;

	while ((q.empty() == false) && (counterOfGeneratedNodes < numV))
	{
		auto curVertId = q.front();

		q.pop_front();

		for (size_t i = 1; (i <= degree) && (counterOfGeneratedNodes < numV); i++)
		{
			Tree[counterOfGeneratedNodes].parent = &(Tree[curVertId]);
			if (i == 1)
				Tree[curVertId].leftSon = &(Tree[counterOfGeneratedNodes]);

			if ((i != degree) && (counterOfGeneratedNodes + 1 < Tree.size()))
				Tree[counterOfGeneratedNodes].rightBro = &(Tree[counterOfGeneratedNodes + 1]);

			q.push_back(counterOfGeneratedNodes);

			counterOfGeneratedNodes++;
		}
	}
}

void recPrintTree(TreeNode *root, std::string &printString, std::string &broString)
{
	if (root != nullptr)
	{
		if (root->leftSon == nullptr)
		{
			std::cout << printString << "O\n";

			if (broString.find('|') != std::string::npos)
				std::cout << broString << "\n";

			return;
		}

		std::string tempPrintString = printString;

		TreeNode *son = root->leftSon;
		printString.append("O ------> ");
		broString.append("          ");

		if (root->leftSon->rightBro != nullptr)
			broString.back() = '|';

		while (son != nullptr)
		{
			if (son->rightBro == nullptr)
				broString.back() = ' ';

			recPrintTree(son, printString, broString);
			printString = broString.substr(0, printString.length());

			// printString[printString.length() - 1 - std::string(" ------> O").length()] = '|';

			son = son->rightBro;
		}

		broString = broString.substr(0, broString.length() - std::string("          ").length());

		printString = tempPrintString;
	}
}

void printTree(TreeNode *root)
{
	std::string printString = "";
	std::string broString = " ";
	std::cout << "\n";
	recPrintTree(root, printString, broString);
}

TEST(RankTests, RankCorrectness)
{
	srand(time(NULL));

	uint32_t numberOfBits = 100;
	std::vector<uint8_t> bits;

	bits.assign(sizeof(numberOfBits) + (numberOfBits + 7) / 8, 0);

	*(uint32_t *)&(bits[0]) = numberOfBits;

	for (size_t i = 0; i < numberOfBits + 10; i++)
	{
		bits[sizeof(numberOfBits) + i / 8] |= uint8_t((rand() % 2)) << (i % 8);
	}

	linearRankCounter linRank;
	linRank.prepare(&bits);

	blockRankCounter blockRank;
	blockRank.prepare(&bits);

	for (size_t i = 0; i < numberOfBits + 10; i++)
	{
		auto lin = linRank.rank(1, i);
		auto block = blockRank.rank(1, i);

		EXPECT_EQ(lin, block);

		lin = linRank.rank(0, i);
		block = blockRank.rank(0, i);

		EXPECT_EQ(lin, block);
	}
}

TEST(SelectTests, SelectCorrectness)
{
	srand(time(NULL));

	uint32_t numberOfBits = 10000;
	std::vector<uint8_t> bits;

	bits.assign(sizeof(numberOfBits) + (numberOfBits + 7) / 8, 0);

	*(uint32_t *)&(bits[0]) = numberOfBits;

	for (size_t i = 0; i < numberOfBits + 10; i++)
	{
		bits[sizeof(numberOfBits) + i / 8] |= uint8_t((rand() % 2)) << (i % 8);
	}

	linearSelectCounter linSelect;
	linSelect.prepare(&bits);

	blockSelectCounter blockSelect;
	blockSelect.prepare(&bits);

	for (size_t i = 0; i < numberOfBits + 10; i++)
	{
		auto lin = linSelect.select(1, i + 1);
		auto block = blockSelect.select(1, i + 1);

		EXPECT_EQ(lin, block);

		lin = linSelect.select(0, i + 1);
		block = blockSelect.select(0, i + 1);

		EXPECT_EQ(lin, block);
	}
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}
