// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERCONNECTION_H
#define USERCONNECTION_H

#include <iostream>
#include <vector>
#include <map>
#include "hash.h"
#include "net.h"
#include "utilstrencodings.h"

class CUserConnection;
class CUserConnectionManager;

typedef std::pair<CNode*, CUserConnection> pairConnectionInfo;
extern std::map<int, std::vector<pairConnectionInfo>> mapTradePairConnections;
extern std::map<std::string, std::vector<pairConnectionInfo>> mapUserConnections; //user public key & connection info
extern std::map<std::string, pairConnectionInfo> mapMNConnection; //MN IP address & connection info
extern CUserConnectionManager userConnectionManager;
extern std::string MNPubKey; //temp
extern std::string DEXKey;
extern std::string dexMasterPrivKey;

class CUserConnection
{
private:
	std::vector<unsigned char> vchSig;

public:
    std::string nUserPubKey;
    std::string nIP;
    std::string nPort;
    std::string nTargetIP;
	int nTargetRoleID;
	uint64_t nLastSeenTime;

	CUserConnection(std::string nUserPubKey, std::string nIP, std::string nPort, std::string nTargetIP, int nTargetRoleID, uint64_t nLastSeenTime) :
		nUserPubKey(nUserPubKey),
        nIP(nIP),
        nPort(nPort),
		nTargetIP(nTargetIP),
		nTargetRoleID(nTargetRoleID),
        nLastSeenTime(nLastSeenTime)
	{}

	CUserConnection() :
		nUserPubKey(""),
        nIP(""),
        nPort(""),
		nTargetIP(""),
		nTargetRoleID(0),
        nLastSeenTime(0)
	{}

    ADD_SERIALIZE_METHODS;
	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
		READWRITE(nUserPubKey);
		READWRITE(nIP);
		READWRITE(nPort);
		READWRITE(nTargetIP);
		READWRITE(nTargetRoleID);
		READWRITE(nLastSeenTime);
		READWRITE(vchSig);
	}

    bool VerifySignature();
};

class CUserConnectionManager
{
private:
	bool IsUserInList(std::string PubKey);

public:
    CUserConnectionManager() {}
    void ProcessUserConnection(CNode* node, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
	bool IsTargetedIPLocal(std::string TargetIP);
	void InputUserConnection(CNode* node, std::string PubKey);
    bool SetDEXPrivKey(std::string dexPrivKey);
	bool GetUserConnection(std::string PubKey, std::vector<CUserConnection>& nodes);
};

#endif
