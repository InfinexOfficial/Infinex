// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACTIVENODEROLE_H
#define ACTIVENODEROLE_H

#include "noderole.h"
#include "net.h"

class CNodeRoleManager;
extern CNodeRoleManager nodeRoleManager;

class CNodeRoleManager
{
public:
	void ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
	bool IsInCharge(int TradePairID, infinidex_node_role_enum RoleType);
	bool UpdateRole(CNodeRole Role);
	bool RemoveRole(int TradePairID, int NodeRoleID);
};

#endif