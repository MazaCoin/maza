// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "consensus/merkle.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"

#include <cassert>

static CBlock CreateGenesisBlock(const char *pszTimestamp,
                                 const CScript &genesisOutputScript,
                                 uint32_t nTime, uint32_t nNonce,
                                 uint32_t nBits, int32_t nVersion,
                                 const CAmount &genesisReward) {
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig =
        CScript() << 486604799 << CScriptNum(4)
                  << std::vector<uint8_t>((const uint8_t *)pszTimestamp,
                                          (const uint8_t *)pszTimestamp +
                                              strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime = nTime;
    genesis.nBits = nBits;
    genesis.nNonce = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation transaction
 * cannot be spent since it did not originally exist in the database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000,
 * hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893,
 * vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase
 * 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce,
                                 uint32_t nBits, int32_t nVersion,
                                 const CAmount &genesisReward) {
    const char *pszTimestamp =
        "February 5, 2014: The Black Hills are not for sale - 1868 Is The LAW!";
    const CScript genesisOutputScript =
        CScript() << 
        ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f")
        
                  << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce,
                              nBits, nVersion, genesisReward);
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
        consensus.BIP34Height = 0;
        consensus.BIP66Height = 756218;
        consensus.powLimit = uint256S(
            "00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.startingDifficulty = uint256S(
            "00000003ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        // two weeks
        consensus.nPowTargetTimespan = 8 * 60; // ???14 * 24 * 60 * 60;
        consensus.nPowTargetSpacing = 2 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        // 95% of 2016
        consensus.nRuleChangeActivationThreshold = consensus.nPowTargetTimespan / consensus.nPowTargetSpacing;
        consensus.nMinerConfirmationWindow = 2016;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime =
            1199145601;
        // December 31, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout =
            1230767999;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork =
            uint256S("0x0000000000000000000000000000000000000000003f94d1ad39168"
                     "2fe038bf5");

        // By default assume that the signatures in ancestors of this block are
        // valid.
        consensus.defaultAssumeValid =
            uint256S("0x000000000000000000ff3a41f208c932d5f91fe8d0739fca36152f6"
                     "073b2ef5e");

        // hard fork time. This will be changed once we set the time. Right now effectively disabled
        consensus.uahfStartTime = 9876543210;


        /**
         * The message start string is designed to be unlikely to occur in
         * normal data. The characters are rarely used upper ASCII, not valid as
         * UTF-8, and produce a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xf8;
        pchMessageStart[1] = 0xb5;
        pchMessageStart[2] = 0x03;
        pchMessageStart[3] = 0xdf;
        nDefaultPort = 12835;
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(1390747675, 2091390249, 0x1e0ffff0, 1,  5000 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock ==
               uint256S("0x00000c7c73d8ce604178dae13f0fc6ec0be3275614366d44b1b4b5c6e238c60c"));
        
        assert(genesis.hashMerkleRoot ==
               uint256S("0x62d496378e5834989dd9594cfc168dbb76f84a39bbda18286cddc7d1d1589f4f"));

        // Note that of those with the service bits flag, most only support a
        // subset of possible options.
        vSeeds.push_back(CDNSSeedData("mazacoin.org", "node.mazacoin.org", true));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<uint8_t>(1, 50);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<uint8_t>(1, 9);
        base58Prefixes[SECRET_KEY] = std::vector<uint8_t>(1, 224);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        checkpointData = {
            .mapCheckpoints = {
                {0,uint256S("0x00000c7c73d8ce604178dae13f0fc6ec0be3275614366d44b1b4b5c6e238c60c")},
                {91800,uint256S("0x00000000000000f35417a67ff0bb5cec6a1c64d13bb1359ae4a03d2c9d44d900")},
                {183600,uint256S("0x0000000000000787f10fa4a547822f8170f1f182ca0de60ecd2de189471da885")},
                {700000, uint256S("000000000000018674cd89025fc8190e5fc1a558dce38392e43f3603cb1cb192")},
                {750000, uint256S("0000000000000024a619312835504165c91b817a50ee724fc3f2a48565fdb555")},
                {800000, uint256S("000000000000010c0245a794d16023ffb7a0e5f0fceb991e9f15706e711272de")},
                {850000, uint256S("000000000000025553ea305539a442cfa620d5224252f641f5250a52b53cdea7")},
                {870000, uint256S("00000000000004386593649e6ad9a2ed3153710d94a55bf8dfa630baf53ec5ec")}}};
        
        // Data as of block
        // (height =468990).
        chainTxData = ChainTxData{
            // UNIX timestamp of last known number of transactions.
            1451416800,
            // Total number of transactions between genesis and that timestamp
            // (the tx=... number in the SetBestChain debug.log lines)
            1138459,
            // Estimated number of transactions per second after that timestamp.
            0.02};
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
        consensus.nSubsidyHalvingInterval = 950000;
        consensus.BIP34Height = 100; // Guess - somewhere between 10 and 100
        consensus.BIP66Height = 0;
        consensus.powLimit = uint256S(
                                      "00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.startingDifficulty = uint256S(
                                                "00000003ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

        // two weeks
        consensus.nPowTargetTimespan = 8 * 60;
        consensus.nPowTargetSpacing = 2 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        // 75% for testchains
        consensus.nRuleChangeActivationThreshold = 1512;
        // nPowTargetTimespan / nPowTargetSpacing
        consensus.nMinerConfirmationWindow = 2016;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601;
        // December 31, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout =  1230767999;

        // hard fork time. This will be changed once we set the time. Right now effectively disabled
        consensus.uahfStartTime = 9876543210;
        
        // The best chain should have at least this much work.
        consensus.nMinimumChainWork =
            uint256S("0x00000000000000000000000000000000000000000000001f057509e"
                     "ba81aed91");

        // By default assume that the signatures in ancestors of this block are
        // valid.
        consensus.defaultAssumeValid =
            uint256S("0x00000000000128796ee387cf110ccb9d2f36cffaf7f73079c995377"
                     "c65ac0dcc");

        pchMessageStart[0] = 0x05;
        pchMessageStart[1] = 0xfe;
        pchMessageStart[2] = 0xa9;
        pchMessageStart[3] = 0x01;
        nDefaultPort = 11835;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1411587941, 2091634749, 0x1e0ffff0, 1, 5000 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x000003ae7f631de18a457fa4fa078e6fa8aff38e258458f8189810de5d62cede"));
        /*
          std::cout << genesis.hashMerkleRoot.ToString() << "\n";
          assert(genesis.hashMerkleRoot ==
          uint256S("0x4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab212"
          "7b7afdeda33b"));
        */
        
        vFixedSeeds.clear();
        vSeeds.clear();
        // nodes with support for servicebits filtering should be at the top
        vSeeds.push_back(CDNSSeedData("mazatest.cryptoadhd.com",
                                      "mazatest.cryptoadhd.com", true));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<uint8_t>(1, 88);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<uint8_t>(1, 188);
        base58Prefixes[SECRET_KEY] = std::vector<uint8_t>(1, 239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;

        checkpointData = {
            .mapCheckpoints = {
                { 0, uint256S("0x000007717e2e2df52a9ff29b0771901c9c12f5cbb4914cdf0c8047b459bb21d8")}
            }};

        // Data as of block
        // 00000000c2872f8f8a8935c8e3c5862be9038c97d4de2cf37ed496991166928a
        // (height 1063660)
        chainTxData = ChainTxData{
            // UNIX timestamp of last known number of transactions.
            1520575285,
            // Total number of transactions between genesis and that timestamp
            // (the tx=... number in the SetBestChain debug.log lines)
            421700,
            // Estimated number of transactions per second after that timestamp.
            0.008333};
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
        consensus.nSubsidyHalvingInterval = 150;
        // BIP34 has not activated on regtest (far in the future so block v1 are
        // not rejected in tests)
        consensus.BIP34Height = 100000000;
        // BIP65 activated on regtest (Used in rpc activation tests)
        consensus.BIP65Height = 1351;
        // BIP66 activated on regtest (Used in rpc activation tests)
        consensus.BIP66Height = 1251;
        consensus.powLimit = uint256S(
                                      "7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.startingDifficulty = uint256S(
                                      "3fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

        consensus.nPowTargetTimespan = 8 * 60;
        consensus.nPowTargetSpacing = 2 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        // 75% for testchains
        consensus.nRuleChangeActivationThreshold = 108;
        // Faster than normal for regtest (144 instead of 2016)
        consensus.nMinerConfirmationWindow = 144;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout =
            999999999999ULL;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are
        // valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        // TBD: Hard fork is always enabled on regtest.
        consensus.uahfStartTime = 20;///9876543210;

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0x0f;
        pchMessageStart[2] = 0xa5;
        pchMessageStart[3] = 0x5a;
        nDefaultPort = 11444;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1390748221, 4, 0x207fffff, 1, 5000 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        
        //std::cout << genesis.GetHash().ToString() << "\n";
        //std::cout << genesis.hashMerkleRoot.ToString() << "\n";

        assert(consensus.hashGenesisBlock ==
               uint256S("57939ce0a96bf42965fee5956528a456d0edfb879b8bd699bcbb4786d27b979d"));
        assert(genesis.hashMerkleRoot ==
               uint256S("62d496378e5834989dd9594cfc168dbb76f84a39bbda18286cddc7d1d1589f4f"));

        //!< Regtest mode doesn't have any fixed seeds.
        vFixedSeeds.clear();
        //!< Regtest mode doesn't have any DNS seeds.
        vSeeds.clear();

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        checkpointData = {.mapCheckpoints = {
                              {0, uint256S("57939ce0a96bf42965fee5956528a456d0edfb879b8bd699bcbb4786d27b979d")},
                          }};

        chainTxData = ChainTxData{0, 0, 0};

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<uint8_t>(1, 111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<uint8_t>(1, 196);
        base58Prefixes[SECRET_KEY] = std::vector<uint8_t>(1, 239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};
        
    }

    void UpdateBIP9Parameters(Consensus::DeploymentPos d, int64_t nStartTime,
                              int64_t nTimeout) {
        consensus.vDeployments[d].nStartTime = nStartTime;
        consensus.vDeployments[d].nTimeout = nTimeout;
    }
};

static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams &Params(const std::string &chain) {
    if (chain == CBaseChainParams::MAIN)
        return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
        return testNetParams;
    else if (chain == CBaseChainParams::REGTEST)
        return regTestParams;
    else
        throw std::runtime_error(
            strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string &network) {
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

void UpdateRegtestBIP9Parameters(Consensus::DeploymentPos d, int64_t nStartTime,
                                 int64_t nTimeout) {
    regTestParams.UpdateBIP9Parameters(d, nStartTime, nTimeout);
}
