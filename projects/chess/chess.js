"use strict";

// ================================================================
// === CONSTANTS
// ================================================================

const PIECES = { P:1, N:2, B:3, R:4, Q:5, K:6 };  // white pieces
const PNAMES = { 1:"Pawn", 2:"Knight", 3:"Bishop", 4:"Rook", 5:"Queen", 6:"King" };

// Unicode chess symbols [white, black] indexed by piece type 1-6
const SYMBOLS = {
  1: ["♙","♟"], 2: ["♘","♞"], 3: ["♗","♝"],
  4: ["♖","♜"], 5: ["♕","♛"], 6: ["♔","♚"],
};

const WHITE = 1, BLACK = -1;

const STORAGE = {
  pieceValues : "chess_piece_values",
  pst         : "chess_pst",
  wins        : "chess_wins",
};


// ================================================================
// === DEFAULT WEIGHTS
// Piece values and piece-square tables (PST).
// PSTs are from White's perspective (row 0 = rank 8, row 7 = rank 1).
// They get mirrored for Black automatically.
// ================================================================

const DEFAULT_PIECE_VALUES = { 1:100, 2:320, 3:330, 4:500, 5:900, 6:20000 };

const DEFAULT_PST = {
  // Pawns: encouraged to advance and control center
  1: [
    [ 0,  0,  0,  0,  0,  0,  0,  0],
    [50, 50, 50, 50, 50, 50, 50, 50],
    [10, 10, 20, 30, 30, 20, 10, 10],
    [ 5,  5, 10, 25, 25, 10,  5,  5],
    [ 0,  0,  0, 20, 20,  0,  0,  0],
    [ 5, -5,-10,  0,  0,-10, -5,  5],
    [ 5, 10, 10,-20,-20, 10, 10,  5],
    [ 0,  0,  0,  0,  0,  0,  0,  0],
  ],
  // Knights: strong in center, weak on edges
  2: [
    [-50,-40,-30,-30,-30,-30,-40,-50],
    [-40,-20,  0,  0,  0,  0,-20,-40],
    [-30,  0, 10, 15, 15, 10,  0,-30],
    [-30,  5, 15, 20, 20, 15,  5,-30],
    [-30,  0, 15, 20, 20, 15,  0,-30],
    [-30,  5, 10, 15, 15, 10,  5,-30],
    [-40,-20,  0,  5,  5,  0,-20,-40],
    [-50,-40,-30,-30,-30,-30,-40,-50],
  ],
  // Bishops: diagonals, avoid corners
  3: [
    [-20,-10,-10,-10,-10,-10,-10,-20],
    [-10,  0,  0,  0,  0,  0,  0,-10],
    [-10,  0,  5, 10, 10,  5,  0,-10],
    [-10,  5,  5, 10, 10,  5,  5,-10],
    [-10,  0, 10, 10, 10, 10,  0,-10],
    [-10, 10, 10, 10, 10, 10, 10,-10],
    [-10,  5,  0,  0,  0,  0,  5,-10],
    [-20,-10,-10,-10,-10,-10,-10,-20],
  ],
  // Rooks: open files, 7th rank bonus
  4: [
    [ 0,  0,  0,  0,  0,  0,  0,  0],
    [ 5, 10, 10, 10, 10, 10, 10,  5],
    [-5,  0,  0,  0,  0,  0,  0, -5],
    [-5,  0,  0,  0,  0,  0,  0, -5],
    [-5,  0,  0,  0,  0,  0,  0, -5],
    [-5,  0,  0,  0,  0,  0,  0, -5],
    [-5,  0,  0,  0,  0,  0,  0, -5],
    [ 0,  0,  0,  5,  5,  0,  0,  0],
  ],
  // Queens: centralised but not too early
  5: [
    [-20,-10,-10, -5, -5,-10,-10,-20],
    [-10,  0,  0,  0,  0,  0,  0,-10],
    [-10,  0,  5,  5,  5,  5,  0,-10],
    [ -5,  0,  5,  5,  5,  5,  0, -5],
    [  0,  0,  5,  5,  5,  5,  0, -5],
    [-10,  5,  5,  5,  5,  5,  0,-10],
    [-10,  0,  5,  0,  0,  0,  0,-10],
    [-20,-10,-10, -5, -5,-10,-10,-20],
  ],
  // King: stay safe in corners early game
  6: [
    [-30,-40,-40,-50,-50,-40,-40,-30],
    [-30,-40,-40,-50,-50,-40,-40,-30],
    [-30,-40,-40,-50,-50,-40,-40,-30],
    [-30,-40,-40,-50,-50,-40,-40,-30],
    [-20,-30,-30,-40,-40,-30,-30,-20],
    [-10,-20,-20,-20,-20,-20,-20,-10],
    [ 20, 20,  0,  0,  0,  0, 20, 20],
    [ 20, 30, 10,  0,  0, 10, 30, 20],
  ],
};


// ================================================================
// === WEIGHT STORAGE
// Piece values and PSTs persist across sessions via localStorage.
// ================================================================

function loadWeights() {
  try {
    const pv  = JSON.parse(localStorage.getItem(STORAGE.pieceValues));
    const pst = JSON.parse(localStorage.getItem(STORAGE.pst));
    return {
      pieceValues : pv  || JSON.parse(JSON.stringify(DEFAULT_PIECE_VALUES)),
      pst         : pst || JSON.parse(JSON.stringify(DEFAULT_PST)),
    };
  } catch {
    return {
      pieceValues : JSON.parse(JSON.stringify(DEFAULT_PIECE_VALUES)),
      pst         : JSON.parse(JSON.stringify(DEFAULT_PST)),
    };
  }
}

function saveWeights(weights) {
  localStorage.setItem(STORAGE.pieceValues, JSON.stringify(weights.pieceValues));
  localStorage.setItem(STORAGE.pst,         JSON.stringify(weights.pst));
}

function loadWins() {
  try { return JSON.parse(localStorage.getItem(STORAGE.wins)) || {white:0, black:0, draw:0}; }
  catch { return {white:0, black:0, draw:0}; }
}

function saveWins(w) { localStorage.setItem(STORAGE.wins, JSON.stringify(w)); }

// Active weights — mutated when user edits them in the UI
let weights = loadWeights();
let wins    = loadWins();


// ================================================================
// === BOARD REPRESENTATION
// board[row][col]: positive = white piece type, negative = black, 0 = empty
// Row 0 = rank 8 (black back rank), Row 7 = rank 1 (white back rank)
// ================================================================

function makeInitialBoard() {
  // prettier-ignore
  return [
    [-4,-2,-3,-5,-6,-3,-2,-4],  // rank 8 — black back row
    [-1,-1,-1,-1,-1,-1,-1,-1],  // rank 7 — black pawns
    [ 0, 0, 0, 0, 0, 0, 0, 0],
    [ 0, 0, 0, 0, 0, 0, 0, 0],
    [ 0, 0, 0, 0, 0, 0, 0, 0],
    [ 0, 0, 0, 0, 0, 0, 0, 0],
    [ 1, 1, 1, 1, 1, 1, 1, 1],  // rank 2 — white pawns
    [ 4, 2, 3, 5, 6, 3, 2, 4],  // rank 1 — white back row
  ];
}

function copyBoard(b) { return b.map(r => r.slice()); }


// ================================================================
// === GAME STATE
// ================================================================

let state = {
  board        : makeInitialBoard(),
  turn         : WHITE,
  // Castling rights: can [white kingside, white queenside, black kingside, black queenside]
  castling     : [true, true, true, true],
  // En passant target square [row, col] or null
  enPassant    : null,
  // Half-move clock (for 50-move rule tracking, not enforced here — engine avoids it)
  halfMove     : 0,
  moveNumber   : 1,
  history      : [],   // stack of {board, castling, enPassant, halfMove, captured}
  lastMove     : null, // {from:[r,c], to:[r,c]} for highlighting
  capturedByWhite : [],
  capturedByBlack : [],
  gameOver     : false,
  mode         : "hvb",   // "hvb" = human vs bot, "bvb" = bot vs bot
  humanColor   : WHITE,   // which color the human plays in hvb mode
  botSpeed     : 500,     // ms delay between bot moves in bvb
  botDepth     : 3,
};


// ================================================================
// === MOVE GENERATION
// ================================================================

// Returns all pseudo-legal moves for a piece at (r, c).
// Pseudo-legal = ignores whether king is left in check.
function getPieceMoves(board, r, c, castling, enPassant) {
  const piece = board[r][c];
  if (piece === 0) return [];
  const color = piece > 0 ? WHITE : BLACK;
  const type  = Math.abs(piece);
  const moves = [];

  const inBounds = (r, c) => r >= 0 && r < 8 && c >= 0 && c < 8;
  const isEnemy  = (r, c) => inBounds(r, c) && board[r][c] !== 0 && Math.sign(board[r][c]) !== color;
  const isEmpty  = (r, c) => inBounds(r, c) && board[r][c] === 0;
  const push     = (tr, tc) => moves.push([r, c, tr, tc]);

  // --- Pawn ---
  if (type === PIECES.P) {
    const dir = color === WHITE ? -1 : 1;   // white moves up (decreasing row index)
    const startRow = color === WHITE ? 6 : 1;

    // Forward 1
    if (isEmpty(r + dir, c)) {
      push(r + dir, c);
      // Forward 2 from start
      if (r === startRow && isEmpty(r + dir * 2, c)) push(r + dir * 2, c);
    }
    // Captures diagonal
    for (const dc of [-1, 1]) {
      if (isEnemy(r + dir, c + dc)) push(r + dir, c + dc);
      // En passant
      if (enPassant && r + dir === enPassant[0] && c + dc === enPassant[1])
        moves.push([r, c, r + dir, c + dc, "ep"]);
    }
  }

  // --- Knight ---
  if (type === PIECES.N) {
    for (const [dr, dc] of [[-2,-1],[-2,1],[-1,-2],[-1,2],[1,-2],[1,2],[2,-1],[2,1]]) {
      const tr = r+dr, tc = c+dc;
      if (inBounds(tr, tc) && Math.sign(board[tr][tc]) !== color) push(tr, tc);
    }
  }

  // --- Sliding pieces: Bishop, Rook, Queen ---
  const bishopDirs = [[-1,-1],[-1,1],[1,-1],[1,1]];
  const rookDirs   = [[-1,0],[1,0],[0,-1],[0,1]];

  if (type === PIECES.B || type === PIECES.Q) {
    for (const [dr, dc] of bishopDirs) {
      let tr = r+dr, tc = c+dc;
      while (inBounds(tr, tc)) {
        if (board[tr][tc] === 0) { push(tr, tc); tr+=dr; tc+=dc; }
        else { if (isEnemy(tr, tc)) push(tr, tc); break; }
      }
    }
  }
  if (type === PIECES.R || type === PIECES.Q) {
    for (const [dr, dc] of rookDirs) {
      let tr = r+dr, tc = c+dc;
      while (inBounds(tr, tc)) {
        if (board[tr][tc] === 0) { push(tr, tc); tr+=dr; tc+=dc; }
        else { if (isEnemy(tr, tc)) push(tr, tc); break; }
      }
    }
  }

  // --- King ---
  if (type === PIECES.K) {
    for (const [dr, dc] of [[-1,-1],[-1,0],[-1,1],[0,-1],[0,1],[1,-1],[1,0],[1,1]]) {
      const tr = r+dr, tc = c+dc;
      if (inBounds(tr, tc) && Math.sign(board[tr][tc]) !== color) push(tr, tc);
    }
    // Castling
    const row = color === WHITE ? 7 : 0;
    if (r === row && c === 4) {
      // Kingside
      const kIdx = color === WHITE ? 0 : 2;
      if (castling[kIdx] && board[row][5] === 0 && board[row][6] === 0)
        moves.push([r, c, row, 6, "castle-k"]);
      // Queenside
      const qIdx = color === WHITE ? 1 : 3;
      if (castling[qIdx] && board[row][3] === 0 && board[row][2] === 0 && board[row][1] === 0)
        moves.push([r, c, row, 2, "castle-q"]);
    }
  }

  return moves;
}

// Returns all legal moves for the given color (filters moves that leave king in check)
function getLegalMoves(board, color, castling, enPassant) {
  const all = [];
  for (let r = 0; r < 8; r++) {
    for (let c = 0; c < 8; c++) {
      if (Math.sign(board[r][c]) === color) {
        all.push(...getPieceMoves(board, r, c, castling, enPassant));
      }
    }
  }
  return all.filter(m => {
    const nb = applyMoveToBoard(board, m, castling).board;
    return !isKingInCheck(nb, color);
  });
}

// Returns true if the given color's king is in check on board b
function isKingInCheck(b, color) {
  // Find king
  let kr = -1, kc = -1;
  outer: for (let r = 0; r < 8; r++) {
    for (let c = 0; c < 8; c++) {
      if (b[r][c] === color * PIECES.K) { kr = r; kc = c; break outer; }
    }
  }
  if (kr === -1) return true; // king captured — shouldn't happen in legal play

  // Check if any enemy piece attacks the king's square
  const opp = -color;
  for (let r = 0; r < 8; r++) {
    for (let c = 0; c < 8; c++) {
      if (Math.sign(b[r][c]) === opp) {
        const moves = getPieceMoves(b, r, c, [false,false,false,false], null);
        if (moves.some(m => m[2] === kr && m[3] === kc)) return true;
      }
    }
  }
  return false;
}

// Apply a move to a board copy, return {board, newCastling, newEnPassant, captured}
function applyMoveToBoard(board, move, castling) {
  const [fr, fc, tr, tc, flag] = move;
  const nb = copyBoard(board);
  const piece = nb[fr][fc];
  const captured = nb[tr][tc];
  const color = Math.sign(piece);
  const type = Math.abs(piece);
  let newCastling = castling.slice();
  let newEnPassant = null;

  nb[tr][tc] = piece;
  nb[fr][fc] = 0;

  // En passant capture
  if (flag === "ep") {
    const dir = color === WHITE ? 1 : -1;
    nb[tr + dir][tc] = 0;
  }

  // Castling: move rook too
  if (flag === "castle-k") {
    nb[tr][5] = nb[tr][7]; nb[tr][7] = 0;
  }
  if (flag === "castle-q") {
    nb[tr][3] = nb[tr][0]; nb[tr][0] = 0;
  }

  // Pawn promotion — always promote to queen
  if (type === PIECES.P && (tr === 0 || tr === 7)) {
    nb[tr][tc] = color * PIECES.Q;
  }

  // En passant opportunity
  if (type === PIECES.P && Math.abs(tr - fr) === 2) {
    newEnPassant = [(fr + tr) / 2, fc];
  }

  // Update castling rights when king or rook moves
  if (type === PIECES.K) {
    if (color === WHITE) { newCastling[0] = false; newCastling[1] = false; }
    else                 { newCastling[2] = false; newCastling[3] = false; }
  }
  if (type === PIECES.R) {
    if (fr === 7 && fc === 7) newCastling[0] = false; // white kingside rook
    if (fr === 7 && fc === 0) newCastling[1] = false; // white queenside rook
    if (fr === 0 && fc === 7) newCastling[2] = false; // black kingside rook
    if (fr === 0 && fc === 0) newCastling[3] = false; // black queenside rook
  }

  return { board: nb, newCastling, newEnPassant, captured };
}


// ================================================================
// === EVALUATION FUNCTION
// Scores the board from White's perspective.
// Positive = White advantage, Negative = Black advantage.
// ================================================================

function evaluate(board) {
  let score = 0;
  for (let r = 0; r < 8; r++) {
    for (let c = 0; c < 8; c++) {
      const p = board[r][c];
      if (p === 0) continue;
      const color = Math.sign(p);
      const type  = Math.abs(p);

      // Material value
      const matVal = weights.pieceValues[type];

      // Piece-square table bonus
      // For black, mirror the row so the table is from that piece's perspective
      const pstRow = color === WHITE ? r : 7 - r;
      const pstVal = (weights.pst[type] && weights.pst[type][pstRow])
        ? weights.pst[type][pstRow][c]
        : 0;

      score += color * (matVal + pstVal);
    }
  }
  return score;
}


// ================================================================
// === TRANSPOSITION TABLE
// Maps Zobrist hash -> {depth, score, flag}
// flag: "exact" | "lower" | "upper"
// Cleared at the start of each search.
// ================================================================

// Zobrist keys: random 32-bit integers for each (piece, square) combo
const ZOBRIST = (() => {
  const rand = () => Math.floor(Math.random() * 0xFFFFFFFF);
  // table[piece+6][row][col]  (piece goes from -6 to 6, shift by 6)
  const t = Array.from({length:13}, () =>
    Array.from({length:8}, () => Array.from({length:8}, rand))
  );
  return t;
})();

function hashBoard(board) {
  let h = 0;
  for (let r = 0; r < 8; r++)
    for (let c = 0; c < 8; c++)
      if (board[r][c] !== 0) h ^= ZOBRIST[board[r][c]+6][r][c];
  return h;
}

let transpositionTable = new Map();


// ================================================================
// === MOVE ORDERING
// Better moves searched first = more alpha-beta cutoffs.
// Order: captures (MVV-LVA) > killer moves > quiet moves
// MVV-LVA = Most Valuable Victim, Least Valuable Attacker
// ================================================================

function scoreMoveForOrdering(board, move) {
  const [fr, fc, tr, tc] = move;
  const victim    = board[tr][tc];
  const aggressor = board[fr][fc];
  if (victim !== 0) {
    // Capture: reward taking high-value pieces with low-value pieces
    return 10 * Math.abs(victim) - Math.abs(aggressor);
  }
  return 0; // quiet move
}

function sortMoves(board, moves) {
  const scored = moves.map(m => ({ m, s: scoreMoveForOrdering(board, m) }));
  scored.sort((a, b) => b.s - a.s);
  return scored.map(x => x.m);
}


// ================================================================
// === QUIESCENCE SEARCH
// Extends search at leaf nodes to resolve captures.
// Prevents the "horizon effect" where the engine misses recaptures.
// ================================================================

function quiescence(board, color, alpha, beta, castling, enPassant, nodesRef) {
  nodesRef.count++;

  // Stand-pat score: assume we can choose not to capture
  const standPat = color === WHITE ? evaluate(board) : -evaluate(board);
  if (standPat >= beta) return beta;
  if (standPat > alpha) alpha = standPat;

  // Only look at captures
  const moves = getLegalMoves(board, color, castling, enPassant)
    .filter(m => board[m[2]][m[3]] !== 0 || m[4] === "ep");

  for (const move of sortMoves(board, moves)) {
    const { board: nb, newCastling, newEnPassant } = applyMoveToBoard(board, move, castling);
    const score = -quiescence(nb, -color, -beta, -alpha, newCastling, newEnPassant, nodesRef);
    if (score >= beta) return beta;
    if (score > alpha) alpha = score;
  }
  return alpha;
}


// ================================================================
// === ALPHA-BETA WITH NULL MOVE PRUNING
//
// Alpha: best score the maximiser (current player) is guaranteed
// Beta:  best score the minimiser (opponent) is guaranteed
// If a node's score >= beta, the opponent would never allow this
// position, so we prune (beta cutoff).
//
// Null Move: give the opponent a free move — if they still can't
// improve, the position is likely very good for us, prune early.
// ================================================================

function alphaBeta(board, depth, alpha, beta, color, castling, enPassant, nodesRef, allowNull) {
  nodesRef.count++;

  // Transposition table lookup
  const hash = hashBoard(board);
  const ttEntry = transpositionTable.get(hash);
  if (ttEntry && ttEntry.depth >= depth) {
    if (ttEntry.flag === "exact") return ttEntry.score;
    if (ttEntry.flag === "lower") alpha = Math.max(alpha, ttEntry.score);
    if (ttEntry.flag === "upper") beta  = Math.min(beta,  ttEntry.score);
    if (alpha >= beta) return ttEntry.score;
  }

  // Leaf node: quiescence search instead of plain evaluate
  if (depth === 0) {
    const q = quiescence(board, color, alpha, beta, castling, enPassant, nodesRef);
    return q;
  }

  const moves = getLegalMoves(board, color, castling, enPassant);

  // No legal moves = checkmate or stalemate
  if (moves.length === 0) {
    if (isKingInCheck(board, color)) {
      // Checkmate — penalise based on depth so engine prefers faster mates
      return -weights.pieceValues[PIECES.K] + (10 - depth) * 100;
    }
    return 0; // Stalemate
  }

  // Null Move Pruning
  // Skip if in check or very low material (endgame) to avoid zugzwang errors
  if (allowNull && depth >= 3 && !isKingInCheck(board, color)) {
    // Give opponent an extra move (null move = pass)
    const nullScore = -alphaBeta(board, depth - 3, -beta, -beta + 1, -color, castling, null, nodesRef, false);
    if (nullScore >= beta) return beta; // Our position is so good opponent can't recover
  }

  let bestScore = -Infinity;
  let flag = "upper";

  for (const move of sortMoves(board, moves)) {
    const { board: nb, newCastling, newEnPassant } = applyMoveToBoard(board, move, castling);
    const score = -alphaBeta(nb, depth - 1, -beta, -alpha, -color, newCastling, newEnPassant, nodesRef, true);

    if (score > bestScore) bestScore = score;
    if (score > alpha) {
      alpha = score;
      flag = "exact";
    }
    if (alpha >= beta) {
      // Beta cutoff — store as lower bound
      transpositionTable.set(hash, { depth, score: bestScore, flag: "lower" });
      return bestScore;
    }
  }

  // Store result in transposition table
  transpositionTable.set(hash, { depth, score: bestScore, flag });
  return bestScore;
}


// ================================================================
// === ITERATIVE DEEPENING
// Search depth 1, 2, 3... until time runs low or max depth reached.
// Always returns the best move found so far, so we never time out
// with no answer.
// ================================================================

function findBestMove(board, color, castling, enPassant, maxDepth) {
  transpositionTable.clear(); // fresh table each search
  const nodesRef = { count: 0 };

  let bestMove = null;
  let bestScore = -Infinity;
  let actualDepth = 0;

  const moves = getLegalMoves(board, color, castling, enPassant);
  if (moves.length === 0) return null;

  // Iterative deepening: search increasing depths
  for (let depth = 1; depth <= maxDepth; depth++) {
    let depthBest = null;
    let depthScore = -Infinity;
    let alpha = -Infinity, beta = Infinity;

    for (const move of sortMoves(board, moves)) {
      const { board: nb, newCastling, newEnPassant } = applyMoveToBoard(board, move, castling);
      const score = -alphaBeta(nb, depth - 1, -beta, -alpha, -color, newCastling, newEnPassant, nodesRef, true);

      if (score > depthScore) {
        depthScore = score;
        depthBest  = move;
      }
      alpha = Math.max(alpha, score);
    }

    if (depthBest) {
      bestMove  = depthBest;
      bestScore = depthScore;
      actualDepth = depth;
    }
  }

  // Update stats display
  document.getElementById("statNodes").textContent = nodesRef.count.toLocaleString();
  document.getElementById("statDepth").textContent = actualDepth;
  document.getElementById("statEval").textContent  = (bestScore / 100).toFixed(2);

  return bestMove;
}


// ================================================================
// === GAME LOGIC (apply move to game state, undo, etc.)
// ================================================================

function applyMove(move) {
  const { board: nb, newCastling, newEnPassant, captured } =
    applyMoveToBoard(state.board, move, state.castling);

  // Push history for undo
  state.history.push({
    board      : copyBoard(state.board),
    castling   : state.castling.slice(),
    enPassant  : state.enPassant,
    halfMove   : state.halfMove,
    lastMove   : state.lastMove,
    capturedByWhite : state.capturedByWhite.slice(),
    capturedByBlack : state.capturedByBlack.slice(),
  });

  // Track captured pieces
  if (captured !== 0) {
    if (state.turn === WHITE) state.capturedByWhite.push(captured);
    else                      state.capturedByBlack.push(captured);
  }
  if (move[4] === "ep") {
    const epCap = state.turn === WHITE ? 1 : -1;  // pawn of opposite color
    if (state.turn === WHITE) state.capturedByWhite.push(-1);
    else                      state.capturedByBlack.push(1);
  }

  state.board     = nb;
  state.castling  = newCastling;
  state.enPassant = newEnPassant;
  state.lastMove  = { from: [move[0], move[1]], to: [move[2], move[3]] };
  state.halfMove++;
  if (state.turn === BLACK) state.moveNumber++;
  state.turn = -state.turn;
}

function undoMove() {
  if (state.history.length === 0) return;
  const prev = state.history.pop();
  state.board           = prev.board;
  state.castling        = prev.castling;
  state.enPassant       = prev.enPassant;
  state.halfMove        = prev.halfMove;
  state.lastMove        = prev.lastMove;
  state.capturedByWhite = prev.capturedByWhite;
  state.capturedByBlack = prev.capturedByBlack;
  state.turn = -state.turn;
  state.gameOver = false;
}

function checkGameOver() {
  const moves = getLegalMoves(state.board, state.turn, state.castling, state.enPassant);
  if (moves.length > 0) return null;
  if (isKingInCheck(state.board, state.turn)) {
    const winner = state.turn === WHITE ? "Black" : "White";
    return { type: "checkmate", winner };
  }
  return { type: "stalemate" };
}


// ================================================================
// === RENDERING
// ================================================================

let selectedCell = null;
let legalMovesForSelected = [];

function renderBoard() {
  const boardEl = document.getElementById("chessBoard");
  boardEl.innerHTML = "";

  // When human plays Black, flip the board so Black is at the bottom
  const flipped = (state.mode === "hvb" && state.humanColor === BLACK);

  for (let ri = 0; ri < 8; ri++) {
    for (let ci = 0; ci < 8; ci++) {
      // Map visual position to actual board position
      const r = flipped ? 7 - ri : ri;
      const c = flipped ? 7 - ci : ci;

      const cell = document.createElement("div");
      cell.className = "chess-cell " + ((r + c) % 2 === 0 ? "light" : "dark");
      cell.dataset.r = r;
      cell.dataset.c = c;

      // Coordinate labels on edge squares (adjust for flip)
      if (ci === 0) {
        const rank = document.createElement("span");
        rank.className = "coord coord-rank";
        rank.textContent = flipped ? r + 1 : 8 - r;
        cell.appendChild(rank);
      }
      if (ri === 7) {
        const file = document.createElement("span");
        file.className = "coord coord-file";
        file.textContent = flipped ? "hgfedcba"[c] : "abcdefgh"[c];
        cell.appendChild(file);
      }

      // Piece symbol
      const p = state.board[r][c];
      if (p !== 0) {
        const sym = document.createElement("span");
        sym.textContent = SYMBOLS[Math.abs(p)][p > 0 ? 0 : 1];
        cell.appendChild(sym);
      }

      // Highlight last move
      if (state.lastMove) {
        const { from, to } = state.lastMove;
        if ((r === from[0] && c === from[1]) || (r === to[0] && c === to[1]))
          cell.classList.add("last-move");
      }

      // Highlight selected cell
      if (selectedCell && selectedCell[0] === r && selectedCell[1] === c)
        cell.classList.add("selected");

      // Highlight legal move targets
      if (legalMovesForSelected.some(m => m[2] === r && m[3] === c))
        cell.classList.add("legal-move");

      // Highlight king in check
      if (isKingInCheck(state.board, state.turn)) {
        if (p === state.turn * PIECES.K) cell.classList.add("in-check");
      }

      cell.addEventListener("click", onCellClick);
      boardEl.appendChild(cell);
    }
  }

  // Captured pieces
  const fmt = pieces => pieces.map(p => SYMBOLS[Math.abs(p)][p > 0 ? 0 : 1]).join("");
  document.getElementById("capWhite").textContent = fmt(state.capturedByWhite);
  document.getElementById("capBlack").textContent = fmt(state.capturedByBlack);

  // Stats bar
  document.getElementById("statTurn").textContent   = state.turn === WHITE ? "White" : "Black";
  document.getElementById("statMove").textContent   = state.moveNumber;

  reportEmbedHeight();
}

function reportEmbedHeight() {
  const root = document.documentElement;
  const body = document.body;
  const card = document.querySelector(".project-card");
  const height = card
    ? Math.ceil(card.getBoundingClientRect().height)
    : Math.ceil(Math.max(root ? root.scrollHeight : 0, body ? body.scrollHeight : 0));

  if (window.self !== window.top && body) {
    root.style.overflowY = "hidden";
    body.style.overflowY = "hidden";
  }

  window.parent.postMessage({ type: "resize", height }, "*");
}

function setStatus(msg) {
  document.getElementById("statusBox").textContent = msg;
  reportEmbedHeight();
}


// ================================================================
// === HUMAN INPUT
// ================================================================

function onCellClick(e) {
  if (state.gameOver) return;
  if (state.mode === "bvb") return;
  // In hvb mode, only allow clicks on the human's turn
  if (state.turn !== state.humanColor) return;

  const r = parseInt(e.currentTarget.dataset.r);
  const c = parseInt(e.currentTarget.dataset.c);
  const piece = state.board[r][c];

  if (selectedCell) {
    // Try to make a move to this cell
    const move = legalMovesForSelected.find(m => m[2] === r && m[3] === c);
    if (move) {
      applyMove(move);
      selectedCell = null;
      legalMovesForSelected = [];
      renderBoard();
      const over = checkGameOver();
      if (over) { handleGameOver(over); return; }
      // Bot responds
      setStatus("Bot thinking...");
      setTimeout(doBotMove, 50);
      return;
    }
    // Clicked same or different friendly piece — deselect or reselect
    selectedCell = null;
    legalMovesForSelected = [];
  }

  // Select a friendly piece (only human's own pieces)
  if (Math.sign(piece) === state.humanColor) {
    selectedCell = [r, c];
    legalMovesForSelected = getLegalMoves(state.board, state.turn, state.castling, state.enPassant)
      .filter(m => m[0] === r && m[1] === c);
  }

  renderBoard();
}


// ================================================================
// === BOT MOVE
// ================================================================

let botTimeout = null;

function doBotMove() {
  if (state.gameOver) return;

  const depth = parseInt(document.getElementById("depthInput").value) || 3;
  const move  = findBestMove(state.board, state.turn, state.castling, state.enPassant, depth);

  if (!move) {
    // No moves — game over already handled
    return;
  }

  applyMove(move);
  renderBoard();
  if (state.mode === "hvb") {
    setStatus(state.turn === state.humanColor ? "Your turn" : "Bot thinking...");
  }

  const over = checkGameOver();
  if (over) { handleGameOver(over); return; }

  // In BvB mode, schedule the next move
  if (state.mode === "bvb") {
    botTimeout = setTimeout(doBotMove, state.botSpeed);
  } else {
    setStatus("Your turn (White)");
  }
}


// ================================================================
// === GAME OVER
// ================================================================

function handleGameOver(result) {
  state.gameOver = true;
  if (result.type === "checkmate") {
    setStatus(`Checkmate! ${result.winner} wins!`);
    if (result.winner === "White") wins.white++;
    else                           wins.black++;
  } else {
    setStatus("Stalemate — Draw!");
    wins.draw++;
  }
  saveWins(wins);
  updateWinDisplay();

  // In Bot vs Bot mode, automatically restart after a short pause
  if (state.mode === "bvb") {
    botTimeout = setTimeout(() => {
      newGame();
    }, 2000); // 2 second pause so the result is visible before reset
  }
}

function updateWinDisplay() {
  document.getElementById("winsWhite").textContent = wins.white;
  document.getElementById("winsBlack").textContent = wins.black;
  document.getElementById("winsDraw").textContent  = wins.draw;
}

// Show the colour picker (HvB) or start immediately (BvB)
function newGame() {
  clearTimeout(botTimeout);

  if (state.mode === "hvb") {
    // Show colour picker — game starts only after player chooses
    showColourPicker();
    return;
  }

  // BvB: start immediately
  startGame();
}

// Actually initialise and start the game
function startGame() {
  clearTimeout(botTimeout);
  state.board           = makeInitialBoard();
  state.turn            = WHITE;
  state.castling        = [true, true, true, true];
  state.enPassant       = null;
  state.halfMove        = 0;
  state.moveNumber      = 1;
  state.history         = [];
  state.lastMove        = null;
  state.capturedByWhite = [];
  state.capturedByBlack = [];
  state.gameOver        = false;
  selectedCell          = null;
  legalMovesForSelected = [];

  document.getElementById("statNodes").textContent = "—";
  document.getElementById("statDepth").textContent = "—";
  document.getElementById("statEval").textContent  = "—";

  renderBoard();

  if (state.mode === "bvb") {
    setStatus("Bot vs Bot running...");
    botTimeout = setTimeout(doBotMove, state.botSpeed);
  } else {
    // If human plays Black, bot (White) goes first
    if (state.humanColor === BLACK) {
      setStatus("Bot thinking...");
      botTimeout = setTimeout(doBotMove, 50);
    } else {
      setStatus("Your turn (White)");
    }
  }
}

function showColourPicker() {
  document.getElementById("colourPicker").style.display = "flex";
  document.getElementById("boardArea").style.display    = "none";
  reportEmbedHeight();
}

function hideColourPicker() {
  document.getElementById("colourPicker").style.display = "none";
  document.getElementById("boardArea").style.display    = "flex";
  reportEmbedHeight();
}


// ================================================================
// === WEIGHTS UI
// ================================================================

function buildPieceValueGrid() {
  const grid = document.getElementById("pieceValueGrid");
  grid.innerHTML = "";
  const pieceInfo = [
    { type:1, sym:"♙", name:"Pawn"   },
    { type:2, sym:"♘", name:"Knight" },
    { type:3, sym:"♗", name:"Bishop" },
    { type:4, sym:"♖", name:"Rook"   },
    { type:5, sym:"♕", name:"Queen"  },
  ];
  for (const { type, sym, name } of pieceInfo) {
    const cell = document.createElement("div");
    cell.className = "pv-cell";
    cell.innerHTML = `
      <span class="pv-symbol">${sym}</span>
      <span class="pv-name">${name}</span>
      <input class="pv-input" type="number" value="${weights.pieceValues[type]}" data-type="${type}" />
    `;
    grid.appendChild(cell);
  }
  grid.querySelectorAll(".pv-input").forEach(inp => {
    inp.addEventListener("change", () => {
      const type = parseInt(inp.dataset.type);
      const val  = parseInt(inp.value);
      if (!isNaN(val)) {
        weights.pieceValues[type] = val;
        saveWeights(weights);
      }
    });
  });
}

function buildPstSelect() {
  const sel = document.getElementById("pstSelect");
  sel.innerHTML = "";
  const names = { 1:"Pawn", 2:"Knight", 3:"Bishop", 4:"Rook", 5:"Queen", 6:"King" };
  for (const type of [1,2,3,4,5,6]) {
    const opt = document.createElement("option");
    opt.value = type;
    opt.textContent = names[type];
    sel.appendChild(opt);
  }
  sel.addEventListener("change", () => buildPstGrid(parseInt(sel.value)));
  buildPstGrid(1);
}

function buildPstGrid(type) {
  const grid = document.getElementById("pstGrid");
  grid.innerHTML = "";
  const pst = weights.pst[type];
  const flat = pst.flat();
  const min = Math.min(...flat), max = Math.max(...flat);

  for (let r = 0; r < 8; r++) {
    for (let c = 0; c < 8; c++) {
      const val = pst[r][c];
      // Colour cells by value: negative = reddish, positive = greenish
      const t   = (val - min) / (max - min + 0.001);
      const bg  = `rgba(${Math.round(200*(1-t))}, ${Math.round(200*t)}, 50, 0.3)`;
      const cell = document.createElement("div");
      cell.className = "pst-cell";
      cell.style.background = bg;
      const inp = document.createElement("input");
      inp.className   = "pst-input";
      inp.type        = "number";
      inp.value       = val;
      inp.dataset.r   = r;
      inp.dataset.c   = c;
      inp.dataset.pst = type;
      inp.addEventListener("change", () => {
        const v = parseInt(inp.value);
        if (!isNaN(v)) {
          weights.pst[type][r][c] = v;
          saveWeights(weights);
          buildPstGrid(type); // re-render to update colours
        }
      });
      cell.appendChild(inp);
      grid.appendChild(cell);
    }
  }
}


// ================================================================
// === UI CONTROLS
// ================================================================

// Inner tabs (Board / Weights)
document.querySelectorAll(".inner-tab").forEach(btn => {
  btn.addEventListener("click", () => {
    document.querySelectorAll(".inner-tab").forEach(b => b.classList.remove("active"));
    document.querySelectorAll(".inner-panel").forEach(p => p.classList.remove("active"));
    btn.classList.add("active");
    document.getElementById("panel-" + btn.dataset.itab).classList.add("active");
    requestAnimationFrame(reportEmbedHeight);
  });
});

// Colour picker buttons
document.getElementById("btnPlayWhite").addEventListener("click", () => {
  state.humanColor = WHITE;
  hideColourPicker();
  startGame();
});
document.getElementById("btnPlayBlack").addEventListener("click", () => {
  state.humanColor = BLACK;
  hideColourPicker();
  startGame();
});

// Mode buttons
document.getElementById("btnHvB").addEventListener("click", () => {
  document.getElementById("btnHvB").classList.add("active");
  document.getElementById("btnBvB").classList.remove("active");
  state.mode = "hvb";
  clearTimeout(botTimeout);
  newGame();
});
document.getElementById("btnBvB").addEventListener("click", () => {
  document.getElementById("btnBvB").classList.add("active");
  document.getElementById("btnHvB").classList.remove("active");
  state.mode = "bvb";
  newGame();
});

// New game / undo
document.getElementById("btnNewGame").addEventListener("click", newGame);
document.getElementById("btnUndo").addEventListener("click", () => {
  if (state.mode === "hvb") {
    undoMove(); // undo bot move
    undoMove(); // undo human move
  } else {
    undoMove();
  }
  clearTimeout(botTimeout);
  state.gameOver = false;
  renderBoard();
  setStatus("Your turn (White)");
});

// Speed buttons
document.querySelectorAll("[data-speed]").forEach(btn => {
  btn.addEventListener("click", () => {
    document.querySelectorAll("[data-speed]").forEach(b => b.classList.remove("active"));
    btn.classList.add("active");
    state.botSpeed = parseInt(btn.dataset.speed);
  });
});

// Reset weights
document.getElementById("btnResetWeights").addEventListener("click", () => {
  weights.pieceValues = JSON.parse(JSON.stringify(DEFAULT_PIECE_VALUES));
  weights.pst         = JSON.parse(JSON.stringify(DEFAULT_PST));
  saveWeights(weights);
  buildPieceValueGrid();
  buildPstGrid(parseInt(document.getElementById("pstSelect").value));
});


// ================================================================
// === BOOT
// ================================================================

updateWinDisplay();
buildPieceValueGrid();
buildPstSelect();
newGame(); // shows colour picker in hvb mode
