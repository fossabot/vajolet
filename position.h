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
#ifndef POSITION_H_
#define POSITION_H_

#include <array>
#include <cstdint>
#include <map>
#include <unordered_map>
#include <vector>

#include "bitBoardIndex.h"
#include "data.h"
#include "hashKeys.h"
#include "move.h"
#include "tables.h"
#include "score.h"
#include "vajolet.h"

typedef enum
{
	white = 0,
	black = 1
}Color;

//---------------------------------------------------
//	class
//---------------------------------------------------



class Position
{
public:

	//--------------------------------------------------------
	// enums
	//--------------------------------------------------------
	enum eNextMove	// color turn. ( it's also used as offset to access bitmaps by index)
	{
		whiteTurn = 0,
		blackTurn=blackKing-whiteKing
	};

	enum eCastle	// castleRights
	{

		wCastleOO  = 1,
		wCastleOOO = 2,
		bCastleOO  = 4,
		bCastleOOO = 8,
		castleOO = wCastleOO,
		castleOOO = wCastleOOO
	};
	

	//--------------------------------------------------------
	// struct
	//--------------------------------------------------------
	/*! \brief define the state of the board
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	struct state
	{

	public:
		uint64_t key,		/*!<  hashkey identifying the position*/
			pawnKey,	/*!<  hashkey identifying the pawn formation*/
			materialKey;/*!<  hashkey identifying the material signature*/

		simdScore nonPawnMaterial; /*!< four score used for white/black opening/endgame non pawn material sum*/

		eNextMove nextMove; /*!< who is the active player*/

		tSquare epSquare;	/*!<  en passant square*/

		unsigned int fiftyMoveCnt,	/*!<  50 move count used for draw rule*/
			pliesFromNull;	/*!<  plies from null move*/

		bitboardIndex capturedPiece; /*!<  index of the captured piece for unmakeMove*/
		simdScore material;
		bitMap checkingSquares[lastBitboard]; /*!< squares of the board from where a king can be checked*/
		bitMap hiddenCheckersCandidate;	/*!< pieces who can make a discover check moving*/
		bitMap pinnedPieces;	/*!< pinned pieces*/


		state(){}

		static eCastle calcCastleRight( const eCastle cr, const Color c )
		{
			return eCastle( cr << (2 * c ) );
		}

		inline bool hasCastleRight( const eCastle cr )const
		{
			return _castleRights & cr;
		};

		inline bool hasCastleRight( const eCastle cr, const Color c )const
		{
			return _castleRights & calcCastleRight( cr, c );
		};

		inline const eCastle& getCastleRights() const
		{
			return _castleRights;
		}

		inline bool hasCastleRights() const
		{
			return _castleRights;
		}

		inline void clearCastleRight()
		{
			_castleRights = (eCastle)0;
		}

		inline void clearCastleRight( const eCastle c)
		{
			_castleRights = (eCastle)(_castleRights & ( ~c ) );
		}

		inline void setCastleRight( const eCastle c)
		{
			_castleRights = (eCastle)( _castleRights | c );
		}

		inline const Move& getCurrentMove() const
		{
			return _currentMove;
		}

		inline void setCurrentMove( const Move & m)
		{
			_currentMove = m;
		}

		inline bool isInCheck(void) const
		{
			return _checkers;
		}

		inline bool isInDoubleCheck(void) const
		{
			return moreThanOneBit(_checkers);
		}

		inline const bitMap& getCheckers(void) const
		{
			return _checkers;
		}

		inline void setCheckers( const bitMap & b)
		{
			_checkers = b;
		}

		inline void addCheckers( const bitMap & b)
		{
			_checkers |= b;
		}


	private:
		eCastle _castleRights; /*!<  actual castle rights*/
		Move _currentMove;
		bitMap _checkers;	/*!< checking pieces*/

	};

	//--------------------------------------------------------
	// public static methods
	//--------------------------------------------------------
	void static initMaterialKeys(void);
	static void initCastleRightsMask(void);
	static void initScoreValues(void);
	static void initPstValues(void);
	
	//--------------------------------------------------------
	// public methods
	//--------------------------------------------------------
	
	/*! \brief constructor
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	Position();
	Position(const Position& other);
	Position& operator=(const Position& other);
	

	inline bitMap getOccupationBitmap() const
	{
		return bitBoard[occupiedSquares];
	}
	inline bitMap getBitmap(const bitboardIndex in) const
	{
		return bitBoard[in];
	}
	inline unsigned int getPieceCount(const bitboardIndex in) const
	{
		return bitCnt(getBitmap(in));
	}

	inline bitboardIndex getPieceAt(const tSquare sq) const
	{
		return squares[sq];
	}
	inline tSquare getSquareOfThePiece(const bitboardIndex piece) const
	{
		return firstOne(getBitmap(piece));
	}
	inline bitMap getOurBitmap(const bitboardIndex piece)const { return Us[piece];}
	inline bitMap getTheirBitmap(const bitboardIndex piece)const { return Them[piece];}


	unsigned int getStateSize() const
	{
		return stateInfo.size();
	}
	
	void display(void) const;
	std::string getFen(void) const;
	std::string getSymmetricFen() const;

	void setupFromFen(const std::string& fenStr);
	void setup(const std::string& code, Color c);

	unsigned long long perft(unsigned int depth);
	unsigned long long divide(unsigned int depth);

	void doNullMove(void);
	void doMove(const Move &m);
	void undoMove();
	/*! \brief undo a null move
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	inline void undoNullMove(void)
	{
		--ply;
		removeState();
		std::swap( Us , Them );

#ifdef ENABLE_CHECK_CONSISTENCY
		checkPosConsistency(0);
#endif
	}


	template<bool trace>Score eval(void);
	bool isDraw(bool isPVline) const;


	bool moveGivesCheck(const Move& m)const ;
	bool moveGivesDoubleCheck(const Move& m)const;
	bool moveGivesSafeDoubleCheck(const Move& m)const;
	Score see(const Move& m) const;
	Score seeSign(const Move& m) const;
	
	uint64_t getKey(void) const
	{
		return getActualStateConst().key;
	}

	uint64_t getExclusionKey(void) const
	{
		return getActualStateConst().key^HashKeys::exclusion;
	}

	uint64_t getPawnKey(void) const
	{
		return getActualStateConst().pawnKey;
	}

	uint64_t getMaterialKey(void) const
	{
		return getActualStateConst().materialKey;
	}

	eNextMove getNextTurn(void) const
	{
		return getActualStateConst().nextMove;
	}

	tSquare getEpSquare(void) const
	{
		return getActualStateConst().epSquare;
	}

	eCastle getCastleRights(void) const
	{
		return getActualStateConst().getCastleRights();
	}

	unsigned int getPly(void) const
	{
		return ply;
	}

	bitboardIndex getCapturedPiece(void) const
	{
		return getActualStateConst().capturedPiece;
	}

	bool isInCheck(void) const
	{
		return getActualStateConst().getCheckers();
	}
	
	/*! \brief return a reference to the actual state
		\author Marco Belli
		\version 1.0
		\version 1.1 get rid of continuos malloc/free
		\date 21/11/2013
	*/
	inline const state& getActualStateConst(void)const
	{
		return stateInfo.back();
	}
	
	inline state& getActualState(void)
	{
		return stateInfo.back();
	}

	inline const state& getState(unsigned int n)const
	{
		return stateInfo[n];	
	}
	
	bool isMoveLegal(const Move &m) const;

	/*! \brief return a bitmap with all the attacker/defender of a given square
		\author Marco Belli
		\version 1.0
		\date 08/11/2013
	*/
	inline bitMap getAttackersTo(const tSquare to) const
	{
		return getAttackersTo(to, bitBoard[occupiedSquares]);
	}

	bitMap getAttackersTo(const tSquare to, const bitMap occupancy) const;


	/*! \brief return the mvvlva score
		\author Marco Belli
		\version 1.0
		\date 08/11/2013
	*/
	inline Score getMvvLvaScore(const Move & m) const
	{
		Score s = pieceValue[ squares[m.getTo()] ][0] + (squares[m.getFrom()]);
		if( m.isEnPassantMove() )
		{
			s += pieceValue[ whitePawns ][0];
		}
		return s;
	}

	/*! \brief return the gamephase for interpolation
		\author Marco Belli
		\version 1.0
		\date 08/11/2013
	*/
	inline unsigned int getGamePhase() const
	{
		const int opening = 700000;
		const int endgame = 100000;
		Score tot = getActualStateConst().nonPawnMaterial[0]+getActualStateConst().nonPawnMaterial[2];
		if(tot>opening) //opening
		{
			return 0;

		}
		if(tot<endgame)	//endgame
		{
			return 65536;
		}
		return (unsigned int)((float)(opening-tot)*(65536.0f/(float)(opening-endgame)));
	}

	inline bool isCaptureMove(const Move & m) const
	{
		return squares[m.getTo()] !=empty || m.isEnPassantMove() ;
	}
	inline bool isCaptureMoveOrPromotion(const Move & m) const
	{
		return isCaptureMove(m) || m.isPromotionMove();
	}
	inline bool isPassedPawnMove(const Move & m) const
	{
		if(isPawn(getPieceAt(m.getFrom())))
		{
			bool color = squares[m.getFrom()] >= separationBitmap;
			bitMap theirPawns = color? bitBoard[whitePawns]:bitBoard[blackPawns];
			bitMap ourPawns = color? bitBoard[blackPawns]:bitBoard[whitePawns];
			return !(theirPawns & PASSED_PAWN[color][m.getFrom()]) && !(ourPawns & SQUARES_IN_FRONT_OF[color][m.getFrom()]);
		}
		return false;
	}
	
	//--------------------------------------------------------
	// public members
	//--------------------------------------------------------
	static bool perftUseHash;
	static simdScore pieceValue[lastBitboard];

private:


	Position& operator=(Position&& ) noexcept = delete;
	Position(Position&& ) noexcept = delete;

	//--------------------------------------------------------
	// private struct
	//--------------------------------------------------------
	struct materialStruct
	{
		typedef enum
		{
			exact,
			multiplicativeFunction,
			exactFunction,
			saturationH,
			saturationL,
		} tType ;
		tType type;
		bool (Position::*pointer)(Score &);
		Score val;

	};
	
	//--------------------------------------------------------
	// private static members
	//--------------------------------------------------------	
	
	/*! \brief helper mask used to speedup castle right management
		\author STOCKFISH
		\version 1.0
		\date 27/10/2013
	*/
	static eCastle castleRightsMask[squareNumber];
	static simdScore pstValue[lastBitboard][squareNumber];
	static simdScore nonPawnValue[lastBitboard];
	std::unordered_map<uint64_t, materialStruct> static materialKeyMap;
	
	//--------------------------------------------------------
	// private members
	//--------------------------------------------------------	
	unsigned int ply;

	/*used for search*/
	pawnTable pawnHashTable;

	std::vector<state> stateInfo;

	/*! \brief board rapresentation
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	std::array<bitboardIndex,squareNumber> squares;		// board square rapresentation to speed up, it contain pieces indexed by square
	std::array<bitMap,lastBitboard> bitBoard;			// bitboards indexed by bitboardIndex enum
	bitMap *Us,*Them;	/*!< pointer to our & their pieces bitboard*/

	//--------------------------------------------------------
	// private methods
	//--------------------------------------------------------

	/*! \brief insert a new state in memory
		\author Marco Belli
		\version 1.0
		\version 1.1 get rid of continuos malloc/free
		\date 21/11/2013
	*/
	inline void insertState(state & s)
	{
		stateInfo.emplace_back(s);
	}

	/*! \brief  remove the last state
		\author Marco Belli
		\version 1.0
		\version 1.1 get rid of continuos malloc/free
		\date 21/11/2013
	*/
	inline void removeState()
	{
		stateInfo.pop_back();
	}

	inline void updateUsThem();


	uint64_t calcKey(void) const;
	uint64_t calcPawnKey(void) const;
	uint64_t calcMaterialKey(void) const;
	simdScore calcMaterialValue(void) const;
	simdScore calcNonPawnMaterialValue(void) const;
	bool checkPosConsistency(int nn) const;
	void clear();
	inline void calcCheckingSquares(void);
	bitMap getHiddenCheckers(tSquare kingSquare,eNextMove next) const;

	void putPiece(const bitboardIndex piece,const tSquare s);
	void movePiece(const bitboardIndex piece,const tSquare from,const tSquare to);
	void removePiece(const bitboardIndex piece,const tSquare s);


	template<Color c> simdScore evalPawn(tSquare sq, bitMap& weakPawns, bitMap& passedPawns) const;
	template<Color c> simdScore evalPassedPawn(bitMap pp, bitMap * attackedSquares) const;
	template<bitboardIndex piece>	simdScore evalPieces(const bitMap * const weakSquares,  bitMap * const attackedSquares ,const bitMap * const holes, bitMap const blockedPawns, bitMap * const kingRing, unsigned int * const kingAttackersCount, unsigned int * const kingAttackersWeight, unsigned int * const kingAdjacentZoneAttacksCount, bitMap & weakPawns) const;

	template<Color c> Score evalShieldStorm(tSquare ksq) const;
	template<Color c> simdScore evalKingSafety(Score kingSafety, unsigned int kingAttackersCount, unsigned int kingAdjacentZoneAttacksCount, unsigned int kingAttackersWeight, bitMap * const attackedSquares) const;

	const materialStruct* getMaterialData();
	bool evalKxvsK(Score& res);
	bool evalKBPsvsK(Score& res);
	bool evalKQvsKP(Score& res);
	bool evalKRPvsKr(Score& res);
	bool evalKBNvsK(Score& res);
	bool evalKQvsK(Score& res);
	bool evalKRvsK(Score& res);
	bool kingsDirectOpposition();
	bool evalKPvsK(Score& res);
	bool evalKPsvsK(Score& res);
	bool evalOppositeBishopEndgame(Score& res);
	bool evalKRvsKm(Score& res);
	bool evalKNNvsK(Score& res);
	bool evalKNPvsK(Score& res);

};

inline Position::eCastle operator|(const Position::eCastle c1, const Position::eCastle c2) { return Position::eCastle(int(c1) | int(c2)); }

#endif /* POSITION_H_ */
