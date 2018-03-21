// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERCONNECTION_H
#define USERCONNECTION_H

#include <iostream>
#include <vector>
#include <map>

class CUserConnection;
class CUserConnectionManager;

extern std::map<std::string, CUserConnection> mapUserConnection; //user public key & connection info
extern std::map<std::string, CUserConnection> mapMNConnection; //MN IP address & connection info
extern CUserConnectionManager userConnectionManager;

class CNode
{

};

class CDataStream
{

};

class CConnman
{

};

class CUserConnection
{
public:
    CNode* nNode;
    std::string nUserPubKey;
    std::string nIP;
    std::string nPort;
	uint64_t nLastSeenTime;

	CUserConnection(CNode* nNode, std::string nUserPubKey, std::string nIP, std::string nPort, uint64_t nLastSeenTime) :
        nNode(nNode),
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
