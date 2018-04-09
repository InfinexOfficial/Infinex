// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "userconnection.h"

class CUserConnection;
class CUserConnectionManager;

//new user to verify their private key to public key signing

std::map<std::string, std::vector<CUserConnection>> mapUserConnections; //user public key & connection info
std::map<std::string, CUserConnection> mapMNConnection; //MN IP address & connection info
CUserConnectionManager userConnectionManager;
std::string MNPubKey;

bool CUserConnectionManager::IsUserInList(std::string PubKey)
{
	return mapUserConnections.count(PubKey);
}

void CUserConnectionManager::ProcessUserConnection(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{
    
}

void CUserConnectionManager::AddUserConnection(CNode* node, std::string IP, std::string port, std::string PubKey)
{
	if (!IsUserInList(PubKey))
	{
		CUserConnection cuc(node, PubKey, IP, port, 0);
		//node->AddRef();
		std::vector<CUserConnection> temp;
		temp.push_back(cuc);
		mapUserConnections.insert(std::make_pair(PubKey, temp));
	}
}

bool CUserConnectionManager::GetUserConnection(std::string PubKey, std::vector<CUserConnection>& nodes)
{
	if (!IsUserInList(PubKey))
		return false;

	nodes = mapUserConnections[PubKey];
	return true;
}

void CUserConnectionManager::UserDisconnected(std::string PubKey, std::string IPAddress)
{
}

void CUserConnectionManager::MNDisconnected(std::string IPAddress)
{
}