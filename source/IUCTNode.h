#pragma once

#include "Common.h"
#include "UnitAction.hpp"
#include "UnitScriptData.h"
#include "MoveArray.h"

namespace SparCraft
{


class IUCTNode
{
    // uct stat counting variables
    size_t                      _numVisits;         // total visits to this node
    double                      _numWins;           // wins from this node
    double                      _uctVal;            // previous computed UCT value
            
    // game specific variables
    size_t                      _player;            // the player who made a move to generate this node
    IDType                      _nodeType;
	const UnitScriptData				_scriptData;
	MoveArray					_moveArray;

    // holds children
    std::vector<IUCTNode>       _children;

    // nodes for traversing the tree
    IUCTNode *                  _parent;
    
public:

    IUCTNode ()
        : _numVisits            (0)
        , _numWins              (0)
        , _uctVal               (0)
        , _player               (Players::Player_None)
        , _nodeType             (SearchNodeType::Default)
        , _parent               (NULL)
    {

    }

	IUCTNode (IUCTNode * parent, const IDType player, const IDType nodeType, const UnitScriptData & scriptData, const size_t & maxChildren, std::vector<IUCTNode> * fromPool = NULL)
        : _numVisits            (0)
        , _numWins              (0)
        , _uctVal               (0)
        , _player               (player)
        , _nodeType             (nodeType)
        , _scriptData           (scriptData)
        , _parent               (parent)
    {
        _children.reserve(maxChildren);
    }

    const size_t    numVisits()                 const           { return _numVisits; }
    const double    numWins()                   const           { return _numWins; }
    const size_t    numChildren()               const           { return _children.size(); }
    const double    getUCTVal()                 const           { return _uctVal; }
    const bool      hasChildren()               const           { return numChildren() > 0; }
    const size_t    getNodeType()               const           { return _nodeType; }
    const IDType    getPlayer()                 const           { return _player; }
	
	MoveArray getMoveArray()					const           { return _moveArray; }
	const UnitScriptData getScriptData()		const			{ return _scriptData; }

    IUCTNode *       getParent()                 const           { return _parent; }
    IUCTNode &       getChild(const size_t & c)                  { return _children[c]; }

    void            setUCTVal(double val)                       { _uctVal = val; }
    void            incVisits()                                 { _numVisits++; }
    void            addWins(double val)                         { _numWins += val; }

    std::vector<IUCTNode> & getChildren()                        { return _children; }

	void setMoveArray(MoveArray moveArray){
		_moveArray = moveArray;
	}

    void addChild(IUCTNode * parent, const IDType player, const IDType nodeType, const UnitScriptData & scriptData, const size_t & maxChildren, std::vector<IUCTNode> * fromPool = NULL)
    {
        _children.push_back(IUCTNode(parent, player, nodeType, scriptData, maxChildren));
    }

    IUCTNode & mostVisitedChild() 
    {
        IUCTNode * mostVisitedChild = NULL;
        size_t mostVisits = 0;

       for (size_t c(0); c < numChildren(); ++c)
       {
           IUCTNode & child = getChild(c);

           if (!mostVisitedChild || (child.numVisits() > mostVisits))
           {
               mostVisitedChild = &child;
               mostVisits = child.numVisits();
           }
       }

       return *mostVisitedChild;
    }

    IUCTNode & bestUCTValueChild(const bool maxPlayer, const IUCTSearchParameters & params) 
    {
        IUCTNode * bestChild = NULL;
        double bestVal = maxPlayer ? std::numeric_limits<double>::min() : std::numeric_limits<double>::max();

        for (size_t c(0); c < numChildren(); ++c)
        {
            IUCTNode & child = getChild(c);
       
            double winRate      = (double)child.numWins() / (double)child.numVisits();
            double uctVal       = params.cValue() * sqrt( log( (double)numVisits() ) / ( child.numVisits() ) );
			double currentVal   = maxPlayer ? (winRate + uctVal) : (winRate - uctVal);

            if (maxPlayer)
            {
                if (currentVal > bestVal)
                {
                    bestVal             = currentVal;
			        bestChild           = &child;
                }
            }
            else if (currentVal < bestVal)
            {
                bestVal             = currentVal;
		        bestChild           = &child;
            }
        }

        return *bestChild;
    }
};
}