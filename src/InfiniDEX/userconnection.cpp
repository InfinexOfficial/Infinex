// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "userconnection.h"

class CUserConnection;
class CUserConnectionManager;

//new user to verify their private key to public key signing

std::map<std::string, CUserConnection> mapUserConnection; //user public key & connection info
std::map<std::string, CUserConnection> mapMNConnection; //MN IP address & connection info
CUserConnectionManager userConnectionManager;

void CUserConnectionManager::ProcessUserConnection(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{
    
}

void CUserConnectionManager::UserDisconnected(std::string IPAddress)
{
}

void CUserConnectionManager::MNDisconnected(std::string IPAddress)
{
}