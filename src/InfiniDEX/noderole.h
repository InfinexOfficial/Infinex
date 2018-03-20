// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODEROLE_H
#define NODEROLE_H

#include <iostream>
#include <vector>
#include <map>

class CNodeRole;
class CNodeRoleManager;

enum infinidex_node_role_enum {
	INFINIDEX_NOTHING=0,
    INFINIDEX_PRICE_BROADCAST=1,
    INFINIDEX_HISTORY_PROVIDER=2,
    INFINIDEX_BALANCE_INFO=3,
    INFINIDEX_TRUSTED_NODE=4,
    INFINIDEX_VERIFICATOR=5,
    INFINIDEX_BACKUP_NODE=6
};

extern std::map<int, std::vector<infinidex_node_role_enum>> mapCoinIDNodeRoles;
extern CNodeRoleManager nodeRoleManager;

#endif