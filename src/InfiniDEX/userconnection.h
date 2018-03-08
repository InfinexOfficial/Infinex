// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERCONNECTION_H
#define USERCONNECTION_H

#include "hash.h"
#include "net.h"
#include "utilstrencodings.h"

class CUserConnection;
class CUserConnectionManager;

extern std::map<std::string, CUserConnection> mapUserConnection; //user public key & connection info
extern std::map<std::string, CUserConnection> mapMNConnection; //MN IP address & connection info
extern CUserConnectionManager userConnectionManager;

class CUserConnection
{
public:
    std::string nUserPubKey;
    std::string nIP;
    std::string nPort;
	uint64_t nLastSeenTime;

	CUserConnection(std::string nUserPubKey, std::string nIP, std::string nPort, uint64_t nLastSeenTime) :
		nUserPubKey(nUserPubKey),
        nIP(nIP),
        nPort(nPort),
        nLastSeenTime(nLastSeenTime)
	{}

	CUserConnection() :
		nUserPubKey(""),
        nIP(""),
        nPort(""),
        nLastSeenTime(0)
	{}

    void UpdateUserConnectionInfo(std::string IPIn, std::string PortIn, uint64_t LastSeenTimeIn)
    {
        nIP = IPIn;
        nPort = PortIn;
        nLastSeenTime = LastSeenTimeIn;
    }

    std::string getIP() { return nIP; }
};

class CUserConnectionManager
{
public:
    CUserConnectionManager() {}

    void ProcessUserConnection(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman);

    void UserDisconnected(std::string IPAddress);

    void MNDisconnected(std::string IPAddress);
};

#endif
