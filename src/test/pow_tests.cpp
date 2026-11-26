// Copyright (c) 2015-2017 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chain.h>
#include <chainparams.h>
#include <pow.h>
#include <random.h>
#include <sstream>
#include <util.h>
#include <test/test_bitcoin.h>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(pow_tests, BasicTestingSetup)

/* Test calculation of next difficulty target with no constraints applying */
BOOST_AUTO_TEST_CASE(get_next_work)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    int64_t nLastRetargetTime = 1358118740; // Block #278207
    CBlockIndex pindexLast;
    pindexLast.nHeight = 280223;
    pindexLast.nTime = 1358378777;  // Block #280223
    pindexLast.nBits =  0x1c0ac141;
    BOOST_CHECK_EQUAL(CalculateNextWorkRequired(&pindexLast, nLastRetargetTime, chainParams->GetConsensus()), 0x1c093f8d);
}

/* Test the constraint on the upper bound for next work */
BOOST_AUTO_TEST_CASE(get_next_work_pow_limit)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    int64_t nLastRetargetTime = 1317972665; // Block #0
    CBlockIndex pindexLast;
    pindexLast.nHeight = 2015;
    pindexLast.nTime = 1318480354;  // Block #2015
    pindexLast.nBits = 0x1e0ffff0;
    BOOST_CHECK_EQUAL(CalculateNextWorkRequired(&pindexLast, nLastRetargetTime, chainParams->GetConsensus()), 0x1e0fffff);
}

/* Test the constraint on the lower bound for actual time taken */
BOOST_AUTO_TEST_CASE(get_next_work_lower_limit_actual)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    int64_t nLastRetargetTime = 1401682934; // NOTE: Not an actual block time
    CBlockIndex pindexLast;
    pindexLast.nHeight = 578591;
    pindexLast.nTime = 1401757934;  // Block #578591
    pindexLast.nBits = 0x1b075cf1;
    BOOST_CHECK_EQUAL(CalculateNextWorkRequired(&pindexLast, nLastRetargetTime, chainParams->GetConsensus()), 0x1b01d73c);
}

/* Test the constraint on the upper bound for actual time taken */
BOOST_AUTO_TEST_CASE(get_next_work_upper_limit_actual)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    int64_t nLastRetargetTime = 1463690315; // NOTE: Not an actual block time
    CBlockIndex pindexLast;
    pindexLast.nHeight = 1001951;
    pindexLast.nTime = 1464900315;  // Block #46367
    pindexLast.nBits = 0x1b015318;
    BOOST_CHECK_EQUAL(CalculateNextWorkRequired(&pindexLast, nLastRetargetTime, chainParams->GetConsensus()), 0x1b054c60);
}

BOOST_AUTO_TEST_CASE(GetBlockProofEquivalentTime_test)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    std::vector<CBlockIndex> blocks(10000);
    for (int i = 0; i < 10000; i++) {
        blocks[i].pprev = i ? &blocks[i - 1] : nullptr;
        blocks[i].nHeight = i;
        blocks[i].nTime = 1269211443 + i * chainParams->GetConsensus().nPowTargetSpacing;
        blocks[i].nBits = 0x207fffff; /* target 0x7fffff000... */
        blocks[i].nChainWork = i ? blocks[i - 1].nChainWork + GetBlockProof(blocks[i - 1]) : arith_uint256(0);
    }

    for (int j = 0; j < 1000; j++) {
        CBlockIndex *p1 = &blocks[InsecureRandRange(10000)];
        CBlockIndex *p2 = &blocks[InsecureRandRange(10000)];
        CBlockIndex *p3 = &blocks[InsecureRandRange(10000)];

        int64_t tdiff = GetBlockProofEquivalentTime(*p1, *p2, *p3, chainParams->GetConsensus());
        BOOST_CHECK_EQUAL(tdiff, p1->GetBlockTime() - p2->GetBlockTime());
    }
}

BOOST_AUTO_TEST_CASE(hive_bct_txid_ascii_canonicalization)
{
    const std::string lowercase = "e07eb7612dad2c8129e568df00b3cea837f6d51dc0fa2497b26fd65d21d322f0";
    // Accepted historical proof at height 4,483,316; must be rejected only
    // at or after the separately configured consensus activation height.
    const std::string mixedcase = "E07eb7612DAD2C8129E568dF00b3cEa837f6d51Dc0fA2497b26fd65D21d322f0";

    BOOST_CHECK(IsCanonicalHiveBctTxid(lowercase));
    BOOST_CHECK(!IsCanonicalHiveBctTxid(mixedcase));
    BOOST_CHECK(!IsCanonicalHiveBctTxid(lowercase.substr(1))); // Too short
    BOOST_CHECK(!IsCanonicalHiveBctTxid(lowercase + "0"));    // Too long
    BOOST_CHECK(!IsCanonicalHiveBctTxid(std::string(64, 'g'))); // Not hex
    BOOST_CHECK_EQUAL(uint256S(lowercase).GetHex(), uint256S(mixedcase).GetHex());

    const auto mainParams = CreateChainParams(CBaseChainParams::MAIN);
    const auto regParams = CreateChainParams(CBaseChainParams::REGTEST);
    BOOST_CHECK(mainParams->GetConsensus().hiveCanonicalBctTxidHeight > 4489099);
    BOOST_CHECK_EQUAL(regParams->GetConsensus().hiveCanonicalBctTxidHeight, 0);
}


/* Maza: CheckProofOfWork() reads one limit per PoW type for every block it
 * checks, and GetNextWorkRequiredLWMA() reads one whenever it picks a target.
 * A network whose parameters supply fewer limits than that reads past the end
 * of the vector while connecting its first block. Regtest supplied none, so no
 * regtest node could start. */
BOOST_AUTO_TEST_CASE(pow_type_limits_cover_every_pow_type)
{
    for (const std::string& network : {CBaseChainParams::MAIN, CBaseChainParams::TESTNET, CBaseChainParams::REGTEST}) {
        const auto chainParams = CreateChainParams(network);
        const Consensus::Params& consensus = chainParams->GetConsensus();

        std::ostringstream supplied;
        supplied << network << " parameters supply " << consensus.powTypeLimits.size()
                 << " proof-of-work limits where every PoW type needs one of " << NUM_BLOCK_TYPES;
        BOOST_REQUIRE_MESSAGE(consensus.powTypeLimits.size() >= static_cast<size_t>(NUM_BLOCK_TYPES), supplied.str());
    }

    // Regtest mines at its easiest target for either type, so each of its
    // limits is the target its first nBits stands for.
    const auto regtestParams = CreateChainParams(CBaseChainParams::REGTEST);
    const Consensus::Params& regtest = regtestParams->GetConsensus();
    BOOST_CHECK_MESSAGE(regtest.powTypeLimits[POW_TYPE_SHA256] == regtest.powLimitSHA,
                        "regtest's sha256d limit is not its easiest target");
    BOOST_CHECK_MESSAGE(regtest.powTypeLimits[POW_TYPE_MINOTAURX] == regtest.powLimitSHA,
                        "regtest's MinotaurX limit is not its easiest target");
}

/* Regtest blocks are mined at the target its first nBits stands for, so that
 * is what the node has to accept. It must still refuse a block whose nBits
 * claims an easier target than its limit, and a hash above its target. Without
 * the limits this reads past the end of an empty vector, which is undefined
 * behaviour and not a clean failure: the unfixed tree is shown by the daemon
 * dying in CheckProofOfWork() on its first block, not by this case. */
BOOST_AUTO_TEST_CASE(regtest_accepts_its_easiest_target)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::REGTEST);
    const Consensus::Params& consensus = chainParams->GetConsensus();

    const unsigned int nBits = chainParams->GenesisBlock().nBits;
    arith_uint256 target;
    target.SetCompact(nBits);

    BOOST_CHECK_MESSAGE(CheckProofOfWork(ArithToUint256(target), nBits, consensus),
                        "regtest rejected a hash at the target its blocks are mined at");

    arith_uint256 above = target;
    ++above;
    BOOST_CHECK_MESSAGE(!CheckProofOfWork(ArithToUint256(above), nBits, consensus),
                        "regtest accepted a hash above its target");

    // One above the limit, as nBits. The limit is 2^255 - 1, and the compact
    // form of 2^255 carries no sign bit, so an easier target is the only thing
    // wrong with this block.
    arith_uint256 easier = UintToArith256(consensus.powLimitSHA);
    ++easier;
    const unsigned int easierBits = easier.GetCompact();
    bool negative = true;
    bool overflow = true;
    arith_uint256 claimed;
    claimed.SetCompact(easierBits, &negative, &overflow);
    BOOST_CHECK_MESSAGE(!negative && !overflow && claimed > UintToArith256(consensus.powLimitSHA),
                        "the easier target does not read back as a positive number above the limit");
    BOOST_CHECK_MESSAGE(!CheckProofOfWork(uint256(), easierBits, consensus),
                        "regtest accepted a block that claims an easier target than its limit");
}

/* Until a network has 20 difficulty-adjustment intervals of blocks,
 * GetNextWorkRequiredBTC() hands out its starting difficulty. Regtest never
 * set one, so its blocks were asked for a target of zero. Of the three fixes
 * only the first has no unit case that fails cleanly: without it the binary
 * aborts while selecting regtest's parameters. The second is caught by
 * pow_type_limits_cover_every_pow_type above, the third by this case. */
BOOST_AUTO_TEST_CASE(regtest_first_block_is_mined_at_its_easiest_target)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::REGTEST);
    const Consensus::Params& consensus = chainParams->GetConsensus();
    const CBlock& genesis = chainParams->GenesisBlock();

    CBlockIndex tip;
    tip.nHeight = 0;
    tip.nTime = genesis.nTime;
    tip.nBits = genesis.nBits;
    CBlockHeader next;
    next.nTime = genesis.nTime;

    BOOST_CHECK_EQUAL(GetNextWorkRequired(&tip, &next, consensus),
                      UintToArith256(consensus.powLimitSHA).GetCompact());
}

BOOST_AUTO_TEST_SUITE_END()
