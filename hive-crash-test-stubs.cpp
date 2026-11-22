// Link-time stubs for wallet symbols that --disable-wallet omits from the
// libraries, but that miner.o (pulled transitively via IncrementExtraNonce)
// references. The CheckHiveProof crash-test never executes wallet code; these
// exist only so the static link resolves. Any accidental call aborts loudly.
#include <wallet/wallet.h>
#include <rpc/server.h>
#include <stdexcept>

CWallet* GetWalletForJSONRPCRequest(const JSONRPCRequest& request)
{
    (void)request;
    throw std::runtime_error("harness stub: wallet disabled");
}

bool EnsureWalletIsAvailable(CWallet* pwallet, bool avoidException)
{
    (void)pwallet;
    (void)avoidException;
    throw std::runtime_error("harness stub: wallet disabled");
}

std::vector<CBeeCreationTransactionInfo> CWallet::GetBCTs(bool includeDead, bool scanRewards, const Consensus::Params& consensusParams, int minHoneyConfirmations)
{
    (void)includeDead; (void)scanRewards; (void)consensusParams; (void)minHoneyConfirmations;
    throw std::runtime_error("harness stub: wallet disabled");
}

bool CCryptoKeyStore::IsLocked() const
{
    throw std::runtime_error("harness stub: wallet disabled");
}

bool CCryptoKeyStore::GetKey(const CKeyID& address, CKey& keyOut) const
{
    (void)address; (void)keyOut;
    throw std::runtime_error("harness stub: wallet disabled");
}
