/* msgrouterruleset.h
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

msgrouterruleset.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

msgrouterruleset.h was developed by Carlos Justiniano for use on the
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
 @file msgrouterruleset.h
 @brief Message Router Rule 
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 cMsgRouterRuleSet is used by the cMsgRouterRulesEngine.
*/

#ifndef MSG_ROUTER_RULESET_H
#define MSG_ROUTER_RULESET_H

#include <string>
#include <vector>

/**
 @class cMsgRouterRule
 @brief cMsgRouterRule holds a single rule which is used as part of a rule set
*/
class cMsgRouterRule
{
public:
	typedef enum MSG_ROUTER_OPERATION_TYPE
	{ 
		MSG_RTO_NOTSET = -1, MSG_RTO_CONTAINS = 0, MSG_RTO_MATCH = 1, MSG_RTO_LIKE = 2
	} eMsgRouterOperationType;

	typedef enum MSG_ROUTER_LOCATION_TYPE
	{ 
		MSG_RTL_NOTSET = -1, MSG_RTL_HEADER = 0, MSG_RTL_PAYLOAD = 1, MSG_RTL_STATUS = 2,
		MSG_RTL_ALL = 3, 
	} eMsgRouterLocationType;

	eMsgRouterLocationType m_Location;
	eMsgRouterOperationType m_OperationType;
	std::string m_MatchPattern;
	std::string m_Value;
};

/**
 @class cMsgRouterRuleSet
 @brief cMsgRouterRuleSet is used by the cMsgRouterRulesEngine.
*/
class cMsgRouterRuleSet
{
public:
    cMsgRouterRuleSet(std::string &RuleName, std::string &RouteTo);
    ~cMsgRouterRuleSet();

	void AddRule(cMsgRouterRule::eMsgRouterLocationType Location, 
				 cMsgRouterRule::eMsgRouterOperationType OperationType,
				 std::string &MatchPattern,
				 std::string &Value);
	cMsgRouterRule *GetFirstRule();
	cMsgRouterRule *GetNextRule();
	const char *GetRouteTo() { return m_RouteTo.c_str(); }
private:
	std::string m_RuleSetName;
	std::string m_RouteTo;
	std::vector<cMsgRouterRule*> m_RuleSet;
	std::vector<cMsgRouterRule*>::iterator m_RuleSetIterator;
};

#endif // MSG_ROUTER_RULESET_H

