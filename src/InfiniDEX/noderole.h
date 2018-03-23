// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODEROLE_H
#define NODEROLE_H

#include <iostream>
#include <vector>
#include <map>

class CNodeRole;
class CNodeRoleManager;

typedef std::pair <std::string, std::string> pairIPPubkey;
typedef std::pair <pairIPPubkey, std::vector<CNodeRole*>> pairIPPubKeyNodeRole;
typedef std::pair <infinidex_node_role_enum, std::vector<CNodeRole*>> pairNodeRoleByRole;
extern std::map<int, pairIPPubKeyNodeRole> mapTradePairGlobalRoleByIPPubKey;
extern std::map<int, pairNodeRoleByRole> mapTradePairGlobalRoleByRole;
extern std::map<int, std::vector<CNodeRole*>> mapTradePairNodeRole;
extern CNodeRoleManager nodeRoleManager;

enum infinidex_node_role_enum {
	INFINIDEX_NOTHING = 0,
	INFINIDEX_USER = 1,
	INFINIDEX_ORDERBOOK_BROADCAST = 2,
	INFINIDEX_HISTORY_PROVIDER = 3,
	INFINIDEX_CHARTDATA_PROVIDER = 4,
	INFINIDEX_BALANCE_INFO = 5,
	INFINIDEX_TRUSTED_NODE = 6,
	INFINIDEX_VERIFICATOR = 7,
	INFINIDEX_BACKUP_NODE = 8
};

extern std::map<int, std::vector<infinidex_node_role_enum>> mapCoinIDNodeRoles;
extern CNodeRoleManager nodeRoleManager;

class CNodeRole
{
public:
	int NodeRoleID;
	int TradePairID;
	infinidex_node_role_enum NodeRole;
	std::string NodeIP;
	std::string NodePubKey;
	bool IsValid;
	int ToReplaceNodeRoleID;
	uint64_t AppointTime;
	std::string AuthorisedSignature;

	CNodeRole(int NodeRoleID, int TradePairID, infinidex_node_role_enum NodeRole, std::string NodeIP, std::string NodePubKey, bool IsValid, int ToReplaceNodeRoleID, uint64_t AppointTime, std::string AuthorisedSignature):
		NodeRoleID(NodeRoleID),
		TradePairID(TradePairID),
		NodeRole(NodeRole),
		NodeIP(NodeIP),
		NodePubKey(NodePubKey),
		IsValid(IsValid),
		ToReplaceNodeRoleID(ToReplaceNodeRoleID),
		AppointTime(AppointTime),
		AuthorisedSignature(AuthorisedSignature)
	{}
};

class CNodeRoleManager
{
	
};

#endif