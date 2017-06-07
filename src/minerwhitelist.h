/*
 * minerwhitelist.h
 *
 *  Created on: Aug 26, 2016
 *      Author: rodrigo
 */

#ifndef MINERWHITELIST_H_
#define MINERWHITELIST_H_


#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>
#include <boost/signals2/signal.hpp>


typedef std::vector<std::string> minerwhitelist_v;

/** Access to the miner list minerWhiteList.dat */
class CMinerWhiteList {
private:
	boost::filesystem::path pathMinerWhiteList;

	template<typename Out>
	void split(const std::string &s, char delim, Out result);

public:
	// possible actions enum
	enum WhiteListAction {ADD_MINER, REMOVE_MINER, ENABLE_CAP, DISABLE_CAP, NONE};

	CMinerWhiteList();
	bool Write(minerwhitelist_v minerwhitelist);
	minerwhitelist_v Read();
	bool Exist(std::string pkey);

	//splits the passed string into a vector. Used to split the miner whitelist into [pkey],[#Add],[#Remove] form
	std::vector<std::string> split(const std::string &s, char delim);

	// finds in the miner whitelist databases the key with the number of transaction, both Add and Remove types, for the passed address
        std::vector<std::string> ReadOne(const std::string &minerAddress);

	/**
	 * returns true if the mier whitelist control is active or not.
	 */
	static bool isEnabled(const int currentHeight);
};

#endif /* MINERWHITELIST_H_ */
