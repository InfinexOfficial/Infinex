// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "userwithdraw.h"

void ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
void ProcessUserWithdrawRequest(CUserWithdraw UserWithdrawRequest);
void SendUserWithdrawalRecords(std::string UserPubKey, int CoinID);
void SendUsersWithdrawalRecords(CNode* node, CConnman& connman, int CoinID);