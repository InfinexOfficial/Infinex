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

			std::map<int, CNodeRole>::reverse_iterator it = mapGlobalNodeRolesByID.rbegin();
			if (it == mapGlobalNodeRolesByID.rend())
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

bool CActiveNodeRole::InputNodeRole(CNodeRole Role)
{
	if (mapNodeRoleByID.count(Role.NodeRoleID))
	{
		
	}

	if (Role.NodePubKey == MNPubKey)
	{
		CTradePair tp = tradePairManager.GetTradePair(Role.TradePairID);
		if (Role.NodeRole == INFINIDEX_BALANCE_INFO)
			userBalanceManager.AssignUserBalanceRole(Role.Char);
		else if (Role.NodeRole == INFINIDEX_BID_BOOK_BROADCAST)
		{
			orderBookManager.InitTradePair(tp.nTradePairID);
			userTradeManager.AssignBidBroadcastRole(tp.nTradePairID);
			//request complete data from other node		
		}
		else if (Role.NodeRole == INFINIDEX_ASK_BOOK_BROADCAST)
		{
			orderBookManager.InitTradePair(tp.nTradePairID);
			userTradeManager.AssignAskBroadcastRole(tp.nTradePairID);
			//request complete data from other node		
		}
		else if (Role.NodeRole == INFINIDEX_USER_HISTORY_PROVIDER)
			userTradeManager.AssignUserHistoryProviderRole(tp.nTradePairID);
		else if (Role.NodeRole == INFINIDEX_MARKET_HISTORY_PROVIDER)
		{
			userTradeManager.AssignMarketHistoryProviderRole(tp.nTradePairID);
			userTradeHistoryManager.AssignMarketTradeHistoryBroadcastRole(tp.nTradePairID);
		}
		else if (Role.NodeRole == INFINIDEX_CHART_DATA_PROVIDER)
			userTradeManager.AssignChartDataProviderRole(tp.nTradePairID);
		else if (Role.NodeRole == INFINIDEX_TRADE_PROCESSOR)
			userTradeManager.AssignMatchUserTradeRole(tp.nTradePairID);
		else if (Role.NodeRole == INFINIDEX_WALLET_ADDRESS)
			userWalletAddressManager.AssignDepositInfoRole(Role.CoinID);
		else if (Role.NodeRole == INFINIDEX_WITHDRAW_INFO)
			userWithdrawManager.AssignWithdrawInfoRole(Role.CoinID);
		else if (Role.NodeRole == INFINIDEX_WITHDRAW_PROCESSOR)
		{
			userWithdrawManager.AssignWithdrawProcessorRole(Role.CoinID);
		}
		else if (Role.NodeRole == INFINIDEX_DEPOSIT_INFO)
			userDepositManager.AssignDepositInfoRole(tp.nTradePairID);
		else if (Role.NodeRole == INFINIDEX_TRUSTED_NODE)
		{

		}
		else if (Role.NodeRole == INFINIDEX_VERIFICATOR)
		{

		}
		else if (Role.NodeRole == INFINIDEX_BACKUP_NODE)
		{

		}
		else if (Role.NodeRole == INFINIDEX_MARKET_OVERVIEW_PROCESSOR)
		{

		}
		else if (Role.NodeRole == INFINIDEX_MARKET_OVERVIEW_PROVIDER)
		{

		}
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