// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2014-2017 The Dash Core developers
// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "chainparamsseeds.h"
#include "consensus/merkle.h"
#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"
#include <assert.h>
#include <boost/assign/list_of.hpp>

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
	CMutableTransaction txNew;
	txNew.nVersion = 1;
	txNew.vin.resize(1);
	txNew.vout.resize(1);
	txNew.vin[0].scriptSig = CScript() << 504365040 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
	txNew.vout[0].nValue = genesisReward;
	txNew.vout[0].scriptPubKey = genesisOutputScript;

	CBlock genesis;
	genesis.nTime = nTime;
	genesis.nBits = nBits;
	genesis.nNonce = nNonce;
	genesis.nVersion = nVersion;
	genesis.vtx.push_back(txNew);
	genesis.hashPrevBlock.SetNull();
	genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
	return genesis;
}

static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
	const char* pszTimestamp = "10 February 2018, Infinex - World Decentralisation!";
	const CScript genesisOutputScript = CScript();
	return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

class CMainParams : public CChainParams {
public:
	CMainParams() {
		strNetworkID = "main";
		consensus.nSubsidyHalvingInterval = 350400; // ~ yearly block count
		consensus.nMasternodePaymentsStartBlock = 4800; // ~ 5 days
		consensus.nSuperblockStartBlock = 99999999;
        consensus.nSuperblockCycle = 99999999;
		consensus.nInstantSendKeepLock = 24;		
		consensus.nGovernanceMinQuorum = 10;
		consensus.nGovernanceFilterElements = 20000;
		consensus.nMasternodeMinimumConfirmations = 15;
		consensus.nMajorityEnforceBlockUpgrade = 750;
		consensus.nMajorityRejectBlockOutdated = 950;
		consensus.nMajorityWindow = 1000;
		consensus.BIP34Height = 0;
		consensus.BIP34Hash = uint256S("0000030a46501e7b1f930175c4c8489aeb1ecb25babc41aa205077caa963b61d");
		consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
		consensus.nPowTargetTimespan = 86400; // Infinex: 1 day
		consensus.nPowTargetSpacing = 90; // Infinex: 90 seconds
		consensus.fPowAllowMinDifficultyBlocks = false;
		consensus.fPowNoRetargeting = false;
		consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
		consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

		// Deployment of BIP68, BIP112, and BIP113.
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1486252800; // Feb 5th, 2017
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1517788800; // Feb 5th, 2018

		// By default assume that the signatures in ancestors of this block are valid.
		consensus.defaultAssumeValid = uint256S("0000030a46501e7b1f930175c4c8489aeb1ecb25babc41aa205077caa963b61d");
		consensus.nMinimumChainWork = uint256S("000000000000000000000000000000000000000000000000000000000000000f");

		/**
		* The message start string is designed to be unlikely to occur in normal data.
		* The characters are rarely used upper ASCII, not valid as UTF-8, and produce
		* a large 32-bit integer with any alignment.
		*/
		pchMessageStart[0] = 0x69; //i
		pchMessageStart[1] = 0x6e; //n
		pchMessageStart[2] = 0x66; //f
		pchMessageStart[3] = 0x78; //x
		vAlertPubKey = ParseHex("03944feb6b0b2697385d8a4daa2d1f0f0e882ae4b19c88c25ff3422654eb9a8a16");
		nDefaultPort = 12881;
		
		nMaxTipAge = 21600;
		nDelayGetHeadersTime = 86400;
		nPruneAfterHeight = 100000;

		genesis = CreateGenesisBlock(1518249600, 566111, 0x1e0ffff0, 1, 11 * COIN);
		consensus.hashGenesisBlock = genesis.GetHash();		

		assert(consensus.hashGenesisBlock == uint256S("0000030a46501e7b1f930175c4c8489aeb1ecb25babc41aa205077caa963b61d"));
		assert(genesis.hashMerkleRoot == uint256S("0317d86c7310cd98418ab673e8673059c4f25fac27b84a060f37adbcdfc0679c"));
		
		vFixedSeeds.clear();
		vSeeds.clear();
		vSeeds.push_back(CDNSSeedData("infinex.info", "alpha.infinex.info"));
		vSeeds.push_back(CDNSSeedData("infinex.info", "bravo.infinex.info"));
		vSeeds.push_back(CDNSSeedData("infinex.info", "charlie.infinex.info"));
		vSeeds.push_back(CDNSSeedData("infinex.info", "delta.infinex.info"));
		vSeeds.push_back(CDNSSeedData("infinex.info", "echo.infinex.info"));
		vSeeds.push_back(CDNSSeedData("infinex.info", "foxtrot.infinex.info"));
		vSeeds.push_back(CDNSSeedData("infinex.info", "golf.infinex.info"));
		vSeeds.push_back(CDNSSeedData("infinex.info", "hotel.infinex.info"));
		vSeeds.push_back(CDNSSeedData("infinex.info", "india.infinex.info"));
		vSeeds.push_back(CDNSSeedData("infinex.info", "juliet.infinex.info"));

		InfiniDEXSeeds.push_back(CDNSSeedData("InfiniDEX1", "InfiniDEX1.infinex.info"));
		InfiniDEXSeeds.push_back(CDNSSeedData("InfiniDEX2", "InfiniDEX2.infinex.info"));
		InfiniDEXSeeds.push_back(CDNSSeedData("InfiniDEX3", "InfiniDEX3.infinex.info"));
		InfiniDEXSeeds.push_back(CDNSSeedData("InfiniDEX4", "InfiniDEX4.infinex.info"));
		InfiniDEXSeeds.push_back(CDNSSeedData("InfiniDEX5", "InfiniDEX5.infinex.info"));
		InfiniDEXSeeds.push_back(CDNSSeedData("InfiniDEX6", "InfiniDEX6.infinex.info"));
		InfiniDEXSeeds.push_back(CDNSSeedData("InfiniDEX7", "InfiniDEX7.infinex.info"));
		InfiniDEXSeeds.push_back(CDNSSeedData("InfiniDEX8", "InfiniDEX8.infinex.info"));
		InfiniDEXSeeds.push_back(CDNSSeedData("InfiniDEX9", "InfiniDEX9.infinex.info"));
		InfiniDEXSeeds.push_back(CDNSSeedData("InfiniDEX10", "InfiniDEX10.infinex.info"));

		// Infinex addresses start with 'i'
		base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 102);
		// Infinex script addresses start with '7'
		base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 20);
		// Infinex private keys start with 'V'
		base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 195);
		// Infinex BIP32 pubkeys start with 'xpub' (Bitcoin defaults)
		base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
		// Infinex BIP32 prvkeys start with 'xprv' (Bitcoin defaults)
		base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();

		// Infinex BIP44 coin type is '8'
		nExtCoinType = 8;

		//vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

		fMiningRequiresPeers = true;
		fDefaultConsistencyChecks = false;
		fRequireStandard = true;
		fMineBlocksOnDemand = false;
		fTestnetToBeDeprecatedFieldRPC = false;

		nPoolMaxTransactions = 3;
		nFulfilledRequestExpireTime = 60 * 60; // fulfilled requests expire in 1 hour
		strSporkPubKey = "029d2133bcde28548fa628f5198dffcf56fee8170082cf3371f7c28f2928e66a5d";
		founderAddress = "i9hFhMf3XMHuA2mJvk7kxxMRzmbVigvAun";

		checkpointData = (CCheckpointData) {
			boost::assign::map_list_of
			(0, uint256S("0000030a46501e7b1f930175c4c8489aeb1ecb25babc41aa205077caa963b61d")),
				1518249600, // * UNIX timestamp of last checkpoint block
				0,    // * total number of transactions between genesis and last checkpoint
					  //   (the tx=... number in the SetBestChain debug.log lines)
				2000        // * estimated number of transactions per day after checkpoint
		};
	}
};
static CMainParams mainParams;

/**
* Testnet (v3)
*/
class CTestNetParams : public CChainParams {
public:
	CTestNetParams() {
		strNetworkID = "test";
		consensus.nSubsidyHalvingInterval = 210240;
		consensus.nMasternodePaymentsStartBlock = 100;
		consensus.nSuperblockStartBlock = 99999999;
        consensus.nSuperblockCycle = 99999999;
		consensus.nInstantSendKeepLock = 6;		
		consensus.nGovernanceMinQuorum = 1;
		consensus.nGovernanceFilterElements = 500;
		consensus.nMasternodeMinimumConfirmations = 1;
		consensus.nMajorityEnforceBlockUpgrade = 51;
		consensus.nMajorityRejectBlockOutdated = 75;
		consensus.nMajorityWindow = 100;
		consensus.BIP34Height = 1;
		consensus.BIP34Hash = uint256S("0000030a46501e7b1f930175c4c8489aeb1ecb25babc41aa205077caa963b61d");
		consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
		consensus.nPowTargetTimespan = 24 * 60 * 60; // Infinex: 1 day
		consensus.nPowTargetSpacing = 2.5 * 60; // Infinex: 2.5 minutes
		consensus.fPowAllowMinDifficultyBlocks = true;
		consensus.fPowNoRetargeting = false;
		consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
		consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

		// Deployment of BIP68, BIP112, and BIP113.
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1506556800; // September 28th, 2017
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1538092800; // September 28th, 2018
		
		// By default assume that the signatures in ancestors of this block are valid.
		consensus.defaultAssumeValid = uint256S("0000030a46501e7b1f930175c4c8489aeb1ecb25babc41aa205077caa963b61d");
		consensus.nMinimumChainWork = uint256S("000000000000000000000000000000000000000000000000000000000000000f");

		pchMessageStart[0] = 0x69; //i
		pchMessageStart[1] = 0x66; //f
		pchMessageStart[2] = 0x78; //x
		pchMessageStart[3] = 0x74; //testnet
		vAlertPubKey = ParseHex("03944feb6b0b2697385d8a4daa2d1f0f0e882ae4b19c88c25ff3422654eb9a8a16");
		nDefaultPort = 13880;
		nMaxTipAge = 0x7fffffff;
		nDelayGetHeadersTime = 24 * 60 * 60;
		nPruneAfterHeight = 1000;

		genesis = CreateGenesisBlock(1518249600, 566111, 0x1e0ffff0, 1, 11 * COIN);
		consensus.hashGenesisBlock = genesis.GetHash();

		assert(consensus.hashGenesisBlock == uint256S("0000030a46501e7b1f930175c4c8489aeb1ecb25babc41aa205077caa963b61d"));
		assert(genesis.hashMerkleRoot == uint256S("0317d86c7310cd98418ab673e8673059c4f25fac27b84a060f37adbcdfc0679c"));

		vFixedSeeds.clear();
		vSeeds.clear();
		vSeeds.push_back(CDNSSeedData("infinex.info", "testnet1.infinex.info"));
		vSeeds.push_back(CDNSSeedData("infinex.info", "testnet2.infinex.info"));
		vSeeds.push_back(CDNSSeedData("infinex.info", "testnet3.infinex.info"));
		vSeeds.push_back(CDNSSeedData("infinex.info", "testnet4.infinex.info"));
		vSeeds.push_back(CDNSSeedData("infinex.info", "testnet5.infinex.info"));
		vSeeds.push_back(CDNSSeedData("infinex.info", "testnet6.infinex.info"));
		vSeeds.push_back(CDNSSeedData("infinex.info", "testnet7.infinex.info"));
		vSeeds.push_back(CDNSSeedData("infinex.info", "testnet8.infinex.info"));
		vSeeds.push_back(CDNSSeedData("infinex.info", "testnet9.infinex.info"));
		vSeeds.push_back(CDNSSeedData("infinex.info", "testnet10.infinex.info"));

		// Testnet Infinex addresses start with 'y'
		base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 140);
		// Testnet Infinex script addresses start with '8' or '9'
		base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 19);
		// Testnet private keys start with '9' or 'c' (Bitcoin defaults)
		base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);
		// Testnet Infinex BIP32 pubkeys start with 'tpub' (Bitcoin defaults)
		base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
		// Testnet Infinex BIP32 prvkeys start with 'tprv' (Bitcoin defaults)
		base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

		// Testnet Infinex BIP44 coin type is '1' (All coin's testnet default)
		nExtCoinType = 1;

		//vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

		fMiningRequiresPeers = true;
		fDefaultConsistencyChecks = false;
		fRequireStandard = false;
		fMineBlocksOnDemand = false;
		fTestnetToBeDeprecatedFieldRPC = true;

		nPoolMaxTransactions = 3;
		nFulfilledRequestExpireTime = 5 * 60; // fulfilled requests expire in 5 minutes
		strSporkPubKey = "029d2133bcde28548fa628f5198dffcf56fee8170082cf3371f7c28f2928e66a5d";
		founderAddress = "i9hFhMf3XMHuA2mJvk7kxxMRzmbVigvAun";

		checkpointData = (CCheckpointData) {
			boost::assign::map_list_of
			(0, uint256S("0000030a46501e7b1f930175c4c8489aeb1ecb25babc41aa205077caa963b61d")),
				1518249600, // * UNIX timestamp of last checkpoint block
				0,       // * total number of transactions between genesis and last checkpoint
						 //   (the tx=... number in the SetBestChain debug.log lines)
				10         // * estimated number of transactions per day after checkpoint
		};
	}
};
static CTestNetParams testNetParams;

/**
* Regression test
*/
class CRegTestParams : public CChainParams {
public:
	CRegTestParams() {
		strNetworkID = "regtest";
		consensus.nSubsidyHalvingInterval = 200;
		consensus.nMasternodePaymentsStartBlock = 100;
		consensus.nSuperblockStartBlock = 99999999;
        consensus.nSuperblockCycle = 99999999;
		consensus.nInstantSendKeepLock = 6;		
		consensus.nGovernanceMinQuorum = 1;
		consensus.nGovernanceFilterElements = 100;
		consensus.nMasternodeMinimumConfirmations = 1;
		consensus.nMajorityEnforceBlockUpgrade = 750;
		consensus.nMajorityRejectBlockOutdated = 950;
		consensus.nMajorityWindow = 1000;
		consensus.BIP34Height = -1;
		consensus.BIP34Hash = uint256();
		consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
		consensus.nPowTargetTimespan = 86400; // Infinex: 1 day
		consensus.nPowTargetSpacing = 90; // Infinex: 90 seconds
		consensus.fPowAllowMinDifficultyBlocks = true;
		consensus.fPowNoRetargeting = true;
		consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
		consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 128801288012880ULL;
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 128801288012880ULL;

		// By default assume that the signatures in ancestors of this block are valid.
		consensus.defaultAssumeValid = uint256S("00000f693b8c6264f3fb53f0b0954d79f89330b4a07227c805d4557b2f72e1eb");
		consensus.nMinimumChainWork = uint256S("000000000000000000000000000000000000000000000000000000000000000f");

		pchMessageStart[0] = 0x69; //i
		pchMessageStart[1] = 0x66; //f
		pchMessageStart[2] = 0x78; //x
		pchMessageStart[3] = 0x72; //reg
		nMaxTipAge = 2592000;
		nDelayGetHeadersTime = 0;
		nDefaultPort = 14880;
		nPruneAfterHeight = 1000;

		genesis = CreateGenesisBlock(1517046252, 14189368, 0x1e0ffff0, 1, 11 * COIN);
		consensus.hashGenesisBlock = genesis.GetHash();

		//assert(consensus.hashGenesisBlock == uint256S("00000f693b8c6264f3fb53f0b0954d79f89330b4a07227c805d4557b2f72e1eb"));
		//assert(genesis.hashMerkleRoot == uint256S("21a791571ea30cbce3ac163143d02ce8e9e3af6bce278cb8b967823128453259"));

		vFixedSeeds.clear(); //! Regtest mode doesn't have any fixed seeds.
		vSeeds.clear();  //! Regtest mode doesn't have any DNS seeds.

		fMiningRequiresPeers = false;
		fDefaultConsistencyChecks = true;
		fRequireStandard = false;
		fMineBlocksOnDemand = true;
		fTestnetToBeDeprecatedFieldRPC = false;

		nFulfilledRequestExpireTime = 5 * 60; // fulfilled requests expire in 5 minutes

		checkpointData = (CCheckpointData) {
			boost::assign::map_list_of
			(0, uint256S("00000f693b8c6264f3fb53f0b0954d79f89330b4a07227c805d4557b2f72e1eb")),
				0,
				0,
				0
		};
		// Regtest Infinex addresses start with 'y'
		base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 140);
		// Regtest Infinex script addresses start with '8' or '9'
		base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 19);
		// Regtest private keys start with '9' or 'c' (Bitcoin defaults)
		base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);
		// Regtest Infinex BIP32 pubkeys start with 'tpub' (Bitcoin defaults)
		base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
		// Regtest Infinex BIP32 prvkeys start with 'tprv' (Bitcoin defaults)
		base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

		// Regtest Infinex BIP44 coin type is '1' (All coin's testnet default)
		nExtCoinType = 1;
	}
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
	assert(pCurrentParams);
	return *pCurrentParams;
}

CChainParams& Params(const std::string& chain)
{
	if (chain == CBaseChainParams::MAIN)
		return mainParams;
	else if (chain == CBaseChainParams::TESTNET)
		return testNetParams;
	else if (chain == CBaseChainParams::REGTEST)
		return regTestParams;
	else
		throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
	SelectBaseParams(network);
	pCurrentParams = &Params(network);
}