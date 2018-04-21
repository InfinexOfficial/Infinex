// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "messagesigner.h"
#include "net_processing.h"
#include "noderole.h"
#include "userconnection.h"

class CUserConnection;
class CUserConnectionManager;

//new user to verify their private key to public key signing
std::map<int, std::vector<pairConnectionInfo>> mapTradePairConnections;
std::map<std::string, std::vector<pairConnectionInfo>> mapUserConnections; //user public key & connection info
std::map<std::string, pairConnectionInfo> mapMNConnection; //MN IP address & connection info
CUserConnectionManager userConnectionManager;
std::vector<std::string> InfiniDEXSeed;

void CUserConnectionManager::ProcessUserConnection(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{
    if (strCommand == NetMsgType::DEXUSERCONNECTION) 
	{
		CUserConnection userConnection;
		vRecv >> userConnection;

		if (!userConnection.VerifySignature()) {
			LogPrintf("CUserConnectionManager::ProcessMessage -- invalid signature\n");
			Misbehaving(pfrom->GetId(), 100);
			return;
		}

		if(IsTargetedIPLocal(userConnection.nTargetIP))
		{
			
		}
	}
}

bool CUserConnection::VerifySignature()
{
	return true;
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

void CUserConnectionManager::InputUserConnection(CNode* node, std::string PubKey)
{
	
}

bool CUserConnectionManager::GetUserConnection(std::string PubKey, std::vector<CUserConnection>& nodes)
{
	return true;
}