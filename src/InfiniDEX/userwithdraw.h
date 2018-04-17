// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERWITHDRAW_H
#define USERWITHDRAW_H

#include <vector>
#include <map>
#include "net.h"
#include "utilstrencodings.h"

class CUserWithdraw;
class CUserWithdrawSetting;
class CUserWithdrawSync;
class CUserWithdrawManager;

typedef std::map<int, CUserWithdraw> mapUserWithdrawByID;
typedef std::pair<int, mapUserWithdrawByID> mapUserWithdrawWithID;
typedef std::map<int, mapUserWithdrawWithID> mapCoinUserWithdraw;
extern std::map<std::string, mapCoinUserWithdraw> mapUserWithdraw;
extern std::map<char, CUserWithdrawSetting> mapUserWithdrawSetting;
extern CUserWithdrawManager userWithdrawManager;

class CUserWithdraw
{
private:
	std::vector<unsigned char> userVchSig;
	std::vector<unsigned char> mnVchSig;
	std::vector<unsigned char> finalVchSig;

public:
    std::string UserPubKey;
	int CoinID;
    uint64_t WithdrawAmount;
	uint64_t WithdrawRequestTime;
	std::string UserHash;
	int UserWithdrawID;
	std::string MNPubKey;
	bool ValidWithdraw;
	uint64_t WithdrawCheckTime;	
	uint64_t WithdrawProcessTime;
    std::string TransactionID;    
    std::string Remark;
    uint64_t LastUpdateTime;

	CUserWithdraw():
		UserPubKey(""),
		CoinID(0),
		WithdrawAmount(0),
		WithdrawRequestTime(0),
		UserHash(""),
		UserWithdrawID(0),
		MNPubKey(""),
		ValidWithdraw(false),
		WithdrawCheckTime(0),
		WithdrawProcessTime(0),
		TransactionID(""),
		Remark(""),
		LastUpdateTime(0)
	{}

	ADD_SERIALIZE_METHODS;
	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
		READWRITE(UserPubKey);
		READWRITE(CoinID);
		READWRITE(WithdrawAmount);
		READWRITE(WithdrawRequestTime);
		READWRITE(UserHash);
		READWRITE(UserWithdrawID);
		READWRITE(MNPubKey);
		READWRITE(ValidWithdraw);
		READWRITE(WithdrawCheckTime);
		READWRITE(WithdrawProcessTime);
		READWRITE(TransactionID);
		READWRITE(Remark);
		READWRITE(LastUpdateTime);
		READWRITE(userVchSig);
		READWRITE(mnVchSig);
		READWRITE(finalVchSig);
	}
		
	bool VerifyUserSignature();
	bool VerifyMNSignature();
	bool VerifyFinalSignature();
	bool MNSign();
	void RelayToUser();
	void RelayToMN();
	void RelayToNetwork();
};

class CUserWithdrawSetting
{
public:
	char Char;
	bool InChargeOfWithdrawRecord;

	CUserWithdrawSetting() :
		Char(),
		InChargeOfWithdrawRecord(false)
	{}

	CUserWithdrawSetting(char Char, bool InChargeOfWithdrawRecord) :
		Char(Char),
		InChargeOfWithdrawRecord(InChargeOfWithdrawRecord)
	{}
};

class CUserWithdrawSync
{
public:
	mapCoinUserWithdraw data;

	// ADD_SERIALIZE_METHODS;
	// template <typename Stream, typename Operation>
	// inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
	// 	READWRITE(data);
	// }

	void BroadcastTo(CNode* node, CConnman& connman);
};

class CUserWithdrawSyncRequest
{
public:
	std::string UserPubKey;

	CUserWithdrawSyncRequest(std::string UserPubKey) :
		UserPubKey(UserPubKey)
	{}
};

class CUserWithdrawManager
{
private:
    std::vector<unsigned char> vchSig;

public:
    CUserWithdrawManager() {}
	void ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
	void InputUserWithdraw(CUserWithdraw userWithdraw);
	void InputUserWithdrawRecord(CUserWithdraw userWithdraw);
	bool InChargeOfUserWithdrawRecord(std::string UserPubKey);
	void AssignWithdrawProcessorRole(int CoinID);
	void AssignWithdrawInfoRole(int CoinID);	
	void ProcessUserWithdrawRequest(CUserWithdraw UserWithdrawRequest);
	void SendUserWithdrawalRecords(std::string UserPubKey, int CoinID);
	void SendUsersWithdrawalRecords(CNode* node, CConnman& connman, int CoinID);
};

#endif