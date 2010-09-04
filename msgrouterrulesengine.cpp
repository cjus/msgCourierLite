/* msgrouterrulesengine.cpp
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

msgrouterrulesengine.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

msgrouterrulesengine.cpp was developed by Carlos Justiniano for use on the
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
 @file msgrouterrulesengine.cpp
 @brief Message Router Rules Engine 
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 cMsgRouterRulesEngine manages a collection of cMsgRouterRuleSet's.
*/

#include "exception.h"
#include "log.h"

using namespace std;

#include "msgrouterrulesengine.h"

cMsgRouterRulesEngine::cMsgRouterRulesEngine()
{
}

cMsgRouterRulesEngine::~cMsgRouterRulesEngine()
{
	try
	{	
		for (m_RuleSetsIterator = m_RuleSets.begin();
			 m_RuleSetsIterator != m_RuleSets.end(); 
			 m_RuleSetsIterator++)
		{
			delete (*m_RuleSetsIterator);
		}
		m_RuleSets.erase(m_RuleSets.begin(),m_RuleSets.end());
		m_RulesMap.erase(m_RulesMap.begin(),m_RulesMap.end());
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

void cMsgRouterRulesEngine::AddRule(string &RuleSetName, string &RouteTo,
				cMsgRouterRule::eMsgRouterLocationType Location, 
				cMsgRouterRule::eMsgRouterOperationType OperationType,
				std::string &MatchPattern,
				std::string &Value)
{
	cMsgRouterRuleSet *pRuleSet = 0;
	map<string,cMsgRouterRuleSet*>::iterator it;

	try
	{
		it = m_RulesMap.find(RuleSetName);
		if (it != m_RulesMap.end())
		{
			pRuleSet = (*it).second;
		}
		else
		{
			MC_NEW(pRuleSet, cMsgRouterRuleSet(RuleSetName, RouteTo));
			m_RuleSets.push_back(pRuleSet);
			m_RulesMap.insert(pair<string,cMsgRouterRuleSet*>(RuleSetName, pRuleSet));
		}

		pRuleSet->AddRule(Location, OperationType, MatchPattern, Value);	
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

bool cMsgRouterRulesEngine::Search(cMsg *pMsg, std::string &Match)
{
	bool bTests = false;
	cMsgRouterRuleSet *pRuleSet;
	cMsgRouterRule *pRule;

	try
	{
		for (m_RuleSetsIterator = m_RuleSets.begin();
			 m_RuleSetsIterator != m_RuleSets.end(); 
			 m_RuleSetsIterator++)
		{
			pRuleSet = *m_RuleSetsIterator;
			if (pRuleSet)
			{
				pRule = pRuleSet->GetFirstRule();
				if (pRule == 0)
					continue;
				do 
				{
					if (pRule->m_OperationType == cMsgRouterRule::MSG_RTO_CONTAINS)
						bTests = TestContains(pMsg, pRule);
					//else if (pRule->m_OperationType == cMsgRouterRule::MSG_RTO_MATCH)
					//	bTests = TestMatch(pMsg, pRule);
					//else if (pRule->m_OperationType == cMsgRouterRule::MSG_RTO_LIKE)
					//	bTests = TestLike(pMsg, pRule);

					if (bTests == false)
						break;
				} while (pRule = pRuleSet->GetNextRule());

				if (bTests == true)
				{
					Match = pRuleSet->GetRouteTo();
					return true;
				}
			}
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return false;
}

bool cMsgRouterRulesEngine::TestContains(cMsg *pMsg, cMsgRouterRule *pRule)
{
	if (pRule->m_Location == cMsgRouterRule::MSG_RTL_STATUS)
	{
		if (strstr(pMsg->GetStatusLine(), pRule->m_Value.c_str())!=0)
			return true;
	}
	else if (pRule->m_Location == cMsgRouterRule::MSG_RTL_HEADER)
	{
		cBuffer *pBuffer = pMsg->GetBuffer();
		int idx = pBuffer->Find((char*)pRule->m_Value.c_str());
		if (idx > 0 && idx < pMsg->GetHeaderSize())
			return true;
	}
	else if (pRule->m_Location == cMsgRouterRule::MSG_RTL_PAYLOAD)
	{
		cBuffer *pBuffer = pMsg->GetBuffer();
		int idx = pBuffer->Find((char*)pRule->m_Value.c_str(), pMsg->GetHeaderSize());
		if (idx > 0)
			return true;
	}
	return false;
}
bool cMsgRouterRulesEngine::TestMatch(cMsg *pMsg, cMsgRouterRule *pRule)
{
	try
	{
		if (pRule->m_Location == cMsgRouterRule::MSG_RTL_STATUS)
		{
			const char *pLine = pMsg->GetStatusLine();
			if (strstr(pLine, pRule->m_MatchPattern.c_str())!= NULL)
				return true;
		}
		else if (pRule->m_Location == cMsgRouterRule::MSG_RTL_HEADER)
		{
			cBuffer *pBuffer = pMsg->GetBuffer();
			string header;
			header.assign(pBuffer->cstr(), pMsg->GetHeaderSize());

			if (strstr(header.c_str(), pRule->m_MatchPattern.c_str())!= NULL)
				return true;
		}
		else if (pRule->m_Location == cMsgRouterRule::MSG_RTL_PAYLOAD)
		{
			// not currently implemented
			return false;
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return false;
}
bool cMsgRouterRulesEngine::TestLike(cMsg *pMsg, cMsgRouterRule *pRule)
{
	// not currently implemented
	return false;
}

