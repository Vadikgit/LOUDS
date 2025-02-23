#ifndef RANK_COUNTER
#define RANK_COUNTER

#include <vector>
#include <list>
#include <iostream>
#include <cstdint>

/////////////////////////////////////////

class linearRankCounter
{
public:
	std::vector<uint8_t> *_ptrToBits;

	void prepare(std::vector<uint8_t> *ptrToData);
	uint32_t rank(uint8_t desired, uint32_t pos);
	size_t size();
};

/////////////////////////////////////////

class blockRankCounter
{
public:
	std::vector<uint8_t> *_ptrToBits;

	std::vector<uint8_t> _superBlocksCounters;
	size_t _numOfBitsInSuperBlockCounter;
	std::vector<uint8_t> _blocksCounters;
	size_t _numOfBitsInBlockCounter;

	void prepare(std::vector<uint8_t> *ptrToData);
	uint32_t rank(uint8_t desired, uint32_t pos);
	size_t size();

	// private:
	uint32_t log2Upper(uint32_t x);
};

#endif