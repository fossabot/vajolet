/*
	This file is part of Vajolet.
	

    Vajolet is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Vajolet is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Vajolet.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef TBTABLE_H
#define TBTABLE_H

#include <cassert>
#include <mutex>          // std::call_once, std::once_flag
#include <string>

#include "hashKey.h"
#include "tbpairs.h"
#include "tbpairs.h"
#include "tbtypes.h"
#include "tbfile.h"
#include "tSquare.h"



// struct TBTable contains indexing information to access the corresponding TBFile.
// There are 2 types of TBTable, corresponding to a WDL or a DTZ file. TBTable
// is populated at init time but the nested PairsData records are populated at
// first access, when the corresponding file is memory mapped.
class TBTable {
private:
	std::string _endgame;
	TBFile _file;
	HashKey _key;
	HashKey _key2;
	unsigned int _pieceCount;
	unsigned int _pawnCount[2]; // [Lead color / other color]
	bool _hasPawns;
	bool _hasUniquePieces;
	const unsigned int _sides;
	PairsData _items[2][4]; // [wtm / btm][FILE_A..FILE_D or 0]
	
	std::once_flag _mappedFlag;
	void _mapFile();
	std::string _getCompleteFileName() const;
public:
	// todo change to reference if *_getPairsData(i, f) = PairsData(); is removed
	PairsData* _getPairsData(const unsigned int stm, const tFile f);
	
	
protected:
	const std::string _extension;
	explicit TBTable(const std::string& code, std::string ext, unsigned int sides);
	explicit TBTable(const TBTable& other, std::string ext, unsigned int sides);
	virtual ~TBTable() {}
    TBTable(TBTable&& other) noexcept =  delete;
    TBTable& operator=(const TBTable& other) =  delete;
    TBTable& operator=(TBTable&& other) noexcept =  delete;
public:
	
	// todo readd?
	//typedef typename std::conditional<Type == WDL, WDLScore, int>::type Ret;
	
	//uint8_t* map;
	

	const HashKey& getKey() const { return _key; };
	const HashKey& getKey2() const { return _key2; };
	unsigned int getPieceCount() const { return _pieceCount; };
	unsigned int getPawnCount(unsigned int x) const { assert(x<2); return _pawnCount[x];};
	bool hasPawns() const { return _hasPawns; };
	bool hasUniquePieces() const { return _hasUniquePieces; };
	void mapFile();
	virtual TBType getType() const = 0;
	std::string getEndGame() const;
	bool hasPawnOnBothSides() const;
	
	
};

#endif