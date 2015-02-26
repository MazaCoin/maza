// Copyright (c) 2011-2013 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

//
// Unit tests for block-chain checkpoints
//

#include "checkpoints.h"

#include "uint256.h"

#include <boost/test/unit_test.hpp>

using namespace std;

BOOST_AUTO_TEST_SUITE(Checkpoints_tests)

BOOST_AUTO_TEST_CASE(sanity)
{
    uint256 p91800 = uint256("0x00000000000000f35417a67ff0bb5cec6a1c64d13bb1359ae4a03d2c9d44d900");
    uint256 p183600 = uint256("0x0000000000000787f10fa4a547822f8170f1f182ca0de60ecd2de189471da885");
    BOOST_CHECK(Checkpoints::CheckBlock(91800, p91800));
    BOOST_CHECK(Checkpoints::CheckBlock(183600, p183600));

    
    // Wrong hashes at checkpoints should fail:
    BOOST_CHECK(!Checkpoints::CheckBlock(91800, p183600));
    BOOST_CHECK(!Checkpoints::CheckBlock(183600, p91800));

    // ... but any hash not at a checkpoint should succeed:
    BOOST_CHECK(Checkpoints::CheckBlock(91800+1, p183600));
    BOOST_CHECK(Checkpoints::CheckBlock(183600+1, p91800));

    BOOST_CHECK(Checkpoints::GetTotalBlocksEstimate() >= 183600);
}    

BOOST_AUTO_TEST_SUITE_END()
