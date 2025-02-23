#include "select_counter.h"
#include <iostream>
#include <vector>
#include <list>

/////////////////////////////////////////

void linearSelectCounter::prepare(std::vector<uint8_t> *ptrToData)
{
    _ptrToBits = ptrToData;
}

int32_t linearSelectCounter::select(uint8_t desired, uint32_t entryNum)
{

    uint32_t numberOfBits = (_ptrToBits->size() >= sizeof(uint32_t)) ? (*((uint32_t *)&((*_ptrToBits)[0]))) : 0;
    uint32_t founded = 0, ctr = 0;

    while (ctr < numberOfBits)
    {
        if (((*_ptrToBits)[sizeof(numberOfBits) + ctr / 8] >> ((sizeof(numberOfBits) * 8 + ctr) % 8)) % 2 == desired)
            founded++;

        if (entryNum == founded)
            return ctr; // ctr + 1; // numering from 1, not from 0

        ctr++;
    }

    return -1;
}

size_t linearSelectCounter::size()
{
    return (sizeof(_ptrToBits));
}

/////////////////////////////////////////

uint32_t blockSelectCounter::log2Upper(uint32_t x)
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

void blockSelectCounter::prepare(std::vector<uint8_t> *ptrToData)
{
    _ptrToBits = ptrToData;

    uint32_t numberOfBits = (_ptrToBits->size() >= sizeof(uint32_t)) ? (*((uint32_t *)&((*_ptrToBits)[0]))) : 0;

    size_t log2OfBitsNumberPlusOne = log2Upper(numberOfBits + 1);
    _numOfBitsInCounter = log2OfBitsNumberPlusOne;

    size_t numberOfBlocks = (numberOfBits + _numOfBitsInCounter - 1) / _numOfBitsInCounter;
    size_t numberOfSelectsForBlock = log2OfBitsNumberPlusOne;

    _0counters.assign((_numOfBitsInCounter * numberOfBlocks + 7) / 8, 0);
    _1counters.assign((_numOfBitsInCounter * numberOfBlocks + 7) / 8, 0);

    size_t numOf0 = 0;
    size_t prevNumOf0 = 0;
    size_t passedBitsForCounters = 0;

    for (size_t i = 0; i < numberOfBits; i++)
    {
        numOf0 += (((*ptrToData)[sizeof(numberOfBits) + i / 8] >> ((sizeof(numberOfBits) * 8 + i) % 8)) + 1) % 2;

        if (numOf0 % numberOfSelectsForBlock == 1 && prevNumOf0 != numOf0)
        {
            for (size_t j = 0; j < _numOfBitsInCounter; j++)
            {
                _0counters[passedBitsForCounters / 8] |= (uint8_t(((i + 1) >> j) % 2) << (passedBitsForCounters % 8));
                passedBitsForCounters++;
            }
        }

        prevNumOf0 = numOf0;
    }

    size_t numOf1 = 0;
    size_t prevNumOf1 = 0;
    passedBitsForCounters = 0;

    for (size_t i = 0; i < numberOfBits; i++)
    {
        numOf1 += (((*ptrToData)[sizeof(numberOfBits) + i / 8] >> ((sizeof(numberOfBits) * 8 + i) % 8))) % 2;

        if (numOf1 % numberOfSelectsForBlock == 1 && prevNumOf1 != numOf1)
        {
            for (size_t j = 0; j < _numOfBitsInCounter; j++)
            {
                _1counters[passedBitsForCounters / 8] |= (uint8_t(((i + 1) >> j) % 2) << (passedBitsForCounters % 8));
                passedBitsForCounters++;
            }
        }

        prevNumOf1 = numOf1;
    }
}

int32_t blockSelectCounter::select(uint8_t desired, uint32_t entryNum)
{

    std::vector<uint8_t> *ptrToCounters;

    ptrToCounters = (desired == 0) ? &_0counters : &_1counters;

    uint32_t numberOfBits = (_ptrToBits->size() >= sizeof(uint32_t)) ? (*((uint32_t *)&((*_ptrToBits)[0]))) : 0;

    if (entryNum > numberOfBits)
        return -1;

    size_t log2OfBitsNumberPlusOne = _numOfBitsInCounter;

    size_t numberOfCounters = (numberOfBits + log2OfBitsNumberPlusOne - 1) / log2OfBitsNumberPlusOne;
    size_t numberOfSelectsForBlock = log2OfBitsNumberPlusOne;

    size_t valInCounter = 0;

    for (size_t j = 0; j < _numOfBitsInCounter; j++)
    {
        valInCounter |= ((((*ptrToCounters)[(((entryNum - 1) / numberOfSelectsForBlock) * _numOfBitsInCounter + j) / 8] >> ((((entryNum - 1) / numberOfSelectsForBlock) * _numOfBitsInCounter + j) % 8))) % 2) << j;
    }

    if (valInCounter == 0)
    {
        return -1;
    }

    valInCounter--;

    size_t numOfRemaining = (entryNum - 1) - ((entryNum - 1) / numberOfSelectsForBlock) * numberOfSelectsForBlock;

    if (numOfRemaining == 0)
        return valInCounter;

    for (size_t i = valInCounter + 1; i < numberOfBits; i++)
    {
        if (((*_ptrToBits)[sizeof(numberOfBits) + i / 8] >> ((sizeof(numberOfBits) * 8 + i) % 8)) % 2 == desired)
            numOfRemaining--;

        if (numOfRemaining == 0)
            return i;
    }

    return -1;
}

size_t blockSelectCounter::size()
{

    return sizeof(_ptrToBits) + sizeof(_numOfBitsInCounter) + _0counters.size() * sizeof(uint8_t) + _1counters.size() * sizeof(uint8_t);
}
