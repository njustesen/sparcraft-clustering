#pragma once

#include <limits>

#include "Timer.h"
#include "GameState.h"
#include "UnitAction.hpp"
#include "GraphViz.hpp"
#include "Array.hpp"
#include "MoveArray.h"
#include "IUCTSearchParameters.hpp"
#include "IUCTSearchResults.hpp"
#include "Player.h"
#include "AllPlayers.h"
#include "IUCTNode.h"
#include "GraphViz.hpp"
#include "IUCTMemoryPool.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/multi_array.hpp>

namespace SparCraft
{

class Game;
class Player;

class IUCTSearch
{
	IUCTSearchParameters 	_params;
    IUCTSearchResults        _results;
	Timer		            _searchTimer;
    IUCTNode                 _rootNode;
    IUCTMemoryPool *         _memoryPool;

    GameState               _currentState;

	// we will use these as variables to save stack allocation every time
    std::vector<UnitAction>                 _actionVec;
	MoveArray                               _moveArray;

    std::vector<PlayerPtr>					_allScripts[Constants::Num_Players];
    PlayerPtr                               _playerModels[Constants::Num_Players];

public:

	IUCTSearch(const IUCTSearchParameters & params);

    // UCT-specific functions
    IUCTNode &      IUCTNodeSelect(IUCTNode & parent);
    StateEvalScore  traverse(IUCTNode & node, GameState & currentState);
	void            uct(GameState & state, size_t depth, const IDType lastPlayerToMove, std::vector<UnitAction> * firstSimMove);

	void            doSearch(GameState & initialState, std::vector<UnitAction> & move);
    
    // Move and Child generation functions
    void            generateChildren(IUCTNode & node, GameState & state);
    void            makeMove(IUCTNode & node, GameState & state);

    // Utility functions
	std::vector<UnitAction>   getMoveVec(const IDType & player, const GameState & state, const std::vector<IDType> & playerScripts);
	const IDType    getPlayerToMove(IUCTNode & node, const GameState & state) const;
    const size_t    getChildNodeType(IUCTNode & parent, const GameState & prevState) const;
	const bool      searchTimeOut();
	const bool      isRoot(IUCTNode & node) const;
	const bool      terminalState(GameState & state, const size_t & depth) const;
    const bool      isFirstSimMove(IUCTNode & node, GameState & state);
    const bool      isSecondSimMove(IUCTNode & node, GameState & state);
    StateEvalScore  performPlayout(GameState & state);
    void            updateState(IUCTNode & node, GameState & state, bool isLeaf);
    void            setMemoryPool(IUCTMemoryPool * pool);
    IUCTSearchResults & getResults();

    // graph printing functions
    void            printSubTree(IUCTNode & node, GameState state, std::string filename);
    void            printSubTreeGraphViz(IUCTNode & node, GraphViz::Graph & g, GameState state);
    std::string     getNodeIDString(IUCTNode & node);
};
}