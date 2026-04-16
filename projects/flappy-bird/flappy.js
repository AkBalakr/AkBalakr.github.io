"use strict";

// ================================================================
// === CONFIGURATION
// ================================================================

const CFG = {
  // Genetic algorithm
  POPULATION_SIZE : 5,     // overridden by the pop input on reset
  ELITE_COUNT     : 3,     // top N birds copied unchanged into next generation
  MUTATION_RATE   : 0.15,  // chance each weight gets nudged
  MUTATION_AMOUNT : 0.4,   // how big that nudge can be

  // Neural network dimensions
  INPUT_COUNT  : 3,        // [dist to pipe, dist to gap top, dist to gap bottom]
  HIDDEN_COUNT : 6,
  OUTPUT_COUNT : 1,        // >0.5 means flap

  // Physics
  GRAVITY      : 0.5,      // added to vertical velocity every frame
  FLAP_FORCE   : -8,       // instant upward velocity on flap
  PIPE_SPEED   : 3,        // pixels pipes move left per frame
  PIPE_GAP     : 130,      // vertical space between top and bottom pipe
  PIPE_WIDTH   : 52,

  // Pipe spawning
  // First pipe appears immediately at reset.
  // Additional pipes spawn every PIPE_EVERY frames.
  PIPE_EVERY   : 110,

  // Bird
  BIRD_X       : 80,       // fixed horizontal position on screen
  BIRD_SIZE    : 16,
};

// One unique color per bird slot — stays consistent across generations
const BIRD_COLORS = [
  "#f0c84a", "#4af0c8", "#f04a4a", "#4a8af0", "#c84af0",
  "#f08c4a", "#4af04a", "#f04ac8", "#4ac8f0", "#8cf04a",
  "#a04af0", "#f0a04a", "#4af0a0", "#f04aa0", "#4aa0f0",
  "#f0f04a", "#4af0f0", "#f04af0", "#a0f04a", "#4ac8c8",
];


// ================================================================
// === NEURAL NETWORK
//
// A simple feedforward network: inputs -> hidden layer -> output.
//
// All weights are stored in one flat array, laid out as:
//   [ input->hidden weights | hidden biases | hidden->output weights | output bias ]
//
// Keeping weights flat makes crossover and mutation trivial —
// the GA just slices and nudges numbers in the array.
// ================================================================

class NeuralNet {
  constructor(weights = null) {
    const I = CFG.INPUT_COUNT;
    const H = CFG.HIDDEN_COUNT;
    const O = CFG.OUTPUT_COUNT;

    // Precompute where each section starts in the flat array
    this.offsets = {
      ih: 0,                   // input->hidden weights
      hb: I * H,               // hidden biases
      ho: I * H + H,           // hidden->output weights
      ob: I * H + H + H * O,  // output bias
    };
    this.totalWeights = I * H + H + H * O + O;
    this.weights = weights ? weights.slice() : this._randomWeights();
  }

  // Initialise all weights randomly between -1 and 1
  _randomWeights() {
    return Array.from({ length: this.totalWeights }, () => Math.random() * 2 - 1);
  }

  // Run a forward pass.
  // inputs: array of CFG.INPUT_COUNT numbers
  // returns: array of CFG.OUTPUT_COUNT numbers, each in range [0, 1]
  forward(inputs) {
    const W = this.weights;
    const { ih, hb, ho, ob } = this.offsets;
    const I = CFG.INPUT_COUNT;
    const H = CFG.HIDDEN_COUNT;
    const O = CFG.OUTPUT_COUNT;

    // --- Hidden layer: weighted sum of inputs + bias, then ReLU ---
    const hidden = new Array(H);
    for (let h = 0; h < H; h++) {
      let sum = W[hb + h]; // bias
      for (let i = 0; i < I; i++) {
        sum += inputs[i] * W[ih + i * H + h];
      }
      hidden[h] = Math.max(0, sum); // ReLU activation
    }

    // --- Output layer: weighted sum of hidden + bias, then sigmoid ---
    const outputs = new Array(O);
    for (let o = 0; o < O; o++) {
      let sum = W[ob + o]; // bias
      for (let h = 0; h < H; h++) {
        sum += hidden[h] * W[ho + h * O + o];
      }
      outputs[o] = 1 / (1 + Math.exp(-sum)); // sigmoid activation
    }

    return outputs;
  }
}


// ================================================================
// === GENETIC ALGORITHM
// ================================================================

// Pick a bird at random, weighted by fitness.
// Birds with higher fitness are more likely to be selected as parents.
function weightedSelect(birds) {
  const total = birds.reduce((sum, b) => sum + b.fitness, 0);
  let r = Math.random() * total;
  for (const bird of birds) {
    r -= bird.fitness;
    if (r <= 0) return bird;
  }
  return birds[birds.length - 1];
}

// Combine two weight arrays by taking the first half from A
// and the second half from B, split at a random point.
function crossover(wA, wB) {
  const cut = Math.floor(Math.random() * wA.length);
  return [...wA.slice(0, cut), ...wB.slice(cut)];
}

// Randomly nudge a fraction of weights by a small amount.
// This lets the GA explore solutions the parents didn't have.
function mutate(weights) {
  return weights.map(w =>
    Math.random() < CFG.MUTATION_RATE
      ? w + (Math.random() * 2 - 1) * CFG.MUTATION_AMOUNT
      : w
  );
}

// Build the weight arrays for the next generation from the current one.
function evolve(birds) {
  // Sort best to worst by fitness
  birds.sort((a, b) => b.fitness - a.fitness);

  const next = [];

  // Elites: the top N birds pass through unchanged
  for (let i = 0; i < CFG.ELITE_COUNT && i < birds.length; i++) {
    next.push(birds[i].brain.weights.slice());
  }

  // Fill the rest of the population with crossover + mutation offspring
  while (next.length < CFG.POPULATION_SIZE) {
    const parentA = weightedSelect(birds);
    const parentB = weightedSelect(birds);
    next.push(mutate(crossover(parentA.brain.weights, parentB.brain.weights)));
  }

  return next;
}


// ================================================================
// === BIRD
// ================================================================

class Bird {
  constructor(brainWeights, colorIndex) {
    this.x       = CFG.BIRD_X;
    this.y       = 170;          // starts vertically centred on the canvas
    this.vy      = 0;            // vertical velocity (positive = falling down)
    this.alive   = true;
    this.fitness = 0;            // frames survived + pipe-passing bonuses
    this.brain   = new NeuralNet(brainWeights);
    this.color   = BIRD_COLORS[colorIndex % BIRD_COLORS.length];
  }

  // Instant upward kick, like tapping in the real game
  flap() {
    this.vy = CFG.FLAP_FORCE;
  }

  // Apply gravity, move, and earn one fitness point for surviving this frame
  update() {
    this.vy      += CFG.GRAVITY;
    this.y       += this.vy;
    this.fitness += 1;
  }

  // Ask the neural net what to do, then flap if output > 0.5
  think(inputs) {
    if (this.brain.forward(inputs)[0] > 0.5) this.flap();
  }

  // Hitbox — slightly inset so the visual square and collision feel fair
  get hitLeft()   { return this.x + 2; }
  get hitRight()  { return this.x + CFG.BIRD_SIZE - 2; }
  get hitTop()    { return this.y + 2; }
  get hitBottom() { return this.y + CFG.BIRD_SIZE - 2; }
}


// ================================================================
// === PIPE PAIR
//
// Each PipePair is two solid green rectangles:
//   Top pipe:    y=0 down to y=gapTop
//   Bottom pipe: y=gapBottom down to y=canvasHeight
//
// The bird must fly through the gap between them.
// Both pipes move left together at PIPE_SPEED px/frame.
// ================================================================

class PipePair {
  constructor(canvasW, canvasH) {
    this.x      = canvasW;        // spawns just off the right edge
    this.width  = CFG.PIPE_WIDTH;
    this.passed = false;          // flipped to true once a bird clears it

    // Randomise the vertical position of the gap, with margins so
    // the gap is never right at the very top or bottom of the screen
    const margin    = 60;
    this.gapTop    = margin + Math.random() * (canvasH - CFG.PIPE_GAP - margin * 2);
    this.gapBottom = this.gapTop + CFG.PIPE_GAP;
    this.canvasH   = canvasH;
  }

  // Move left by PIPE_SPEED pixels each frame
  update() {
    this.x -= CFG.PIPE_SPEED;
  }

  // True once the pipe has scrolled completely off the left edge
  isOffScreen() {
    return this.x + this.width < 0;
  }

  // Returns true if the bird's hitbox overlaps either pipe rectangle
  collides(bird) {
    // Step 1: is the bird horizontally inside the pipe column?
    const overlapX = bird.hitRight > this.x && bird.hitLeft < this.x + this.width;
    if (!overlapX) return false;

    // Step 2: is the bird outside the gap (touching top or bottom pipe)?
    const hitsTopPipe    = bird.hitTop    < this.gapTop;
    const hitsBottomPipe = bird.hitBottom > this.gapBottom;
    return hitsTopPipe || hitsBottomPipe;
  }

  // Draw the top pipe (down from ceiling) and bottom pipe (up from floor)
  draw(ctx) {
    ctx.fillStyle   = "#3a9e4a";
    ctx.strokeStyle = "#2a7a38";
    ctx.lineWidth   = 2;

    // Top pipe: from y=0 down to where the gap begins
    ctx.fillRect(this.x, 0, this.width, this.gapTop);
    ctx.strokeRect(this.x, 0, this.width, this.gapTop);

    // Bottom pipe: from where the gap ends down to canvas bottom
    ctx.fillRect(this.x, this.gapBottom, this.width, this.canvasH - this.gapBottom);
    ctx.strokeRect(this.x, this.gapBottom, this.width, this.canvasH - this.gapBottom);
  }
}


// ================================================================
// === SIMULATION STATE
// ================================================================

const canvas = document.getElementById("gameCanvas");
const ctx    = canvas.getContext("2d");
const W      = canvas.width;
const H      = canvas.height;

const sim = {
  birds       : [],
  pipes       : [],
  generation  : 0,
  bestFitEver : 0,
  frameCount  : 0,
  pipesPassed : 0,
  running     : false,
  speed       : 1,     // ticks to run per animation frame (for fast-forward)
  animId      : null,
};


// ================================================================
// === SETUP
// ================================================================

// Spawn a fresh population of birds.
// If weightsArray is given, use those weights; otherwise randomise.
function spawnPopulation(weightsArray = null) {
  sim.birds = Array.from({ length: CFG.POPULATION_SIZE }, (_, i) =>
    new Bird(weightsArray ? weightsArray[i] : null, i)
  );
  sim.pipes      = [];
  sim.frameCount = 0;
  sim.pipesPassed = 0;

  // Spawn one pipe immediately so birds have something to react to from frame 1
  sim.pipes.push(new PipePair(W, H));
}

// Full reset: read population size from input, clear all stats, respawn.
// NOTE: we do NOT cancel the animation loop here.
// loop() runs forever — cancelling and restarting it risks running two
// loops at once. We just reset the state and let the loop keep going.
function resetSim() {
  const inputEl      = document.getElementById("popSize");
  const requestedSize = parseInt(inputEl.value) || 5;
  CFG.POPULATION_SIZE = Math.max(1, Math.min(50, requestedSize));

  sim.running     = false;
  sim.generation  = 0;
  sim.bestFitEver = 0;

  spawnPopulation();
  updateHUD();
  document.getElementById("btnStart").textContent = "> START";
}


// ================================================================
// === SENSORS
//
// Computes the three normalised inputs fed into each bird's neural net
// every frame. We always look at the NEXT pipe (the one not yet passed).
//
// Dividing by W or H keeps values roughly in [0, 1] so no single
// input dominates just because of its units.
// ================================================================

function getSensorInputs(bird) {
  // Find the next upcoming pipe — the first one whose right edge
  // is still ahead of the bird's left hitbox edge
  const pipe = sim.pipes.find(p => p.x + p.width > bird.hitLeft);

  if (!pipe) {
    // No pipe on screen — return safe neutral values
    return [1, 0.5, 0.5];
  }

  const birdCY = bird.y + CFG.BIRD_SIZE / 2;

  // Input 1: horizontal distance from bird's front to pipe's left edge
  const distToPipe   = (pipe.x - bird.hitRight) / W;

  // Input 2: vertical distance from bird centre UP to the gap's top edge
  //          positive = bird is below the top pipe, negative = above it
  const distToGapTop = (birdCY - pipe.gapTop) / H;

  // Input 3: vertical distance from bird centre DOWN to the gap's bottom edge
  //          positive = gap bottom is below the bird, negative = bird fell through
  const distToGapBot = (pipe.gapBottom - birdCY) / H;

  return [distToPipe, distToGapTop, distToGapBot];
}


// ================================================================
// === TICK — advance the simulation by one frame
// ================================================================

function tick() {
  sim.frameCount++;

  // Spawn additional pipes on a regular schedule
  if (sim.frameCount % CFG.PIPE_EVERY === 0) {
    sim.pipes.push(new PipePair(W, H));
  }

  // Move all pipes left, remove any that have gone off screen
  sim.pipes.forEach(p => p.update());
  sim.pipes = sim.pipes.filter(p => !p.isOffScreen());

  // Update every living bird
  sim.birds.filter(b => b.alive).forEach(bird => {
    // 1. Brain reads sensors, decides whether to flap
    bird.think(getSensorInputs(bird));

    // 2. Apply gravity and move vertically
    bird.update();

    // 3. Kill if bird hits the ceiling or floor
    if (bird.y < 0 || bird.y + CFG.BIRD_SIZE > H) {
      bird.alive = false;
      return;
    }

    // 4. Kill if bird hits any pipe
    for (const pipe of sim.pipes) {
      if (pipe.collides(bird)) {
        bird.alive = false;
        return;
      }
    }
  });

  // Check if the lead bird has cleared any pipes
  const leadBird = getBestLivingBird();
  sim.pipes.forEach(pipe => {
    if (!pipe.passed && leadBird && leadBird.hitLeft > pipe.x + pipe.width) {
      pipe.passed = true;
      sim.pipesPassed++;
      // Bonus fitness for every bird still alive when a pipe is cleared
      sim.birds.filter(b => b.alive).forEach(b => b.fitness += 200);
    }
  });

  // If all birds are dead, end the generation and evolve
  if (!sim.birds.some(b => b.alive)) {
    endGeneration();
  }
}


// ================================================================
// === GENERATION MANAGEMENT
// ================================================================

function endGeneration() {
  const bestThisGen = Math.max(...sim.birds.map(b => b.fitness));
  if (bestThisGen > sim.bestFitEver) sim.bestFitEver = bestThisGen;

  sim.generation++;

  const nextWeights = evolve(sim.birds);
  spawnPopulation(nextWeights);
}


// ================================================================
// === DRAW — render one frame to the canvas
// ================================================================

function getBestLivingBird() {
  const alive = sim.birds.filter(b => b.alive);
  if (!alive.length) return null;
  return alive.reduce((best, b) => b.fitness > best.fitness ? b : best, alive[0]);
}

function draw() {
  // 1. Clear with background colour
  ctx.fillStyle = "#0d0d14";
  ctx.fillRect(0, 0, W, H);

  // 2. Draw all pipe pairs (green rectangles scrolling right to left)
  sim.pipes.forEach(p => p.draw(ctx));

  // 3. Draw all birds
  //    - Dead birds: dark grey
  //    - Alive birds: their unique colour
  //    - Best living bird: white outline so it's easy to track
  const best = getBestLivingBird();

  sim.birds.forEach(bird => {
    if (!bird.alive) {
      ctx.fillStyle = "#2a2a3a";
      ctx.fillRect(bird.x, bird.y, CFG.BIRD_SIZE, CFG.BIRD_SIZE);
      return;
    }

    ctx.fillStyle = bird.color;
    ctx.fillRect(bird.x, bird.y, CFG.BIRD_SIZE, CFG.BIRD_SIZE);

    if (bird === best) {
      ctx.strokeStyle = "#ffffff";
      ctx.lineWidth   = 2;
      ctx.setLineDash([]);
      ctx.strokeRect(bird.x, bird.y, CFG.BIRD_SIZE, CFG.BIRD_SIZE);
    }
  });

  // 4. Draw sensor lines from the best living bird to the next pipe
  const nextPipe = best && sim.pipes.find(p => p.x + p.width > best.hitLeft);

  if (best && nextPipe) {
    const bx = best.x + CFG.BIRD_SIZE / 2;
    const by = best.y + CFG.BIRD_SIZE / 2;
    const px = nextPipe.x;
    const pw = nextPipe.width;

    ctx.lineWidth = 1.5;
    ctx.setLineDash([4, 4]);

    // Yellow line: horizontal distance from bird to pipe's left edge (input 1)
    ctx.strokeStyle = "#f0c84a";
    ctx.beginPath(); ctx.moveTo(bx, by); ctx.lineTo(px, by); ctx.stroke();

    // Cyan line: bird centre up to gap top edge (input 2)
    ctx.strokeStyle = "#4af0c8";
    ctx.beginPath(); ctx.moveTo(px + pw / 2, by); ctx.lineTo(px + pw / 2, nextPipe.gapTop); ctx.stroke();

    // Purple line: bird centre down to gap bottom edge (input 3)
    ctx.strokeStyle = "#c84af0";
    ctx.beginPath(); ctx.moveTo(px + pw / 2 + 5, by); ctx.lineTo(px + pw / 2 + 5, nextPipe.gapBottom); ctx.stroke();

    ctx.setLineDash([]);

    // Update the live readouts in the sidebar
    const [d1, d2, d3] = getSensorInputs(best);
    document.getElementById("p1").textContent = d1.toFixed(3);
    document.getElementById("p2").textContent = d2.toFixed(3);
    document.getElementById("p3").textContent = d3.toFixed(3);

  } else {
    document.getElementById("p1").textContent = "—";
    document.getElementById("p2").textContent = "—";
    document.getElementById("p3").textContent = "—";
  }
}


// ================================================================
// === HUD
// ================================================================

function updateHUD() {
  const alive = sim.birds.filter(b => b.alive).length;
  document.getElementById("statGen").textContent   = sim.generation;
  document.getElementById("statAlive").textContent = alive;
  document.getElementById("statBest").textContent  = sim.bestFitEver;
  document.getElementById("statPipes").textContent = sim.pipesPassed;
}


// ================================================================
// === MAIN LOOP
// ================================================================

function loop() {
  if (sim.running) {
    // Run multiple ticks per frame when speed > 1 (fast-forward mode)
    for (let i = 0; i < sim.speed; i++) {
      tick();
    }
  }

  draw();
  updateHUD();

  requestAnimationFrame(loop);
}


// ================================================================
// === UI CONTROLS
// ================================================================

document.getElementById("btnStart").addEventListener("click", () => {
  sim.running = !sim.running;
  document.getElementById("btnStart").textContent = sim.running ? "|| PAUSE" : "> RESUME";
});

document.getElementById("btnReset").addEventListener("click", () => {
  resetSim();
});

const speedButtons = document.querySelectorAll(".speed-group button[data-speed]");

speedButtons.forEach(btn => {
  btn.addEventListener("click", () => {
    speedButtons.forEach(b => b.classList.remove("active"));
    btn.classList.add("active");
    sim.speed = parseInt(btn.dataset.speed);
  });
});


// ================================================================
// === BOOT
// ================================================================

resetSim(); // build the first population and draw the initial frame
loop();     // start the animation loop (simulation starts paused)
