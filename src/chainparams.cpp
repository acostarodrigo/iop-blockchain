// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "consensus/merkle.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

#include "chainparamsseeds.h"

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
    genesis.vtx.push_back(txNew);
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
	const char* pszTimestamp = "La Nacion May 16th 2016 - Sarmiento cerca del descenso";
	const CScript genesisOutputScript = CScript() << ParseHex("04ce49f9cdc8d23176c818fd7e27e7b614d128a47acfdad0e4542300e7efbd8879f1337af3188c0dcb0747fdf26d0cb3b0fca0f4e5d7aec53c43f4a933f570ae86") << OP_CHECKSIG;
	return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
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
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.nPreminedBlocks = 42000; // 42000 blocks equals the Premined amount
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        /* IoP beta change - BIP30 being enforced since beginning */
        consensus.BIP34Height = -1;
        consensus.BIP34Hash = uint256();
        consensus.powLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1462060800; // May 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800; // May 1st, 2017

        // Deployment of SegWit (BIP141 and BIP143)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 0; // Never / undefined

        // Miner White list params
        consensus.minerWhiteListActivationHeight = 110; //block height that activates the white list.
        consensus.minerWhiteListAdminPubKey.insert("03902b311c298f7d32eb2ccb71abde7afd39745f505e6e677cabc3964eea7960dc"); //Markus
        consensus.minerWhiteListAdminPubKey.insert("038f21c88b0d7f60e736cc447a3a6716c81a7b403b27bede2b67522d7c29b6e608"); //Rodrigo
        consensus.minerWhiteListAdminPubKey.insert("02ffc2c8d14db4644b30d061803998621d7cc8903da1c166d382ac033bf5972bc9"); //David
        consensus.minerWhiteListAdminPubKey.insert("03dfe647225b43395a94441fa20c75fdb644441d5115da26ef0da4f680acd28a28"); //Luis
        consensus.minerWhiteListAdminPubKey.insert("035344b025a91c24b47831919923a57e8db3e30f85b64d4fbf60ae8a6ffe4606c7"); //Guillermo

        consensus.minerWhiteListAdminAddress.insert("pUSydiLr9kFjtL7VbtfMYXMz7GLV413coQ"); //Markus
        consensus.minerWhiteListAdminAddress.insert("pGNcLNCavQLGXwXkVDwoHPCuQUBoXzJtPh"); //Rodrigo
        consensus.minerWhiteListAdminAddress.insert("pEppLTPsCKc978Dupb8vo5NXuCc3z1r6C7"); //David
        consensus.minerWhiteListAdminAddress.insert("pK2JKbgaqUNq2SyVcrB7H7wo4S8YJKsLyz"); //Luis
        consensus.minerWhiteListAdminAddress.insert("pJAKv7ii6MmyWL9ovVByVsq25HbPwoRiuW"); //Guillermo
        consensus.minerWhiteListMinAdminConsensus = 3; // 3 admin must generate transaction to add or remove miners
        consensus.minerCapSystemChangeHeight = 40320;
        // Voting System Params
        consensus.ccBlockStartAdditionalHeight = 1000;



        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xfd;
		pchMessageStart[1] = 0xb0;
		pchMessageStart[2] = 0xbb;
		pchMessageStart[3] = 0xd3;
        nDefaultPort = 4877;
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(1463452181, 1875087468, 0x1d00ffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x00000000bf5f2ee556cb9be8be64e0776af14933438dbb1af72c41bfb6c82db3"));
        assert(genesis.hashMerkleRoot == uint256S("0x951bc46d2da95047fb4a2c0f9b7d6e45591c3ffb49ab2fdffb8e96ef2b8f2be1"));

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("Seed server 1", "ham1.fermat.cloud"));
        vSeeds.push_back(CDNSSeedData("Seed server 2", "ham2.fermat.cloud"));
        vSeeds.push_back(CDNSSeedData("Seed server 3", "ham3.fermat.cloud"));
        vSeeds.push_back(CDNSSeedData("Seed server 4", "ham4.fermat.cloud"));
        vSeeds.push_back(CDNSSeedData("Seed server 5", "ham5.fermat.cloud"));
        vSeeds.push_back(CDNSSeedData("Seed server 5", "ham5.fermat.cloud"));
        vSeeds.push_back(CDNSSeedData("dejf DNS Seed", "iop-ts-main.amberloom.com"));

		base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,117);
		base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,174);
		base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,49);
		base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x27)(0x80)(0x91)(0x5F).convert_to_container<std::vector<unsigned char> >();
		base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0xAE)(0x34)(0x16)(0xF6).convert_to_container<std::vector<unsigned char> >();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = false;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (    0, uint256S("0x00000000bf5f2ee556cb9be8be64e0776af14933438dbb1af72c41bfb6c82db3"))
            (10000, uint256S("0x0000000009c613396a7cd6a2d6d7dc2e07e6d4485ec03bb64e951273133663ee"))
            (20000, uint256S("0x000000000205ce279aed9220fbac67f6f7a863f898f98ef0cdeae863e2d19bc1"))
            (30000, uint256S("0x0000000003a1cdeed2c42ddac58bfb807f93d8458f1d7da834640e32898b20b6"))
            (38500, uint256S("0x0000000004bde9ba366c93490f1d6a701f036cfa7491243f2f544c21b84fa97f")),
            1496060819, // * UNIX timestamp of last checkpoint block
            42052,   // * total number of transactions between genesis and last checkpoint
            //   (the tx=... number in the SetBestChain debug.log lines)
            100     // * estimated number of transactions per day after checkpoint
        };
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
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.nPreminedBlocks = 42000; // 42000 blocks equals the Premined amount
        consensus.nMajorityEnforceBlockUpgrade = 51;
        consensus.nMajorityRejectBlockOutdated = 75;
        consensus.nMajorityWindow = 100;
        /* IoP beta change - BIP30 being enforced since beginning */
        consensus.BIP34Height = -1;
        consensus.BIP34Hash = uint256();
        consensus.powLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1456790400; // March 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800; // May 1st, 2017

        // Deployment of SegWit (BIP141 and BIP143)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1462060800; // May 1st 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1493596800; // May 1st 2017

        // Miner White list params
		consensus.minerWhiteListActivationHeight = 110; //block height that activates the white list.
		consensus.minerWhiteListAdminPubKey.insert("03f331bdfe024cf106fa1dcedb8b78e084480fa665d91c50b61822d7830c9ea840"); //admin 1
		consensus.minerWhiteListAdminPubKey.insert("036144b1fd268a585380ac254df1819fdc369ecb5a73ba26960f13b0839124297f"); //admin 2
		consensus.minerWhiteListAdminPubKey.insert("033888a31bd3bfde32acf4a8f9f6c214b3c18b0144246bf4d2efd4a4a85bf19302"); //admin 3
		consensus.minerWhiteListAdminPubKey.insert("032c4f9d3bf27d2488e5c60c0285fc3ae983df56b554dc72044f07679291852e22"); //admin 4
		consensus.minerWhiteListAdminPubKey.insert("02d442d291a80da908cd8b349414f723d0ca539b9d306f68a3b0ce96e7747ff2ba"); //admin 5
		consensus.minerWhiteListAdminAddress.insert("uh2SKjE6R1uw3b5smZ8i1G8rDoQv458Lsj"); //admin 1
		consensus.minerWhiteListAdminAddress.insert("uWRctToxeWVQrDp7cf8d9S6WsShPEqZYi5"); //admin 2
		consensus.minerWhiteListAdminAddress.insert("uawKNKdCq8nMzHubWMd4mUZUEE8nXRuxmR"); //admin 3
		consensus.minerWhiteListAdminAddress.insert("udnHN51uCoNYJuveLFFweTCArWJq1Ajb4c"); //admin 4
		consensus.minerWhiteListAdminAddress.insert("uTUusyZae9W1AC7XeBdCj7aURsetYUwfQ3"); //admin 5
		consensus.minerWhiteListMinAdminConsensus = 3; // 3 admin must generate transaction to add or remove miners
		consensus.minerCapSystemChangeHeight=7800;
		// Voting System Params
		consensus.ccBlockStartAdditionalHeight = 10;

        pchMessageStart[0] = 0xb1;
		pchMessageStart[1] = 0xfc;
		pchMessageStart[2] = 0x50;
		pchMessageStart[3] = 0xb3;
		nDefaultPort = 7475;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1463452342, 3335213172, 0x1d00ffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x000000006f2bb863230cda4f4fbee520314077e599a90b9c6072ea2018d7f3a3"));
        assert(genesis.hashMerkleRoot == uint256S("0x951bc46d2da95047fb4a2c0f9b7d6e45591c3ffb49ab2fdffb8e96ef2b8f2be1"));

        vFixedSeeds.clear();
		vSeeds.clear();
		vSeeds.push_back(CDNSSeedData("Seed server 4", "ham4.fermat.cloud"));
		vSeeds.push_back(CDNSSeedData("Seed server 5", "ham5.fermat.cloud"));
		vSeeds.push_back(CDNSSeedData("Seed server 6", "ham6.fermat.cloud"));
		vSeeds.push_back(CDNSSeedData("Seed server 7", "ham7.fermat.cloud"));
		vSeeds.push_back(CDNSSeedData("Seed server 8", "ham8.fermat.cloud"));
    vSeeds.push_back(CDNSSeedData("dejf DNS Seed", "iop-ts-test.amberloom.com"));

		base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,130);
		base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,49);
		base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,76);
		base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0xBB)(0x8F)(0x48)(0x52).convert_to_container<std::vector<unsigned char> >();
		base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x2B)(0x7F)(0xA4)(0x2A).convert_to_container<std::vector<unsigned char> >();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
			( 0, uint256S("0x000000006f2bb863230cda4f4fbee520314077e599a90b9c6072ea2018d7f3a3")),
			1463452342,
			0,
			300
        };

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
        consensus.nSubsidyHalvingInterval = 7000;
        consensus.nPreminedBlocks = 500;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.BIP34Height = -1; // BIP34 has not necessarily activated on regtest
        consensus.BIP34Hash = uint256();
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 999999999999ULL;

        // Miner White list params
		consensus.minerWhiteListActivationHeight = 5000; //block height that activates the white list.
		consensus.minerWhiteListAdminPubKey.insert("03760087582c5e225aea2a6781f4df8b12d7124e4f039fbd3e6d053fdcaacc60eb"); //admin 1
		consensus.minerWhiteListAdminPubKey.insert("034c5063ce799cc05de6c4aaa7293cef44aa5ae0bf64efc4a8bf42b3a1d032c17e"); //admin 2
		consensus.minerWhiteListAdminPubKey.insert("0256aa483612a87b892e08e19bac862c75981110ebc69739e2364f0c59b11b45e4"); //admin 3
		consensus.minerWhiteListAdminAddress.insert("ucNbB1K3BaHWY5tXrWiyWn11QB51vPDuVE"); //admin 1
		consensus.minerWhiteListAdminAddress.insert("uR9oF2hoCyTtxE9YqjcNMuggJFWKxQYo4M"); //admin 2
		consensus.minerWhiteListAdminAddress.insert("ud2yj4ncSAjaCfEV37PXSReLpfAn8LBWd2"); //admin 3
		consensus.minerWhiteListMinAdminConsensus = 2; // 1 admin must generate transaction to add or remove miners
		consensus.minerCapSystemChangeHeight = 5200;
		// Voting System Params
		consensus.ccBlockStartAdditionalHeight = 10;

        pchMessageStart[0] = 0x35;
		pchMessageStart[1] = 0xb2;
		pchMessageStart[2] = 0xcc;
		pchMessageStart[3] = 0x9e;
		nDefaultPort = 14877;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1463452384, 2528424328, 0x207fffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x13ac5baa4b3656eec3ae4ab24b44ae602b9d1e549d9f1f238c1bfce54571b8b5"));
        assert(genesis.hashMerkleRoot == uint256S("0x951bc46d2da95047fb4a2c0f9b7d6e45591c3ffb49ab2fdffb8e96ef2b8f2be1"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;

        checkpointData = (CCheckpointData){
            boost::assign::map_list_of
            ( 0, uint256S("0x13ac5baa4b3656eec3ae4ab24b44ae602b9d1e549d9f1f238c1bfce54571b8b5")),
            0,
            0,
            0
        };
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,130);
		base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,49);
		base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,76);
		base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0xBB)(0x8F)(0x48)(0x52).convert_to_container<std::vector<unsigned char> >();
		base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x2B)(0x7F)(0xA4)(0x2A).convert_to_container<std::vector<unsigned char> >();
    }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
            return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
            return testNetParams;
    else if (chain == CBaseChainParams::REGTEST)
            return regTestParams;
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}
 
