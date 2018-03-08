// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef COININFO_H
#define COININFO_H

#include "hash.h"
#include "net.h"
#include "utilstrencodings.h"

class CCoinInfo;
class CCoinInfoManager;

extern std::vector<CCoinInfo> vecCompleteCoinInfo;
extern std::vector<CCoinInfo> vecCoinInfoInNode;
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

	CCoinInfo(int nCoinInfoID, std::string nName, std::string nSymbol, std::string nLogoURL, int nBlockTime, int nBlockHeight, std::string nWalletVersion, bool nWalletActive, std::string nWalletStatus, uint64_t nLastUpdate) :
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
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
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

	uint256 GetHash() const
	{
		CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
		ss << nCoinInfoID;
		ss << nName;
		ss << nSymbol;
		ss << nLogoURL;
        ss << nBlockTime;
		ss << nBlockHeight;
		ss << nWalletVersion;
		ss << nWalletActive;        
		ss << nWalletStatus;
		ss << nLastUpdate;
		return ss.GetHash();
	}

	bool Sign(std::string strSignKey);
	bool CheckSignature();
	void Relay(CConnman& connman);
};

class CCoinInfoManager
{
private:
    std::vector<unsigned char> vchSig;

public:

    CCoinInfoManager() {}

    void ProcessCoinInfo(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
};

#endif