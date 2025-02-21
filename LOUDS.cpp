// LOUDS.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <vector>
#include <list>
#include <chrono>>

//#define DEBUG

//#define SHOW_RANK
//#define TEST_RANK_CORRECTNESS
//#define TEST_RANK_PERFORMANCE

//#define SHOW_SELECT
//#define TEST_SELECT_CORRECTNESS
//#define TEST_SELECT_PERFORMANCE

//#define TEST_TREE_CUSTOM
//#define SHOW_TREE_AUTOGEN
#define TEST_AUTOGEN_TREE_PERFORMANCE


struct TreeNode
{
    TreeNode * parent;
    TreeNode * leftSon;
    TreeNode * rightBro;
};

/////////////////////////////////////////

class linearRankCounter
{
public:
    std::vector<uint8_t>* _ptrToBits;

    void prepare(std::vector<uint8_t>* ptrToData) 
    {
        _ptrToBits = ptrToData;
    }
    
    uint32_t rank(uint8_t desired, uint32_t pos) {
        //pos--; // numering from 1, not from 0
        uint32_t numberOfBits = (_ptrToBits->size() >= sizeof(uint32_t)) ? (*((uint32_t*)&((*_ptrToBits)[0]))) : 0;
        uint32_t founded = 0;

        for (size_t i = 0; i < std::min(numberOfBits, pos + 1); i++)
        {
            if (((*_ptrToBits)[sizeof(numberOfBits) + i / 8] >> ((sizeof(numberOfBits) * 8 + i) % 8)) % 2 == desired)
                founded++;
        }

        return founded;
    }

    size_t size()
    {
        return (sizeof(_ptrToBits));
    }
};

/////////////////////////////////////////

class linearSelectCounter
{
public:
    std::vector<uint8_t>* _ptrToBits;

    void prepare(std::vector<uint8_t>* ptrToData)
    {
        _ptrToBits = ptrToData;
    }

    int32_t select(uint8_t desired, uint32_t entryNum) {

        uint32_t numberOfBits = (_ptrToBits->size() >= sizeof(uint32_t)) ? (*((uint32_t*)&((*_ptrToBits)[0]))) : 0;
        uint32_t founded = 0, ctr = 0;

        while (ctr < numberOfBits)
        {
            if (((*_ptrToBits)[sizeof(numberOfBits) + ctr / 8] >> ((sizeof(numberOfBits) * 8 + ctr) % 8)) % 2 == desired)
                founded++;

            if (entryNum == founded)
                return ctr;//ctr + 1; // numering from 1, not from 0

            ctr++;
        }

        return -1;
    }

    size_t size()
    {
        return (sizeof(_ptrToBits));
    }
};

/////////////////////////////////////////

class blockRankCounter
{
public:
    std::vector<uint8_t>* _ptrToBits;
    
    std::vector<uint8_t> _superBlocksCounters;
    size_t _numOfBitsInSuperBlockCounter;
    std::vector<uint8_t> _blocksCounters;
    size_t _numOfBitsInBlockCounter;


    void prepare(std::vector<uint8_t>* ptrToData);
    uint32_t rank(uint8_t desired, uint32_t pos);
    size_t size();

//private:
    uint32_t log2Upper(uint32_t x);
};

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

void blockRankCounter::prepare(std::vector<uint8_t>* ptrToData)
{
    _ptrToBits = ptrToData;

    uint32_t numberOfBits = (_ptrToBits->size() >= sizeof(uint32_t)) ? (*((uint32_t*)&((*_ptrToBits)[0]))) : 0;

    size_t log2OfBitsNumber = log2Upper(numberOfBits);
    _numOfBitsInSuperBlockCounter = log2OfBitsNumber;
    size_t numOfBitsInSuperBlock = (log2OfBitsNumber * log2OfBitsNumber / 2);
    size_t numberOfSuperBlocks = (numberOfBits + numOfBitsInSuperBlock - 1) / numOfBitsInSuperBlock;
    
    _superBlocksCounters.assign((_numOfBitsInSuperBlockCounter * numberOfSuperBlocks + 7) / 8, 0);

    size_t log2OfLog2OfBitsNumber = log2Upper(log2OfBitsNumber);
    _numOfBitsInBlockCounter = 2 * log2OfLog2OfBitsNumber;
    //_numOfBitsInBlockCounter = log2OfLog2OfBitsNumber;
    size_t numOfBitsInBlock =  log2OfBitsNumber / 2;
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

uint32_t blockRankCounter::rank(uint8_t desired, uint32_t pos) {

    uint32_t numberOfBits = (_ptrToBits->size() >= sizeof(uint32_t)) ? (*((uint32_t*)&((*_ptrToBits)[0]))) : 0;
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
    return sizeof(_ptrToBits) + sizeof(_numOfBitsInSuperBlockCounter) + _superBlocksCounters.size() * sizeof(uint8_t) 
        + sizeof(_numOfBitsInBlockCounter) + _blocksCounters.size() * sizeof(uint8_t);
}

/////////////////////////////////////////

class blockSelectCounter
{
public:
    std::vector<uint8_t>* _ptrToBits;


    size_t _numOfBitsInCounter;
    std::vector<uint8_t> _0counters;
    std::vector<uint8_t> _1counters;


    void prepare(std::vector<uint8_t>* ptrToData);
    int32_t select(uint8_t desired, uint32_t entryNum);
    size_t size();

    //private:
    uint32_t log2Upper(uint32_t x);
};

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

void blockSelectCounter::prepare(std::vector<uint8_t>* ptrToData)
{
    _ptrToBits = ptrToData;

    uint32_t numberOfBits = (_ptrToBits->size() >= sizeof(uint32_t)) ? (*((uint32_t*)&((*_ptrToBits)[0]))) : 0;

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

int32_t blockSelectCounter::select(uint8_t desired, uint32_t entryNum) {
    
    std::vector<uint8_t>* ptrToCounters;

    ptrToCounters = (desired == 0) ? &_0counters : &_1counters;

    uint32_t numberOfBits = (_ptrToBits->size() >= sizeof(uint32_t)) ? (*((uint32_t*)&((*_ptrToBits)[0]))) : 0;

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
    
    return sizeof(_ptrToBits) + sizeof(_numOfBitsInCounter) + _0counters.size() * sizeof(uint8_t)
         + _1counters.size() * sizeof(uint8_t);
}

/////////////////////////////////////////

template <class rankCounter, class selectCounter>
class LOUDS
{
public:
    LOUDS() {};
    ~LOUDS() {};

    void treeToLOUDS(const TreeNode& root);
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

template <class rankCounter, class selectCounter>
void LOUDS<rankCounter, selectCounter>::treeToLOUDS(const TreeNode& root) {

    std::list<TreeNode> q;
    std::vector<uint32_t> numOfChildren;

    q.push_back(root);

    while (q.empty() == false)
    {
        auto curVert = q.front();

        numOfChildren.push_back(0);

        q.pop_front();

        //uint32_t numOfCurVertChildren = 0;

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

    *((uint32_t*)&m_louds[0]) = bitsRequired;
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
uint32_t LOUDS<rankCounter, selectCounter>::numOfBits() {
    if (m_louds.size() < sizeof(uint32_t))
        return 0;

    return *((uint32_t*)&m_louds[0]);
}

template <class rankCounter, class selectCounter>
void LOUDS<rankCounter, selectCounter>::printBits() {
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
int32_t LOUDS<rankCounter, selectCounter>::select(uint8_t desired, uint32_t entryNum) {
    /*
    uint32_t numberOfBits = numOfBits();
    uint32_t founded = 0, ctr = 0;

    while (ctr < numberOfBits)
    {
        if ((m_louds[sizeof(numberOfBits) + ctr / 8] >> ((sizeof(numberOfBits) * 8 + ctr) % 8)) % 2 == desired)
            founded++;
        
        if (entryNum == founded)
            return ctr;//ctr + 1; // numering from 1, not from 0

        ctr++;
    }

    return -1;*/

    return _selectCounter.select(desired, entryNum);
}

template <class rankCounter, class selectCounter>
uint32_t LOUDS<rankCounter, selectCounter>::rank(uint8_t desired, uint32_t pos) {
    /*//pos--; // numering from 1, not from 0
    uint32_t numberOfBits = numOfBits();
    uint32_t founded = 0;

    for (size_t i = 0; i < std::min(numberOfBits, pos + 1); i++)
    {
        if ((m_louds[sizeof(numberOfBits) + i / 8] >> ((sizeof(numberOfBits) * 8 + i) % 8)) % 2 == desired)
            founded++;
    }

    return founded;*/

    return _rankCounter.rank(desired, pos);
}

template <class rankCounter, class selectCounter>
int32_t LOUDS<rankCounter, selectCounter>::firstChild(int32_t i) {
    auto selectRes = select(0, i + 1);

    //auto numberOfBits = numOfBits();

    //if (selectRes == -1)
    //    return -1;

    //if (selectRes + 1 < numberOfBits)
    //    if ((m_louds[sizeof(numberOfBits) + (selectRes + 1) / 8] >> ((sizeof(numberOfBits) * 8 + (selectRes + 1)) % 8)) == 0) // first bit after previous node is 0 => no children
    //        return -1;

    return selectRes - i;
}

template <class rankCounter, class selectCounter>
int32_t LOUDS<rankCounter, selectCounter>::lastChild(int32_t i) {
    auto lastChildPos = select(0, i + 2) - 1;

    //auto numberOfBits = numOfBits();

    //if (lastChildPos == -1)
    //    return -1;

    //if ((m_louds[sizeof(numberOfBits) + lastChildPos / 8] >> ((sizeof(numberOfBits) * 8 + lastChildPos) % 8)) == 0) // 0 in the pos of last children => no last children
    //    return -1;

    return rank(1, lastChildPos - 1);
}

template <class rankCounter, class selectCounter>
int32_t LOUDS<rankCounter, selectCounter>::childrenCount(int32_t i) {
    auto firstChildCur = firstChild(i);
    auto firstChildNext = firstChild(i + 1);

    //if (firstChildCur != -1)
    //    return 0;

    return firstChildNext - firstChildCur;
}

template <class rankCounter, class selectCounter>
int LOUDS<rankCounter, selectCounter>::parent(int32_t i) {
    return rank(0, select(1, (i + 1)) - 1) - 1;
}

////////////////////////////////////////////////////////////////////////////

void randTreeGen(std::vector<TreeNode>& Tree, int numV) 
{
    Tree.assign(numV, { nullptr, nullptr, nullptr });
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

        TreeNode* cur = &(Tree[rnd]);

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

void randTreeGen2(std::vector<TreeNode>& Tree, int numV)
{
    Tree.assign(numV, { nullptr, nullptr, nullptr });

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

void nDegreeTreeGen(std::vector<TreeNode>& Tree, size_t degree, int numV)
{
    Tree.assign(numV, { nullptr, nullptr, nullptr });
    
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

void recPrintTree(TreeNode* root, std::string & printString, std::string& broString)
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

        TreeNode* son = root->leftSon;
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
            
            //printString[printString.length() - 1 - std::string(" ------> O").length()] = '|';

            son = son->rightBro;
        }

        

        broString =broString.substr(0, broString.length() - std::string("          ").length());

        printString = tempPrintString;
    }
}

void printTree(TreeNode* root)
{
    std::string printString = "";
    std::string broString = " ";
    std::cout << "\n";
    recPrintTree(root, printString, broString);
}

int main()
{
#ifdef SHOW_RANK

    //srand(time(NULL));

    uint32_t numberOfBits = 10;
    std::vector<uint8_t> bits;

    bits.assign(sizeof(numberOfBits) + (numberOfBits + 7) / 8, 0);
    
    *(uint32_t*)&(bits[0]) = numberOfBits;

    for (size_t i = 0; i < numberOfBits; i++)
    {
        bits[sizeof(numberOfBits) + i / 8] |= uint8_t((rand() % 2)) << (i % 8);
    }


    std::cout << numberOfBits << " bits: ";

    if (numberOfBits > 0)
    {
        for (size_t i = sizeof(numberOfBits) * 8; i < bits.size() * 8; i++)
        {
            if (i == numberOfBits + sizeof(numberOfBits) * 8)
                std::cout << '|';

            std::cout << (bits[i / 8] >> (i % 8)) % 2;
        }
    }

    std::cout << "\n";

    linearRankCounter linRank;
    linRank.prepare(&bits);


    blockRankCounter blockRank;
    blockRank.prepare(&bits);


    std::cout << "Rank:\n";
    for (size_t i = 0; i < numberOfBits; i++)
    {
        std::cout << "rank(1, " << i << "): lin = " << linRank.rank(1, i) << ";\tbl = " << blockRank.rank(1, i) << ";\n";
    }

    size_t log2OfBitsNumber = blockRank.log2Upper(numberOfBits);
    std::cout << "\nNumber of Superblocks: " << ((numberOfBits + (log2OfBitsNumber * log2OfBitsNumber / 2) - 1) / (log2OfBitsNumber * log2OfBitsNumber / 2));
    std::cout << "\nNumber of bits in Superblock: " << (log2OfBitsNumber * log2OfBitsNumber / 2) << "\n";

    for (size_t i = 0; i < ((numberOfBits + (log2OfBitsNumber * log2OfBitsNumber / 2) - 1) / (log2OfBitsNumber * log2OfBitsNumber / 2)); i++)
    {
        size_t superBlockRank = 0;

        for (size_t j = 0; j < log2OfBitsNumber; j++)
        {
            superBlockRank |= (((blockRank._superBlocksCounters[(i * log2OfBitsNumber + j) / 8] >> ((i * log2OfBitsNumber + j) % 8))) % 2) << j;
        }

        std::cout << "Super block " << i << ": " << superBlockRank << "\n";
    }

    std::cout << "\nNumber of Blocks: " << ((numberOfBits + (log2OfBitsNumber * log2OfBitsNumber / 2) - 1) / (log2OfBitsNumber * log2OfBitsNumber / 2)) * log2OfBitsNumber;
    std::cout << "\nNumber of bits in Block: " << (log2OfBitsNumber / 2) << "\n";

    for (size_t i = 0; i < ((numberOfBits + (log2OfBitsNumber * log2OfBitsNumber / 2) - 1) / (log2OfBitsNumber * log2OfBitsNumber / 2)) * log2OfBitsNumber; i++)
    {
        size_t BlockRank = 0;

        for (size_t j = 0; j < blockRank._numOfBitsInBlockCounter; j++)
        {
            BlockRank |= (((blockRank._blocksCounters[(i * blockRank._numOfBitsInBlockCounter + j) / 8] >> ((i * blockRank._numOfBitsInBlockCounter + j) % 8))) % 2) << j;
        }


        std::cout << "Block " << i << ": " << BlockRank << "\n";
    }

#endif // SHOW_RANK

#ifdef TEST_RANK_CORRECTNESS

    std::cout << "\n=========================================================\n TEST_RANK_CORRECTNESS started... ";
    srand(time(NULL));

    uint32_t numberOfBits = 100;
    std::vector<uint8_t> bits;

    bits.assign(sizeof(numberOfBits) + (numberOfBits + 7) / 8, 0);

    *(uint32_t*)&(bits[0]) = numberOfBits;

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

        if (lin != block)
        {
            std::cout << "rank(1, " << i << "): lin = " << lin << ";\tbl = " << block << ";\n";
        }

        lin = linRank.rank(0, i);
        block = blockRank.rank(0, i);

        if (lin != block)
        {
            std::cout << "rank(0, " << i << "): lin = " << lin << ";\tbl = " << block << ";\n";
        }
    }

    std::cout << "\n TEST_RANK_CORRECTNESS finished \n=========================================================\n";
#endif // TEST_RANK_CORRECTNESS

#ifdef TEST_RANK_PERFORMANCE
    std::cout << "\n=========================================================\n TEST_RANK_PERFORMANCE started... ";
    srand(time(NULL));

    uint32_t numberOfBits = 10000;
    std::vector<uint8_t> bits;

    bits.assign(sizeof(numberOfBits) + (numberOfBits + 7) / 8, 0);

    *(uint32_t*)&(bits[0]) = numberOfBits;

    for (size_t i = 0; i < numberOfBits; i++)
    {
        bits[sizeof(numberOfBits) + i / 8] |= uint8_t((rand() % 2)) << (i % 8);
    }

    linearRankCounter linRank;
    linRank.prepare(&bits);


    blockRankCounter blockRank;
    blockRank.prepare(&bits);

    std::chrono::time_point<std::chrono::system_clock> t1, t2;

    uint32_t res0, res1;

    t1 = std::chrono::system_clock::now();

    for (size_t i = 0; i < numberOfBits + 10; i++)
    {
        res0 = linRank.rank(1, i);
        res1 = linRank.rank(0, i);
    }

    t2 = std::chrono::system_clock::now();

    std::cout << res0 << res1;
    std::cout << "\nlinear: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << " mcs;\t" << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / static_cast<double>(numberOfBits) << " mcs/rank\n";


    t1 = std::chrono::system_clock::now();

    for (size_t i = 0; i < numberOfBits + 10; i++)
    {
        res0 = blockRank.rank(1, i);
        res1 = blockRank.rank(0, i);
    }

    t2 = std::chrono::system_clock::now();

    std::cout << res0 << res1;

    std::cout << "\nblock: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << " mcs;\t" << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / static_cast<double>(numberOfBits) << " mcs/rank\n";

    std::cout << "\nblock requires " << blockRank.size() * 8 << " bits to store rank for " << numberOfBits << " bits (koef " << static_cast<double>(blockRank.size() * 8) / numberOfBits << "), superBlocksSize: " <<
        blockRank._superBlocksCounters.size() * 8 << " bits, blocksSize: " << blockRank._blocksCounters.size() * 8 << " bits\n";


    std::cout << "\n TEST_RANK_PERFORMANCE finished \n=========================================================\n";


#endif // TEST_RANK_PERFORMANCE


#ifdef SHOW_SELECT
    //srand(time(NULL));

    uint32_t numberOfBits = 100;
    std::vector<uint8_t> bits;

    bits.assign(sizeof(numberOfBits) + (numberOfBits + 7) / 8, 0);

    *(uint32_t*)&(bits[0]) = numberOfBits;

    for (size_t i = 0; i < numberOfBits; i++)
    {
        bits[sizeof(numberOfBits) + i / 8] |= uint8_t((rand() % 2)) << (i % 8);
    }


    std::cout << numberOfBits << " bits: ";

    if (numberOfBits > 0)
    {
        for (size_t i = sizeof(numberOfBits) * 8; i < bits.size() * 8; i++)
        {
            if (i == numberOfBits + sizeof(numberOfBits) * 8)
                std::cout << '|';

            std::cout << (bits[i / 8] >> (i % 8)) % 2;
        }
    }

    std::cout << "\n";

    linearSelectCounter linSelecct;
    linSelecct.prepare(&bits);


    blockSelectCounter blockSelect;
    blockSelect.prepare(&bits);


    std::cout << "Select:\n";
    for (size_t i = 0; i < numberOfBits + 10; i++)
    {
        std::cout << "select(0, " << i + 1 << "): lin = " << linSelecct.select(0, i + 1) << ";\tbl = " << blockSelect.select(0, i + 1) << ";\n";
    }


    size_t log2OfBitsNumberPlusOne = blockSelect.log2Upper(numberOfBits + 1);
    
    size_t numberOfCounters = (numberOfBits + log2OfBitsNumberPlusOne - 1) / log2OfBitsNumberPlusOne;
    size_t numberOfSelectsForBlock = log2OfBitsNumberPlusOne;

    
    std::cout << "\nNumber of counters: " << numberOfCounters;
    std::cout << "\nNumber of selects in counter: " << numberOfSelectsForBlock << "\n";


    for (size_t i = 0; i < numberOfCounters; i++)
    {
        size_t counterSelect = 0;

        for (size_t j = 0; j < log2OfBitsNumberPlusOne; j++)
        {
            counterSelect |= (((blockSelect._0counters[(i * log2OfBitsNumberPlusOne + j) / 8] >> ((i * log2OfBitsNumberPlusOne + j) % 8))) % 2) << j;
        }

        std::cout << "Counter " << i << ": " << counterSelect << "\n";
    }

#endif // SHOW_SELECT

#ifdef TEST_SELECT_CORRECTNESS

    std::cout << "\n=========================================================\n TEST_SELECT_CORRECTNESS started... ";
    srand(time(NULL));

    uint32_t numberOfBits = 10000;
    std::vector<uint8_t> bits;

    bits.assign(sizeof(numberOfBits) + (numberOfBits + 7) / 8, 0);

    *(uint32_t*)&(bits[0]) = numberOfBits;

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

        if (lin != block)
        {
            std::cout << "select(1, " << i + 1 << "): lin = " << lin << ";\tbl = " << block << ";\n";
        }

        lin = linSelect.select(0, i + 1);
        block = blockSelect.select(0, i + 1);

        if (lin != block)
        {
            std::cout << "select(0, " << i + 1 << "): lin = " << lin << ";\tbl = " << block << ";\n";
        }
    }

    std::cout << "\n TEST_SELECT_CORRECTNESS finished \n=========================================================\n";
#endif // TEST_SELECT_CORRECTNESS

#ifdef TEST_SELECT_PERFORMANCE
    std::cout << "\n=========================================================\n TEST_SELECT_PERFORMANCE started... ";
    srand(time(NULL));

    uint32_t numberOfBits = 50000;
    std::vector<uint8_t> bits;

    bits.assign(sizeof(numberOfBits) + (numberOfBits + 7) / 8, 0);

    *(uint32_t*)&(bits[0]) = numberOfBits;

    for (size_t i = 0; i < numberOfBits; i++)
    {
        bits[sizeof(numberOfBits) + i / 8] |= uint8_t((rand() % 2)) << (i % 8);
    }

    linearSelectCounter linSelect;
    linSelect.prepare(&bits);

    blockSelectCounter blockSelect;
    blockSelect.prepare(&bits);


    std::chrono::time_point<std::chrono::system_clock> t1, t2;

    uint32_t res0, res1;

    t1 = std::chrono::system_clock::now();

    for (size_t i = 0; i < numberOfBits + 10; i++)
    {
        res0 = linSelect.select(1, i + 1);
        res1 = linSelect.select(0, i + 1);
    }

    t2 = std::chrono::system_clock::now();

    std::cout << res0 << res1;
    std::cout << "\nlinear: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << " mcs;\t" << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / static_cast<double>(numberOfBits) << " mcs/rank\n";


    t1 = std::chrono::system_clock::now();

    for (size_t i = 0; i < numberOfBits + 10; i++)
    {
        res0 = blockSelect.select(1, i + 1);
        res1 = blockSelect.select(0, i + 1);
    }

    t2 = std::chrono::system_clock::now();

    std::cout << res0 << res1;

    std::cout << "\nblock: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << " mcs;\t" << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / static_cast<double>(numberOfBits) << " mcs/select\n";

    std::cout << "\nblock requires " << blockSelect.size() * 8 << " bits to store select for " << numberOfBits << " bits\n";


    std::cout << "\n TEST_SELECT_PERFORMANCE finished \n=========================================================\n";


#endif // TEST_SELECT_PERFORMANCE

#ifdef TEST_TREE_CUSTOM
    std::vector<TreeNode> Tree;

    Tree.assign(10, { nullptr, nullptr, nullptr });
    
    Tree[0].leftSon = &(Tree[1]);
    Tree[1].rightBro = &(Tree[2]);

    Tree[2].leftSon = &(Tree[3]);
    Tree[3].rightBro = &(Tree[4]);
    Tree[4].rightBro = &(Tree[5]);

    Tree[3].leftSon = &(Tree[6]);
    Tree[6].rightBro = &(Tree[7]);
    Tree[4].leftSon = &(Tree[8]);
    Tree[5].leftSon = &(Tree[9]);

    LOUDS<linearRankCounter, linearSelectCounter> lds;

    std::cout << "Linear counters:\n";

    lds.treeToLOUDS(Tree[0]);
    
    lds.printBits();

    std::cout << "select: " << " (0, 3) = " << lds.select(0, 3) << ";\t(0, 6) = " << lds.select(0, 6) << ";\t(0, 8) = " << lds.select(0, 8) << ";\t(0, 10) = " << lds.select(0, 10) << ";\t(0, 11) = " << lds.select(0, 11) << ";\t(0, 13) = " << lds.select(0, 13) << '\n';
    std::cout << "select: " << " (1, 3) = " << lds.select(1, 3) << ";\t(1, 6) = " << lds.select(1, 6) << ";\t(1, 8) = " << lds.select(1, 8) << ";\t(1, 10) = " << lds.select(1, 10) << ";\t(1, 11) = " << lds.select(1, 11) << ";\t(1, 13) = " << lds.select(1, 13) << '\n';

    std::cout << '\n';

    std::cout << "rank: " << " (0, 3) = " << lds.rank(0, 3) << ";\t(0, 6) = " << lds.rank(0, 6) << ";\t(0, 8) = " << lds.rank(0, 8) << ";\t(0, 10) = " << lds.rank(0, 10) << ";\t(0, 11) = " << lds.rank(0, 11) << ";\t(0, 13) = " << lds.rank(0, 13) << '\n';
    std::cout << "rank: " << " (1, 3) = " << lds.rank(1, 3) << ";\t(1, 6) = " << lds.rank(1, 6) << ";\t(1, 8) = " << lds.rank(1, 8) << ";\t(1, 10) = " << lds.rank(1, 10) << ";\t(1, 11) = " << lds.rank(1, 11) << ";\t(1, 13) = " << lds.rank(1, 13) << '\n';

    printTree(&(Tree[0]));

    std::cout << "\nfirstChild(3): " << lds.firstChild(3) << '\n';
    std::cout << "lastChild(3): " << lds.lastChild(3) << '\n';
    std::cout << "childrenCount(1): " << lds.childrenCount(1) << '\n';
    std::cout << "childrenCount(2): " << lds.childrenCount(2) << '\n';
    std::cout << "parent(6): " << lds.parent(6) << '\n';

    std::cout << "--------------------------------------------------\n" << "orig structure size in bytes: " << Tree.size() * sizeof(Tree[0]) << '\n'
        << "compressed size in bytes: " << lds.size() << '\n';
    

    LOUDS<blockRankCounter, blockSelectCounter> ldsbl;
    std::cout << "\n\n\nBlock counters:\n";

    ldsbl.treeToLOUDS(Tree[0]);

    std::cout << "\nfirstChild(3): " << ldsbl.firstChild(3) << '\n';
    std::cout << "lastChild(3): " << ldsbl.lastChild(3) << '\n';
    std::cout << "childrenCount(1): " << ldsbl.childrenCount(1) << '\n';
    std::cout << "childrenCount(2): " << ldsbl.childrenCount(2) << '\n';
    std::cout << "parent(6): " << ldsbl.parent(6) << '\n';

    std::cout << "--------------------------------------------------\n" << "orig structure size in bytes: " << Tree.size() * sizeof(Tree[0]) << '\n'
        << "compressed size in bytes: " << ldsbl.size() << '\n';

#endif // TEST_TREE_CUSTOM

#ifdef SHOW_TREE_AUTOGEN

    std::vector<TreeNode> Tree2;
    //randTreeGen(Tree2, 100);
    //nDegreeTreeGen(Tree2, 2, 31);
    randTreeGen2(Tree2, 100);

    for (size_t i = 0; i < Tree2.size(); i++)
    {
        if (Tree2[i].parent != nullptr)
            std::cout << (Tree2[i].parent - &(Tree2[0])) << " -> " << i << "\n";
    }

    printTree(&(Tree2[0]));

#endif // SHOW_TREE_AUTOGEN

#ifdef TEST_AUTOGEN_TREE_PERFORMANCE

    std::chrono::time_point<std::chrono::system_clock> t1, t2;

    std::vector<TreeNode> Tree;
    randTreeGen2(Tree, 30000);

    uint64_t visited = 0;


    t1 = std::chrono::system_clock::now();

    for (size_t i = 0; i < Tree.size(); i++)
    {
        TreeNode* son = Tree[i].leftSon;

        while (son != nullptr)
        {
            son = son->rightBro;
            visited++;
        }
    }

    t2 = std::chrono::system_clock::now();

    std::cout << "\npointers: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << " mcs;\t size in bytes: " << Tree.size() * sizeof(Tree[0]) << "; " << visited << " children\n";

    ///////////////////////////////
    
    LOUDS<linearRankCounter, linearSelectCounter> ldsLin;
    ldsLin.treeToLOUDS(Tree[0]);

    visited = 0;

    t1 = std::chrono::system_clock::now();

    for (size_t i = 0; i < Tree.size(); i++)
    {
        visited += ldsLin.childrenCount(i);
    }
    
    t2 = std::chrono::system_clock::now();

    std::cout << "\nlinear: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << " mcs;\t size in bytes: " << ldsLin.size() << "; " << visited << " children\n";
    
    ///////////////////////////////

    LOUDS<blockRankCounter, blockSelectCounter> ldsBlock;
    ldsBlock.treeToLOUDS(Tree[0]);

    visited = 0;

    t1 = std::chrono::system_clock::now();

    for (size_t i = 0; i < Tree.size(); i++)
    {
        visited += ldsBlock.childrenCount(i);
    }

    t2 = std::chrono::system_clock::now();

    std::cout << "\nblock: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << " mcs;\t size in bytes: " << ldsBlock.size() << "; " << visited << " children\n";

#endif // TEST_AUTOGEN_TREE_PERFORMANCE


    /*std::vector<TreeNode> Tree2;
    //randTreeGen(Tree2, 100);
    //nDegreeTreeGen(Tree2, 2, 31);
    randTreeGen2(Tree2, 100);

    LOUDS<linearRankCounter, linearSelectCounter> lds;

    lds.treeToLOUDS(Tree2[0]);

    lds.printBits();


    printTree(&(Tree2[0]));

    std::cout << "--------------------------------------------------\n" << "orig structure size in bytes: " << Tree2.size() * sizeof(Tree2[0]) << '\n'
        << "compressed size in bytes: " << lds.m_louds.size() << '\n';
    */

}
