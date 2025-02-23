#include "louds.h"
#include <iostream>
#include <vector>
#include <cstdint>
#include <list>
#include <chrono>

template class LOUDS<linearRankCounter, linearSelectCounter>;
template class LOUDS<linearRankCounter, blockSelectCounter>;
template class LOUDS<blockRankCounter, linearSelectCounter>;
template class LOUDS<blockRankCounter, blockSelectCounter>;

/////////////////////////////////////////
template <class rankCounter, class selectCounter>
void LOUDS<rankCounter, selectCounter>::treeToLOUDS(const TreeNode &root)
{
    std::list<TreeNode> q;
    std::vector<uint32_t> numOfChildren;

    q.push_back(root);

    while (q.empty() == false)
    {
        auto curVert = q.front();

        numOfChildren.push_back(0);

        q.pop_front();

        // uint32_t numOfCurVertChildren = 0;

        auto curVertP = curVert.leftSon;

        while (curVertP != nullptr)
        {
            (*numOfChildren.rbegin())++;

            q.push_back(*curVertP);

            curVertP = curVertP->rightBro;
        }
    }

#ifdef DEBUG
    for (size_t i = 0; i < numOfChildren.size(); i++)
        std::cout << i << ": " << numOfChildren[i] << '\n';
#endif // DEBUG

    uint32_t bitsRequired = 2 * numOfChildren.size() + 1;
    uint32_t bitsPassed = 0;

    m_louds.clear();
    m_louds.resize(sizeof(bitsRequired) + (bitsRequired + 7) / 8);

    *((uint32_t *)&m_louds[0]) = bitsRequired;
    bitsPassed += sizeof(bitsRequired) * 8;

    if (bitsRequired > 1)
    {
        m_louds[bitsPassed / 8] |= (uint8_t(1) << (bitsPassed % 8));
        bitsPassed++; // 1
        bitsPassed++; // 0

        for (size_t i = 0; i < numOfChildren.size(); i++)
        {
            for (size_t j = 1; j <= numOfChildren[i]; j++)
            {
                m_louds[bitsPassed / 8] |= (uint8_t(1) << (bitsPassed % 8));
                bitsPassed++; // 1
            }
            bitsPassed++; // 0
        }
    }

    _rankCounter.prepare(&m_louds);
    _selectCounter.prepare(&m_louds);
}

template <class rankCounter, class selectCounter>
uint32_t LOUDS<rankCounter, selectCounter>::numOfBits()
{
    if (m_louds.size() < sizeof(uint32_t))
        return 0;

    return *((uint32_t *)&m_louds[0]);
}

template <class rankCounter, class selectCounter>
void LOUDS<rankCounter, selectCounter>::printBits()
{
    std::cout << numOfBits() << " bits: ";

    if (numOfBits() > 0)
    {
        for (size_t i = sizeof(numOfBits()) * 8; i < m_louds.size() * 8; i++)
        {
            if (i == numOfBits() + sizeof(numOfBits()) * 8)
                std::cout << '|';

            std::cout << (m_louds[i / 8] >> (i % 8)) % 2;
        }
    }

    std::cout << "\n";
}

template <class rankCounter, class selectCounter>
int32_t LOUDS<rankCounter, selectCounter>::select(uint8_t desired, uint32_t entryNum)
{
    return _selectCounter.select(desired, entryNum);
}

template <class rankCounter, class selectCounter>
uint32_t LOUDS<rankCounter, selectCounter>::rank(uint8_t desired, uint32_t pos)
{
    return _rankCounter.rank(desired, pos);
}

template <class rankCounter, class selectCounter>
int32_t LOUDS<rankCounter, selectCounter>::firstChild(int32_t i)
{
    auto selectRes = select(0, i + 1);

    // auto numberOfBits = numOfBits();

    // if (selectRes == -1)
    //     return -1;

    // if (selectRes + 1 < numberOfBits)
    //     if ((m_louds[sizeof(numberOfBits) + (selectRes + 1) / 8] >> ((sizeof(numberOfBits) * 8 + (selectRes + 1)) % 8)) == 0) // first bit after previous node is 0 => no children
    //         return -1;

    return selectRes - i;
}

template <class rankCounter, class selectCounter>
int32_t LOUDS<rankCounter, selectCounter>::lastChild(int32_t i)
{
    auto lastChildPos = select(0, i + 2) - 1;

    // auto numberOfBits = numOfBits();

    // if (lastChildPos == -1)
    //     return -1;

    // if ((m_louds[sizeof(numberOfBits) + lastChildPos / 8] >> ((sizeof(numberOfBits) * 8 + lastChildPos) % 8)) == 0) // 0 in the pos of last children => no last children
    //     return -1;

    return rank(1, lastChildPos - 1);
}

template <class rankCounter, class selectCounter>
int32_t LOUDS<rankCounter, selectCounter>::childrenCount(int32_t i)
{
    auto firstChildCur = firstChild(i);
    auto firstChildNext = firstChild(i + 1);

    // if (firstChildCur != -1)
    //     return 0;

    return firstChildNext - firstChildCur;
}

template <class rankCounter, class selectCounter>
int LOUDS<rankCounter, selectCounter>::parent(int32_t i)
{
    return rank(0, select(1, (i + 1)) - 1) - 1;
}
