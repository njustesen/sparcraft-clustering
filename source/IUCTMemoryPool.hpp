#pragma once

#include "Common.h"
#include "IUCTNode.h"

namespace SparCraft
{
    class IUCTMemoryPool;
}

class SparCraft::IUCTMemoryPool
{	
    std::vector< std::vector<IUCTNode> > _pool;

    const size_t    _poolSize;
    const size_t    _maxChildren;
    
    size_t    _currentIndex;

public:

	IUCTMemoryPool(const size_t & poolSize, const size_t & maxChildren)
        : _pool             (poolSize, std::vector<IUCTNode>())
        , _poolSize         (poolSize)
        , _maxChildren      (maxChildren)
        , _currentIndex     (0)
    {
        for (size_t s(0); s<poolSize; ++s)
        {
            _pool[s].reserve(maxChildren);
        }
    }

    std::vector<IUCTNode> * alloc()
    {
        std::vector<IUCTNode> & ret(_pool[_currentIndex]);
        if (ret.size() > 0)
        {
            ret.clear();
        }

        _currentIndex = (_currentIndex + 1) % _poolSize;
        return &ret;
    }

    void clearPool()
    {
        for (size_t i(0); i<_poolSize; ++i)
        {
            _pool[i].clear();
        }

        _currentIndex = 0;
    }
};
