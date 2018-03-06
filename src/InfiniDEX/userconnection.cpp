// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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
        
    }
}