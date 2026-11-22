// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pow.h>

#include <arith_uint256.h>
#include <chain.h>
#include <primitives/block.h>
#include <uint256.h>
#include <util.h>
#include <core_io.h>            // Maza: Hive
#include <script/standard.h>    // Maza: Hive
#include <base58.h>             // Maza: Hive
#include <pubkey.h>             // Maza: Hive
#include <hash.h>               // Maza: Hive
#include <sync.h>               // Maza: Hive
#include <validation.h>         // Maza: Hive
#include <utilstrencodings.h>   // Maza: Hive

BeePopGraphPoint beePopGraph[1024*40];       // Maza: Hive

// Hive proofs serialize the BCT txid as ASCII.  uint256S() is case-insensitive,
// but the Hive hash uses those exact ASCII bytes.  Without this check, changing
// hex letter case creates free independent bee-hash lottery entries for one BCT.
bool IsCanonicalHiveBctTxid(const std::string& txidStr)
{
    return txidStr.size() == 64 && txidStr == uint256S(txidStr).GetHex();
}

// Maza: MinotaurX+Hive1.2: Diff adjustment for pow algos (post-MinotaurX activation)
// Modified LWMA-3
// Copyright (c) 2017-2021 The Bitcoin Gold developers, Zawy, iamstenman (Microbitcoin), The Litecoin Cash developers
// MIT License
// Algorithm by Zawy, a modification of WT-144 by Tom Harding
// For updates see
// https://github.com/zawy12/difficulty-algorithms/issues/3#issuecomment-442129791
unsigned int GetNextWorkRequiredLWMA(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params, const POW_TYPE powType) {
    const bool verbose = LogAcceptCategory(BCLog::MINOTAURX);
    const arith_uint256 powLimit = UintToArith256(params.powTypeLimits[powType]);   // Max target limit (easiest diff)
    const int64_t T = params.nPowTargetSpacing * 2;                                 // Target freq
    const int64_t N = params.lwmaAveragingWindow;                                   // Window size
    const int64_t k = N * (N + 1) * T / 2;                                          // Constant for proper averaging after weighting solvetimes
    const int64_t height = pindexLast->nHeight;                                     // Block height
    
    // TESTNET ONLY: Allow minimum difficulty blocks if we haven't seen a block for ostensibly 10 blocks worth of time.
    // Reading this code because you're porting LCC features? Considering doing this on your mainnet?
    // ***** THIS IS NOT SAFE TO DO ON YOUR MAINNET! *****
    if (params.fPowAllowMinDifficultyBlocks && pblock->GetBlockTime() > pindexLast->GetBlockTime() + T * 10) {
        if (verbose) LogPrintf("* GetNextWorkRequiredLWMA: Allowing %s pow limit (apparent testnet stall)\n", POW_TYPE_NAMES[powType]);
        return powLimit.GetCompact();
    }

    // Not enough blocks on chain? Return limit
    if (height < N) {
        if (verbose) LogPrintf("* GetNextWorkRequiredLWMA: Allowing %s pow limit (short chain)\n", POW_TYPE_NAMES[powType]);
        return powLimit.GetCompact();
    }

    arith_uint256 avgTarget, nextTarget;
    int64_t thisTimestamp, previousTimestamp;
    int64_t sumWeightedSolvetimes = 0, j = 0, blocksFound = 0;

    // Find previousTimestamp (N blocks of this blocktype back), and build list of wanted-type blocks as we go
    std::vector<const CBlockIndex*> wantedBlocks;
    const CBlockIndex* blockPreviousTimestamp = pindexLast;
    while (blocksFound < N) {
        // Reached forkpoint before finding N blocks of correct powtype? Return min
        if (blockPreviousTimestamp->GetBlockHeader().nVersion >= 0x20000000) {
            if (verbose) LogPrintf("* GetNextWorkRequiredLWMA: Allowing %s pow limit (previousTime calc reached forkpoint at height %i)\n", POW_TYPE_NAMES[powType], blockPreviousTimestamp->nHeight);
            return powLimit.GetCompact();
        }

        // Wrong block type? Skip
        if (blockPreviousTimestamp->GetBlockHeader().IsHiveMined(params) || blockPreviousTimestamp->GetBlockHeader().GetPoWType() != powType) {
            assert (blockPreviousTimestamp->pprev);
            blockPreviousTimestamp = blockPreviousTimestamp->pprev;
            continue;
        }
    
        wantedBlocks.push_back(blockPreviousTimestamp);

        blocksFound++;
        if (blocksFound == N)   // Don't step to next one if we're at the one we want
            break;

        assert (blockPreviousTimestamp->pprev);
        blockPreviousTimestamp = blockPreviousTimestamp->pprev;
    }
    previousTimestamp = blockPreviousTimestamp->GetBlockTime();
    //if (verbose) LogPrintf("* GetNextWorkRequiredLWMA: previousTime: First in period is %s at height %i\n", blockPreviousTimestamp->GetBlockHeader().GetHash().ToString().c_str(), blockPreviousTimestamp->nHeight);

    // Iterate forward from the oldest block (ie, reverse-iterate through the wantedBlocks vector)
    for (auto it = wantedBlocks.rbegin(); it != wantedBlocks.rend(); ++it) {
        const CBlockIndex* block = *it;

        // Prevent solvetimes from being negative in a safe way. It must be done like this. 
        // Do not attempt anything like  if (solvetime < 1) {solvetime=1;}
        // The +1 ensures new coins do not calculate nextTarget = 0.
        thisTimestamp = (block->GetBlockTime() > previousTimestamp) ? block->GetBlockTime() : previousTimestamp + 1;

        // 6*T limit prevents large drops in diff from long solvetimes which would cause oscillations.
        int64_t solvetime = std::min(6 * T, thisTimestamp - previousTimestamp);

        // The following is part of "preventing negative solvetimes". 
        previousTimestamp = thisTimestamp;
       
        // Give linearly higher weight to more recent solvetimes.
        j++;
        sumWeightedSolvetimes += solvetime * j; 

        arith_uint256 target;
        target.SetCompact(block->nBits);
        avgTarget += target / N / k; // Dividing by k here prevents an overflow below.
    } 

    nextTarget = avgTarget * sumWeightedSolvetimes;

    if (nextTarget > powLimit) {
        if (verbose) LogPrintf("* GetNextWorkRequiredLWMA: Allowing %s pow limit (target too high)\n", POW_TYPE_NAMES[powType]);
        return powLimit.GetCompact();
    }

    return nextTarget.GetCompact();
}

// Maza: DarkGravity V3 (https://github.com/dashpay/dash/blob/master/src/pow.cpp#L82)
// By Evan Duffield <evan@dash.org>
// Used for sha256 from block 100000 till MinotaurX activation
unsigned int DarkGravityWave(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimitSHA);
    int64_t nPastBlocks = 24;

    // Maza: Allow minimum difficulty blocks if we haven't seen a block for ostensibly 10 blocks worth of time
    //if (params.fPowAllowMinDifficultyBlocks && pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing * 10)
    //    return bnPowLimit.GetCompact();

    

	// make sure we have at least (nPastBlocks + 1) blocks, otherwise just return powLimit
    if (!pindexLast || pindexLast->nHeight < nPastBlocks) 
        return bnPowLimit.GetCompact();

    const CBlockIndex *pindex = pindexLast;
    arith_uint256 bnPastTargetAvg;

    for (unsigned int nCountBlocks = 1; nCountBlocks <= nPastBlocks; nCountBlocks++) {
        // Maza: Hive: Skip over Hivemined blocks; we only want to consider PoW blocks
       

        arith_uint256 bnTarget = arith_uint256().SetCompact(pindex->nBits);
        if (nCountBlocks == 1) {
            bnPastTargetAvg = bnTarget;
        } else {
            // NOTE: that's not an average really...
            bnPastTargetAvg = (bnPastTargetAvg * nCountBlocks + bnTarget) / (nCountBlocks + 1);
        }

        if(nCountBlocks != nPastBlocks) {
            assert(pindex->pprev); // should never fail
            pindex = pindex->pprev;
        }
    }

    arith_uint256 bnNew(bnPastTargetAvg);

    int64_t nActualTimespan = pindexLast->GetBlockTime() - pindex->GetBlockTime();
    // NOTE: is this accurate? nActualTimespan counts it for (nPastBlocks - 1) blocks only...
    int64_t nTargetTimespan = nPastBlocks * params.nPowTargetSpacing;

    if (nActualTimespan < nTargetTimespan/3)
        nActualTimespan = nTargetTimespan/3;
    if (nActualTimespan > nTargetTimespan*3)
        nActualTimespan = nTargetTimespan*3;

    // Retarget
    bnNew *= nActualTimespan;
    bnNew /= nTargetTimespan;

    if (bnNew > bnPowLimit) {
        bnNew = bnPowLimit;
    }

    return bnNew.GetCompact();
}

unsigned int GetNextWorkRequiredBTC(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{


    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimitSHA).GetCompact();
	unsigned int nStartingDifficulty =  UintToArith256(params.startingDifficulty).GetCompact();
    // Genesis block
    if (pindexLast == NULL)
        return nProofOfWorkLimit;

    // Start at difficulty of 1
    if (pindexLast->nHeight+1 < (params.DifficultyAdjustmentInterval() * 20))
        return nStartingDifficulty;
    // Only change once per interval
    if ((pindexLast->nHeight+1) % params.DifficultyAdjustmentInterval() != 0)
    {
        if (params.fPowAllowMinDifficultyBlocks)
        {
            // Special difficulty rule for testnet:
            // If the new block's timestamp is more than 2* 2.5 minutes
            // then allow mining of a min-difficulty block.
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing*2)
                return nProofOfWorkLimit;
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % params.DifficultyAdjustmentInterval() != 0 && pindex->nBits == nProofOfWorkLimit)
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }
        return pindexLast->nBits;
    }

    // Go back by what we want to be 1 day worth of blocks
	 const CBlockIndex* pindexFirst = pindexLast;
    for (int i = 0; pindexFirst && i < (params.DifficultyAdjustmentInterval()*20)-1; i++)
        pindexFirst = pindexFirst->pprev;
    assert(pindexFirst);
    

   return CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params);
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    // Most recent algo first
    if (pindexLast->nHeight + 1 >= params.nPowDGWHeight) {
        return DarkGravityWave(pindexLast, pblock, params);
    }
    else {
        return GetNextWorkRequiredBTC(pindexLast, pblock, params);
    }
}

// for DIFF_BTC only!
unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
	if (params.fPowNoRetargeting)
		return pindexLast->nBits;
	int64_t nInterval = params.DifficultyAdjustmentInterval(); // 4 blocks
	int64_t nAveragingInterval = nInterval * 20;
	int64_t nAveragingTargetTimespan = nAveragingInterval * 120; 
	int64_t nMaxAdjustDown = 20; // 20% adjustment down
	int64_t nMaxAdjustUp = 15; // 15% adjustment up
	int64_t nMinActualTimespan = nAveragingTargetTimespan * (100 - nMaxAdjustUp) / 100;
	int64_t nMaxActualTimespan = nAveragingTargetTimespan * (100 + nMaxAdjustDown) / 100;
    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    if (nActualTimespan < nMinActualTimespan)
        nActualTimespan = nMinActualTimespan;
    if (nActualTimespan > nMaxActualTimespan)
        nActualTimespan = nMaxActualTimespan;

    // Retarget
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimitSHA);
    arith_uint256 bnNew;
	
    bnNew.SetCompact(pindexLast->nBits);
	
    bnNew *= nActualTimespan;
    bnNew /= nAveragingTargetTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;
	
    return bnNew.GetCompact();
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Maza: MinotaurX+Hive1.2: Use highest pow limit for limit check
    arith_uint256 powLimit = 0;
    for (int i = 0; i < NUM_BLOCK_TYPES; i++)
        if (UintToArith256(params.powTypeLimits[i]) > powLimit)
            powLimit = UintToArith256(params.powTypeLimits[i]);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > powLimit)
        return false;

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return false;

    return true;
}


// Maza: Hive: Get the current Bee Hash Target (Hive 1.0)
unsigned int GetNextHiveWorkRequired(const CBlockIndex* pindexLast, const Consensus::Params& params) {
    // Maza: MinotaurX+Hive1.2
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimitHive);

    arith_uint256 beeHashTarget = 0;
    int hiveBlockCount = 0;
    int totalBlockCount = 0;

    // Step back till we have found 24 hive blocks, or we ran out...
    while (hiveBlockCount < params.hiveDifficultyWindow && pindexLast->pprev && IsMinotaurXEnabled(pindexLast, params)) {        
        if (pindexLast->GetBlockHeader().IsHiveMined(params)) {
            beeHashTarget += arith_uint256().SetCompact(pindexLast->nBits);
            hiveBlockCount++;
        }
        totalBlockCount++;
        pindexLast = pindexLast->pprev;
    }

    if (hiveBlockCount < params.hiveDifficultyWindow) {          // Should only happen when chain is starting
        LogPrintf("GetNextHive12WorkRequired: Insufficient hive blocks.\n");
        return bnPowLimit.GetCompact();
    }

    beeHashTarget /= hiveBlockCount;    // Average the bee hash targets in window

    // Retarget based on totalBlockCount
    int targetTotalBlockCount = hiveBlockCount * params.hiveBlockSpacingTarget;
    beeHashTarget *= totalBlockCount;
    beeHashTarget /= targetTotalBlockCount;

    if (beeHashTarget > bnPowLimit)
        beeHashTarget = bnPowLimit;

    return beeHashTarget.GetCompact();
}

// Maza: Hive: Get count of all live and gestating BCTs on the network
bool GetNetworkHiveInfo(int& immatureBees, int& immatureBCTs, int& matureBees, int& matureBCTs, CAmount& potentialLifespanRewards, const Consensus::Params& consensusParams, bool recalcGraph) {
    int totalBeeLifespan = consensusParams.beeLifespanBlocks + consensusParams.beeGestationBlocks;
    immatureBees = immatureBCTs = matureBees = matureBCTs = 0;
    
    CBlockIndex* pindexPrev = chainActive.Tip();
    assert(pindexPrev != nullptr);
    int tipHeight = pindexPrev->nHeight;

    // Maza: MinotaurX+Hive1.2: Get correct hive block reward
    auto blockReward = GetBlockSubsidy(pindexPrev->nHeight, consensusParams);
    blockReward += blockReward >> 1;

    
    
    potentialLifespanRewards = (consensusParams.beeLifespanBlocks * blockReward) / consensusParams.hiveBlockSpacingTargetTypical_1_1;
    
    if (recalcGraph) {
        for (int i = 0; i < totalBeeLifespan; i++) {
            beePopGraph[i].immaturePop = 0;
            beePopGraph[i].maturePop = 0;
        }
    }

    if (IsInitialBlockDownload())   // Refuse if we're downloading
        return false;

    // Count bees in next blockCount blocks
    CBlock block;
    CScript scriptPubKeyBCF = GetScriptForDestination(DecodeDestination(consensusParams.beeCreationAddress));
    CScript scriptPubKeyCF = GetScriptForDestination(DecodeDestination(consensusParams.hiveCommunityAddress));

    for (int i = 0; i < totalBeeLifespan; i++) {
        if (fHavePruned && !(pindexPrev->nStatus & BLOCK_HAVE_DATA) && pindexPrev->nTx > 0) {
            LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (pruned data); can't calculate network bee count.");
            return false;
        }

        if (!pindexPrev->GetBlockHeader().IsHiveMined(consensusParams)) {                          // Don't check Hivemined blocks (no BCTs will be found in them)
            if (!ReadBlockFromDisk(block, pindexPrev, consensusParams)) {
                LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (not found on disk); can't calculate network bee count.");
                return false;
            }
            int blockHeight = pindexPrev->nHeight;
            CAmount beeCost = GetBeeCost(blockHeight, consensusParams);
            if (block.vtx.size() > 0) {
                for(const auto& tx : block.vtx) {
                    CAmount beeFeePaid;
                    if (tx->IsBCT(consensusParams, scriptPubKeyBCF, &beeFeePaid)) {                 // If it's a BCT, total its bees
                        if (tx->vout.size() > 1 && tx->vout[1].scriptPubKey == scriptPubKeyCF) {    // If it has a community fund contrib...
                            CAmount donationAmount = tx->vout[1].nValue;
                            CAmount expectedDonationAmount = (beeFeePaid + donationAmount) / consensusParams.communityContribFactor;  // ...check for valid donation amount
                            // Maza: MinotaurX+Hive1.2
                            if (IsMinotaurXEnabled(pindexPrev, consensusParams))
                                expectedDonationAmount += expectedDonationAmount >> 1;
                            if (donationAmount != expectedDonationAmount)
                                continue;
                            beeFeePaid += donationAmount;                                           // Add donation amount back to total paid
                        }
                        int beeCount = beeFeePaid / beeCost;
                        if (i < consensusParams.beeGestationBlocks) {
                            immatureBees += beeCount;
                            immatureBCTs++;
                        } else {
                            matureBees += beeCount; 
                            matureBCTs++;
                        }

                        // Add these bees to pop graph
                        if (recalcGraph) {
                            /*
                            int beeStart = blockHeight + consensusParams.beeGestationBlocks;
                            int beeStop = beeStart + consensusParams.beeLifespanBlocks;
                            beeStart -= tipHeight;
                            beeStop -= tipHeight;
                            for (int j = beeStart; j < beeStop; j++) {
                                if (j > 0 && j < totalBeeLifespan) {
                                    if (i < consensusParams.beeGestationBlocks) // THIS IS WRONG
                                        beePopGraph[j].immaturePop += beeCount;
                                    else
                                        beePopGraph[j].maturePop += beeCount;
                                }
                            }*/
                            int beeBornBlock = blockHeight;
                            int beeMaturesBlock = beeBornBlock + consensusParams.beeGestationBlocks;
                            int beeDiesBlock = beeMaturesBlock + consensusParams.beeLifespanBlocks;
                            for (int j = beeBornBlock; j < beeDiesBlock; j++) {
                                int graphPos = j - tipHeight;
                                if (graphPos > 0 && graphPos < totalBeeLifespan) {
                                    if (j < beeMaturesBlock)
                                        beePopGraph[graphPos].immaturePop += beeCount;
                                    else
                                        beePopGraph[graphPos].maturePop += beeCount;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (!pindexPrev->pprev)     // Check we didn't run out of blocks
            return true;

        pindexPrev = pindexPrev->pprev;
    }

    return true;
}

// Maza: Hive: Check the hive proof for given block
bool CheckHiveProof(const CBlock* pblock, const Consensus::Params& consensusParams) {
    bool verbose = LogAcceptCategory(BCLog::HIVE);

    if (verbose)
        LogPrintf("********************* Hive: CheckHiveProof *********************\n");

    // Get height (a CBlockIndex isn't always available when this func is called, eg in reads from disk)
    int blockHeight;
    CBlockIndex* pindexPrev;
    {
        LOCK(cs_main);
        // mapBlockIndex::operator[] INSERTS a nullptr when the key is absent,
        // so the old code dereferenced it (crash) before reaching the null
        // check below — a corrupt blk*.dat entry whose hashPrevBlock is not
        // in the index segfaulted the node during load/reindex. Use find()
        // and reject before any dereference; behaviour for every block whose
        // prev index exists is byte-for-byte identical (same pointer, height
        // computed from the same pindexPrev, just after the guard).
        auto itPrev = mapBlockIndex.find(pblock->hashPrevBlock);
        if (itPrev == mapBlockIndex.end() || itPrev->second == nullptr) {
            LogPrintf("CheckHiveProof: Couldn't get previous block's CBlockIndex!\n");
            return false;
        }
        pindexPrev = itPrev->second;
        blockHeight = pindexPrev->nHeight + 1;
    }
    if (verbose)
        LogPrintf("CheckHiveProof: nHeight             = %i\n", blockHeight);

    // Check hive is enabled on network
    if (!IsMinotaurXEnabled(pindexPrev, consensusParams)) {
        LogPrintf("CheckHiveProof: Can't accept a Hive block; Hive is not yet enabled on the network.\n");
        return false;
    }

    // Maza: Hive 1.1: Check that there aren't too many consecutive Hive blocks
    if (IsMinotaurXEnabled(pindexPrev, consensusParams)) {
        int hiveBlocksAtTip = 0;
        CBlockIndex* pindexTemp = pindexPrev;
        while (pindexTemp->GetBlockHeader().IsHiveMined(consensusParams)) {
            assert(pindexTemp->pprev);
            pindexTemp = pindexTemp->pprev;
            hiveBlocksAtTip++;
        }
        if (hiveBlocksAtTip >= consensusParams.maxConsecutiveHiveBlocks) {
            LogPrintf("CheckHiveProof: Too many Hive blocks without a POW block.\n");
            return false;
        }
    } else {
        if (pindexPrev->GetBlockHeader().IsHiveMined(consensusParams)) {
            LogPrint(BCLog::HIVE, "CheckHiveProof: Hive block must follow a POW block.\n");
            return false;
        }
    }

    // Block mustn't include any BCTs
    CScript scriptPubKeyBCF = GetScriptForDestination(DecodeDestination(consensusParams.beeCreationAddress));
    if (pblock->vtx.size() > 1)
        for (unsigned int i=1; i < pblock->vtx.size(); i++)
            if (pblock->vtx[i]->IsBCT(consensusParams, scriptPubKeyBCF)) {
                LogPrintf("CheckHiveProof: Hivemined block contains BCTs!\n");
                return false;                
            }
    
    // Coinbase tx must be valid
    CTransactionRef txCoinbase = pblock->vtx[0];
    //LogPrintf("CheckHiveProof: Got coinbase tx: %s\n", txCoinbase->ToString());
    if (!txCoinbase->IsCoinBase()) {
        LogPrintf("CheckHiveProof: Coinbase tx isn't valid!\n");
        return false;
    }

    // Must have exactly 2 or 3 outputs
    if (txCoinbase->vout.size() < 2 || txCoinbase->vout.size() > 3) {
        LogPrintf("CheckHiveProof: Didn't expect %i vouts!\n", txCoinbase->vout.size());
        return false;
    }

    // vout[0] must be long enough to contain all encodings
    if (txCoinbase->vout[0].scriptPubKey.size() < 144) {
        LogPrintf("CheckHiveProof: vout[0].scriptPubKey isn't long enough to contain hive proof encodings\n");
        return false;
    }

    // vout[1] must start OP_RETURN OP_BEE (bytes 0-1)
    if (txCoinbase->vout[0].scriptPubKey[0] != OP_RETURN || txCoinbase->vout[0].scriptPubKey[1] != OP_BEE) {
        LogPrintf("CheckHiveProof: vout[0].scriptPubKey doesn't start OP_RETURN OP_BEE\n");
        return false;
    }

    // Grab the bee nonce (bytes 3-6; byte 2 has value 04 as a size marker for this field)
    uint32_t beeNonce = ReadLE32(&txCoinbase->vout[0].scriptPubKey[3]);
    if (verbose)
        LogPrintf("CheckHiveProof: beeNonce            = %i\n", beeNonce);

    // Grab the bct height (bytes 8-11; byte 7 has value 04 as a size marker for this field)
    uint32_t bctClaimedHeight = ReadLE32(&txCoinbase->vout[0].scriptPubKey[8]);
    if (verbose)
        LogPrintf("CheckHiveProof: bctHeight           = %i\n", bctClaimedHeight);

    // Get community contrib flag (byte 12)
    bool communityContrib = txCoinbase->vout[0].scriptPubKey[12] == OP_TRUE;
    if (verbose)
        LogPrintf("CheckHiveProof: communityContrib    = %s\n", communityContrib ? "true" : "false");

    // Grab the txid (bytes 14-78; byte 13 has val 64 as size marker)
    std::vector<unsigned char> txid(&txCoinbase->vout[0].scriptPubKey[14], &txCoinbase->vout[0].scriptPubKey[14 + 64]);
    std::string txidStr = std::string(txid.begin(), txid.end());
    if (verbose)
        LogPrintf("CheckHiveProof: bctTxId             = %s\n", txidStr);

    // Historical mixed-case proofs remain valid before the fork height.
    // Reject all alternate encodings afterwards, before hashing the bee.
    if (blockHeight >= consensusParams.hiveCanonicalBctTxidHeight &&
        !IsCanonicalHiveBctTxid(txidStr)) {
        LogPrintf("CheckHiveProof: Noncanonical BCT txid encoding at height %i\n", blockHeight);
        return false;
    }

    // Check bee hash against target
    std::string deterministicRandString = GetDeterministicRandString(pindexPrev);
    if (verbose)
        LogPrintf("CheckHiveProof: detRandString       = %s\n", deterministicRandString);
    arith_uint256 beeHashTarget;
    beeHashTarget.SetCompact(GetNextHiveWorkRequired(pindexPrev, consensusParams));
    if (verbose)
        LogPrintf("CheckHiveProof: beeHashTarget       = %s\n", beeHashTarget.ToString());
    
    // Maza: MinotaurX+Hive1.2: Use the correct inner Hive hash
    
    arith_uint256 beeHash(CBlockHeader::MinotaurHashArbitrary(std::string(deterministicRandString + txidStr + std::to_string(beeNonce)).c_str()).ToString());
    if (verbose)
      LogPrintf("CheckHive12Proof: beeHash           = %s\n", beeHash.GetHex());
    if (beeHash >= beeHashTarget) {
      LogPrintf("CheckHive12Proof: Bee does not meet hash target!\n");
      return false;
    }
    
    
    // Grab the message sig (bytes 79-end; byte 78 is size)
    std::vector<unsigned char> messageSig(&txCoinbase->vout[0].scriptPubKey[79], &txCoinbase->vout[0].scriptPubKey[79 + 65]);
    if (verbose)
        LogPrintf("CheckHiveProof: messageSig          = %s\n", HexStr(&messageSig[0], &messageSig[messageSig.size()]));
    
    // Grab the honey address from the honey vout
    CTxDestination honeyDestination;
    if (!ExtractDestination(txCoinbase->vout[1].scriptPubKey, honeyDestination)) {
        LogPrintf("CheckHiveProof: Couldn't extract honey address\n");
        return false;
    }
    if (!IsValidDestination(honeyDestination)) {
        LogPrintf("CheckHiveProof: Honey address is invalid\n");
        return false;
    }
    if (verbose)
        LogPrintf("CheckHiveProof: honeyAddress        = %s\n", EncodeDestination(honeyDestination));

    // Verify the message sig
    const CKeyID *keyID = boost::get<CKeyID>(&honeyDestination);
    if (!keyID) {
        LogPrintf("CheckHiveProof: Can't get pubkey for honey address\n");
        return false;
    }
    CHashWriter ss(SER_GETHASH, 0);
    ss << deterministicRandString;
    uint256 mhash = ss.GetHash();
    CPubKey pubkey;
    if (!pubkey.RecoverCompact(mhash, messageSig)) {
        LogPrintf("CheckHiveProof: Couldn't recover pubkey from hash\n");
        return false;
    }
    if (pubkey.GetID() != *keyID) {
        LogPrintf("CheckHiveProof: Signature mismatch! GetID() = %s, *keyID = %s\n", pubkey.GetID().ToString(), (*keyID).ToString());
        return false;
    }

    // Grab the BCT utxo
    bool deepDrill = false;
    uint32_t bctFoundHeight;
    CAmount bctValue;
    CScript bctScriptPubKey;

    {
        LOCK(cs_main);

        COutPoint outBeeCreation(uint256S(txidStr), 0);
        COutPoint outCommFund(uint256S(txidStr), 1);
        Coin coin;
        CTransactionRef bct = nullptr;
        CBlockIndex foundAt;

        if (pcoinsTip && pcoinsTip->GetCoin(outBeeCreation, coin)) {        // First try the UTXO set (this pathway will hit on incoming blocks)
            if (verbose)
                LogPrintf("CheckHiveProof: Using UTXO set for outBeeCreation\n");
            bctValue = coin.out.nValue;
            bctScriptPubKey = coin.out.scriptPubKey;
            bctFoundHeight = coin.nHeight;

        } else {                                                            // UTXO set isn't available when eg reindexing, so drill into block db (not too bad, since Alice put her BCT height in the coinbase tx)
            if (verbose)
                LogPrintf("! CheckHiveProof: Warn: Using deep drill for outBeeCreation\n");
            if (!GetTxByHashAndHeight(uint256S(txidStr), bctClaimedHeight, bct, foundAt, pindexPrev, consensusParams)) {
                LogPrintf("CheckHiveProof: Couldn't locate indicated BCT\n");
                return false;
            }
            deepDrill = true;
            bctFoundHeight = foundAt.nHeight;
            bctValue = bct->vout[0].nValue;
            bctScriptPubKey = bct->vout[0].scriptPubKey;

            
        }

        if (communityContrib) {
            CScript scriptPubKeyCF = GetScriptForDestination(DecodeDestination(consensusParams.hiveCommunityAddress));
            CAmount donationAmount;

            if(bct == nullptr) {                                                                // If we dont have a ref to the BCT
                if (pcoinsTip && pcoinsTip->GetCoin(outCommFund, coin)) {                       // First try UTXO set
                    if (verbose)
                        LogPrintf("CheckHiveProof: Using UTXO set for outCommFund\n");
                    if (coin.out.scriptPubKey != scriptPubKeyCF) {                              // If we find it, validate the scriptPubKey and store amount
                        LogPrintf("CheckHiveProof: Community contrib was indicated but not found\n");
                        return false;
                    }
                    donationAmount = coin.out.nValue;
                } else {                                                                        // Fallback if we couldn't use UTXO set
                    if (verbose)
                        LogPrintf("! CheckHiveProof: Warn: Using deep drill for outCommFund\n");
                    if (!GetTxByHashAndHeight(uint256S(txidStr), bctClaimedHeight, bct, foundAt, pindexPrev, consensusParams)) {
                        LogPrintf("CheckHiveProof: Couldn't locate indicated BCT\n");           // Still couldn't find it
                        return false;
                    }
                    deepDrill = true;
                }
            }
            if(bct != nullptr) {                                                                // We have the BCT either way now (either from first or second drill). If got from UTXO set bct == nullptr still.
                if (bct->vout.size() < 2 || bct->vout[1].scriptPubKey != scriptPubKeyCF) {      // So Validate the scriptPubKey and store amount
                    LogPrintf("CheckHiveProof: Community contrib was indicated but not found\n");
                    return false;
                }
                donationAmount = bct->vout[1].nValue;
            }

            // Check for valid donation amount
            CAmount expectedDonationAmount = (bctValue + donationAmount) / consensusParams.communityContribFactor;
            expectedDonationAmount += expectedDonationAmount >> 1;

            if (donationAmount != expectedDonationAmount) {
                LogPrintf("CheckHiveProof: BCT pays community fund incorrect amount %i (expected %i)\n", donationAmount, expectedDonationAmount);
                return false;
            }

            // Update amount paid
            bctValue += donationAmount;
        }
    }

    if (bctFoundHeight != bctClaimedHeight) {
        LogPrintf("CheckHiveProof: Claimed BCT height of %i conflicts with found height of %i\n", bctClaimedHeight, bctFoundHeight);
        return false;
    }

    // Check bee maturity
    int bctDepth = blockHeight - bctFoundHeight;
    if (bctDepth < consensusParams.beeGestationBlocks) {
        LogPrintf("CheckHiveProof: Indicated BCT is immature.\n");
        return false;
    }
    if (bctDepth > consensusParams.beeGestationBlocks + consensusParams.beeLifespanBlocks) {
        LogPrintf("CheckHiveProof: Indicated BCT is too old.\n");
        return false;
    }

    // Check for valid bee creation script and get honey scriptPubKey from BCT
    CScript scriptPubKeyHoney;
    if (!CScript::IsBCTScript(bctScriptPubKey, scriptPubKeyBCF, &scriptPubKeyHoney)) {
        LogPrintf("CheckHiveProof: Indicated utxo is not a valid BCT script\n");
        return false;
    }

    CTxDestination honeyDestinationBCT;
    if (!ExtractDestination(scriptPubKeyHoney, honeyDestinationBCT)) {
        LogPrintf("CheckHiveProof: Couldn't extract honey address from BCT UTXO\n");
        return false;
    }

    // Check BCT's honey address actually matches the claimed honey address
    if (honeyDestination != honeyDestinationBCT) {
        LogPrintf("CheckHiveProof: BCT's honey address does not match claimed honey address!\n");
        return false;
    }


    // Find bee count
    CAmount beeCost = GetBeeCost(bctFoundHeight, consensusParams);
    if (bctValue < consensusParams.minBeeCost) {
        LogPrintf("CheckHiveProof: BCT fee is less than the minimum possible bee cost\n");
        return false;
    }
    if (bctValue < beeCost) {
        LogPrintf("CheckHiveProof: BCT fee is less than the cost for a single bee\n");
        return false;
    }
    unsigned int beeCount = bctValue / beeCost;
    if (verbose) {
        LogPrintf("CheckHiveProof: bctValue            = %i\n", bctValue);
        LogPrintf("CheckHiveProof: beeCost             = %i\n", beeCost);
        LogPrintf("CheckHiveProof: beeCount            = %i\n", beeCount);
    }
    
    // Check enough bees were bought to include claimed beeNonce
    if (beeNonce >= beeCount) {
        LogPrintf("CheckHiveProof: BCT did not create enough bees for claimed nonce!\n");
        return false;
    }

    if (verbose)
        LogPrintf("CheckHiveProof: Pass at %i%s\n", blockHeight, deepDrill ? " (used deepdrill)" : "");

    return true;
}
