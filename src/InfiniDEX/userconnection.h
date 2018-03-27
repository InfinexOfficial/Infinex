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

extern std::map<std::string, std::vector<CUserConnection>> mapUserConnections; //user public key & connection info
extern std::map<std::string, CUserConnection> mapMNConnection; //MN IP address & connection info
extern CUserConnectionManager userConnectionManager;

class CNode{};
class CDataStream{};
class CConnman{};

class CUserConnection
{
public:
    CNode* nNode;
    std::string nUserPubKey;
    std::string nIP;
    std::string nPort;
	uint64_t nLastSeenTime;

	CUserConnection(CNode& Node, std::string nUserPubKey, std::string nIP, std::string nPort, uint64_t nLastSeenTime) :
		nNode(&Node),
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
};

class CUserConnectionManager
{
private:
	bool IsUserInList(std::string PubKey);

public:
    CUserConnectionManager() {}
    void ProcessUserConnection(CNode* node, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
	void AddUserConnection(CNode* node, std::string IP, std::string port, std::string PubKey); //to remove IP & port on actual implementation
	bool GetUserConnection(std::string PubKey, std::vector<CUserConnection>& nodes);
    void UserDisconnected(std::string PubKey, std::string IPAddress);
    void MNDisconnected(std::string IPAddress);
};

#endif
