Fork or clone your this chess project into a new GitHub repository.

Add support for FEN stringsLinks to an external site. to your game setup so that instead of the current way you are setting up your game board you are setting it up with a call similar to the following call.

FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

Your routine should be able to take just the board position portion of a FEN string, or the entire FEN string like so:

FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

(you can ignore the end for now)

This will allow you to quickly check that your castling, promotion and en passant code is working.

--Adding FEN string compatibility. This is called within setup game. Currently only creates the board with pieces on it. Doesn't record whose turn, the en passant, or castling.

--Implemented pawn, knight, and king movements. Checks their current positions and the positions they can move to and will return true if the move is valid. Will capture pieces that are on a spot they can move to.

--Implemented rook, bishop, and queen movement. All are able to slide in their correct directions, capture pieces in their way, and cannot move past pieces that are in their line of movement.

--Implemented a "generateAllMoves" function that determines where all the pieces are able to currently move to at the current state of the board.

--Both white and black pieces can move correctly and capture each other correctly. Black pieces cannot capture black pieces and the same with white pieces.
