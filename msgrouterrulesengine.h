/* msgrouterrulesengine.h
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

msgrouterrulesengine.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

msgrouterrulesengine.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with main.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file msgrouterrulesengine.h
 @brief Message Router Rules Engine 
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 cMsgRouterRulesEngine manages a collection of cMsgRouterRuleSet's.
*/

#ifndef MSG_ROUTER_RULESENGINE_H
#define MSG_ROUTER_RULESENGINE_H

#include <map>
#include <vector>
#include <string>

#include "msgrouterruleset.h"
#include "msg.h"

/**
 @class cMsgRouterRulesEngine
 @brief cMsgRouterRuleSet is used by the cMsgRouterRulesEngine.
*/
class cMsgRouterRulesEngine
{
public:
    cMsgRouterRulesEngine();
    ~cMsgRouterRulesEngine();

	void AddRule(std::string &RuleSetName, std::string &RouteTo,
				 cMsgRouterRule::eMsgRouterLocationType Location, 
				 cMsgRouterRule::eMsgRouterOperationType OperationType,
				 std::string &MatchPattern,
				 std::string &Value);

	bool Search(cMsg *pMsg, std::string &Match);
private:
	std::vector<cMsgRouterRuleSet*> m_RuleSets;
	std::vector<cMsgRouterRuleSet*>::iterator m_RuleSetsIterator;
	std::map<std::string, cMsgRouterRuleSet*> m_RulesMap;

	bool TestContains(cMsg *pMsg, cMsgRouterRule *pRule);
	bool TestMatch(cMsg *pMsg, cMsgRouterRule *pRule);
	bool TestLike(cMsg *pMsg, cMsgRouterRule *pRule);
};

#endif // MSG_ROUTER_RULESENGINE_H

