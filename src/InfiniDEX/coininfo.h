// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef COININFO_H
#define COININFO_H

#include <map>
#include <memory>
#include <vector>
#include "net.h"
#include "utilstrencodings.h"

class CCoinInfo;
class CCoinInfoManager;

extern std::map<int, std::shared_ptr<CCoinInfo>> mapCompleteCoinInfoWithID;
extern std::map<std::string, std::shared_ptr<CCoinInfo>> mapCompleteCoinInfoWithSymbol;
extern std::vector<CCoinInfo> completeCoinInfo;
extern CCoinInfoManager coinInfoManager;

class CCoinInfo
{
private:
	std::vector<unsigned char> vchSig;

public:
	int nCoinInfoID;
	std::string nName;
	std::string nSymbol;
	std::string nLogoURL;
	int nBlockTime;
	int nBlockHeight;
	std::string nWalletVersion;
	bool nWalletActive;
	std::string nWalletStatus;
	uint64_t nLastUpdate;

	CCoinInfo(int nCoinInfoID, std::string nName, std::string nSymbol, std::string nLogoURL, int nBlockTime,
		int nBlockHeight, std::string nWalletVersion, bool nWalletActive, std::string nWalletStatus, uint64_t nLastUpdate) :
		nCoinInfoID(nCoinInfoID),
		nName(nName),
		nSymbol(nSymbol),
		nLogoURL(nLogoURL),
		nBlockTime(nBlockTime),
		nBlockHeight(nBlockHeight),
		nWalletVersion(nWalletVersion),
		nWalletActive(nWalletActive),
		nWalletStatus(nWalletStatus),
		nLastUpdate(nLastUpdate)
	{}

	CCoinInfo() :
		nCoinInfoID(0),
		nName(""),
		nSymbol(""),
		nLogoURL(""),
		nBlockTime(0),
		nBlockHeight(0),
		nWalletVersion(""),
		nWalletActive(false),
		nWalletStatus(""),
		nLastUpdate(0)
	{}

	ADD_SERIALIZE_METHODS;
	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) 
	{
		READWRITE(nCoinInfoID);
		READWRITE(nName);
		READWRITE(nSymbol);
		READWRITE(nLogoURL);
		READWRITE(nBlockTime);
		READWRITE(nBlockHeight);
		READWRITE(nWalletVersion);
		READWRITE(nWalletActive);
		READWRITE(nWalletStatus);
		READWRITE(nLastUpdate);
		READWRITE(vchSig);
	}

	bool VerifySignature();
	bool Sign();
	void Relay(CNode* node, CConnman& connman);	
};

class CCoinInfoManager
{
public:
	CCoinInfoManager() {}
	void ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
	void BroadcastToConnectedNode(CConnman& connman, std::vector<CCoinInfo> coinInfo);
	void PushCoinInfoToNode(std::string symbol, CNode* pnode, CConnman& connman);	
	void InputCoinInfo(CCoinInfo CoinInfo);
	bool IsCoinInCompleteListByCoinID(int CoinID);
	bool IsCoinInCompleteListBySymbol(std::string Symbol);
	bool GetCoinInfoByCoinID(int CoinID, CCoinInfo &CoinInfo);
	bool GetCoinInfoBySymbol(std::string Symbol, CCoinInfo &CoinInfo);
	//to remove on actual implementation
	bool AddCoinInfo(std::string coinID, std::string name, std::string symbol, std::string logoURL, std::string blockTime, std::string blockHeight, std::string walletVersion, std::string walletActive, std::string walletStatus);
	void Broadcast();
};

#endif