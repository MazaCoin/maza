// A/B regression harness for the CheckHiveProof null-deref crash.
//
// Reproduces the crash case from hive-vuln-analysis-2026-11-21.md section 3:
// a Hive block whose hashPrevBlock is ABSENT from mapBlockIndex (corrupt
// blk*.dat / reindex path, validation.cpp:1111). Pre-fix code does
// mapBlockIndex[hash] (std::map::operator[] INSERTS nullptr) then derefs
// it before the null check -> SIGSEGV. Post-fix code must reject cleanly
// with return false and no crash.
//
// Exit codes: 0 = PASS (clean false), 1 = FAIL (returned true), 132/139 = crashed.
#include "chainparams.h"
#include "chainparamsbase.h"
#include "pow.h"
#include "primitives/block.h"
#include "uint256.h"

int main()
{
    SelectParams(CBaseChainParams::MAIN);
    const Consensus::Params& consensus = Params().GetConsensus();

    CBlock block; // empty header block, mapBlockIndex is EMPTY
    block.hashPrevBlock.SetHex("deadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeef");

    bool ok = CheckHiveProof(&block, consensus);

    if (ok) {
        printf("FAIL: CheckHiveProof returned true for absent prev block\n");
        return 1;
    }
    printf("PASS: absent prev block rejected cleanly (return false, no crash)\n");
    return 0;
}
