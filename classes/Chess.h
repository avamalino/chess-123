#pragma once

#include "Game.h"
#include "Grid.h"


constexpr int pieceSize = 80;

enum ChessPiece
{
    NoPiece,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};

struct BitMove;

class Chess : public Game
{
public:
    Chess();
    ~Chess();

    void setUpBoard() override;

    bool canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    bool actionForEmptyHolder(BitHolder &holder) override;
    std::vector<BitMove> generateAllMoves();

    void stopGame() override;

    Player *checkForWinner() override;
    bool checkForDraw() override;
    bool canPawnMoveFromTo(Bit &bit, bool isWhite, ChessSquare* srcSquare, ChessSquare* dstSquare, int xdiff, int ydiff, int sy);
    bool canKnightMoveFromTo(Bit &bit, ChessSquare* srcSquare, ChessSquare* dstSquare, int xdiff, int ydiff);
    bool canKingMoveFromTo(Bit &bit, ChessSquare* srcSquare, ChessSquare* dstSquare, int xdiff, int ydiff);


    std::string initialStateString() override;
    std::string stateString() override;
    void setStateString(const std::string &s) override;
    //void FENtoBoard(const std::string& fen);


    Grid* getGrid() override { return _grid; }

private:
    Bit* PieceForPlayer(const int playerNumber, ChessPiece piece);
    Player* ownerAt(int x, int y) const;
    void FENtoBoard(const std::string& fen);
    char pieceNotation(int x, int y) const;

    Grid* _grid;
};