// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRADEMANAGER_H
#define TRADEMANAGER_H

#include <memory>
#include "net.h"
#include "trade.h"
#include "tradepair.h"

class CTradeManager
{
public:
	void ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
	void InputUserTrade(CUserTrade& userTrade, CNode* pfrom, CConnman& connman);
	void InputTradeCancel(CCancelTrade& cancelTrade, CNode* pfrom, CConnman& connman);
	bool ProcessTradeCancelRequest(CCancelTrade& cancelTrade, CUserTradeSetting& userTradeSetting);
	void ReturnTradeCancelBalance(CCancelTrade& cancelTrade);
	void InitTradePair(int TradePairID);
	void AssignBidBroadcastRole(int TradePairID, bool toAssign = true);
	void AssignAskBroadcastRole(int TradePairID, bool toAssign = true);
	void AssignUserHistoryProviderRole(int TradePairID, bool toAssign = true);
	void AssignMarketHistoryProviderRole(int TradePairID, bool toAssign = true);
	void AssignChartDataProviderRole(int TradePairID, bool toAssign = true);
	void AssignMatchUserTradeRole(int TradePairID, bool toAssign = true);
	uint64_t GetBidRequiredAmount(uint64_t Price, uint64_t Qty, int TradeFee);
	uint64_t GetAskExpectedAmount(uint64_t Price, uint64_t Qty, int TradeFee);
	bool IsSubmittedBidValid(const std::shared_ptr<CUserTrade>& userTrade, CTradePair& TradePair);
	bool IsSubmittedAskValid(const std::shared_ptr<CUserTrade>& userTrade, CTradePair& TradePair);
	bool IsSubmittedBidAmountValid(const std::shared_ptr<CUserTrade>& userTrade, int nTradeFee);
	bool IsSubmittedAskAmountValid(const std::shared_ptr<CUserTrade>& userTrade, int nTradeFee);
	bool ProcessUserTradeRequest(const std::shared_ptr<CUserTrade>& userTrade, CTradePair& tradePair);
	void SaveProcessedUserTrade(const std::shared_ptr<CUserTrade>& userTrade, CTradePair& tradePair);
	void InputMatchUserBuyRequest(const std::shared_ptr<CUserTrade>& userTrade, CTradePair& tradePair);
	void InputMatchUserSellRequest(const std::shared_ptr<CUserTrade>& userTrade, CTradePair& tradePair);
	bool ReduceBalanceQty(int TradePairID, int UserTradeID1, int UserTradeID2, uint64_t Qty);
	int64_t GetBalanceAmount(int TradePairID, uint64_t Price, int UserTradeID);
	void RequestUserTradeData(std::string UserPubKey, int TradePairID, CConnman& connman);
	bool GenerateActualTrade(std::shared_ptr<CActualTrade> actualTrade, CUserTradeSetting& setting);
	bool InputActualTrade(const std::shared_ptr<CActualTrade>& actualTrade, CUserTradeSetting& setting, CTradePair& tradePair);
	bool InputActualTradeFromNetwork(const std::shared_ptr<CActualTrade>& actualTrade, CUserTradeSetting& setting, CTradePair& tradePair);
	bool IsActualTradeInList(int TradePairID, int ActualTradeID, std::string Hash);
};

#endif