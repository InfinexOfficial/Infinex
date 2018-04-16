// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "noderole.h"
#include "userconnection.h"

class CUserConnection;
class CUserConnectionManager;

//new user to verify their private key to public key signing
std::map<int, std::vector<pairConnectionInfo>> mapTradePairConnections;
std::map<std::string, std::vector<pairConnectionInfo>> mapUserConnections; //user public key & connection info
std::map<std::string, pairConnectionInfo> mapMNConnection; //MN IP address & connection info
CUserConnectionManager userConnectionManager;
std::string MNPubKey;
std::string DEXKey = "028afd3503f2aaa0898b853e1b28cdcb5fd422b5dc6426c92cf2b14c4b4ebeb969";
std::string dexMasterPrivKey;

bool CUserConnectionManager::IsUserInList(std::string PubKey)
{
	return mapUserConnections.count(PubKey);
}

void CUserConnectionManager::ProcessUserConnection(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{
    
}

bool CUserConnectionManager::IsTargetedIPLocal(std::string TargetIP)
{
	LOCK(cs_mapLocalHost);
    BOOST_FOREACH(const PAIRTYPE(CNetAddr, LocalServiceInfo) &item, mapLocalHost)
    {
        if(item.first.ToString() == TargetIP)
			return true;
    }
	return false;
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

bool CUserConnectionManager::SetDEXPrivKey(std::string dexPrivKey)
{
	CNodeRole role;
	role.DEXSign(dexPrivKey);
	if (role.VerifySignature()) {		
		LogPrintf("CUserConnectionManager::SetDEXPrivKey -- Successfully initialized as dex signer\n");
		dexMasterPrivKey = dexPrivKey;
		return true;
	}
	else {
		return false;
	}
}