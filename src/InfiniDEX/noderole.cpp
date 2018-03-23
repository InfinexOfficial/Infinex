// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "noderole.h"

std::map<int, pairIPPubKeyNodeRole> mapTradePairGlobalRoleByIPPubKey;
std::map<int, pairNodeRoleByRole> mapTradePairGlobalRoleByRole;
std::map<int, std::vector<CNodeRole*>> mapTradePairNodeRole;
CNodeRoleManager nodeRoleManager;