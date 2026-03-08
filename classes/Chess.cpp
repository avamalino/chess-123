#include "Chess.h"
#include "Logger.h"
#include "Bitboard.h"
#include <limits>
#include <cmath>
#include <unordered_map>
#include <string>
#include <iostream>
using namespace std;

Chess::Chess()
{
    _grid = new Grid(8, 8);
}

Chess::~Chess()
{
    delete _grid;
}

char Chess::pieceNotation(int x, int y) const
{
    const char *wpieces = { "0PNBRQK" };
    const char *bpieces = { "0pnbrqk" };
    Bit *bit = _grid->getSquare(x, y)->bit();
    char notation = '0';
    if (bit) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag()-128];
    }
    return notation;
}

Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char* pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    Bit* bit = new Bit();
    // should possibly be cached from player class?
    const char* pieceName = pieces[piece - 1];
    std::string spritePath = std::string("") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);

    int tag = piece;
    if (playerNumber == 1){
        tag += 128;
    }

    bit->setGameTag(tag);

    return bit;
}

void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;

    _grid->initializeChessSquares(pieceSize, "boardsquare.png");
    //FENtoBoard("b2r3r/k3Rp1p/p2q1np1/Np1P4/3p1Q2/P4PPB/1PP4P/1K6");
    //FENtoBoard("rnb1kbnr/pppp1ppp/8/4p3/6q1/8/PPPPPPPP/RNBQKBNR");
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    Logger::LogInfo(stateString());

    startGame();
}

void Chess::FENtoBoard(const std::string& fen) {
    // convert a FEN string to a board
    // FEN is a space delimited string with 6 fields
    // 1: piece placement (from white's perspective)
    // NOT PART OF THIS ASSIGNMENT BUT OTHER THINGS THAT CAN BE IN A FEN STRING
    // ARE BELOW

    unordered_map<char, ChessPiece> dict = {
    {'r', Rook},
    {'n', Knight},
    {'b', Bishop},
    {'q', Queen},
    {'k', King},
    {'p', Pawn},
    {'R', Rook},
    {'N', Knight},
    {'B', Bishop},
    {'Q', Queen},
    {'K', King},
    {'P', Pawn}
    };
    
    int row = 0;
    int col = 0;
    for (char ch: fen){
        if (ch == ' '){
            break;
        }
        if (ch == '/'){
            row++;
            col = 0;
        } else if (ch >= '1' && ch <= '8') {
            col += ch - '0';
        } else {
            if (col >= 8 || row >= 8){
                Logger::LogError("Invalid FEN string: too many pieces");
                return;
            }
                ChessPiece type = dict.at(ch);
                int player = isupper(ch) ? 0: 1;
                ChessSquare* square = _grid->getSquare(col, 7 - row);
                Bit* piece = PieceForPlayer(player, type);
                square->dropBitAtPoint(piece, square->getPosition());
                col++;
            }
        }
    
    
    //create a new "FEN" string that only has the piece placement, and then parse that to set up the board
    // 2: active color (W or B)
    // 3: castling availability (KQkq or -)
    // 4: en passant target square (in algebraic notation, or -)
    // 5: halfmove clock (number of halfmoves since the last capture or pawn advance)
}

bool Chess::actionForEmptyHolder(BitHolder &holder)
{
    return false;
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    // need to implement friendly/unfriendly in bit so for now this hack
    int currentPlayer = getCurrentPlayer()->playerNumber() * 128;
    int pieceColor = bit.gameTag() & 128;
    if (pieceColor == currentPlayer) return true;
    return false;
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) //source, destination
{
    ChessPiece piece = (ChessPiece)(bit.gameTag() & 7);
    bool isWhite = (bit.gameTag() & 128) == 0;

    ChessSquare* srcSquare = (ChessSquare*)&src;
    ChessSquare* dstSquare = (ChessSquare*)&dst;

    //get x and y positions of source and destination squares
    int sx = srcSquare->getColumn();
    int sy = srcSquare->getRow();
    int dx = dstSquare->getColumn();
    int dy = dstSquare->getRow();

    //change in x and y positions
    int xdiff = dx - sx;
    int ydiff = dy - sy;

    if (piece == Pawn){
        return canPawnMoveFromTo(bit, isWhite, srcSquare, dstSquare, xdiff, ydiff, sy);
    }
    if (piece == Knight){
        return canKnightMoveFromTo(bit, srcSquare, dstSquare, xdiff, ydiff);
    }
    if (piece == King){
        return canKingMoveFromTo(bit, srcSquare, dstSquare, xdiff, ydiff);
    }
    return true;
}

bool Chess::canKingMoveFromTo(Bit &bit, ChessSquare* srcSquare, ChessSquare* dstSquare, int xdiff, int ydiff){
    Player* srcOwner;
    Player* dstOwner;
    //move up down left right
    if (((abs(xdiff) == 1 && ydiff == 0) || (abs(ydiff) == 1 && xdiff == 0))){
        if (dstSquare->bit()){
            srcOwner = bit.getOwner();
            dstOwner = dstSquare->bit()->getOwner();
            if (srcOwner != dstOwner){
                    return true;
            } else {
                return false;
            }
        }
        return true;
    }
    //move diagonally
    if (((abs(xdiff) == 1) && (abs(ydiff) == 1))){
        if (dstSquare->bit()){
            srcOwner = bit.getOwner();
            dstOwner = dstSquare->bit()->getOwner();
            if (srcOwner != dstOwner){
                    return true;
            } else {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool Chess::canKnightMoveFromTo(Bit &bit, ChessSquare* srcSquare, ChessSquare* dstSquare, int xdiff, int ydiff){
    //can move 2 up and 1 over or 2 over and 1 up
    if ((abs(xdiff) == 2 && abs(ydiff) == 1) || (abs(xdiff)== 1 && (abs(ydiff) == 2))){
        //is square empty?
        if (!dstSquare->bit()){
            return true;
        }
        //if not empty, is there an enemy to capture?
        Player* srcOwner = bit.getOwner();
        Player* dstOwner = dstSquare->bit()->getOwner();
        if (srcOwner != dstOwner){
            return true;
        }
    }
    return false;
}

bool Chess::canPawnMoveFromTo(Bit &bit, bool isWhite, ChessSquare* srcSquare, ChessSquare* dstSquare, int xdiff, int ydiff, int sy){
        int direction = isWhite ? 1 : -1;

        // pawns can move forward one if square is empty.
        if (xdiff == 0 && ydiff == direction && !dstSquare->bit()){
            return true;
        }
        //can move forward two only if the pawn is still on its starting square and 
        //two squares in front of it are empty
        if (xdiff == 0 && ydiff == 2 * direction){
            if (isWhite && sy == 1 && !dstSquare->bit()){
                return true;
            }
            //same for black but in the opposite direction
            if (!isWhite && sy == 6 && !dstSquare->bit()){
                return true;
            }
        }
        //diagonal capture, can capture left or right but only if there is an enemy piece there
        if (abs(xdiff) == 1 && ydiff == direction && dstSquare->bit()){
            Player* srcOwner = bit.getOwner();
            Player* dstOwner = dstSquare->bit()->getOwner();
            if (srcOwner != dstOwner){
                return true;
            }
            
        }
        return false;
    }

std::vector<BitMove> Chess::generateAllMoves(){
    std::vector<BitMove> moves;
    Player* current = getCurrentPlayer();
    _grid->forEachSquare([&](ChessSquare* srcSquare, int sx, int sy){
        Bit* bit = srcSquare->bit();

        if (!bit){
            return;
        }
        if (bit->getOwner() != current){
            return;
        }
        BitHolder& srcHolder = *srcSquare;

        if (!canBitMoveFrom(*bit, srcHolder)){
            return;
        }
        _grid->forEachSquare([&](ChessSquare* dstSquare, int dx, int dy){
            BitHolder& dstHolder = *dstSquare;
            if (canBitMoveFromTo(*bit, srcHolder, dstHolder)){
                int fromIndex = sy * 8 + sx;
                int toIndex = dy * 8 + dx;
                ChessPiece piece = (ChessPiece)(bit->gameTag() & 7);
                moves.emplace_back(fromIndex, toIndex, piece);
            }
        });
    });
    return moves;
}

void Chess::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

Player* Chess::ownerAt(int x, int y) const
{
    if (x < 0 || x >= 8 || y < 0 || y >= 8) {
        return nullptr;
    }

    auto square = _grid->getSquare(x, y);
    if (!square || !square->bit()) {
        return nullptr;
    }
    return square->bit()->getOwner();
}

Player* Chess::checkForWinner()
{
    return nullptr;
}

bool Chess::checkForDraw()
{
    return false;
}

std::string Chess::initialStateString()
{
    return stateString();
}

std::string Chess::stateString()
{
    std::string s;
    s.reserve(64);
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
            s += pieceNotation( x, y );
        }
    );
    return s;}

void Chess::setStateString(const std::string &s)
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y * 8 + x;
        char playerNumber = s[index] - '0';
        if (playerNumber) {
            square->setBit(PieceForPlayer(playerNumber - 1, ChessPiece::Pawn));
        } else {
            square->setBit(nullptr);
        }
    });
}
