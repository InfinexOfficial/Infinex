// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACTIVENODEROLE_H
#define ACTIVENODEROLE_H

#include <map>
#include <vector>
#include "net.h"
#include "noderole.h"

class CActiveNodeRole;

extern CActiveNodeRole activeNodeRole;

class CActiveNodeRole
{
public:
	void ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
	bool InputNodeRole(CNodeRole Role);
	void BroadcastToConnectedNode(CConnman& connman, std::vector<CNodeRole> nodeRoles);
	bool IsInCharge(int TradePairID, infinidex_node_role_enum RoleType);
	bool RemoveRole(int TradePairID, int NodeRoleID);
};

#endif