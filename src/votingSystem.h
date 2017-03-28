/*
 * votingSystem.h
 *
 *  Created on: Nov 14, 2016
 *      Author: rodrigo
 */

#ifndef VOTINGSYSTEM_H_
#define VOTINGSYSTEM_H_

#include "amount.h"
#include "utilstrencodings.h"
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include "base58.h"
#include "clientversion.h"
#include "chain.h"
#include "chainparams.h"
#include "main.h"
#include "primitives/block.h"
#include "primitives/transaction.h"

#include "streams.h"
#include "stdio.h"
#include <fstream>
#include <iostream>
#include <string>

#include <rpc/blockchain.cpp>
#include <minerCap.cpp>

#include "votingSystem.h"

#include "util.h"
#include "uint256.h"
#include "utilstrencodings.h"



#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

class CCBeneficiary {
private:
	CIoPAddress address;
	CAmount amount;
public:
	// constructs a new contribution contract
	CCBeneficiary(CIoPAddress address, CAmount amount){
		this->address = address;
		this->amount = amount;
	}

	CAmount getAmount(){
		return this->amount;
	}

	CIoPAddress getAddress(){
		return this->address;
	}
};


class ContributionContract {
private:
	boost::filesystem::path pathContributionContract;

public:
	std::string version;
	int blockStart;
	int blockEnd;
	int blockPending;
	int genesisBlockHeight;
	CTransaction genesisTx;
	uint256 genesisTxHash;
	std::vector<CAmount> votes;
	std::string opReturn;
	int totalVoteYesCount;
	int activeVoteYesCount;

	// possible ContributionContract states
	enum CCState {
		SUBMITTED, 				// Transaction confirmed on blockchain. No votes yet.
		APPROVED, 				// YES > NO. Current height  < (BlockStart + 1000 blocks).
		NOT_APPROVED, 			// NO > YES. Current height  < (BlockStart + 1000 blocks).
		QUEUED_FOR_EXECUTION,	// YES > NO. Current height  < (BlockStart + 1000 blocks).
		IN_EXECUTION,			// YES > NO. Current height  > BlockStart  and Current height  < BlockEnd
		QUEUED,					// YES > NO. Current height  > BlockStart  and Current height  < BlockEnd && Reward + Subsidy > 1 IoP
		EXECUTION_CANCELLED, 	// NO > YES. Current height  > BlockStart  and Current height  < BlockEnd
		EXECUTED,				// YES > NO. Current height  > BlockEnd
		UNKNOWN};				// initial state

	std::vector<CCBeneficiary> beneficiaries;
	CAmount blockReward;
	CCState state;

	// constructs a new contribution contract
	ContributionContract(){
		// initialize values
		this->version = "";
		this->blockStart = 0;
		this->blockPending = 0;
		this->blockEnd = 0;
		this->genesisBlockHeight = 0;
		this->state = UNKNOWN;
		this->blockReward = 0;
		this->opReturn = "";
		this->totalVoteYesCount = 0;
		this->activeVoteYesCount = 0;
	}

	static std::string getState(CCState state){
		switch (state){
		case SUBMITTED: return "SUBMITTED";
		case APPROVED: return "APPROVED";
		case NOT_APPROVED: return "NOT_APPROVED";
		case QUEUED_FOR_EXECUTION: return "QUEUED_FOR_EXECUTION";
		case IN_EXECUTION: return "IN_EXECUTION";
		case QUEUED: return "QUEUED";
		case EXECUTION_CANCELLED: return "EXECUTION_CANCELLED";
		case EXECUTED: return "EXECUTED";
		default: return "UNKNOWN";
		}
	}

	/**
	 * Validates if the passed script is from a contribution contract.
	 */
	static bool isContributionContract(const CScript scriptPubKey){
		// 1st condition must be op_return
		if (scriptPubKey[0] != OP_RETURN)
			return false;

		// 2nd condition, first 2 bytes must be 0x4343
		CScript::const_iterator pc = scriptPubKey.begin();
		opcodetype opcode;
		std::vector<unsigned char> value;

		while (pc < scriptPubKey.end()){
			scriptPubKey.GetOp(pc, opcode, value);
		}

		// we get the OP_Return data into the string and validate is CC for Contribution Contract
		std::string opreturn = HexStr(value);

		if (opreturn.size() > 2){
			if (opreturn.substr(0, 4).compare("4343") == 0){
				return true;
			}

		}
		// not a valid CC
		return false;
	}

	/**
	 * Extracts and return the opreturn string from the scriptPubKey
	 */
	static bool getOPReturn(CScript scriptPubKey, std::string &out){
		// 1st condition must be op_return
			if (scriptPubKey[0] != OP_RETURN)
				return false;

			CScript::const_iterator pc = scriptPubKey.begin();
			opcodetype opcode;
			std::vector<unsigned char> value;

			while (pc < scriptPubKey.end()){
				scriptPubKey.GetOp(pc, opcode, value);
			}

			// we get the OP_Return data into the string
			out = HexStr(value);
			return true;
	}

	static int HexToInt(std::string hexstr){
		 int x;
		std::stringstream ss;
		ss << std::hex << hexstr;
		ss >> x;
		return x;
	}

	// gets a contribution contract from the passed Transaction
	static  bool getContributionContract(CTransaction genesisTx, ContributionContract &out){
		ContributionContract cc;

		// parse the op_return and return it.
		std::string value = "";

		CScript ccScript;
		int outputIndex = 0;
		for (unsigned int i = 0; i < genesisTx.vout.size(); i++){
			if (isContributionContract(genesisTx.vout[i].scriptPubKey)){
				getOPReturn(genesisTx.vout[i].scriptPubKey, value);
				outputIndex = i;
			}
		}

		if (value.size() == 0)
			return false;

		// sets the op_return string
		cc.opReturn = value;

		// get the version
		std::string textVersion(value.substr(4,4));


		cc.version = textVersion;

		// get the block start height
		std::string strBlockStart = value.substr(8,6);
		cc.blockStart = HexToInt(strBlockStart);


		// get the block end height
		std::string strBlockend = value.substr(14,4);
		cc.blockEnd = HexToInt(strBlockend);


		// get the block reward
		std::string strReward = value.substr(18,6);
		cc.blockReward= HexToInt(strReward);

		// the hash of the genesis transaction
		cc.genesisTxHash = genesisTx.GetHash();
		cc.genesisTx = genesisTx;

		//let's get the beneficiaries of this contract
		for (unsigned int x = outputIndex+1; x < genesisTx.vout.size(); x++){
			CScript redeemScript = genesisTx.vout[x].scriptPubKey;
			CTxDestination destinationAddress;
			ExtractDestination(redeemScript, destinationAddress);
			CIoPAddress address(destinationAddress);
			CCBeneficiary beneficiary = CCBeneficiary(address,genesisTx.vout[x].nValue);
			cc.beneficiaries.push_back(beneficiary);
		}

		out = cc;
		return true;
	}

	static bool getActiveContracts(int currentHeight ,std::vector<ContributionContract>& ccOut){
		std::vector<std::string> ccPointers;
		ccPointers = loadCCPointers();

		bool found = false;

		CAmount totalReward = 0;

		for (auto i : ccPointers){
			std::vector<std::string> strs;
			boost::split(strs, i, boost::is_any_of(","));


			//we are loading transaction stored on blocks burried under current height
			if (atoi(strs[0]) <= currentHeight){
				CTransaction ccGenesisTx;
				ccGenesisTx = loadCCGenesisTransaction(atoi(strs[0]), uint256S(strs[1]));
				if (!ccGenesisTx.IsNull() && ccGenesisTx.vin.size() > 0){
					BOOST_FOREACH(const CTxOut& out, ccGenesisTx.vout) {
						if (isContributionContract(out.scriptPubKey)){
							ContributionContract cc = ContributionContract();
							if (getContributionContract(ccGenesisTx, cc)){
								cc.genesisBlockHeight = atoi(strs[0]); //I set the block height
								if (cc.isValid()){
									if (cc.isActive(currentHeight)){
										totalReward = totalReward + cc.blockReward;
										//if we exceed the maximun allowed value we stop here.
										if (totalReward > COIN *1)
											return found;
										else {
											//we haven't reached the max queue, so let's include another CC
											cc.state = IN_EXECUTION;
											found = true;
											ccOut.push_back(cc);
										}
									}

								}
							}
						}
					}
				}
			}
		}
		return found;
	}


	// stores this contribution contract to disk.
	bool persist(int blockHeight, uint256 genesisTxHash){
		pathContributionContract = GetDataDir() / "cc.dat";

		std::vector<std::string> cc;
		cc = loadCCPointers();

		//won't persist it if already exists.
		for (auto i : cc){
			std::vector<std::string> strs;
			boost::split(strs, i, boost::is_any_of(","));

			if (genesisTxHash.Compare(uint256S(strs[1])) == 0)
				return false;
		}

		std::string height = std::to_string(blockHeight);
		cc.push_back(height + "," + genesisTxHash.ToString());

		try{
			std::ofstream file(pathContributionContract.string().c_str());
			for (unsigned int i=0; i < cc.size();i++){
				file << cc[i] << std::endl;
			}
			file.close();
			return true;
		} catch (const std::exception& e){
			return error("%s: Serialize or I/O error - %s", __func__, e.what());
		}
	}

	// ToString override.
	std::string ToString(){
		std::string output = "\nVersion: " + this->version + "\n";
		output = output  + "Block start: " + std::to_string(this->blockStart) + "\n";
		output = output  + "Block end: " + std::to_string(this->blockEnd) + "\n";
		output = output  + "Blocks pending: " + std::to_string(this->blockPending) + "\n";
		output = output  + "CC Start height: " + std::to_string(this->blockStart + this->genesisBlockHeight + Params().GetConsensus().ccBlockStartAdditionalHeight) + "\n";
		output = output  + "Block Reward: " + std::to_string(this->blockReward) + "\n";
		output = output  + "Genesis Tx: " + this->genesisTxHash.ToString() + "\n";
		output = output  + "Genesis block height: " + std::to_string(this->genesisBlockHeight) + "\n";
		output = output  + "OP Return: " + this->opReturn + "\n";

		for (CCBeneficiary ccb : this->beneficiaries){
			output = output  + " Beneficiary : " + ccb.getAddress().ToString() + " - " + std::to_string(ccb.getAmount()) + "\n";
		}
		return output;
	}

	static std::vector<std::string> loadCCPointers(){
		//sets the directory where the file will be saved.
		boost::filesystem::path pathContributionContract = GetDataDir() / "cc.dat";
		std::vector<std::string> cc;
				try{
					std::ifstream file(pathContributionContract.string().c_str());

					std::string pkey;
					while (file >> pkey) {
						cc.push_back(pkey);
					}

					file.close();
				} catch (const std::exception& e){
					//return error("%s: Serialize or I/O error - %s", __func__, e.what());
				}
				return cc;
	}

	// gets true if the contract is valid in all the rules.
		bool isValid(){

			// Version 1.1 checks
			if (isCCVersion11){
				// valid version is 11
				if (this->version.compare("0101") != 0){
					LogPrint("Invalid Contract", "Invalid version: %s\n", this->version);
					return false;
				}
			}
			// valid version is 10
			if (this->version.compare("0100") != 0){
				LogPrint("Invalid Contract", "Invalid version: %s\n", this->version);
				return false;
			}

			// we validate that this transaction freezes at least 1000 IoP
			if (this->genesisTx.vout[0].nValue < COIN * 1000){
				LogPrint("Invalid Contract", "Genesis transaction doesn't freeze 1000 IoPs\n");
				return false;
			}


			// can't pay more than 0.1 IoP
			if (this->blockReward > 10000000){ //COIN * 0.1
				LogPrint("Invalid Contract", "Contract reward is too high: %s\n", this->blockReward);
				return false;
			}


			// Block start is defined as current Height + 1000 + n, and can't be more than 6 months, or 11960 blocks.
			if (this->blockStart > 11960 || this->blockStart <= 0){
				LogPrint("Invalid Contract", "Invalid contract start block: %s\n", this->blockStart);
				return false;
			}


			// block end is defined as the amount of blocks that this contract will be executed and the reward included in.
			// Max Value is 120960 blocks-
			if (this->blockEnd > 120960 || this->blockEnd <= 0 ){
				LogPrint("Invalid Contract", "Invalid contract end block: %s\n", this->blockEnd);
				return false;
			}


			// sum of beneficiaries amount, must be equal to block reward.
			CAmount totalAmount = 0;
			BOOST_FOREACH(CCBeneficiary beneficiary, this->beneficiaries){
				totalAmount = totalAmount + beneficiary.getAmount();
			}
			if (totalAmount != this->blockReward){
				LogPrint("Invalid Contract", "Contract pays too much to beneficiaries: %s\n", totalAmount);
				return false;
			}

			// at this point the Contribution contract is valid.
			return true;

		}

		static bool getContributionContracts(int currentHeight, std::vector<ContributionContract>& ccOut){
				std::vector<std::string> ccPointers;
				ccPointers = loadCCPointers();

				bool found = false;

				for (auto i : ccPointers){
					std::vector<std::string> strs;
					boost::split(strs, i, boost::is_any_of(","));

					int ccBlockHeight = atoi(strs[0]);
					if ( ccBlockHeight<= currentHeight){
						CTransaction ccGenesisTx;
						ccGenesisTx = loadCCGenesisTransaction(atoi(strs[0]), uint256S(strs[1]));
						if (ccGenesisTx.vin.size() > 0){
							BOOST_FOREACH(const CTxOut& out, ccGenesisTx.vout) {
								if (isContributionContract(out.scriptPubKey)){
									ContributionContract cc = ContributionContract();
									if (getContributionContract(ccGenesisTx, cc)){
										cc.genesisBlockHeight = atoi(strs[0]); //I set the block height
										if (cc.isValid()){
												cc.votes = cc.getCCVotes(currentHeight);
												cc.blockPending = cc.getPendingBlocks(currentHeight);
												cc.state = cc.getCCState(currentHeight);

												found = true;
												ccOut.push_back(cc);
										}
									}
								}
							}
						}
					}
				}

				return found;
			}

		static bool getContributionContractsByHeight(int startHeight,int currentHeight, std::vector<ContributionContract>& ccOut){
						std::vector<std::string> ccPointers;
						ccPointers = loadCCPointers();

						bool found = false;

						for (auto i : ccPointers){
							std::vector<std::string> strs;
							boost::split(strs, i, boost::is_any_of(","));

							int ccBlockHeight = atoi(strs[0]);
							if ( ccBlockHeight<= currentHeight && ccBlockHeight>=startHeight ){
								CTransaction ccGenesisTx;
								ccGenesisTx = loadCCGenesisTransaction(atoi(strs[0]), uint256S(strs[1]));
								if (ccGenesisTx.vin.size() > 0){
									BOOST_FOREACH(const CTxOut& out, ccGenesisTx.vout) {
										if (isContributionContract(out.scriptPubKey)){
											ContributionContract cc = ContributionContract();
											if (getContributionContract(ccGenesisTx, cc)){
												cc.genesisBlockHeight = atoi(strs[0]); //I set the block height
												if (cc.isValid()){
														cc.votes = cc.getCCVotes(currentHeight);
														cc.blockPending = cc.getPendingBlocks(currentHeight);
														cc.state = cc.getCCState(currentHeight);

														found = true;
														ccOut.push_back(cc);
												}
											}
										}
									}
								}
							}
						}

						return found;
					}

		CCState getCCState(int currentHeight){
			//if not valid, then it won't be executed.
			if (!isValid())
				return EXECUTION_CANCELLED;

			// 1000 IoPs that where used to create the CC must still be locked, which means that there must
			// not be another transaction that uses that input in the Active period.
			UniValue utxo(UniValue::VOBJ);
			UniValue ret(UniValue::VOBJ);
			utxo.push_back(Pair("tx", this->genesisTxHash.ToString()));
			utxo.push_back(Pair("n", 0));
			ret = gettxout(utxo, false);
			// if I didn't get a result, then no utxo and the locked coins of the CC are already spent.

			if (ret.isNull()){
				LogPrint("Inactive Contract", "Contract %s is not active. No UTXO\n", this->genesisTxHash.ToString());
				return EXECUTION_CANCELLED;;
			}

			// possible states are SUBMITTED, APPROVED, NOT APPROVED depending on the votes count
			if (currentHeight < this->blockStart + this->genesisBlockHeight){
				std::vector<CAmount> votes;
				votes.push_back(0);
				votes.push_back(0);


				votes = getCCVotes(currentHeight);
				if (votes[0] == 0 && votes[1] == 0){
					return SUBMITTED;
				}

				if (votes[0] > votes[1]){
					return APPROVED;
				}

				if (votes[0] <= votes[1]){
					return NOT_APPROVED;
				}

			}


			// possible states are NOT_APPROVED and QUEUED_FOR_EXECUTION depending on the votes count
			if (currentHeight >= this->blockStart + this->genesisBlockHeight  &&
					currentHeight < this->blockStart + this->genesisBlockHeight + Params().GetConsensus().ccBlockStartAdditionalHeight){
				std::vector<CAmount> votes;
				votes.push_back(0);
				votes.push_back(0);

				votes = getCCVotes(currentHeight);
				if (votes[0] > votes[1]){
					return QUEUED_FOR_EXECUTION;
				}

				if (votes[0] == 0 && votes[1] == 0){
					return EXECUTION_CANCELLED;
				}

				if (votes[0] <= votes[1]){
					return EXECUTION_CANCELLED;
				}

			}

			// possible states are NOT_APPROVED, IN_EXECUTION, QUEUED and EXECUTION_CANCELLED depending on the votes count
			if (currentHeight >= this->blockStart + this->genesisBlockHeight + Params().GetConsensus().ccBlockStartAdditionalHeight &&
					getPendingBlocks(currentHeight) > 0){
				std::vector<CAmount> votes;
				votes.push_back(0);
				votes.push_back(0);

				votes = getCCVotes(currentHeight);

				if (votes[0] == 0 && votes[1] == 0){
					return EXECUTION_CANCELLED;
				}


				if (votes[0] > votes[1]){
					if (!isActive(currentHeight))
						return EXECUTION_CANCELLED;

					// if it is not active, then is queued.
					std::vector<ContributionContract> vcc;
					ContributionContract::getActiveContracts(currentHeight, vcc);
					for (ContributionContract& cc : vcc) {
					    if (cc.genesisTxHash.Compare(this->genesisTxHash) == 0)
					    	return IN_EXECUTION;
					}

					return QUEUED;
				}

				if (votes[0] <= votes[1]){
					return EXECUTION_CANCELLED;
				}

			}

			// If no pending executions are available, then it was executed.
			if (getPendingBlocks(currentHeight) == 0)
				return EXECUTED;

			return UNKNOWN;
		}


		// a CC is Active if it is between the blocks period and the amount of YES votes is bigger than the NO votes.
		bool isActive(int currentHeight){
			// first condition to be active: current height must be greater that blockstart
			if (currentHeight < this->blockStart + this->genesisBlockHeight + Params().GetConsensus().ccBlockStartAdditionalHeight){
				return false;
			}

			// in rule of CC 1.1, we required a maximun of 5 different positive transaction votes
			if (this->activeVoteYesCount < 5 && this->version.compare("1010") == 0){
				LogPrint("Inactive Contract", "Not enought positive voting transactions for this contract. %s is not active. Only %s voteYes transaction\n", this->genesisTxHash.ToString(), this->activeVoteYesCount);
				return false;
			}


			// 1000 IoPs that where used to create the CC must still be locked, which means that there must
			// not be another transaction that uses that input in the Active period.
			UniValue utxo(UniValue::VOBJ);
			UniValue ret(UniValue::VOBJ);
			utxo.push_back(Pair("tx", this->genesisTxHash.ToString()));
			utxo.push_back(Pair("n", 0));
			ret = gettxout(utxo, false);
			// if I didn't get a result, then no utxo and the locked coins of the CC are already spent.

			if (ret.isNull()){
				LogPrint("Inactive Contract", "Contract %s is not active. No UTXO\n", this->genesisTxHash.ToString());
				return false;
			}

			// must have pending blocks to be included in
			this->blockPending = getPendingBlocks(currentHeight);

			if (this->blockPending == 0){
				return false;
			}

			// The amount of YES votes must be greater than NO votes.
			// I need to search all the Votes transaction since the genesis block.
			std::vector<CAmount> votes;
			votes.push_back(0);
			votes.push_back(0);

			votes = getCCVotes(currentHeight);

			if (votes[0] <= votes[1] || (votes[0] == 0 && votes[1] == 0)){
				return false;
			}

			return true;
		}

		/**
		 * Gets the amount of pending blocks for this CC.
		 * It checks all the coinbase transactions generated since the start of the execution of this CC
		 * and finds for a match in the beneficiaries. Each match is considered an execution
		 */
		int getPendingBlocks(int currentHeight){
			// before counting the blocks we make sure that the CC is supposed to be active and ready
			// to be executed.
			if (currentHeight < this->blockStart + this->genesisBlockHeight + Params().GetConsensus().ccBlockStartAdditionalHeight)
				return this->blockEnd;

			int executions = 0;

			//the contract is within the running window. Let's count the matches
			for (int i = this->blockStart + this->genesisBlockHeight + Params().GetConsensus().ccBlockStartAdditionalHeight; i<currentHeight+1; i++){
				// boolean vector initialized to false.
				std::vector<bool> matches (this->beneficiaries.size(), false);

				CBlockIndex* blockIndex = chainActive[i];
				if (blockIndex != NULL){
					CBlock block;


					if (ReadBlockFromDisk(block, blockIndex, Params().GetConsensus())){
						CTransaction cb = block.vtx[0];
						BOOST_FOREACH(CTxOut out, cb.vout){
							for (int x=0; x<matches.size();x++){
								if (out.nValue == this->beneficiaries[x].getAmount()){ //we have a match in the amount
									CScript redeemScript = out.scriptPubKey;
									CTxDestination destinationAddress;
									ExtractDestination(redeemScript, destinationAddress);
									CIoPAddress address(destinationAddress);

									// we have a match in the address too.
									if (address.CompareTo(this->beneficiaries[x].getAddress()) == 0)
										matches[x] = true;
								}
							}
						}

					}

				}
				// if all validations are true, then I found a coinbase that delivers rewards to this CC.
				if (std::all_of(std::begin(matches), std::end(matches), [](bool i) { return i;}))
					executions++;
			}

			this->blockPending = this->blockEnd - executions;
			return this->blockPending;
		}

		// gets the total numbers of valid votes for the CC.
		// position 0 are YES votes, Position 1 are NO votes
		std::vector<CAmount> getCCVotes(int currentHeight){
			std::vector<CAmount> votes;
			votes.push_back(0);
			votes.push_back(0);

			for (int i = this->genesisBlockHeight; i<currentHeight+1; i++){
				CBlockIndex* blockIndex = chainActive[i];
				if (blockIndex == NULL)
					return votes;

				CBlock block;

				if (ReadBlockFromDisk(block, blockIndex, Params().GetConsensus())){
					BOOST_FOREACH(CTransaction tx, block.vtx){
						if (i <= this->blockStart + this->genesisBlockHeight)
							getVote(tx, votes, true); // if the CC is not yet active, then count positive votes
						else
							if (i <= this->blockStart + this->genesisBlockHeight + Params().GetConsensus().ccBlockStartAdditionalHeight)
								getVote(tx, votes, false);
					}
				}

			}
			return votes;
		}



		bool getVote(CTransaction tx, std::vector<CAmount> &votes, bool includePositive){
			// can't be coinbase
			if (tx.IsCoinBase())
				return false;

			// first output is vote power
			CAmount amount = tx.vout[0].nValue;

			// new rule on version 1.1. "Fee" must go to a trash address, so we need at least three outputs for a voting transaction.
			// output 0: freeze output of 100 or 25 IoPs
			// putput 1: "fee" output to trash address
			// output 2: op_return
			if (this->version.compare("1010") == 0){
				if (tx.vout.size() < 3)
					return false;
			} else{
			// In version 1.0, fee must be at least 1% of the vote power.
				CAmount fee = getVoteFee(tx);
				if (fee < amount / 100)
					return false;
			}


			// boolean used to store if the voting transaction is utxo or not.
			// We are including non utxo transactions into the count because CC 1.1 introduces the need to invalidate a contract
			// after 50% of the positive votes have been withdrawn. By calculating non utxo transactions, we can find out the total number of votes
			// and how many votes are still active.
			bool isUtxo = false;

			// will get the op_return data. Depending on the contract version, output will be index 1 or 2
			CTxOut output;
			if (this->version.compare("1000")== 0)
				output = tx.vout[0];
			else
				output = tx.vout[1];

			// we didn't get the op_Return output so this is not a valid voting transaction
			if (output.scriptPubKey[0] != OP_RETURN)
				return false;

			// Now we are going to parse the op_return data
			CScript::const_iterator pc = output.scriptPubKey.begin();
			opcodetype opcode;
			std::vector<unsigned char> value;

			while (pc < output.scriptPubKey.end()){
				output.scriptPubKey.GetOp(pc, opcode, value);
			}
			std::string opreturn = HexStr(value);

			// We have the op_return data, we are only continuing if basic structure is correct.
			if (opreturn.size() != 72 && opreturn.substr(0, 6).compare("564f54") != 0)
				return false;

			//referenced transaction must be the same as Contribution Contract genesis hash
			if (opreturn.substr(8, 64).compare(this->genesisTxHash.ToString()) != 0)
				return false;

			// vote power must be locked, meaning the output must in an utxo
			UniValue utxo(UniValue::VOBJ);
			UniValue ret(UniValue::VOBJ);
			utxo.push_back(Pair("tx", tx.GetHash().ToString()));
			utxo.push_back(Pair("n", 0));
			ret = gettxout(utxo, false);

			// if I didn't get a result, then no utxo and the locked coins of the Vote transaction are already spent.
			if (ret.isNull())
				isUtxo = false;
			else
				isUtxo = true;

			// For CC version 1.1, we are forcing the voter to send the "voting fee" of 1% of the voting power to a vanity address.
			// At this point, we can validate that to consider this transaction valid.
			if (this->version.compare("0101") == 0){
				// Output index 1, must be 1% of the vote power.
				if (tx.vout[1].nValue < (amount / 100))
					return false;

				//logica para obtener el address y compararla con el vanity address
			}

			// we focues on VoteYes now
			if (opreturn.substr(6, 2).compare("01") == 0  && includePositive){
				// we validate version CC 1.1 specific rules
				if (this->version.compare("0101") == 0){
					// To vote positive, you must freeze at least 100 IoP
					if (amount < (100 * COIN))
						return false;
				}

				// the voteYes is valid at this point, it may be active or not. So we already increase the total vote counter.
				++this->totalVoteYesCount;

				// if the voting transaction is still an utxo, we increase the voting counter successfully.
				if (isUtxo){
					votes[0] = votes[0] + amount;
					++this->activeVoteYesCount;
					return true;
				}
			}

			// we focus on the voteNO now.
			if (opreturn.substr(6, 2).compare("00") == 0){
				// if the VoteNo was withdrawn, we can ignore it.
				if (!isUtxo)
					return false;

				// we focus in all CC 1.1 version
				if (this->version.compare("0101") == 0){
					// voting holder must freeze 20 IoPS
					if (amount < COIN * 20)
						return false;
				}

				votes[1] = votes[1] + amount * 5; //negative votes weight x5
				return true;
			}
			// in case the transaction is either VoteYes or VoteNo, we ignore it.
			return false;
		}



		/**
		 * Gets the fee of the Voting Transaction. This is necessary to make sure the voters are complying the rules
		 * of fee. Current rule for CC 1.0 is 1% fee of the vote power.
		 */
		CAmount getVoteFee(const CTransaction voteTransaction){
			CAmount inputsAmount = 0 * COIN;
			// loop vote transaction inputs and search for outpoints transactions in the mempool.
			for (unsigned int i = 0; i < voteTransaction.vin.size(); i++) {
				std::shared_ptr<const CTransaction> ptx = mempool.get(voteTransaction.vin[i].prevout.hash);
				CTransaction inputTransaction = *ptx;
				inputsAmount = inputsAmount + inputTransaction.GetValueOut();
			}

			return inputsAmount;
		}


		/**
		 * gets the genesis Transaction of the contribution Contract by getting the block at the specified id
		 * and iterating transaction within the block.
		 */
		static CTransaction loadCCGenesisTransaction(int blockHeight, uint256 txHash){
			CTransaction txOut;
			CBlockIndex* blockIndex = chainActive[blockHeight];

			// if the index passed is not valid, no CC genesis transaction available
			if (blockIndex == NULL)
				return txOut;

			CBlock block;

			if (ReadBlockFromDisk(block, blockIndex, Params().GetConsensus())){
				BOOST_FOREACH(const CTransaction& tx, block.vtx){
					if (tx.GetHash().Compare(txHash) == 0)
					return tx;
				}
			}


			// block or transaction does not exists.
			return txOut;
		}

};

#endif /* VOTINGSYSTEM_H_ */
