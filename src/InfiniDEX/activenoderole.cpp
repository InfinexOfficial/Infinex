// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "activenoderole.h"

CNodeRoleManager nodeRoleManager;

bool CNodeRoleManager::IsInCharge(int TradePairID, infinidex_node_role_enum RoleType)
{
	if (!mapTradePairNodeRole.count(TradePairID))
	{
		return false;
	}

	for (auto& a : mapTradePairNodeRole[TradePairID])
	{
		if (a->NodeRole == RoleType && a->IsValid)
			return true;
	}
	return false;
}

bool CNodeRoleManager::AddNewRole(CNodeRole Role)
{
	return true;
}

bool CNodeRoleManager::RemoveRole(int TradePairID, int NodeRoleID)
{
	if (!mapTradePairNodeRole.count(TradePairID))
		return true;

	for (std::vector<std::shared_ptr<CNodeRole>>::iterator it = mapTradePairNodeRole[TradePairID].begin(); it != mapTradePairNodeRole[TradePairID].end(); ++it)
	{
		if ((*it)->NodeRoleID == NodeRoleID)
		{
			mapTradePairNodeRole[TradePairID].erase(it);
		}
	}
	return true;
}