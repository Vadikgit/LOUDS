#ifndef SELECT_COUNTER
#define SELECT_COUNTER

#include <iostream>
#include <vector>
#include <list>
#include <cstdint>

/////////////////////////////////////////

class linearSelectCounter
{
public:
	std::vector<uint8_t> *_ptrToBits;

	void prepare(std::vector<uint8_t> *ptrToData);
	int32_t select(uint8_t desired, uint32_t entryNum);
	size_t size();
};

/////////////////////////////////////////

class blockSelectCounter
{
public:
	std::vector<uint8_t> *_ptrToBits;

	size_t _numOfBitsInCounter;
	std::vector<uint8_t> _0counters;
	std::vector<uint8_t> _1counters;

	void prepare(std::vector<uint8_t> *ptrToData);
	int32_t select(uint8_t desired, uint32_t entryNum);
	size_t size();

	// private:
	uint32_t log2Upper(uint32_t x);
};

#endif