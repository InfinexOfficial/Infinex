// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "activemasternode.h"
#include "net_processing.h"
#include "userconnection.h"

class CUserConnection;
class CUserConnectionManager;

CUserConnectionManager userConnectionManager;

void CUserConnection::ProcessUserConnection(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{
    if (strCommand == NetMsgType::DEXUSERCONNECTION) 
    {
        std::string PublicKey;
		vRecv >> PublicKey;

        if(ConnectionMessage == "")
            return;
		
        std::string IPIn = pFrom->addrLocal.ToStringIP();
        std::string PortIn = pFrom->addrLocal.ToStringPort();
        uint64_t LastSeenTimeIn = GetAdjustedTime();

        if(mapUserConnection.count(PublicKey))
        {            
            mapUserConnection[PublicKey].UpdateUserConnectionInfo(IPIn, PortIn, LastSeenTimeIn);
            return;
        }

        CUserConnection UserConnection = new CUserConnection(ConnectionMessage,IPIn,PortIn,LastSeenTimeIn);
        mapUserConnection[ConnectionMessage] = UserConnection;
	}
    else if (strCommand == NetMsgType::DEXMNCONNECTION)
    {
        std::string IPIn = pFrom->addrLocal.ToStringIP();
        std::string PortIn = pFrom->addrLocal.ToStringPort();
        uint64_t LastSeenTimeIn = GetAdjustedTime();
        
        if(mapMNConnection.count(IPIn))
        {
            return;
        }

        bool validMN = false;
        std::map<COutPoint, CMasternode> mapMasternodes = mnodeman.GetFullMasternodeMap();
        for (auto& mnpair : mapMasternodes) 
        {
            CMasternode mn = mnpair.second;
            if(mn.addr.ToStringIP() == IPIn) 
            {
                validMN = true;
                CUserConnection MNConnection = new CUserConnection(mn.pubKeyMasternode, IPIn, PortIn, LastSeenTimeIn);
                mapMNConnection[IPIn] = MNConnection;
                return;
            }
        }
    }
}