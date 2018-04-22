// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "coininfo.h"
#include "messagesigner.h"
#include "net_processing.h"
#include "noderole.h"
#include "nodesetup.h"
#include "tradepair.h"
#include <boost/lexical_cast.hpp>

class CNodeSetup;
class CNodeSetupManager;

CNodeSetup nodeSetup;
CNodeSetupManager nodeSetupManager;

bool CNodeSetup::VerifySignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(TotalCoinCount) + boost::lexical_cast<std::string>(LastCoinID) + boost::lexical_cast<std::string>(TotalTradePairCount)
		+ boost::lexical_cast<std::string>(LastTradePairID) + boost::lexical_cast<std::string>(TotalNodeRoleCount) + boost::lexical_cast<std::string>(LastNodeRoleID)
		+ boost::lexical_cast<std::string>(LastUpdateTime);
	CPubKey pubkey(ParseHex(DEXKey));

	if (!CMessageSigner::VerifyMessage(pubkey, vchSig, strMessage, strError)) 
	{
		LogPrintf("CTradePair::VerifySignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

void CNodeSetupManager::ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{
	if (strCommand == NetMsgType::DEXNODESETUP)
	{
		CNodeSetup incoming;
		vRecv >> incoming;

		if (!incoming.VerifySignature())
		{
			LogPrintf("CNodeSetupManager::ProcessMessage -- invalid signature\n");
			Misbehaving(pfrom->GetId(), 100);
			return;

			if (nodeSetup.LastUpdateTime < incoming.LastUpdateTime)
			{
				nodeSetup = incoming;
				if (completeCoinInfo.size() < nodeSetup.TotalCoinCount)
					nodeSetup.CoinSyncInProgress = true;
				if (completeTradePair.size() < nodeSetup.TotalTradePairCount)
					nodeSetup.TradePairSyncInProgress = true;
				if (completeNodeRoles.size() < nodeSetup.TotalNodeRoleCount)
					nodeSetup.NodeRoleSyncInProgress = true;

				std::map<int, std::shared_ptr<CCoinInfo>>::reverse_iterator it = mapCompleteCoinInfoWithID.rbegin();
				if (it == mapCompleteCoinInfoWithID.rend())
					nodeSetup.CoinSyncInProgress = true;
				else if (it->first < nodeSetup.LastCoinID)
					nodeSetup.CoinSyncInProgress = true;

				std::map<int, CTradePair>::reverse_iterator it2 = mapTradePair.rbegin();
				if (it2 == mapTradePair.rend())
					nodeSetup.TradePairSyncInProgress = true;
				else if (it2->first < nodeSetup.LastTradePairID)
					nodeSetup.TradePairSyncInProgress = true;

				NodeRoleWithID::reverse_iterator it3 = mapGlobalNodeRoles.rbegin();
				if (it3 == mapGlobalNodeRoles.rend())
					nodeSetup.NodeRoleSyncInProgress = true;
				else if (it3->first < nodeSetup.LastNodeRoleID)
					nodeSetup.NodeRoleSyncInProgress = true;
			}
		}
	}
}