#include "Player_IUCT.h"

using namespace SparCraft;

Player_IUCT::Player_IUCT (const IDType & playerID, const IUCTSearchParameters & params) 
{
	_playerID = playerID;
    _params = params;
}

void Player_IUCT::getMoves(GameState & state, const MoveArray & moves, std::vector<UnitAction> & moveVec)
{
    moveVec.clear();
    
    IUCTSearch uct(_params);

    uct.doSearch(state, moveVec);
    _prevResults = uct.getResults();
}

IUCTSearchParameters & Player_IUCT::getParams()
{
    return _params;
}

IUCTSearchResults & Player_IUCT::getResults()
{
    return _prevResults;
}