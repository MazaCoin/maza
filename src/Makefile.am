# Copyright (c) 2013-2016 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

DIST_SUBDIRS = secp256k1 univalue

AM_LDFLAGS = $(PTHREAD_CFLAGS) $(LIBTOOL_LDFLAGS) $(HARDENED_LDFLAGS)
AM_CXXFLAGS = $(HARDENED_CXXFLAGS) $(ERROR_CXXFLAGS)
AM_CPPFLAGS = $(HARDENED_CPPFLAGS)
EXTRA_LIBRARIES =

if EMBEDDED_UNIVALUE
LIBUNIVALUE = univalue/libunivalue.la

$(LIBUNIVALUE): $(wildcard univalue/lib/*) $(wildcard univalue/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)
else
LIBUNIVALUE = $(UNIVALUE_LIBS)
endif

MAZA_CONFIG_INCLUDES=-I$(builddir)/config
MAZA_INCLUDES=-I$(builddir) -I$(builddir)/obj $(BDB_CPPFLAGS) $(BOOST_CPPFLAGS) $(LEVELDB_CPPFLAGS) $(CRYPTO_CFLAGS) $(SSL_CFLAGS)

MAZA_INCLUDES += -I$(srcdir)/secp256k1/include
MAZA_INCLUDES += $(UNIVALUE_CFLAGS)

LIBMAZA_SERVER=libmaza_server.a
LIBMAZA_COMMON=libmaza_common.a
LIBMAZA_CONSENSUS=libmaza_consensus.a
LIBMAZA_CLI=libmaza_cli.a
LIBMAZA_UTIL=libmaza_util.a
LIBMAZA_CRYPTO=crypto/libmaza_crypto.a
LIBMAZAQT=qt/libmazaqt.a
LIBSECP256K1=secp256k1/libsecp256k1.la

if ENABLE_ZMQ
LIBMAZA_ZMQ=libmaza_zmq.a
endif
if BUILD_MAZA_LIBS
LIBMAZACONSENSUS=libmazaconsensus.la
endif
if ENABLE_WALLET
LIBMAZA_WALLET=libmaza_wallet.a
endif

$(LIBSECP256K1): $(wildcard secp256k1/src/*) $(wildcard secp256k1/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)

# Make is not made aware of per-object dependencies to avoid limiting building parallelization
# But to build the less dependent modules first, we manually select their order here:
EXTRA_LIBRARIES += \
  $(LIBMAZA_CRYPTO) \
  $(LIBMAZA_UTIL) \
  $(LIBMAZA_COMMON) \
  $(LIBMAZA_CONSENSUS) \
  $(LIBMAZA_SERVER) \
  $(LIBMAZA_CLI) \
  $(LIBMAZA_WALLET) \
  $(LIBMAZA_ZMQ)

lib_LTLIBRARIES = $(LIBMAZACONSENSUS)

bin_PROGRAMS =
noinst_PROGRAMS =
TESTS =
BENCHMARKS =

if BUILD_MAZAD
  bin_PROGRAMS += mazad
endif

if BUILD_MAZA_UTILS
  bin_PROGRAMS += maza-cli maza-tx
endif

.PHONY: FORCE check-symbols check-security
# maza core #
MAZA_CORE_H = \
  addrdb.h \
  addrman.h \
  base58.h \
  bloom.h \
  blockencodings.h \
  chain.h \
  chainparams.h \
  chainparamsbase.h \
  chainparamsseeds.h \
  checkpoints.h \
  checkqueue.h \
  clientversion.h \
  coins.h \
  compat.h \
  compat/byteswap.h \
  compat/endian.h \
  compat/sanity.h \
  compressor.h \
  config.h \
  consensus/consensus.h \
  core_io.h \
  core_memusage.h \
  cuckoocache.h \
  globals.h \
  httprpc.h \
  httpserver.h \
  indirectmap.h \
  init.h \
  key.h \
  keystore.h \
  dbwrapper.h \
  limitedmap.h \
  memusage.h \
  merkleblock.h \
  miner.h \
  net.h \
  net_processing.h \
  netaddress.h \
  netbase.h \
  netmessagemaker.h \
  noui.h \
  policy/fees.h \
  policy/policy.h \
  pow.h \
  protocol.h \
  random.h \
  reverselock.h \
  rpc/blockchain.h \
  rpc/client.h \
  rpc/misc.h \
  rpc/protocol.h \
  rpc/server.h \
  rpc/register.h \
  scheduler.h \
  script/sigcache.h \
  script/sign.h \
  script/standard.h \
  script/ismine.h \
  streams.h \
  support/allocators/secure.h \
  support/allocators/zeroafterfree.h \
  support/cleanse.h \
  support/events.h \
  support/lockedpool.h \
  sync.h \
  threadsafety.h \
  threadinterrupt.h \
  timedata.h \
  torcontrol.h \
  txdb.h \
  txmempool.h \
  ui_interface.h \
  undo.h \
  util.h \
  utilmoneystr.h \
  utiltime.h \
  validation.h \
  validationinterface.h \
  versionbits.h \
  wallet/coincontrol.h \
  wallet/crypter.h \
  wallet/db.h \
  wallet/finaltx.h \
  wallet/rpcdump.h \
  wallet/rpcwallet.h \
  wallet/wallet.h \
  wallet/walletdb.h \
  warnings.h \
  zmq/zmqabstractnotifier.h \
  zmq/zmqconfig.h\
  zmq/zmqnotificationinterface.h \
  zmq/zmqpublishnotifier.h


obj/build.h: FORCE
	@$(MKDIR_P) $(builddir)/obj
	@$(top_srcdir)/share/genbuild.sh $(abs_top_builddir)/src/obj/build.h \
	  $(abs_top_srcdir)
libmaza_util_a-clientversion.$(OBJEXT): obj/build.h

# server: shared between mazad and maza-qt
libmaza_server_a_CPPFLAGS = $(AM_CPPFLAGS) $(MAZA_INCLUDES) $(MINIUPNPC_CPPFLAGS) $(EVENT_CFLAGS) $(EVENT_PTHREADS_CFLAGS)
libmaza_server_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libmaza_server_a_SOURCES = \
  addrman.cpp \
  addrdb.cpp \
  bloom.cpp \
  blockencodings.cpp \
  chain.cpp \
  checkpoints.cpp \
  config.cpp \
  globals.cpp \
  httprpc.cpp \
  httpserver.cpp \
  init.cpp \
  dbwrapper.cpp \
  merkleblock.cpp \
  miner.cpp \
  net.cpp \
  net_processing.cpp \
  noui.cpp \
  policy/fees.cpp \
  policy/policy.cpp \
  pow.cpp \
  rest.cpp \
  rpc/abc.cpp \
  rpc/blockchain.cpp \
  rpc/mining.cpp \
  rpc/misc.cpp \
  rpc/net.cpp \
  rpc/rawtransaction.cpp \
  rpc/server.cpp \
  script/sigcache.cpp \
  script/ismine.cpp \
  timedata.cpp \
  torcontrol.cpp \
  txdb.cpp \
  txmempool.cpp \
  ui_interface.cpp \
  validation.cpp \
  validationinterface.cpp \
  versionbits.cpp \
  $(MAZA_CORE_H)

if ENABLE_ZMQ
libmaza_zmq_a_CPPFLAGS = $(MAZA_INCLUDES) $(ZMQ_CFLAGS)
libmaza_zmq_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libmaza_zmq_a_SOURCES = \
  zmq/zmqabstractnotifier.cpp \
  zmq/zmqnotificationinterface.cpp \
  zmq/zmqpublishnotifier.cpp
endif


# wallet: shared between mazad and maza-qt, but only linked
# when wallet enabled
libmaza_wallet_a_CPPFLAGS = $(AM_CPPFLAGS) $(MAZA_INCLUDES)
libmaza_wallet_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libmaza_wallet_a_SOURCES = \
  wallet/crypter.cpp \
  wallet/db.cpp \
  wallet/finaltx.cpp \
  wallet/rpcdump.cpp \
  wallet/rpcwallet.cpp \
  wallet/wallet.cpp \
  wallet/walletdb.cpp \
  $(MAZA_CORE_H)

# crypto primitives library
crypto_libmaza_crypto_a_CPPFLAGS = $(AM_CPPFLAGS) $(MAZA_CONFIG_INCLUDES)
crypto_libmaza_crypto_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libmaza_crypto_a_SOURCES = \
  crypto/aes.cpp \
  crypto/aes.h \
  crypto/chacha20.h \
  crypto/chacha20.cpp \
  crypto/common.h \
  crypto/hmac_sha256.cpp \
  crypto/hmac_sha256.h \
  crypto/hmac_sha512.cpp \
  crypto/hmac_sha512.h \
  crypto/ripemd160.cpp \
  crypto/ripemd160.h \
  crypto/sha1.cpp \
  crypto/sha1.h \
  crypto/sha256.cpp \
  crypto/sha256.h \
  crypto/sha512.cpp \
  crypto/sha512.h

# consensus: shared between all executables that validate any consensus rules.
libmaza_consensus_a_CPPFLAGS = $(AM_CPPFLAGS) $(MAZA_INCLUDES)
libmaza_consensus_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libmaza_consensus_a_SOURCES = \
  amount.h \
  arith_uint256.cpp \
  arith_uint256.h \
  consensus/merkle.cpp \
  consensus/merkle.h \
  consensus/params.h \
  consensus/validation.h \
  hash.cpp \
  hash.h \
  prevector.h \
  primitives/block.cpp \
  primitives/block.h \
  primitives/transaction.cpp \
  primitives/transaction.h \
  pubkey.cpp \
  pubkey.h \
  script/bitcoinconsensus.cpp \
  script/interpreter.cpp \
  script/interpreter.h \
  script/script.cpp \
  script/script.h \
  script/script_error.cpp \
  script/script_error.h \
  serialize.h \
  tinyformat.h \
  uint256.cpp \
  uint256.h \
  utilstrencodings.cpp \
  utilstrencodings.h \
  version.h

# common: shared between mazad, and maza-qt and non-server tools
libmaza_common_a_CPPFLAGS = $(AM_CPPFLAGS) $(MAZA_INCLUDES)
libmaza_common_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libmaza_common_a_SOURCES = \
  amount.cpp \
  base58.cpp \
  chainparams.cpp \
  coins.cpp \
  compressor.cpp \
  core_read.cpp \
  core_write.cpp \
  key.cpp \
  keystore.cpp \
  netaddress.cpp \
  netbase.cpp \
  protocol.cpp \
  scheduler.cpp \
  script/sign.cpp \
  script/standard.cpp \
  warnings.cpp \
  $(MAZA_CORE_H)

# util: shared between all executables.
# This library *must* be included to make sure that the glibc
# backward-compatibility objects and their sanity checks are linked.
libmaza_util_a_CPPFLAGS = $(AM_CPPFLAGS) $(MAZA_INCLUDES)
libmaza_util_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libmaza_util_a_SOURCES = \
  support/lockedpool.cpp \
  chainparamsbase.cpp \
  clientversion.cpp \
  compat/glibc_sanity.cpp \
  compat/glibcxx_sanity.cpp \
  compat/strnlen.cpp \
  random.cpp \
  rpc/protocol.cpp \
  support/cleanse.cpp \
  sync.cpp \
  threadinterrupt.cpp \
  util.cpp \
  utilmoneystr.cpp \
  utilstrencodings.cpp \
  utiltime.cpp \
  $(MAZA_CORE_H)

if GLIBC_BACK_COMPAT
libmaza_util_a_SOURCES += compat/glibc_compat.cpp
endif

# cli: shared between maza-cli and maza-qt
libmaza_cli_a_CPPFLAGS = $(AM_CPPFLAGS) $(MAZA_INCLUDES)
libmaza_cli_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libmaza_cli_a_SOURCES = \
  rpc/client.cpp \
  $(MAZA_CORE_H)

nodist_libmaza_util_a_SOURCES = $(srcdir)/obj/build.h
#

# mazad binary #
mazad_SOURCES = mazad.cpp
mazad_CPPFLAGS = $(AM_CPPFLAGS) $(MAZA_INCLUDES)
mazad_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
mazad_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
mazad_SOURCES += mazad-res.rc
endif

mazad_LDADD = \
  $(LIBMAZA_SERVER) \
  $(LIBMAZA_COMMON) \
  $(LIBUNIVALUE) \
  $(LIBMAZA_UTIL) \
  $(LIBMAZA_WALLET) \
  $(LIBMAZA_ZMQ) \
  $(LIBMAZA_CONSENSUS) \
  $(LIBMAZA_CRYPTO) \
  $(LIBLEVELDB) \
  $(LIBMEMENV) \
  $(LIBSECP256K1)

mazad_LDADD += $(BOOST_LIBS) $(BDB_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(MINIUPNPC_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(ZMQ_LIBS)

# maza-cli binary #
maza_cli_SOURCES = maza-cli.cpp
maza_cli_CPPFLAGS = $(AM_CPPFLAGS) $(MAZA_INCLUDES) $(EVENT_CFLAGS)
maza_cli_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
maza_cli_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
maza_cli_SOURCES += maza-cli-res.rc
endif

maza_cli_LDADD = \
  $(LIBMAZA_CLI) \
  $(LIBUNIVALUE) \
  $(LIBMAZA_UTIL) \
  $(LIBMAZA_CRYPTO)

maza_cli_LDADD += $(BOOST_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(EVENT_LIBS)
#

# maza-tx binary #
maza_tx_SOURCES = maza-tx.cpp
maza_tx_CPPFLAGS = $(AM_CPPFLAGS) $(MAZA_INCLUDES)
maza_tx_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
maza_tx_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
maza_tx_SOURCES += maza-tx-res.rc
endif

maza_tx_LDADD = \
  $(LIBUNIVALUE) \
  $(LIBMAZA_COMMON) \
  $(LIBMAZA_UTIL) \
  $(LIBMAZA_CONSENSUS) \
  $(LIBMAZA_CRYPTO) \
  $(LIBSECP256K1)

maza_tx_LDADD += $(BOOST_LIBS) $(CRYPTO_LIBS)
#

# mazaconsensus library #
if BUILD_MAZA_LIBS
include_HEADERS = script/bitcoinconsensus.h
libmazaconsensus_la_SOURCES = $(crypto_libmaza_crypto_a_SOURCES) $(libmaza_consensus_a_SOURCES)

if GLIBC_BACK_COMPAT
  libmazaconsensus_la_SOURCES += compat/glibc_compat.cpp
endif

libmazaconsensus_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined $(RELDFLAGS)
libmazaconsensus_la_LIBADD = $(LIBSECP256K1)
libmazaconsensus_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(builddir)/obj -I$(srcdir)/secp256k1/include -DBUILD_MAZA_INTERNAL
libmazaconsensus_la_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)

endif
#

CTAES_DIST =  crypto/ctaes/bench.c
CTAES_DIST += crypto/ctaes/ctaes.c
CTAES_DIST += crypto/ctaes/ctaes.h
CTAES_DIST += crypto/ctaes/README.md
CTAES_DIST += crypto/ctaes/test.c

CLEANFILES = $(EXTRA_LIBRARIES)

CLEANFILES += *.gcda *.gcno
CLEANFILES += compat/*.gcda compat/*.gcno
CLEANFILES += consensus/*.gcda consensus/*.gcno
CLEANFILES += crypto/*.gcda crypto/*.gcno
CLEANFILES += policy/*.gcda policy/*.gcno
CLEANFILES += primitives/*.gcda primitives/*.gcno
CLEANFILES += script/*.gcda script/*.gcno
CLEANFILES += support/*.gcda support/*.gcno
CLEANFILES += univalue/*.gcda univalue/*.gcno
CLEANFILES += wallet/*.gcda wallet/*.gcno
CLEANFILES += wallet/test/*.gcda wallet/test/*.gcno
CLEANFILES += zmq/*.gcda zmq/*.gcno

DISTCLEANFILES = obj/build.h

EXTRA_DIST = $(CTAES_DIST)

clean-local:
	-$(MAKE) -C secp256k1 clean
	-$(MAKE) -C univalue clean
	-rm -f leveldb/*/*.gcda leveldb/*/*.gcno leveldb/helpers/memenv/*.gcda leveldb/helpers/memenv/*.gcno
	-rm -rf test/__pycache__

.rc.o:
	@test -f $(WINDRES)
	## FIXME: How to get the appropriate modulename_CPPFLAGS in here?
	$(AM_V_GEN) $(WINDRES) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(CPPFLAGS) -DWINDRES_PREPROC -i $< -o $@

.mm.o:
	$(AM_V_CXX) $(OBJCXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
	  $(CPPFLAGS) $(AM_CXXFLAGS) $(QT_INCLUDES) $(AM_CXXFLAGS) $(PIE_FLAGS) $(CXXFLAGS) -c -o $@ $<

check-symbols: $(bin_PROGRAMS)
if GLIBC_BACK_COMPAT
	@echo "Checking glibc back compat..."
	$(AM_V_at) READELF=$(READELF) CPPFILT=$(CPPFILT) $(top_srcdir)/contrib/devtools/symbol-check.py < $(bin_PROGRAMS)
endif

check-security: $(bin_PROGRAMS)
if HARDEN
	@echo "Checking binary security..."
	$(AM_V_at) READELF=$(READELF) OBJDUMP=$(OBJDUMP) $(top_srcdir)/contrib/devtools/security-check.py < $(bin_PROGRAMS)
endif

%.pb.cc %.pb.h: %.proto
	@test -f $(PROTOC)
	$(AM_V_GEN) $(PROTOC) --cpp_out=$(@D) --proto_path=$(<D) $<

if EMBEDDED_LEVELDB
include Makefile.leveldb.include
endif

if ENABLE_TESTS
include Makefile.test.include
endif

if ENABLE_BENCH
include Makefile.bench.include
endif

if ENABLE_QT
include Makefile.qt.include
endif

if ENABLE_QT_TESTS
include Makefile.qttest.include
endif
