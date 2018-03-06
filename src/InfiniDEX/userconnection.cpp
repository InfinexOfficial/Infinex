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
		
        if(mapUserConnection.count(PublicKey))
        {
            
        }

		CUserConnection UserConnection;

	}
}