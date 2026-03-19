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
    if(piece == NoPiece) return nullptr;
    
    const char* pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };
    const char* pieceName = pieces[piece - 1];

    Bit* bit = new Bit();
    // should possibly be cached from player class?
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
    getPlayerAt(1)->setAIPlayer(true);

    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;

    _grid->initializeChessSquares(pieceSize, "boardsquare.png");
    //FENtoBoard("b2r3r/k3Rp1p/p2q1np1/Np1P4/3p1Q2/P4PPB/1PP4P/1K6");
    //FENtoBoard("rnb1kbnr/pppp1ppp/8/4p3/6q1/8/PPPPPPPP/RNBQKBNR");
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

    std::vector<BitMove> moves = generateAllMoves();
    int limit = std::min(20, (int)moves.size());

    for (int i = 0; i < limit; i++){
        BitMove move = moves[i];

        int fromX = move.from % 8;
        int fromY = move.from / 8;

        int toX = move.to % 8;
        int toY = move.to / 8;

        Logger::LogInfo("Move " + std::to_string(i) +": (" + std::to_string(fromX) + ", " + std::to_string(fromY) +") -> (" + std::to_string(toX) + ", " + std::to_string(toY) + ")");

        static const char* pieceNames[] = { "NoPiece", "Pawn", "Knight", "Bishop", "Rook", "Queen", "King" };
        //Logger::LogInfo("Piece: " + std::to_string((int)move.piece));
        Logger::LogInfo(std::string("Piece: ") + pieceNames[move.piece]);
    }

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
                if (!square){
                    Logger::LogError("Invalid square while parsing FEN");
                    continue;
                }
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

bool Chess::gameHasAI(){
    return true;
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

    if (srcSquare == dstSquare){
        return false;
    }

    bool basicMoveLegal = false;
    switch (piece){
        case Pawn:
            basicMoveLegal = canPawnMoveFromTo(bit, isWhite, srcSquare, dstSquare, xdiff, ydiff, sy);
            break;
        case Knight:
            basicMoveLegal = canKnightMoveFromTo(bit, srcSquare, dstSquare, xdiff, ydiff);
            break;
        case King:
            basicMoveLegal = canKingMoveFromTo(bit, srcSquare, dstSquare, xdiff, ydiff);
            break;
        case Rook:
            basicMoveLegal = canRookMoveFromTo(bit, srcSquare, dstSquare, xdiff, ydiff);
            break;
        case Queen:
            basicMoveLegal = canQueenMoveFromTo(bit, srcSquare, dstSquare, xdiff, ydiff);
            break;
        case Bishop:
            basicMoveLegal = canBishopMoveFromTo(bit, srcSquare, dstSquare, xdiff, ydiff);
            break;
        default:
            return false;
    }

    if (!basicMoveLegal){
        return false;
    }

    Player* movingPlayer = bit.getOwner();

    int fromIndex = sy * 8 + sx;
    int toIndex = dy * 8 + dx;
    MoveState state = makeMove(BitMove(fromIndex, toIndex, piece));
    bool stillInCheck = isKingInCheck(movingPlayer);
    undoMove(BitMove(fromIndex, toIndex, piece), state);

    return !stillInCheck;
}

bool Chess::isPathClear(ChessSquare* srcSquare, ChessSquare* dstSquare) const
{
    int xdiff = dstSquare->getColumn() - srcSquare->getColumn();
    int ydiff = dstSquare->getRow() - srcSquare->getRow();

    int xdir = (xdiff == 0) ? 0 : (xdiff > 0 ? 1 : -1);
    int ydir = (ydiff == 0) ? 0 : (ydiff > 0 ? 1 : -1);
    int steps = max(abs(xdiff), abs(ydiff));

    for (int i = 1; i < steps; i++){
        ChessSquare* square = _grid->getSquare(srcSquare->getColumn() + i * xdir, srcSquare->getRow() + i * ydir);
        if (square && square->bit()){
            return false;
        }
    }

    return true;
}

bool Chess::canPieceAttackSquare(Bit* bit, ChessSquare* srcSquare, ChessSquare* dstSquare) const
{
    if (!bit || !srcSquare || !dstSquare || srcSquare == dstSquare){
        return false;
    }

    ChessPiece piece = (ChessPiece)(bit->gameTag() & 7);
    bool isWhite = (bit->gameTag() & 128) == 0;

    int xdiff = dstSquare->getColumn() - srcSquare->getColumn();
    int ydiff = dstSquare->getRow() - srcSquare->getRow();

    switch (piece){
        case Pawn: {
            int direction = isWhite ? 1 : -1;
            return (abs(xdiff) == 1 && ydiff == direction);
        }
        case Knight:
            return (abs(xdiff) == 2 && abs(ydiff) == 1) || (abs(xdiff) == 1 && abs(ydiff) == 2);
        case Bishop:
            return abs(xdiff) == abs(ydiff) && isPathClear(srcSquare, dstSquare);
        case Rook:
            return (xdiff == 0 || ydiff == 0) && isPathClear(srcSquare, dstSquare);
        case Queen:
            return ((xdiff == 0 || ydiff == 0) || (abs(xdiff) == abs(ydiff))) && isPathClear(srcSquare, dstSquare);
        case King:
            return max(abs(xdiff), abs(ydiff)) == 1;
        default:
            return false;
    }
}

bool Chess::getKingSquare(Player* player, ChessSquare*& kingSquare) const
{
    kingSquare = nullptr;
    _grid->forEachSquare([&](ChessSquare* square, int x, int y){
        if (kingSquare || !square->bit()){
            return;
        }

        Bit* bit = square->bit();
        if (bit->getOwner() == player && ((ChessPiece)(bit->gameTag() & 7) == King)){
            kingSquare = square;
        }
    });

    return kingSquare != nullptr;
}

bool Chess::getCheckingPieces(Player* defendingPlayer, std::vector<ChessSquare*>& checkingPieces) const
{
    checkingPieces.clear();

    ChessSquare* kingSquare = nullptr;
    if (!getKingSquare(defendingPlayer, kingSquare)){
        return false;
    }

    _grid->forEachSquare([&](ChessSquare* square, int x, int y){
        Bit* attacker = square->bit();
        if (!attacker || attacker->getOwner() == defendingPlayer){
            return;
        }

        if (canPieceAttackSquare(attacker, square, kingSquare)){
            checkingPieces.push_back(square);
        }
    });

    return !checkingPieces.empty();
}

bool Chess::isKingInCheck(Player* player) const
{
    std::vector<ChessSquare*> checkers;
    return getCheckingPieces(player, checkers);
}

bool Chess::canQueenMoveFromTo(Bit &bit, ChessSquare* srcSquare, ChessSquare* dstSquare, int xdiff, int ydiff){
    //can move any number of squares along a rank, file, or diagonal, but cannot leap over other pieces
    if (xdiff != 0 && ydiff != 0 && abs(xdiff) != abs(ydiff)){
        return false;
    }
    //check for pieces in the way
    int xdir = xdiff == 0 ? 0 : (xdiff > 0 ? 1 : -1);
    int ydir = ydiff == 0 ? 0 : (ydiff > 0 ? 1 : -1);
    int steps = max(abs(xdiff), abs(ydiff));
    for (int i = 1; i < steps; i++){
        ChessSquare* intermediateSquare = _grid->getSquare(srcSquare->getColumn() + i * xdir, srcSquare->getRow() + i * ydir);
        if (intermediateSquare->bit()){
            return false;
        }
    }
    //can capture an enemy piece on the destination square, but cannot capture a friendly piece
    if (dstSquare->bit()){
        Player* srcOwner = bit.getOwner();
        Player* dstOwner = dstSquare->bit()->getOwner();
        if (srcOwner == dstOwner){
            return false;
        }
    }
    return true;
}

bool Chess::canBishopMoveFromTo(Bit &bit, ChessSquare* srcSquare, ChessSquare* dstSquare, int xdiff, int ydiff){
    //can move any number of squares diagonally, but cannot leap over other pieces
    if (abs(xdiff) != abs(ydiff)){
        return false;
    }
    //check for pieces in the way
    int xdir = xdiff > 0 ? 1 : -1;
    int ydir = ydiff > 0 ? 1 : -1;
    for (int i = 1; i < abs(xdiff); i++){
        ChessSquare* intermediateSquare = _grid->getSquare(srcSquare->getColumn() + i * xdir, srcSquare->getRow() + i * ydir);
        if (intermediateSquare->bit()){
            return false;
        }
    }
    //can capture an enemy piece on the destination square, but cannot capture a friendly piece
    if (dstSquare->bit()){
        Player* srcOwner = bit.getOwner();
        Player* dstOwner = dstSquare->bit()->getOwner();
        if (srcOwner == dstOwner){
            return false;
        }
    }
    return true;
}

bool Chess::canRookMoveFromTo(Bit &bit, ChessSquare* srcSquare, ChessSquare* dstSquare, int xdiff, int ydiff){
    //can move any number of squares along a rank or file, but cannot leap over other pieces
    if (xdiff != 0 && ydiff != 0){
        return false;
    }
    //check for pieces in the way
    int xdir = xdiff == 0 ? 0 : (xdiff > 0 ? 1 : -1);
    int ydir = ydiff == 0 ? 0 : (ydiff > 0 ? 1 : -1);
    int steps = max(abs(xdiff), abs(ydiff));
    for (int i = 1; i < steps; i++){
        ChessSquare* intermediateSquare = _grid->getSquare(srcSquare->getColumn() + i * xdir, srcSquare->getRow() + i * ydir);
        if (intermediateSquare->bit()){
            return false;
        }
    }
    //can capture an enemy piece on the destination square, but cannot capture a friendly piece
    if (dstSquare->bit()){
        Player* srcOwner = bit.getOwner();
        Player* dstOwner = dstSquare->bit()->getOwner();
        if (srcOwner == dstOwner){
            return false;
        }
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

                if (piece == NoPiece) return;

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

int Chess::evaluate() {
    int score = 0;

    _grid->forEachSquare([&](ChessSquare* square, int x, int y){
        Bit* bit = square->bit();
        if (!bit) return;

        ChessPiece piece = (ChessPiece)(bit->gameTag() & 7);
        int value = 0;

        if (piece == NoPiece) return;

        switch(piece){
            case Pawn: value = 100; break;
            case Knight: value = 320; break;
            case Bishop: value = 330; break;
            case Rook: value = 500; break;
            case Queen: value = 900; break;
            case King: value = 20000; break;
            default: break;
        }

        if ((bit->gameTag() & 128) == 0){
            score += value;
        } else {
            score -= value;
        }
    });
    return score;
}

MoveState Chess::makeMove(const BitMove& move){
    int sx = move.from % 8;
    int sy = move.from / 8;
    int dx = move.to % 8;
    int dy = move.to / 8;

    ChessSquare* src = _grid->getSquare(sx, sy);
    ChessSquare* dst = _grid->getSquare(dx, dy);

    MoveState state;
    state.captured = dst->bit();
    state.movingWasPickedUp = false;

    // Detach captured piece so destination holder does not delete it during simulation.
    if (state.captured){
        state.captured->setParent(nullptr);
    }

    Bit* moving = src->bit();

    if (!moving){
        Logger::LogError("makeMove called with empty src square");
        return state;
    }

    state.movingWasPickedUp = moving->getPickedUp();
    if (state.movingWasPickedUp){
        // Ensure BitHolder::setBit(nullptr) treats moved-away source pointers as stale, not deletable.
        moving->setPickedUp(false);
    }

    dst->setBit(moving);
    src->setBit(nullptr);

    return state;
}

bool Chess::applyMoveToBoard(const BitMove& move){
    int sx = move.from % 8;
    int sy = move.from / 8;
    int dx = move.to % 8;
    int dy = move.to / 8;

    ChessSquare* src = _grid->getSquare(sx, sy);
    ChessSquare* dst = _grid->getSquare(dx, dy);

    if (!src || !dst){
        Logger::LogError("applyMoveToBoard received an invalid square");
        return false;
    }

    Bit* moving = src->bit();
    if (!moving){
        Logger::LogError("applyMoveToBoard called with empty src square");
        return false;
    }

    if (!canBitMoveFromTo(*moving, *src, *dst)){
        Logger::LogError("applyMoveToBoard called with illegal move");
        return false;
    }

    if (dst->bit()){
        pieceTaken(dst->bit());
    }

    if (!dst->dropBitAtPoint(moving, moving->getPosition())){
        Logger::LogError("applyMoveToBoard failed to drop piece on destination");
        return false;
    }

    src->draggedBitTo(moving, dst);
    bitMovedFromTo(*moving, *src, *dst);
    return true;
}

void Chess::undoMove(const BitMove& move, MoveState state){
    int sx = move.from % 8;
    int sy = move.from / 8;
    int dx = move.to % 8;
    int dy = move.to / 8;

    ChessSquare* src = _grid->getSquare(sx, sy);
    ChessSquare* dst = _grid->getSquare(dx, dy);

    Bit* moving = dst->bit();

    src->setBit(moving);
    if (state.captured){
        state.captured->setParent(nullptr);
        dst->setBit(state.captured);
    } else {
        dst->setBit(nullptr);
    }

    if (moving && state.movingWasPickedUp){
        moving->setPickedUp(true);
    }
}

int Chess::negamax(int depth, int alpha, int beta, int color){
    if (depth == 0){
        return color * evaluate();
    }

    std::vector<BitMove> moves = generateAllMoves();

    if (moves.empty()){
        return color * evaluate();
    }

    int maxEval = std::numeric_limits<int>::min() / 2;

    for (const auto& move : moves){
        MoveState state = makeMove(move);

        int eval = -negamax(depth - 1, -beta, -alpha, -color);

        undoMove(move, state);

        maxEval = std::max(maxEval, eval);
        alpha = std::max(alpha, eval);

        if (alpha >= beta){
            break;
        }
    }
    return maxEval;
}

BitMove Chess::findBestMove(int depth){
    std::vector<BitMove> moves = generateAllMoves();

    if (moves.empty()){
        if (isKingInCheck(getCurrentPlayer())){
            Logger::LogInfo("No legal moves remaining. Checkmate.");
        } else {
            Logger::LogInfo("No legal moves remaining. Stalemate.");
        }
        return BitMove();
    }

    BitMove bestMove;
    int bestValue = std::numeric_limits<int>::min();

    int color = (getCurrentPlayer()->playerNumber() == 0) ? 1 : -1;

    for (const auto& move: moves){
        MoveState state = makeMove(move);

        int eval = -negamax(depth - 1, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), -color);
        undoMove(move, state);

        if (eval > bestValue){
            bestValue = eval;
            bestMove = move;
        }
    }
    return bestMove;
}

void Chess::makeAIMove(){
    BitMove bestMove = findBestMove(5);

    if (!applyMoveToBoard(bestMove)){
        return;
    }

    Logger::LogInfo("AI made a move");
}

void Chess::updateAI(){
    static bool thinking = false;
    if (thinking) return;
    thinking = true;

    BitMove bestMove = findBestMove(3);  //trying 1 first before 3

    if (bestMove.from == bestMove.to){
        thinking = false;
        return;
    }

    if (!applyMoveToBoard(bestMove)){
        thinking = false;
        return;
    }

    int fromX = bestMove.from % 8;
    int fromY = bestMove.from / 8;
    int toX = bestMove.to % 8;
    int toY = bestMove.to / 8;

    Logger::LogInfo(
        "AI move: (" + std::to_string(fromX) + "," + std::to_string(fromY) +
        ") -> (" + std::to_string(toX) + "," + std::to_string(toY) + ")"
    );

    thinking = false;
}