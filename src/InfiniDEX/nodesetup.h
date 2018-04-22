// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODESETUP_H
#define NODESETUP_H

#include "net.h"
#include "utilstrencodings.h"

class CNodeSetup;
class CNodeSetupManager;

extern CNodeSetup nodeSetup;
extern CNodeSetupManager nodeSetupManager;

class CNodeSetup
{
private:
	std::vector<unsigned char> vchSig;

public:
	int TotalCoinCount;
	int LastCoinID;
	bool CoinSyncInProgress;
	int TotalTradePairCount;
	int LastTradePairID;
	bool TradePairSyncInProgress;
	int TotalNodeRoleCount;
	int LastNodeRoleID;
	bool NodeRoleSyncInProgress;
	uint64_t LastUpdateTime;

	CNodeSetup() :
		TotalCoinCount(0),
		LastCoinID(0),
		CoinSyncInProgress(false),
		TotalTradePairCount(0),
		LastTradePairID(0),
		TradePairSyncInProgress(false),
		TotalNodeRoleCount(0),
		LastNodeRoleID(0),
		NodeRoleSyncInProgress(false),
		LastUpdateTime(0)
	{}

	ADD_SERIALIZE_METHODS;
	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
		READWRITE(TotalCoinCount);
		READWRITE(LastCoinID);
		READWRITE(CoinSyncInProgress);
		READWRITE(TotalTradePairCount);
		READWRITE(LastTradePairID);
		READWRITE(TradePairSyncInProgress);
		READWRITE(TotalNodeRoleCount);
		READWRITE(LastNodeRoleID);
		READWRITE(NodeRoleSyncInProgress);
		READWRITE(LastUpdateTime);
		READWRITE(vchSig);
	}

	bool VerifySignature();
};

class CNodeSetupManager
{
public:
	CNodeSetupManager() {}
	void ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
};

#endif