// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERCONNECTION_H
#define USERCONNECTION_H

#include "hash.h"
#include "net.h"
#include "utilstrencodings.h"

class CUserPubKeyInfo;
class CUserConnection;
class CUserConnectionManager;

extern std::map<std::string, CUserConnection> mapUserConnection; //user public key & connection info

class CUserConnection
{
public:
    std::string nUserPubKey;
    std::string nIP;
    std::string nPort;
	uint64_t nLastSeenTime;

	CUserBalance(std::string nUserPubKey, std::string nIP, std::string nPort, uint64_t nLastSeenTime) :
		nUserPubKey(nUserPubKey),
        nIP(nIP),
        nPort(nPort),
        nLastSeenTime(nLastSeenTime)
	{}

	CUserBalance() :
		nUserPubKey(""),
        nIP(""),
        nPort(""),
        nLastSeenTime(0)
	{}
};

class CUserConnectionManager
{
public:
    CUserConnectionManager() {}

    void ProcessUserConnection(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
}

#endif
