// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACTIVENODEROLE_H
#define ACTIVENODEROLE_H

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <noderole.h>

class CNodeRoleManager;

class CNodeRoleManager
{
	bool IsInCharge(int TradePairID, infinidex_node_role_enum RoleType);
	bool AddNewRole(CNodeRole Role);
	bool RemoveRole(int TradePairID, int NodeRoleID);
};

#endif