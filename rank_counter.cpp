#include "rank_counter.h"
#include <vector>
#include <list>
#include <iostream>

/////////////////////////////////////////

void linearRankCounter::prepare(std::vector<uint8_t> *ptrToData)
{
    _ptrToBits = ptrToData;
}

uint32_t linearRankCounter::rank(uint8_t desired, uint32_t pos)
{
    // pos--; // numering from 1, not from 0
    uint32_t numberOfBits = (_ptrToBits->size() >= sizeof(uint32_t)) ? (*((uint32_t *)&((*_ptrToBits)[0]))) : 0;
    uint32_t founded = 0;

    for (size_t i = 0; i < std::min(numberOfBits, pos + 1); i++)
    {
        if (((*_ptrToBits)[sizeof(numberOfBits) + i / 8] >> ((sizeof(numberOfBits) * 8 + i) % 8)) % 2 == desired)
            founded++;
    }

    return founded;
}

size_t linearRankCounter::size()
{
    return (sizeof(_ptrToBits));
}

/////////////////////////////////////////

uint32_t blockRankCounter::log2Upper(uint32_t x)
{
    size_t res = 0;
    uint32_t binCounter = 1;

    while (x > binCounter)
    {
        res++;
        binCounter = (binCounter << 1);
    }

    return res;
}

void blockRankCounter::prepare(std::vector<uint8_t> *ptrToData)
{
    _ptrToBits = ptrToData;

    uint32_t numberOfBits = (_ptrToBits->size() >= sizeof(uint32_t)) ? (*((uint32_t *)&((*_ptrToBits)[0]))) : 0;

    size_t log2OfBitsNumber = log2Upper(numberOfBits);
    _numOfBitsInSuperBlockCounter = log2OfBitsNumber;
    size_t numOfBitsInSuperBlock = (log2OfBitsNumber * log2OfBitsNumber / 2);
    size_t numberOfSuperBlocks = (numberOfBits + numOfBitsInSuperBlock - 1) / numOfBitsInSuperBlock;

    _superBlocksCounters.assign((_numOfBitsInSuperBlockCounter * numberOfSuperBlocks + 7) / 8, 0);

    size_t log2OfLog2OfBitsNumber = log2Upper(log2OfBitsNumber);
    _numOfBitsInBlockCounter = 2 * log2OfLog2OfBitsNumber;
    //_numOfBitsInBlockCounter = log2OfLog2OfBitsNumber;
    size_t numOfBitsInBlock = log2OfBitsNumber / 2;
    size_t numberOfBlocks = numberOfSuperBlocks * (numOfBitsInSuperBlock / numOfBitsInBlock);
    _blocksCounters.assign((_numOfBitsInBlockCounter * numberOfBlocks + 7) / 8, 0);

    size_t numOf1 = 0;
    size_t numOf1InStartOfSuperBlock = 0;
    size_t passedBitsForSuperBlocks = 0;
    size_t passedBitsForBlocks = 0;

    for (size_t i = 0; i < numberOfBits; i++)
    {
        if (i % (numOfBitsInSuperBlock) == 0)
        {
            numOf1InStartOfSuperBlock = numOf1;

            for (size_t j = 0; j < _numOfBitsInSuperBlockCounter; j++)
            {
                _superBlocksCounters[passedBitsForSuperBlocks / 8] |= (uint8_t((numOf1 >> j) % 2) << (passedBitsForSuperBlocks % 8));
                passedBitsForSuperBlocks++;
            }
        }

        if (i % (numOfBitsInBlock) == 0)
        {
            for (size_t j = 0; j < _numOfBitsInBlockCounter; j++)
            {
                _blocksCounters[passedBitsForBlocks / 8] |= (uint8_t(((numOf1 - numOf1InStartOfSuperBlock) >> j) % 2) << (passedBitsForBlocks % 8));
                passedBitsForBlocks++;
            }
        }

        numOf1 += ((*ptrToData)[sizeof(numberOfBits) + i / 8] >> ((sizeof(numberOfBits) * 8 + i) % 8)) % 2;
    }
}

uint32_t blockRankCounter::rank(uint8_t desired, uint32_t pos)
{

    uint32_t numberOfBits = (_ptrToBits->size() >= sizeof(uint32_t)) ? (*((uint32_t *)&((*_ptrToBits)[0]))) : 0;
    uint32_t rank = 0;
    uint32_t superBlockRank = 0;
    uint32_t blockRank = 0;

    pos = std::min(numberOfBits - 1, pos);

    if (desired == 0)
    {
        return pos - this->rank(1, pos) + 1;
    }

    size_t log2OfBitsNumber = _numOfBitsInSuperBlockCounter;

    for (size_t j = 0; j < _numOfBitsInSuperBlockCounter; j++)
    {
        superBlockRank |= (((_superBlocksCounters[((pos / (log2OfBitsNumber * log2OfBitsNumber / 2)) * _numOfBitsInSuperBlockCounter + j) / 8] >> (((pos / (log2OfBitsNumber * log2OfBitsNumber / 2)) * _numOfBitsInSuperBlockCounter + j) % 8))) % 2) << j;
    }

    rank += superBlockRank;

    for (size_t j = 0; j < _numOfBitsInBlockCounter; j++)
    {
        blockRank |= (((_blocksCounters[((pos / (log2OfBitsNumber / 2)) * _numOfBitsInBlockCounter + j) / 8] >> (((pos / (log2OfBitsNumber / 2)) * _numOfBitsInBlockCounter + j) % 8))) % 2) << j;
    }

    rank += blockRank;

    for (size_t i = pos / (log2OfBitsNumber / 2) * (log2OfBitsNumber / 2); i <= pos; i++)
    {
        if (((*_ptrToBits)[sizeof(numberOfBits) + i / 8] >> ((sizeof(numberOfBits) * 8 + i) % 8)) % 2 == desired)
            rank++;
    }

    return rank;
}

size_t blockRankCounter::size()
{
    return sizeof(_ptrToBits) + sizeof(_numOfBitsInSuperBlockCounter) + _superBlocksCounters.size() * sizeof(uint8_t) + sizeof(_numOfBitsInBlockCounter) + _blocksCounters.size() * sizeof(uint8_t);
}
