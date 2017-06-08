/*
 * minerwhitelist.cpp
 *
 *  Created on: Aug 26, 2016
 *      Author: rodrigo
 */

#include "hash.h"
#include "clientversion.h"
#include "chainparams.h"
#include "consensus/consensus.h"
#include "minerwhitelist.h"
#include "random.h"
#include "serialize.h"
#include "stdio.h"
#include "streams.h"
#include "tinyformat.h"
#include "util.h"
#include <iostream>
#include <fstream>


#include <boost/filesystem.hpp>


using namespace std;

/**
 * IoP changes by Rodrigo Acosta
 */
//
// MinerWhiteList
//
CMinerWhiteList::CMinerWhiteList() {
	pathMinerWhiteList = GetDataDir() / "minerwhitelist.dat";
}

/**
 * new functionality version 4.1.0
 * Private function that splits the passed string using the passed character to delimit the string.
 * Returns a new vector whit all the splits.
 */
template<typename Out>
void CMinerWhiteList::split(const std::string &s, char delim, Out result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

/**
 * new functionality version 4.1.0
 * public function that returns a string vector of the splited string
 */
std::vector<std::string> CMinerWhiteList::split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

bool CMinerWhiteList::Write(minerwhitelist_v minerwhitelist) {
	/**
	 * I'm writting plain text and not locking file or using temp files.
	 * My original implementation using bitcoin code is not working properly. There is room for improvemente here.
	 */
	try{
		//delete duplicates before storing
		std::sort(minerwhitelist.begin(), minerwhitelist.end());
		minerwhitelist.erase(std::unique(minerwhitelist.begin(), minerwhitelist.end()), minerwhitelist.end());

		ofstream file(pathMinerWhiteList.string().c_str());
		for (unsigned int i=0; i < minerwhitelist.size();i++){
			file << minerwhitelist[i] << endl;
		}
		file.close();
		return true;
	} catch (const std::exception& e){
		return error("%s: Serialize or I/O error - %s", __func__, e.what());
	}
}


minerwhitelist_v CMinerWhiteList::Read() {
	/**
	 * I'm reading plain data. My original copy from bitcoin code which added the hash of the current data to
	 * verify if data was changed or corrupted is not working right. There is space to improve this.
	 */
	std::vector<std::string> pkeys;
	try{
		std::ifstream file(pathMinerWhiteList.string().c_str());

		std::string pkey;
		while (file >> pkey) {
			// new functionality version 4.1.0
			//if this is an old key witch doesn't inform the number of add and remove transactions involved
			// (form should be [address],[#add],[#remove], then we reset it with 1 positive vote.
			if (split(pkey,',').size() != 3)
				pkey = pkey + ",1" + ",0";

			pkeys.push_back(pkey);
		}

		file.close();
	} catch (const std::exception& e){
		//return error("%s: Serialize or I/O error - %s", __func__, e.what());
	}

	// we are always adding the admin white list addresses as part of the database.
	std::set<std::string> minerWhiteListAdminAddress = Params().GetConsensus().minerWhiteListAdminAddress;
	set<string>::iterator it;
	for (it = minerWhiteListAdminAddress.begin(); it != minerWhiteListAdminAddress.end(); it++){
		if (std::find(pkeys.begin(), pkeys.end(), *it) == pkeys.end())
			pkeys.push_back(*it);
	}

	return pkeys;
}


bool CMinerWhiteList::Exist(std::string pkey){
	std::vector<string> pkeys;
	pkeys = Read();

    return (std::find(pkeys.begin(), pkeys.end(), pkey) != pkeys.end());
}

/**
 * returns true if the miner whitelist control is enabled or not.
 */
bool CMinerWhiteList::isEnabled(const int currentHeight){
	if (currentHeight >= Params().GetConsensus().minerWhiteListActivationHeight)
		return true;
	else
		return false;
}

// finds in the miner whitelist databases the key with the number of transaction, both Add and Remove types, for the passed Primary key.
std::vector<std::string> CMinerWhiteList::ReadOne(const std::string &minerAddress){    
	std::vector<std::string> value;
        try{
                std::ifstream file(pathMinerWhiteList.string().c_str());

                std::string pkey;
                while (file >> pkey) {
                        // new functionality version 4.1.0
                        //if this is an old key witch doesn't inform the number of add and remove transactions involved
                        // (form should be [address],[#add],[#remove], then we reset it with 1 positive vote.
                        if (split(pkey,',').size() == 3 )
                                pkey = pkey + ",1" + ",0";

			value = split(pkey,',');
                        if (value.at(0).compare(minerAddress) == 0){
				file.close();
				return value;
			}
               }
                file.close();
        } catch (const std::exception& e){
                //return error("%s: Serialize or I/O error - %s", __func__, e.what());
        }

	return value;
}

std::string CMinerWhiteList::vectorToString(const std::vector<std::string> &input){
	std::ostringstream oss;

	  if (!input.empty())
	  {
	    // Convert all but the last element to avoid a trailing ","
	    std::copy(input.begin(), input.end()-1,  std::ostream_iterator<std::string>(oss, ","));

	    // Now add the last element with no delimiter
	    oss << input.back();
	  }

	 return oss.str();
}
