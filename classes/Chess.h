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

struct MoveState;

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

    bool gameHasAI();
    void updateAI();
    void stopGame() override;

    Player *checkForWinner() override;
    bool checkForDraw() override;
    bool canPawnMoveFromTo(Bit &bit, bool isWhite, ChessSquare* srcSquare, ChessSquare* dstSquare, int xdiff, int ydiff, int sy);
    bool canKnightMoveFromTo(Bit &bit, ChessSquare* srcSquare, ChessSquare* dstSquare, int xdiff, int ydiff);
    bool canKingMoveFromTo(Bit &bit, ChessSquare* srcSquare, ChessSquare* dstSquare, int xdiff, int ydiff);
    bool canRookMoveFromTo(Bit &bit, ChessSquare* srcSquare, ChessSquare* dstSquare, int xdiff, int ydiff);
    bool canQueenMoveFromTo(Bit &bit, ChessSquare* srcSquare, ChessSquare* dstSquare, int xdiff, int ydiff);
    bool canBishopMoveFromTo(Bit &bit, ChessSquare* srcSquare, ChessSquare* dstSquare, int xdiff, int ydiff);


    std::string initialStateString() override;
    std::string stateString() override;
    void setStateString(const std::string &s) override;
    int evaluate();
    MoveState makeMove(const BitMove& move);
    void undoMove(const BitMove& move, MoveState state);
    bool applyMoveToBoard(const BitMove& move);
    int negamax(int depth, int alpha, int beta, int color);
    BitMove findBestMove(int depth);
    void makeAIMove();
    //void FENtoBoard(const std::string& fen);


    Grid* getGrid() override { return _grid; }

private:
    Bit* PieceForPlayer(const int playerNumber, ChessPiece piece);
    Player* ownerAt(int x, int y) const;
    void FENtoBoard(const std::string& fen);
    char pieceNotation(int x, int y) const;

    Grid* _grid;
};