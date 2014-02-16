#pragma once

#include "Common.h"
#include "Player.h"
#include "AllPlayers.h"
#include "IUCTSearch.h"
#include "IUCTMemoryPool.hpp"

namespace SparCraft
{
class Player_IUCT : public Player
{
    IUCTSearchParameters     _params;
    IUCTSearchResults        _prevResults;
public:
    Player_IUCT (const IDType & playerID, const IUCTSearchParameters & params);
	void getMoves(GameState & state, const MoveArray & moves, std::vector<UnitAction> & moveVec);
    IDType getType() { return PlayerModels::IUCT; }
    IUCTSearchParameters & getParams();
    IUCTSearchResults & getResults();
};
}