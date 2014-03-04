#include "IUCTSearch.h"

using namespace SparCraft;

IUCTSearch::IUCTSearch(const IUCTSearchParameters & params) 
	: _params(params)
    , _memoryPool(NULL)
{
    for (size_t p(0); p<Constants::Num_Players; ++p)
    {
        // set ordered move script player objects
        for (size_t s(0); s<_params.getOrderedMoveScripts().size(); ++s)
        {
            _allScripts[p].push_back(AllPlayers::getPlayerPtr(p, _params.getOrderedMoveScripts()[s]));
        }

        // set player model objects
        if (_params.playerModel(p) != PlayerModels::None)
        {
            _playerModels[p] = AllPlayers::getPlayerPtr(p, _params.playerModel(p));
        }
    }
}

void IUCTSearch::setMemoryPool(IUCTMemoryPool * pool)
{
    _memoryPool = pool;
}

void IUCTSearch::doSearch(GameState & initialState, std::vector<UnitAction> & move)
{
    Timer t;
    t.start();
	UnitScriptData * emptyScriptData;
    _rootNode = IUCTNode(NULL, Players::Player_None, SearchNodeType::RootNode, emptyScriptData, _params.maxChildren(), _memoryPool ? _memoryPool->alloc() : NULL);

    // do the required number of traversals
    for (size_t traversals(0); traversals < _params.maxTraversals(); ++traversals)
    {
        GameState state(initialState);
        traverse(_rootNode, state);

        if (traversals && (traversals % 5 == 0))
        {
            if (_params.timeLimit() && (t.getElapsedTimeInMilliSec() >= _params.timeLimit()))
            {
                break;
            }
        }

        _results.traversals++;

        //printSubTree(_rootNode, initialState, "__uct.txt");
        //system("\"C:\\Program Files (x86)\\Graphviz2.30\\bin\\dot.exe\" < __uct.txt -Tpng > uct.png");
    }

    // Get script data
	UnitScriptData * scriptData;
    if (_params.rootMoveSelectionMethod() == IUCTMoveSelect::HighestValue)
    {
		scriptData = _rootNode.bestUCTValueChild(true, _params).getScriptData();
    }
    else if (_params.rootMoveSelectionMethod() == IUCTMoveSelect::MostVisited)
    {
		scriptData = _rootNode.mostVisitedChild().getScriptData();        
    }

	// Convert scripts to moves
	GameState copy(initialState);
	MoveArray moves;
	copy.generateMoves(moves, _params.maxPlayer());
	(*scriptData).calculateMoves(_params.maxChildren(), moves, copy, move);

    if (_params.graphVizFilename().length() > 0)
    {
        //printSubTree(_rootNode, initialState, _params.graphVizFilename());
        //system("\"C:\\Program Files (x86)\\Graphviz2.30\\bin\\dot.exe\" < __uct.txt -Tpng > uct.png");
    }

    double ms = t.getElapsedTimeInMilliSec();
    _results.timeElapsed = ms;
    //printf("Search took %lf ms\n", ms);
    //printf("Hello\n");

}

const bool IUCTSearch::searchTimeOut()
{
	return (_params.timeLimit() && (_searchTimer.getElapsedTimeInMilliSec() >= _params.timeLimit()));
}

const bool IUCTSearch::terminalState(GameState & state, const size_t & depth) const
{
	return (depth <= 0 || state.isTerminal());
}

const size_t IUCTSearch::getChildNodeType(IUCTNode & parent, const GameState & prevState) const
{
    if (!prevState.bothCanMove())
    {
        return SearchNodeType::SoloNode;
    }
    else
    {
        if (parent.getNodeType() == SearchNodeType::RootNode)
        {
            return SearchNodeType::FirstSimNode;
        }
        else if (parent.getNodeType() == SearchNodeType::SoloNode)
        {
            return SearchNodeType::FirstSimNode;
        }
        else if (parent.getNodeType() == SearchNodeType::SecondSimNode)
        {
            return SearchNodeType::FirstSimNode;
        }
        else if (parent.getNodeType() == SearchNodeType::FirstSimNode)
        {
            return SearchNodeType::SecondSimNode;
        }
    }

    return SearchNodeType::Default;
}

const IDType IUCTSearch::getPlayerToMove(IUCTNode & node, const GameState & state) const
{
	const IDType whoCanMove(state.whoCanMove());

	// if both players can move
	if (whoCanMove == Players::Player_Both)
	{
        // pick the first move based on our policy
		const IDType policy(_params.playerToMoveMethod());
		const IDType maxPlayer(_params.maxPlayer());

        // the max player always chooses at the root
        if (isRoot(node))
        {
            return maxPlayer;
        }

        // the type of node this is
        const IDType nodeType = node.getNodeType();

        // the 2nd player in a sim move is always the enemy of the first
        if (nodeType == SearchNodeType::FirstSimNode)
        {
            return state.getEnemy(node.getPlayer());
        }
        // otherwise use our policy to see who goes first in a sim move state
        else
        {
		    if (policy == SparCraft::PlayerToMove::Alternate)
		    {
			    return state.getEnemy(node.getPlayer());
		    }
		    else if (policy == SparCraft::PlayerToMove::Not_Alternate)
		    {
			    return node.getPlayer();
		    }
		    else if (policy == SparCraft::PlayerToMove::Random)
		    {
			    return rand() % 2;
		    }

            // we should never get to this state
		    System::FatalError("UCT Error: Nobody can move for some reason");
		    return Players::Player_None;
        }
	}
	else
	{
		return whoCanMove;
	}
}

IUCTNode & IUCTSearch::IUCTNodeSelect(IUCTNode & parent)
{
    IUCTNode *  bestNode    = NULL;
    bool        maxPlayer   = isRoot(parent) || (parent.getChild(0).getPlayer() == _params.maxPlayer());
    double      bestVal     = maxPlayer ? std::numeric_limits<double>::min() : std::numeric_limits<double>::max();
         
    // loop through each child to find the best node
    for (size_t c(0); c < parent.numChildren(); ++c)
    {
        IUCTNode & child = parent.getChild(c);

		double currentVal(0);
	
        // if we have visited this node already, get its UCT value
		if (child.numVisits() > 0)
		{
			double winRate    = (double)child.numWins() / (double)child.numVisits();
            double uctVal     = _params.cValue() * sqrt( log( (double)parent.numVisits() ) / ( child.numVisits() ) );
			currentVal        = maxPlayer ? (winRate + uctVal) : (winRate - uctVal);
            
            child.setUCTVal(currentVal);
		}
		else
		{
            // if we haven't visited it yet, return it and visit immediately
			return child;
		}

        // choose the best node depending on max or min player
        if (maxPlayer)
        {
            if (currentVal > bestVal)
            {
                bestVal             = currentVal;
			    bestNode            = &child;
            }
        }
        else if (currentVal < bestVal)
        {
            bestVal             = currentVal;
			bestNode            = &child;
        }
	}

    return *bestNode;
}

void IUCTSearch::updateState(IUCTNode & node, GameState & state, bool isLeaf)
{
    // if it's the first sim move with children, or the root node
    if ((node.getNodeType() != SearchNodeType::FirstSimNode) || isLeaf)
    {
        // if this is a second sim node
        if (node.getNodeType() == SearchNodeType::SecondSimNode)
        {
            // make the parent's moves on the state because they haven't been done yet
			std::vector<UnitAction> move;
			scriptsToMove(state, move, *node.getParent());
            state.makeMoves(move);
        }

        // do the current node moves and call finished moving
        std::vector<UnitAction> move;
		scriptsToMove(state, move, node);
        state.makeMoves(move);
        state.finishedMoving();
    }
}

std::vector<UnitAction> IUCTSearch::scriptsToMove(GameState state, std::vector<UnitAction> & moveVec, IUCTNode & node){

	GameState copy(state);
	UnitScriptData * scriptData;
	(*scriptData).calculateMoves(_params.maxChildren(), *node.getMoveArray(), copy, moveVec);

}
	

StateEvalScore IUCTSearch::traverse(IUCTNode & node, GameState & currentState)
{
    StateEvalScore playoutVal;

    _results.totalVisits++;

    // if we haven't visited this node yet, do a playout
    if (node.numVisits() == 0)
    {
        // update the status of the current state with this node's moves
        //updateState(node, currentState, !node.hasChildren());
        updateState(node, currentState, true);

        // do the playout
        playoutVal = currentState.eval(_params.maxPlayer(), _params.evalMethod(), _params.simScript(Players::Player_One), _params.simScript(Players::Player_Two));

        _results.nodesVisited++;
    }
    // otherwise we have seen this node before
    else
    {
        // update the state for a non-leaf node
        updateState(node, currentState, false);

        if (currentState.isTerminal())
        {
            playoutVal = currentState.eval(_params.maxPlayer(), EvaluationMethods::LTD2);
        }
        else
        {
            // if the children haven't been generated yet
            if (!node.hasChildren())
            {
                generateChildren(node, currentState);
            }

            IUCTNode & next = IUCTNodeSelect(node);
            playoutVal = traverse(next, currentState);
        }
    }

    node.incVisits();
    
    if (playoutVal.val() > 0)
    {
        node.addWins(1);
    }
    else if (playoutVal.val() == 0)
    {
        node.addWins(0.5);
    }

    return playoutVal;
}

// generate the children of state 'node'
// state is the GameState after node's moves have been performed
void IUCTSearch::generateChildren(IUCTNode & node, GameState & state)
{
    // figure out who is next to move in the game
    const IDType playerToMove(getPlayerToMove(node, state));

    // generate all the moves possible from this state
	MoveArray * moveArr;
	state.generateMoves(*moveArr, playerToMove);
	(*moveArr).shuffleMoveActions();
	node.setMoveArray(moveArr);
	
	// Add all NOKDPS script vector
	UnitScriptData * nokdpsScriptData;
	setAllScripts(playerToMove, state, *nokdpsScriptData, PlayerModels::NOKDPS);
	//IUCTNode child = IUCTNode(&node, playerToMove, getChildNodeType(node, state), nokdpsScriptData, _params.maxChildren, _memoryPool ? _memoryPool->alloc() : NULL);

	node.addChild(&node,playerToMove,getChildNodeType(node,state),nokdpsScriptData,_params.maxChildren(), _memoryPool ? _memoryPool->alloc(): NULL);
	// Add all KITER-NOKDPS script vector
	UnitScriptData * kiterScriptData;
	setAllScripts(playerToMove, state, *kiterScriptData, PlayerModels::Kiter_NOKDPS);
	//IUCTNode child = IUCTNode(&node, playerToMove, getChildNodeType(node, state), kiterScriptData, _params.maxChildren, _memoryPool ? _memoryPool->alloc() : NULL);
	node.addChild(&node, playerToMove, getChildNodeType(node, state), kiterScriptData, _params.maxChildren(), _memoryPool ? _memoryPool->alloc() : NULL);
    // Add random script vectors
    for (size_t child(0); (child < _params.maxChildren() -2); ++child)
    {
		// Generate random script vector
		UnitScriptData * randomScriptData;
		setRandomScripts(playerToMove, state, *randomScriptData);

		// Continue if script vector already in a child

        // add the child to the tree
        node.addChild(&node, playerToMove, getChildNodeType(node, state), randomScriptData, _params.maxChildren(), _memoryPool ? _memoryPool->alloc() : NULL);
        _results.nodesCreated++;
    }
}

void IUCTSearch::setRandomScripts(const IDType & player, const GameState & state, UnitScriptData & data){
    for (size_t unitIndex(0); unitIndex < state.numUnits(player); ++unitIndex)
    {
		if (rand() % 10 >= 5){
			data.setUnitScript(state.getUnit(player, unitIndex), PlayerModels::NOKDPS);
		} else {
			data.setUnitScript(state.getUnit(player, unitIndex), PlayerModels::Kiter_NOKDPS);
		}
    }
}

void  PortfolioGreedySearch::setAllScripts(const IDType & player, const GameState & state, UnitScriptData & data, const IDType & script){
    for (size_t unitIndex(0); unitIndex < state.numUnits(player); ++unitIndex)
    {
        data.setUnitScript(state.getUnit(player, unitIndex), script);
    }
}

StateEvalScore IUCTSearch::performPlayout(GameState & state)
{
    GameState copy(state);
    copy.finishedMoving();

    return copy.eval(_params.maxPlayer(), _params.evalMethod(), _params.simScript(Players::Player_One), _params.simScript(Players::Player_Two));
}

const bool IUCTSearch::isRoot(IUCTNode & node) const
{
    return &node == &_rootNode;
}

void IUCTSearch::printSubTree(IUCTNode & node, GameState s, std::string filename)
{
    std::ofstream out(filename.c_str());

    GraphViz::Graph G("g");
    G.set("bgcolor", "#ffffff");

    printSubTreeGraphViz(node, G, s);

    G.print(out);
}

/*
void IUCTSearch::printSubTreeGraphViz(IUCTNode & node, GraphViz::Graph & g, GameState state)
{
    if (node.getNodeType() == SearchNodeType::FirstSimNode && node.hasChildren())
    {
        // don't make any moves if it is a first simnode
    }
    else
    {
        if (node.getNodeType() == SearchNodeType::SecondSimNode)
        {
            state.makeMoves(node.getParent()->getMove());
        }

        state.makeMoves(node.getMove());
        state.finishedMoving();
    }

    std::stringstream label;
    std::stringstream move;

    for (size_t a(0); a<node.getMove().size(); ++a)
    {
        move << node.getMove()[a].moveString() << "\\n";
    }

    if (node.getMove().size() == 0)
    {
        move << "root";
    }

    std::string firstSim = SearchNodeType::getName(node.getNodeType());

    Unit p1 = state.getUnit(0,0);
    Unit p2 = state.getUnit(1,0);

    label   << move.str() 
            << "\\nVal: "       << node.getUCTVal() 
            << "\\nWins: "      << node.numWins() 
            << "\\nVisits: "    << node.numVisits() 
            << "\\nChildren: "  << node.numChildren() 
            << "\\n"            << firstSim
            << "\\nPtr: "       << &node
            << "\\n---------------"
            << "\\nFrame: " << state.getTime()
            << "\\nHP: " << p1.currentHP() << "  " << p2.currentHP()
            << "\\nAtk: " << p1.nextAttackActionTime() << "  " << p2.nextAttackActionTime()
            << "\\nMove: " << p1.nextMoveActionTime() << "  " << p2.nextMoveActionTime()
            << "\\nPrev: " << p1.previousActionTime() << "  " << p2.previousActionTime();
    
    std::string fillcolor       ("#aaaaaa");

    if (node.getPlayer() == Players::Player_One)
    {
        fillcolor = "#ff0000";
    }
    else if (node.getPlayer() == Players::Player_Two)
    {
        fillcolor = "#00ff00";
    }
    
    GraphViz::Node n(getNodeIDString(node));
    n.set("label",      label.str());
    n.set("fillcolor",  fillcolor);
    n.set("color",      "#000000");
    n.set("fontcolor",  "#000000");
    n.set("style",      "filled,bold");
    n.set("shape",      "box");
    g.addNode(n);

    // recurse for each child
    for (size_t c(0); c<node.numChildren(); ++c)
    {
        IUCTNode & child = node.getChild(c);
        if (child.numVisits() > 0)
        {
            GraphViz::Edge edge(getNodeIDString(node), getNodeIDString(child));
            g.addEdge(edge);
            printSubTreeGraphViz(child, g, state);
        }
    }
}
*/
 
std::string IUCTSearch::getNodeIDString(IUCTNode & node)
{
    std::stringstream ss;
    ss << (unsigned long long)&node;
    return ss.str();
}

IUCTSearchResults & IUCTSearch::getResults()
{
    return _results;
}