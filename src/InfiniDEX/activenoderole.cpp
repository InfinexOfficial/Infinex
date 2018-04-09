// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "activenoderole.h"
#include "chartdata.h"
#include "coininfo.h"
#include "markettradehistory.h"
#include "orderbook.h"
#include "trade.h"
#include "tradepair.h"
#include "userbalance.h"
#include "userconnection.h"
#include "userdeposit.h"
#include "usertradehistory.h"
#include "userwalletaddress.h"
#include "userwithdraw.h"

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

bool CNodeRoleManager::UpdateRole(CNodeRole Role)
{
	if (!Role.VerifySignature())
		return false;
	
	if (mapNodeRoleByID.count(Role.NodeRoleID))
	{
		auto& temp = mapNodeRoleByID[Role.NodeRoleID];
		if (temp->LastUpdateTime >= Role.LastUpdateTime)
			return true;

		if (temp->IsValid && !Role.IsValid)
		{

		}
		return true;
	}
	
	if (Role.NodePubKey == MNPubKey)
	{
		CTradePair tp = tradePairManager.GetTradePair(Role.TradePairID);
		if (Role.NodeRole == INFINIDEX_BALANCE_INFO)
		{
			globalUserBalanceHandler.nIsInChargeOfGlobalUserBalance = true;
		}
		else if (Role.NodeRole == INFINIDEX_BID_BOOK_BROADCAST)
			userTradeManager.AssignBidBroadcastRole(tp.nTradePairID);
		else if (Role.NodeRole == INFINIDEX_ASK_BOOK_BROADCAST)
			userTradeManager.AssignAskBroadcastRole(tp.nTradePairID);
		else if (Role.NodeRole == INFINIDEX_USER_HISTORY_PROVIDER)
			actualTradeManager.AssignUserHistoryProviderRole(tp.nTradePairID);
		else if (Role.NodeRole == INFINIDEX_MARKET_HISTORY_PROVIDER)
			actualTradeManager.AssignMarketHistoryProviderRole(tp.nTradePairID);
		else if (Role.NodeRole == INFINIDEX_CHART_DATA_PROVIDER)
			actualTradeManager.AssignChartDataProviderRole(tp.nTradePairID);
		else if (Role.NodeRole == INFINIDEX_TRADE_PROCESSOR)
			userTradeManager.AssignNodeToMatchUserTrade(tp.nTradePairID);
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