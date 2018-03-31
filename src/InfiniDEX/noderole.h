// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODEROLE_H
#define NODEROLE_H

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include "userconnection.h"

enum infinidex_node_role_enum {
	INFINIDEX_NOTHING = 0,
	INFINIDEX_USER = 1,
	INFINIDEX_BID_BOOK_BROADCAST = 2,
	INFINIDEX_ASK_BOOK_BROADCAST = 3,
	INFINIDEX_USER_HISTORY_PROVIDER = 4,
	INFINIDEX_MARKET_HISTORY_PROVIDER = 5,
	INFINIDEX_CHART_DATA_PROVIDER = 6,
	INFINIDEX_TRADE_PROCESSOR = 7,
	INFINIDEX_WALLET_ADDRESS = 8,
	INFINIDEX_BALANCE_INFO = 9,
	INFINIDEX_WITHDRAW_INFO = 10,
	INFINIDEX_WITHDRAW_PROCESSOR = 11,
	INFINIDEX_DEPOSIT_INFO = 12,
	INFINIDEX_TRUSTED_NODE = 13,
	INFINIDEX_VERIFICATOR = 14,
	INFINIDEX_BACKUP_NODE = 15,
	INFINIDEX_MARKET_OVERVIEW_PROCESSOR = 16,
	INFINIDEX_MARKET_OVERVIEW_PROVIDER = 17
};

class CNodeRole;

typedef std::pair <std::string, std::string> pairIPPubkey;
typedef std::pair <pairIPPubkey, std::vector<std::shared_ptr<CNodeRole>>> pairIPPubKeyNodeRole;
typedef std::pair <infinidex_node_role_enum, std::vector<std::shared_ptr<CNodeRole>>> pairNodeRoleByRole;
extern std::map<int, pairIPPubKeyNodeRole> mapTradePairGlobalRoleByIPPubKey;
extern std::map<int, pairNodeRoleByRole> mapTradePairGlobalRoleByRole;
extern std::map<int, std::vector<infinidex_node_role_enum>> mapCoinIDNodeRoles;

extern std::map<int, std::shared_ptr<CNodeRole>> mapNodeRole;
extern std::map<int, std::shared_ptr<CNodeRole>> mapNodeRoleByID;
extern std::map<int, std::vector<std::shared_ptr<CNodeRole>>> mapTradePairNodeRole;

class CNodeRole
{
private:
	std::vector<unsigned char> vchSig;

public:
	int NodeRoleID;
	int TradePairID;
	infinidex_node_role_enum NodeRole;
	std::string NodeIP;
	std::string NodePubKey;
	bool IsValid;
	int ToReplaceNodeRoleID;
	uint64_t LastUpdateTime;

	CNodeRole(int NodeRoleID, int TradePairID, infinidex_node_role_enum NodeRole, std::string NodeIP, std::string NodePubKey, bool IsValid, int ToReplaceNodeRoleID, uint64_t LastUpdateTime):
		NodeRoleID(NodeRoleID),
		TradePairID(TradePairID),
		NodeRole(NodeRole),
		NodeIP(NodeIP),
		NodePubKey(NodePubKey),
		IsValid(IsValid),
		ToReplaceNodeRoleID(ToReplaceNodeRoleID),
		LastUpdateTime(LastUpdateTime)
	{}

	CNodeRole() :
		NodeRoleID(0),
		TradePairID(0),
		NodeRole(INFINIDEX_NOTHING),
		NodeIP(""),
		NodePubKey(""),
		IsValid(false),
		ToReplaceNodeRoleID(0),
		LastUpdateTime(0)
	{}

	bool VerifySignature();
};

#endif