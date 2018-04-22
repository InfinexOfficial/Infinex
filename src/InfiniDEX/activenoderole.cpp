// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "activenoderole.h"
#include "chartdata.h"
#include "coininfo.h"
#include "net_processing.h"
#include "noderole.h"
#include "nodesetup.h"
#include "orderbook.h"
#include "trade.h"
#include "trademanager.h"
#include "tradepair.h"
#include "userbalance.h"
#include "userconnection.h"
#include "userdeposit.h"
#include "userinfo.h"
#include "usertradehistory.h"
#include "userwalletaddress.h"
#include "userwithdraw.h"

class CActiveNodeRole;

CActiveNodeRole activeNodeRole;

void CActiveNodeRole::ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{
	if (strCommand == NetMsgType::DEXNODEROLES)
	{
		std::vector<CNodeRole> incomings;
		vRecv >> incomings;

		std::vector<CNodeRole> toBroadcast;
		for (auto incoming : incomings)
		{
			if (!incoming.VerifySignature()) {
				LogPrintf("CNodeRoleManager::ProcessMessage -- invalid signature\n");
				Misbehaving(pfrom->GetId(), 100);
				return;
			}

			if (InputNodeRole(incoming))
				toBroadcast.push_back(incoming);
		}
		if (toBroadcast.size() > 0)
			BroadcastToConnectedNode(connman, toBroadcast);

		if (nodeSetup.NodeRoleSyncInProgress)
		{
			bool stillSync = false;

			if (completeNodeRoles.size() < nodeSetup.TotalNodeRoleCount)
				stillSync = true;

			NodeRoleWithID::reverse_iterator it = mapGlobalNodeRoles.rbegin();
			if (it == mapGlobalNodeRoles.rend())
				stillSync = true;
			else if (it->first < nodeSetup.LastNodeRoleID)
				stillSync = true;

			if (!stillSync)
				nodeSetup.NodeRoleSyncInProgress = false;
		}
	}
	else if (strCommand == NetMsgType::DEXNODEROLE)
	{
		CNodeRole incomingNodeRole;
		vRecv >> incomingNodeRole;

		if (!incomingNodeRole.VerifySignature()) {
			LogPrintf("CNodeRoleManager::ProcessMessage -- invalid signature\n");
			Misbehaving(pfrom->GetId(), 100);
			return;
		}

		if (InputNodeRole(incomingNodeRole))
			incomingNodeRole.Broadcast(connman);
	}
	else if (strCommand == NetMsgType::DEXINITIALSYNC)
	{
		connman.PushMessage(pfrom, NetMsgType::DEXCOINSINFO, completeCoinInfo);
		connman.PushMessage(pfrom, NetMsgType::DEXTRADEPAIRS, completeTradePair);
		connman.PushMessage(pfrom, NetMsgType::DEXNODEROLES, completeNodeRoles);
	}
}

void CActiveNodeRole::BroadcastToConnectedNode(CConnman& connman, std::vector<CNodeRole> nodeRoles)
{
	for (auto a : mapMNConnection)
	{
		if (!a.second.first->fDisconnect)
			connman.PushMessage(a.second.first, NetMsgType::DEXNODEROLES, nodeRoles);
	}
	for (auto a : mapUserConnections)
	{
		for (auto b : a.second)
		{
			if (!b.first->fDisconnect)
				connman.PushMessage(b.first, NetMsgType::DEXNODEROLES, nodeRoles);
		}
	}
}

bool CActiveNodeRole::InputNodeRole(CNodeRole &Role)
{
	std::shared_ptr<CNodeRole> nodeRole = std::make_shared<CNodeRole>(Role);
	if (!mapGlobalNodeRoles.count(Role.NodeRoleID))
	{
		completeNodeRoles.push_back(Role);
		std::pair<int, std::shared_ptr<CNodeRole>> temp = std::make_pair(Role.NodeRoleID, nodeRole);
		mapGlobalNodeRoles.insert(temp);
		mapGlobalNodeRolesByRole[Role.NodeRole].insert(temp);
		if (Role.TradePairID > 0)
			mapGlobalNodeRolesByTradePairID[Role.TradePairID].insert(temp);
		if (Role.Char != NULL)
			mapGlobalNodeRolesByChar[Role.Char].insert(temp);
		if (Role.NodePubKey == MNPubKey)
		{
			mapLocalNodeRoles[Role.NodeRole].insert(temp);
			ProcessNewRole(Role);
		}
	}
	else
	{
		auto& a = mapGlobalNodeRoles[Role.NodeRoleID];
		if (a->LastUpdateTime < Role.LastUpdateTime)
		{
			*a = *nodeRole;
			std::vector<CNodeRole>::reverse_iterator it = completeNodeRoles.rbegin();
			while (it != completeNodeRoles.rend())
			{
				if (it->NodeRoleID == Role.NodeRoleID)
				{
					*it = Role;
					break;
				}
			}
		}
	}

	return true;
}

bool CActiveNodeRole::ProcessNewRole(CNodeRole &Role)
{
	if (Role.TradePairID > 0)
	{
		if (Role.NodeRole == INFINIDEX_BID_BOOK_BROADCAST)
		{
			orderBookManager.InitTradePair(Role.TradePairID);
			userTradeManager.AssignBidBroadcastRole(Role.TradePairID);
		}
		else if (Role.NodeRole == INFINIDEX_ASK_BOOK_BROADCAST)
		{
			orderBookManager.InitTradePair(Role.TradePairID);
			userTradeManager.AssignAskBroadcastRole(Role.TradePairID);
		}
		else if (Role.NodeRole == INFINIDEX_USER_HISTORY_PROVIDER)
			userTradeManager.AssignUserHistoryProviderRole(Role.TradePairID);
		else if (Role.NodeRole == INFINIDEX_MARKET_HISTORY_PROVIDER)
		{
			userTradeManager.AssignMarketHistoryProviderRole(Role.TradePairID);
			userTradeHistoryManager.AssignMarketTradeHistoryBroadcastRole(Role.TradePairID);
		}
		else if (Role.NodeRole == INFINIDEX_CHART_DATA_PROVIDER)
			userTradeManager.AssignChartDataProviderRole(Role.TradePairID);
		else if (Role.NodeRole == INFINIDEX_TRADE_PROCESSOR)
			userTradeManager.AssignMatchUserTradeRole(Role.TradePairID);
	}
	if (Role.Char != NULL)
	{
		if (Role.NodeRole == INFINIDEX_BALANCE_HANDLER)
		{
			if (!Role.IsBackup)
				userBalanceManager.AssignUserBalanceRole(Role.Char);
			else
				userBalanceManager.AssignBackupRole(Role.Char);
		}
		else if (Role.NodeRole == INFINIDEX_MARKET_OVERVIEW_PROCESSOR)
		{

		}
		else if (Role.NodeRole == INFINIDEX_MARKET_OVERVIEW_PROVIDER)
		{

		}
		else if (Role.NodeRole == INFINIDEX_WALLET_ADDRESS)
			userWalletAddressManager.AssignDepositInfoRole(Role.CoinID);
		else if (Role.NodeRole == INFINIDEX_WITHDRAW_INFO)
			userWithdrawManager.AssignWithdrawInfoRole(Role.CoinID);
		else if (Role.NodeRole == INFINIDEX_DEPOSIT_INFO)
			userDepositManager.AssignDepositInfoRole(Role.CoinID);
	}
	return true;
}

bool CActiveNodeRole::IsInCharge(int TradePairID, infinidex_node_role_enum RoleType)
{
	return true;
}

bool CActiveNodeRole::RemoveRole(int TradePairID, int NodeRoleID)
{
	return true;
}