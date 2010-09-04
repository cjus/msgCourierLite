/* msgrouterruleset.cpp
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

msgrouterruleset.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

msgrouterruleset.cpp was developed by Carlos Justiniano for use on the
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
 @file msgrouterruleset.cpp
 @brief Message Router Rule 
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 cMsgRouterRuleSet is used by the cMsgRouterRulesEngine.
*/

#include "exception.h"
#include "log.h"
#include "msgrouterruleset.h"

using namespace std;

cMsgRouterRuleSet::cMsgRouterRuleSet(string &RuleName, string &RouteTo)
{
	try
	{	
		m_RuleSetName = RuleName;
		m_RouteTo = RouteTo;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

cMsgRouterRuleSet::~cMsgRouterRuleSet()
{
	try
	{	
		for (m_RuleSetIterator = m_RuleSet.begin();
			 m_RuleSetIterator != m_RuleSet.end(); 
			 m_RuleSetIterator++)
		{
			delete (*m_RuleSetIterator);
		}
		m_RuleSet.erase(m_RuleSet.begin(),m_RuleSet.end());
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

void cMsgRouterRuleSet::AddRule(cMsgRouterRule::eMsgRouterLocationType Location, 
								cMsgRouterRule::eMsgRouterOperationType OperationType,
								string &MatchPattern, string &Value)
{
	cMsgRouterRule *pRule;
	MC_NEW(pRule, cMsgRouterRule());

	try
	{
		pRule->m_Location = Location;
		pRule->m_OperationType = OperationType;
		pRule->m_MatchPattern = MatchPattern;
		pRule->m_Value = Value;
		m_RuleSet.push_back(pRule);
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

cMsgRouterRule *cMsgRouterRuleSet::GetFirstRule()
{
	cMsgRouterRule *pRule = NULL;
	try
	{	
		m_RuleSetIterator = m_RuleSet.begin();
		pRule = (*m_RuleSetIterator);
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return pRule;
}

cMsgRouterRule *cMsgRouterRuleSet::GetNextRule()
{
	cMsgRouterRule *pRule = NULL;
	try
	{
		m_RuleSetIterator++;
		if (m_RuleSetIterator == m_RuleSet.end())
			return 0;
		pRule = (*m_RuleSetIterator);
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return pRule;
}

