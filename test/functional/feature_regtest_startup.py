#!/usr/bin/env python3
# Copyright (c) 2026 The Maza Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Regtest nodes must start, extend their chain, and load that chain again.

CRegTestParams asserted a genesis merkle root that the regtest genesis block
does not have, so every regtest node aborted while selecting its parameters.
It supplied no proof-of-work limit for its PoW types, which crashed the node
on the first block it connected, and no starting difficulty, so its blocks
were asked for a target of zero.
"""
from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import assert_equal

# A regtest P2PKH address. The node runs without a wallet, so blocks are mined
# to it with generatetoaddress.
FAUCET = "yWziQMcwmKjRdzi7eWjwiQX8EjWcd6dSg6"


class RegtestStartupTest(BitcoinTestFramework):
    def set_test_params(self):
        self.setup_clean_chain = True
        self.num_nodes = 1
        self.extra_args = [["-disablewallet"]]

    def run_test(self):
        node = self.nodes[0]
        assert_equal(node.getblockchaininfo()["chain"], "regtest")
        assert_equal(node.getblockcount(), 0)

        # 101 blocks is one past coinbase maturity.
        blocks = node.generatetoaddress(101, FAUCET)
        assert_equal(len(blocks), 101)
        assert_equal(node.getblockcount(), 101)

        first = node.getblock(blocks[0], 2)
        assert_equal(first["height"], 1)
        assert_equal(first.get("previousblockhash"), node.getblockhash(0))
        coinbase = first["tx"][0]
        paid_to = [vout["scriptPubKey"].get("addresses", [None])[0]
                   for vout in coinbase["vout"]]
        assert_equal(paid_to.count(FAUCET), 1)
        assert_equal(coinbase["vout"][paid_to.index(FAUCET)]["value"], 5000)

        # The chain that was just mined has to load again from disk.
        best = node.getbestblockhash()
        self.restart_node(0)
        node = self.nodes[0]
        assert_equal(node.getblockcount(), 101)
        assert_equal(node.getbestblockhash(), best)
        assert_equal(len(node.generatetoaddress(1, FAUCET)), 1)
        assert_equal(node.getblockcount(), 102)


if __name__ == '__main__':
    RegtestStartupTest().main()
