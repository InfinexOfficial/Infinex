// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pow.h"

#include "arith_uint256.h"
#include "chain.h"
#include "chainparams.h"
#include "primitives/block.h"
#include "uint256.h"
#include "util.h"

#include <math.h>

unsigned int static InfiniLoop(const CBlockIndex* pindexLast, const Consensus::Params& params) {
	const arith_uint256 nPowLimit = UintToArith256(params.powLimit);
	int64_t nHourlyBlocks = 3600 / params.nPowTargetSpacing; //target hourly block count	

	if (!pindexLast || pindexLast->nHeight < 2) {
		return pindexLast->nBits;
	}

	const CBlockIndex *pindex = pindexLast;
	arith_uint256 nPastAvgDiff = 0;

	if (pindexLast->nHeight < nHourlyBlocks) {
		nHourlyBlocks = pindexLast->nHeight;
	}

	for (unsigned int nCountBlocks = 1; nCountBlocks <= nHourlyBlocks; nCountBlocks++) {
		arith_uint256 nPastDiff = arith_uint256().SetCompact(pindex->nBits);
		nPastAvgDiff = nPastAvgDiff + nPastDiff;

		if (nCountBlocks < nHourlyBlocks)
			pindex = pindex->pprev;
	}
	nPastAvgDiff = nPastAvgDiff / nHourlyBlocks;

	arith_uint256 nTargetDiff(nPastAvgDiff);

	int64_t nActualHourTimespan = pindexLast->GetBlockTime() - pindex->GetBlockTime();
	int64_t nTargetTimespan = params.nPowTargetSpacing * nHourlyBlocks;
	nPastAvgDiff = nPastAvgDiff * nActualHourTimespan / nTargetTimespan;

	if (pindexLast->nHeight > 500) {
		if (nPastAvgDiff > nTargetDiff * 2)
			nTargetDiff = nTargetDiff * 2;
		else if (nTargetDiff > nPastAvgDiff * 2)
			nTargetDiff = nTargetDiff / 2;
		else
			nTargetDiff = nPastAvgDiff;
	}
	else
		nTargetDiff = nPastAvgDiff;

	if (nTargetDiff > nPowLimit) {
		nTargetDiff = nPowLimit;
	}

	return nTargetDiff.GetCompact();
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
	return InfiniLoop(pindexLast, params);
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
	bool fNegative;
	bool fOverflow;
	arith_uint256 bnTarget;

	bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

	// Check range
	if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
		return error("CheckProofOfWork(): nBits below minimum work");

	// Check proof of work matches claimed amount
	if (UintToArith256(hash) > bnTarget)
		return error("CheckProofOfWork(): hash doesn't match nBits");

	return true;
}

arith_uint256 GetBlockProof(const CBlockIndex& block)
{
	arith_uint256 bnTarget;
	bool fNegative;
	bool fOverflow;
	bnTarget.SetCompact(block.nBits, &fNegative, &fOverflow);
	if (fNegative || fOverflow || bnTarget == 0)
		return 0;
	// We need to compute 2**256 / (bnTarget+1), but we can't represent 2**256
	// as it's too large for a arith_uint256. However, as 2**256 is at least as large
	// as bnTarget+1, it is equal to ((2**256 - bnTarget - 1) / (bnTarget+1)) + 1,
	// or ~bnTarget / (nTarget+1) + 1.
	return (~bnTarget / (bnTarget + 1)) + 1;
}

int64_t GetBlockProofEquivalentTime(const CBlockIndex& to, const CBlockIndex& from, const CBlockIndex& tip, const Consensus::Params& params)
{
	arith_uint256 r;
	int sign = 1;
	if (to.nChainWork > from.nChainWork) {
		r = to.nChainWork - from.nChainWork;
	}
	else {
		r = from.nChainWork - to.nChainWork;
		sign = -1;
	}
	r = r * arith_uint256(params.nPowTargetSpacing) / GetBlockProof(tip);
	if (r.bits() > 63) {
		return sign * std::numeric_limits<int64_t>::max();
	}
	return sign * r.GetLow64();
}