// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Copyright (c) 2014-2021 The Litecoin Cash developers
// Copyright (c) 2014-2021 The Maza developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>
#include <consensus/merkle.h>

#include <tinyformat.h>
#include <util.h>
#include <utilstrencodings.h>
#include <base58.h> // Maza: Needed for DecodeDestination()

#include <assert.h>

#include <chainparamsseeds.h>

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "February 5, 2014: The Black Hills are not for sale - 1868 Is The LAW!";
    const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

void CChainParams::UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    consensus.vDeployments[d].nStartTime = nStartTime;
    consensus.vDeployments[d].nTimeout = nTimeout;
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 950000;
        consensus.BIP16Height = 1; 
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256S("0x000000003302fe58f139f1d45f3a0a67601d39e63b82bc4918f48b8cd5df6ab0");
        consensus.BIP65Height = 2105603;  // future block predicted beginning February 2022
        consensus.BIP66Height = 800000; 
        consensus.powLimitSHA = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
		consensus.startingDifficulty = uint256S("00000003ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 8 * 60; // Maza: 8 minutes
        consensus.nPowTargetSpacing = 2 * 60; // Maza: 2 minutes
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 6048; // 75% of 8064
        consensus.nMinerConfirmationWindow = 8064; // nPowTargetTimespan / nPowTargetSpacing * 4
		consensus.nPowDGWHeight = 100000;
		consensus.powForkTime = 1644645600; //set to minotaurx start time for MAZA
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1644645600; // Feb 12, 2022
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1644645600 + 31536000;  // Start + 1 year

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1644645600; // Feb 12, 2022
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1644645600 + 31536000;  // Start + 1 year

        // Maza: MinotaurX+Hive1.2: Deployment
        consensus.vDeployments[Consensus::DEPLOYMENT_MINOTAURX].bit = 7;
        consensus.vDeployments[Consensus::DEPLOYMENT_MINOTAURX].nStartTime = 1644645600;  // Feb 12, 2022
        consensus.vDeployments[Consensus::DEPLOYMENT_MINOTAURX].nTimeout = 1644645600 + 31536000;  // Start + 1 year

        
        // Maza: Hive: Consensus Fields
        consensus.minBeeCost = 10000;                       // Minimum cost of a bee, used when no more block rewards
        consensus.beeCostFactor = 2500;                     // Bee cost is block_reward/beeCostFactor
        consensus.beeCreationAddress = "MCreateBeeMainXXXXXXXXXXXXXXVQWqkH";        // Unspendable address for bee creation
        consensus.hiveCommunityAddress = "4xscpVDbThrWVk4GD177JqniTvZ8RPa6qo";      // Community fund address
        consensus.communityContribFactor = 10;              // Optionally, donate bct_value/maxCommunityContribFactor to community fund
        consensus.beeGestationBlocks = 30*24;               // The number of blocks for a new bee to mature
        consensus.beeLifespanBlocks = 30*24*14;             // The number of blocks a bee lives for after maturation
        consensus.powLimitHive = uint256S("0fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");  // Highest (easiest) bee hash target
        consensus.minHiveCheckBlock = 2105603;              // Don't bother checking below this height for Hive blocks (not used for consensus/validation checks, just efficiency when looking for potential BCTs)
        consensus.hiveBlockSpacingTarget = 2;               // Target Hive block frequency (1 out of this many blocks should be Hivemined)
        consensus.hiveBlockSpacingTargetTypical_1_1 = 2;    // Observed Hive block frequency in Hive 1.1 (1 out of this many blocks are observed to be Hive)
        consensus.hiveNonceMarker = 192;                    // Nonce marker for hivemined blocks

        // Maza: Hive 1.1-related consensus fields
        consensus.minK = 2;                                 // Minimum chainwork scale for Hive blocks (see Hive whitepaper section 5)
        consensus.maxK = 16;                                // Maximum chainwork scale for Hive blocks (see Hive whitepaper section 5)
        consensus.maxHiveDiff = 0.006;                      // Hive difficulty at which max chainwork bonus is awarded
        consensus.maxKPow = 5;                              // Maximum chainwork scale for PoW blocks
        consensus.powSplit1 = 0.005;                        // Below this Hive difficulty threshold, PoW block chainwork bonus is halved
        consensus.powSplit2 = 0.0025;                       // Below this Hive difficulty threshold, PoW block chainwork bonus is halved again
        consensus.maxConsecutiveHiveBlocks = 2;             // Maximum hive blocks that can occur consecutively before a PoW block is required
        consensus.hiveDifficultyWindow = 36;                // How many blocks the SMA averages over in hive difficulty adjust
        // SECURITY FIX: Replace max() with the community-agreed future activation height
        // before releasing the mainnet/testnet consensus upgrade.
        consensus.hiveCanonicalBctTxidHeight = 4520000;

        // Maza: MinotaurX+Hive1.2-related consensus fields
        consensus.lwmaAveragingWindow = 90;                 // Averaging window size for LWMA diff adjust
        consensus.powTypeLimits.emplace_back(uint256S("0x00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));   // sha256d limit
        consensus.powTypeLimits.emplace_back(uint256S("0x000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));   // MinotaurX limit

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x000000000000000000000000000000000000000000000ac96eea62eb8eaf493d");  // Maza: 2036783

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00000000000002a124800233414bbbc58a789290f3fb1eb5352cdfd7033dfa27"); // Maza: 2036783

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xf8;
        pchMessageStart[1] = 0xb5;
        pchMessageStart[2] = 0x03;
        pchMessageStart[3] = 0xdf;
        nDefaultPort = 12835;
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(1390747675, 2091390249, 0x1e0ffff0, 1, 5000 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x00000c7c73d8ce604178dae13f0fc6ec0be3275614366d44b1b4b5c6e238c60c"));
        assert(genesis.hashMerkleRoot == uint256S("0x62d496378e5834989dd9594cfc168dbb76f84a39bbda18286cddc7d1d1589f4f"));

        // Note that of those with the service bits flag, most only support a subset of possible options
        vSeeds.emplace_back("node.mazacoin.org");
	vSeeds.emplace_back("seed.mazanode.com");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,50);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,9);
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1,224);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "maza";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        checkpointData = {
            {
                {  91800, uint256S("0x00000000000000f35417a67ff0bb5cec6a1c64d13bb1359ae4a03d2c9d44d900")},
                {  183600, uint256S("0x0000000000000787f10fa4a547822f8170f1f182ca0de60ecd2de189471da885")},
                {  1148232, uint256S("0x00000000000000026e94b971fd0e966d9dba98eaf828a7814de2ef333312bb2c")},
                {  2036783, uint256S("0x00000000000002a124800233414bbbc58a789290f3fb1eb5352cdfd7033dfa27")}
                
            }
        };

        chainTxData = ChainTxData{
            // Data as of block 0000000000000012e28998c604bbfc58bc0ae30523bca8a2f41320f4db2655d1 (height 2511842).
            1635190116, // * UNIX timestamp of last known number of transactions
            3090764,   // * total number of transactions between genesis and that timestamp
                        //   (the tx=... number in the SetBestChain debug.log lines)
            0.0151      // * estimated number of transactions per second after that timestamp
        };
    }
};

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 950000;
        consensus.BIP16Height = 0; // always enforce BIP16
        consensus.BIP34Height = 100;
        consensus.BIP34Hash = uint256S("0x000000095bbba46901bc8b723224e93b127319bb28e163a3d00857c7aef528be"); // Block hash at block 1
        consensus.BIP65Height = 628001;
        consensus.BIP66Height = 100000;
        consensus.powLimitSHA = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
		consensus.startingDifficulty = uint256S("00000003ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 8 * 60; // Maza: 8 minutes
        consensus.nPowTargetSpacing = 2 * 60; // Maza: 2 minutes
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 15; // Require 75% of last 20 blocks to activate rulechanges
        consensus.nMinerConfirmationWindow = 20;
		consensus.nPowDGWHeight = 10;
		consensus.powForkTime = 1639094400;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1639090000; 
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1639090000+ 315360; 

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1639090000; 
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1639090000 + 315360; 

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1639094400; 
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1639094400 + 315360; 

        // Maza: MinotaurX+Hive1.2: Deployment
        consensus.vDeployments[Consensus::DEPLOYMENT_MINOTAURX].bit = 7;
        consensus.vDeployments[Consensus::DEPLOYMENT_MINOTAURX].nStartTime = 1639094400;  // Nov 30 2021  (later than above)
        consensus.vDeployments[Consensus::DEPLOYMENT_MINOTAURX].nTimeout = 1639094400 + 31536000;  // Start + 1 year
 
        // Maza: Hive: Consensus Fields
        consensus.minBeeCost = 10000;                       // Minimum cost of a bee, used when no more block rewards
        consensus.beeCostFactor = 2500;                     // Bee cost is block_reward/beeCostFactor
        consensus.beeCreationAddress = "ccReateBeetestnetXXXXXXXXXXXVPRtyV";        // Unspendable address for bee creation
        consensus.hiveCommunityAddress = "cUr9QKe9f7vk6174C45yyW6CLJ8Qq1MKLL";      // Community fund address
        consensus.communityContribFactor = 10;              // Optionally, donate bct_value/maxCommunityContribFactor to community fund
        consensus.beeGestationBlocks = 40;                  // The number of blocks for a new bee to mature
        consensus.beeLifespanBlocks = 48*24*14;             // The number of blocks a bee lives for after maturation
        consensus.powLimitHive = uint256S("0fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");  // Highest (easiest) bee hash target
        consensus.minHiveCheckBlock = 628001;                   // Don't bother checking below this height for Hive blocks (not used for consensus/validation checks, just efficiency when looking for potential BCTs)
        consensus.hiveBlockSpacingTarget = 2;               // Target Hive block frequency (1 out of this many blocks should be Hivemined)
        consensus.hiveBlockSpacingTargetTypical_1_1 = 2;    // Observed Hive block frequency in Hive 1.1 (1 out of this many blocks are observed to be Hive)
        consensus.hiveNonceMarker = 192;                    // Nonce marker for hivemined blocks

        // Maza: Hive 1.1-related consensus fields
        consensus.minK = 2;                                 // Minimum chainwork scale for Hive blocks (see Hive whitepaper section 5)
        consensus.maxK = 10;                                // Maximum chainwork scale for Hive blocks (see Hive whitepaper section 5)
        consensus.maxHiveDiff = 0.002;                      // Hive difficulty at which max chainwork bonus is awarded
        consensus.maxKPow = 5;                              // Maximum chainwork scale for PoW blocks
        consensus.powSplit1 = 0.001;                        // Below this Hive difficulty threshold, PoW block chainwork bonus is halved
        consensus.powSplit2 = 0.0005;                       // Below this Hive difficulty threshold, PoW block chainwork bonus is halved again
        consensus.maxConsecutiveHiveBlocks = 2;             // Maximum hive blocks that can occur consecutively before a PoW block is required
        consensus.hiveDifficultyWindow = 36;                // How many blocks the SMA averages over in hive difficulty adjust
        // SECURITY FIX: Replace max() with the community-agreed future activation height
        // before releasing the mainnet/testnet consensus upgrade.
        consensus.hiveCanonicalBctTxidHeight = 886290;

        // Maza: MinotaurX+Hive1.2-related consensus fields
        consensus.lwmaAveragingWindow = 90;                 // Averaging window size for LWMA diff adjust
        consensus.powTypeLimits.emplace_back(uint256S("0x00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));   // sha256d limit
        consensus.powTypeLimits.emplace_back(uint256S("0x000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));   // MinotaurX limit

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x000000000000000000000000000000000000000000000000005f2e22e5a21778");  // Block 627011

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00000000070a1a9e19174cf9b46a3a99ac672e560716bccf50c3087e0c542802"); // Block 627011

        pchMessageStart[0] = 0x05;
        pchMessageStart[1] = 0xfe;
        pchMessageStart[2] = 0xa9;
        pchMessageStart[3] = 0x01;
        nDefaultPort = 11835;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1411587941UL, 2091634749UL, 0x1e0ffff0, 1, 5000 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x000003ae7f631de18a457fa4fa078e6fa8aff38e258458f8189810de5d62cede"));
        assert(genesis.hashMerkleRoot == uint256S("0x62d496378e5834989dd9594cfc168dbb76f84a39bbda18286cddc7d1d1589f4f"));

        vFixedSeeds.clear();
        vSeeds.emplace_back("mazatest.cryptoadhd.com");
	vSeeds.emplace_back("testnet.seed.mazanode.com");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,88);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,188);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tmaza";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;

        checkpointData = (CCheckpointData) {
            {
				{1, uint256S("00000002a0b59d902c562804e26e28b9208dd766e08867fb896dd5bbed4e9a15")},
				{110, uint256S("000000031a3c2984813b9f1c842f741759b207bb2408170de536decc8e738652")},
                {261, uint256S("000000000babe88050bc39ce5aeaa3b002013dc0a812f5d4e073447bf9668502")},      
                {1999, uint256S("0000000002efad4b1cd3160a512c46ba31181194165b0d8f8d68a722536df4f6")},    
                {558275, uint256S("0000000015510795ae4174f9f4bfb119b303b25e9ca59e47f518c305850ee28b")},
            }
        };

        chainTxData = ChainTxData{  // As at block 558275
            1539159148,
            376369,
            0.001
        };
    }
};

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 150;
        consensus.BIP16Height = 0; // always enforce P2SH BIP16 on regtest
        consensus.BIP34Height = 100000000; // BIP34 has not activated on regtest (far in the future so block v1 are not rejected in tests)
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 1351; // BIP65 activated on regtest (Used in rpc activation tests)
        consensus.BIP66Height = 1251; // BIP66 activated on regtest (Used in rpc activation tests)
        consensus.powLimitSHA = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.startingDifficulty = consensus.powLimitSHA; // Regtest mines at the easiest target from its first block
        consensus.nPowTargetTimespan = 8 * 60; // 8 minutes
        consensus.nPowTargetSpacing = 2 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
		consensus.nPowDGWHeight = 4001;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // Maza fields
        
        consensus.hiveNonceMarker = 192;                    // Nonce marker for hivemined blocks
        consensus.hiveCanonicalBctTxidHeight = 0;           // Regtest: immediate activation

        // Maza: MinotaurX+Hive1.2: One proof-of-work limit per PoW type. Regtest
        // accepts the easiest target for every type.
        consensus.powTypeLimits.emplace_back(consensus.powLimitSHA);   // sha256d limit
        consensus.powTypeLimits.emplace_back(consensus.powLimitSHA);   // MinotaurX limit

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0x0f;
        pchMessageStart[2] = 0xa5;
        pchMessageStart[3] = 0x5a;
        nDefaultPort = 11444;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1390748221, 4, 0x207fffff, 1, 5000 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x57939ce0a96bf42965fee5956528a456d0edfb879b8bd699bcbb4786d27b979d"));
        assert(genesis.hashMerkleRoot == uint256S("0x62d496378e5834989dd9594cfc168dbb76f84a39bbda18286cddc7d1d1589f4f"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        checkpointData = {
            {
                {0, uint256S("000008ca1832a4baf228eb1553c03d3a2c8e02399550dd6ea8d65cec3ef23d2e")}
            }
        };

        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,140);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,19);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "rmaza";
    }
};

static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return std::unique_ptr<CChainParams>(new CMainParams());
    else if (chain == CBaseChainParams::TESTNET)
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    else if (chain == CBaseChainParams::REGTEST)
        return std::unique_ptr<CChainParams>(new CRegTestParams());
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);
}

void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    globalChainParams->UpdateVersionBitsParameters(d, nStartTime, nTimeout);
}
