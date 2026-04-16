"use strict";

// ================================================================
// === CONFIGURATION
// ================================================================

const BACKEND_URL = "http://localhost:8000";

// Current keypair — stored in memory so encrypt/decrypt can reuse it
let currentKeys = {
  pub  : "",   // contents of rsa.pub
  priv : "",   // contents of rsa.priv
  bits : 0,
};

// Local fallback key material when backend is unavailable.
let localKeyMaterial = null;


// ================================================================
// === INNER TABS
// ================================================================

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

document.querySelectorAll(".inner-tab").forEach(btn => {
  btn.addEventListener("click", () => {
    document.querySelectorAll(".inner-tab").forEach(b => b.classList.remove("active"));
    document.querySelectorAll(".inner-panel").forEach(p => p.classList.remove("active"));
    btn.classList.add("active");
    document.getElementById("panel-" + btn.dataset.itab).classList.add("active");
    requestAnimationFrame(reportEmbedHeight);
  });
});


// ================================================================
// === UTILITY HELPERS
// ================================================================

function setStatus(msg, type = "") {
  const el = document.getElementById("rsaStatus");
  el.textContent  = msg;
  el.className    = "rsa-status " + type;
  document.getElementById("statStatus").textContent = msg || "Ready";
  reportEmbedHeight();
}

function setBusy(msg) {
  setStatus(msg + "...", "busy");
  ["btnEncrypt", "btnDecrypt", "btnGenKeys"].forEach(id => {
    document.getElementById(id).disabled = true;
  });
}

function clearBusy() {
  ["btnEncrypt", "btnDecrypt", "btnGenKeys"].forEach(id => {
    document.getElementById(id).disabled = false;
  });
}

// Append a line to the operation log
function logStep(msg, highlight = false, isError = false) {
  const log  = document.getElementById("stepLog");
  const line = document.createElement("div");
  line.className = "step-line" + (highlight ? " highlight" : "") + (isError ? " step-error" : "");
  line.textContent = msg;
  log.appendChild(line);
  reportEmbedHeight();
}

function clearLog() {
  document.getElementById("stepLog").innerHTML = "";
  reportEmbedHeight();
}

// Copy a textarea's content to clipboard
function copyText(id) {
  const el = document.getElementById(id);
  navigator.clipboard.writeText(el.value).catch(() => {});
}

// Count bits in a hex string (approximate: 4 bits per hex char)
function hexBits(hexStr) {
  if (!hexStr) return 0;
  return hexStr.replace(/\s/g, "").length * 4;
}

// Block size formula: floor((bits - 1) / 8)
function blockSize(bits) {
  return Math.floor((bits - 1) / 8);
}


// ================================================================
// === BACKEND + LOCAL FALLBACK CRYPTO HELPERS
// ================================================================

const BASE62_ALPHABET = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const SMALL_PRIMES = [
  2n, 3n, 5n, 7n, 11n, 13n, 17n, 19n, 23n, 29n,
  31n, 37n, 41n, 43n, 47n,
];

function bitLengthBigInt(n) {
  return n > 0n ? n.toString(2).length : 0;
}

function bytesToBigInt(bytes) {
  let value = 0n;
  for (const b of bytes) {
    value = (value << 8n) | BigInt(b);
  }
  return value;
}

function bigIntToBytes(value) {
  if (value === 0n) return new Uint8Array([0]);
  let hex = value.toString(16);
  if (hex.length % 2 !== 0) hex = "0" + hex;
  const out = new Uint8Array(hex.length / 2);
  for (let i = 0; i < out.length; i += 1) {
    out[i] = parseInt(hex.slice(i * 2, i * 2 + 2), 16);
  }
  return out;
}

function gcdBigInt(a, b) {
  let x = a < 0n ? -a : a;
  let y = b < 0n ? -b : b;
  while (y !== 0n) {
    const t = x % y;
    x = y;
    y = t;
  }
  return x;
}

function lcmBigInt(a, b) {
  return (a / gcdBigInt(a, b)) * b;
}

function modPow(base, exponent, modulus) {
  if (modulus === 1n) return 0n;
  let b = base % modulus;
  let e = exponent;
  let out = 1n;
  while (e > 0n) {
    if (e & 1n) out = (out * b) % modulus;
    e >>= 1n;
    b = (b * b) % modulus;
  }
  return out;
}

function modInverse(a, m) {
  let oldR = a;
  let r = m;
  let oldS = 1n;
  let s = 0n;

  while (r !== 0n) {
    const q = oldR / r;
    [oldR, r] = [r, oldR - q * r];
    [oldS, s] = [s, oldS - q * s];
  }

  if (oldR !== 1n) {
    throw new Error("No modular inverse exists for this key pair.");
  }

  return ((oldS % m) + m) % m;
}

function randomBitsBigInt(bits, options = {}) {
  const byteLen = Math.ceil(bits / 8);
  const bytes = new Uint8Array(byteLen);

  if (window.crypto && typeof window.crypto.getRandomValues === "function") {
    window.crypto.getRandomValues(bytes);
  } else {
    for (let i = 0; i < bytes.length; i += 1) {
      bytes[i] = Math.floor(Math.random() * 256);
    }
  }

  const extraBits = byteLen * 8 - bits;
  if (extraBits > 0) {
    bytes[0] &= (0xff >>> extraBits);
  }

  if (options.forceTopBit) {
    const topBit = 7 - extraBits;
    bytes[0] |= (1 << topBit);
  }

  if (options.forceOdd) {
    bytes[byteLen - 1] |= 1;
  }

  return bytesToBigInt(bytes);
}

function randomBetween(min, max) {
  if (max < min) throw new Error("Invalid random range.");
  const span = max - min + 1n;
  const bits = Math.max(1, bitLengthBigInt(span - 1n));
  let candidate;
  do {
    candidate = randomBitsBigInt(bits);
  } while (candidate >= span);
  return min + candidate;
}

function isProbablePrime(n, rounds) {
  if (n < 2n) return false;
  for (const p of SMALL_PRIMES) {
    if (n === p) return true;
    if (n % p === 0n) return false;
  }
  if (n % 2n === 0n) return false;

  let d = n - 1n;
  let s = 0;
  while ((d & 1n) === 0n) {
    d >>= 1n;
    s += 1;
  }

  const k = Math.max(8, rounds);
  for (let i = 0; i < k; i += 1) {
    const a = randomBetween(2n, n - 2n);
    let x = modPow(a, d, n);

    if (x === 1n || x === n - 1n) continue;

    let witness = true;
    for (let r = 1; r < s; r += 1) {
      x = (x * x) % n;
      if (x === n - 1n) {
        witness = false;
        break;
      }
    }

    if (witness) return false;
  }

  return true;
}

async function generatePrime(bits, rounds) {
  let tries = 0;
  while (true) {
    tries += 1;
    const candidate = randomBitsBigInt(bits, { forceTopBit: true, forceOdd: true });
    if (isProbablePrime(candidate, rounds)) {
      return candidate;
    }

    // Yield occasionally so the UI stays responsive during generation.
    if (tries % 20 === 0) {
      await new Promise((resolve) => setTimeout(resolve, 0));
    }
  }
}

function base62ToBigInt(text) {
  let out = 0n;
  for (const ch of text) {
    const idx = BASE62_ALPHABET.indexOf(ch);
    if (idx < 0) throw new Error("Username contains unsupported characters.");
    out = out * 62n + BigInt(idx);
  }
  return out;
}

function parseHexBigInt(raw) {
  const cleaned = (raw || "").trim().replace(/^0x/i, "");
  if (!cleaned || !/^[0-9a-fA-F]+$/.test(cleaned)) {
    throw new Error("Invalid key format: expected hexadecimal values.");
  }
  return BigInt("0x" + cleaned);
}

function parsePublicKeyString(raw) {
  const lines = (raw || "")
    .split(/\r?\n/)
    .map((line) => line.trim())
    .filter(Boolean);

  if (lines.length < 2) {
    throw new Error("Public key is missing required fields.");
  }

  return {
    n: parseHexBigInt(lines[0]),
    e: parseHexBigInt(lines[1]),
    s: lines[2] ? parseHexBigInt(lines[2]) : null,
    username: lines[3] || "localuser",
  };
}

function parsePrivateKeyString(raw) {
  const lines = (raw || "")
    .split(/\r?\n/)
    .map((line) => line.trim())
    .filter(Boolean);

  if (lines.length < 2) {
    throw new Error("Private key is missing required fields.");
  }

  return {
    n: parseHexBigInt(lines[0]),
    d: parseHexBigInt(lines[1]),
  };
}

function formatHex(value) {
  return value.toString(16).toUpperCase();
}

function formatLocalKeygenSteps(material) {
  return [
    "(a) username " + material.username,
    "(b) the signature s   (" + bitLengthBigInt(material.s) + ") " + formatHex(material.s),
    "(c) the first large prime p (" + bitLengthBigInt(material.p) + ") " + formatHex(material.p),
    "(d) the second large prime q  (" + bitLengthBigInt(material.q) + ") " + formatHex(material.q),
    "(e) the public modulus n  (" + bitLengthBigInt(material.n) + ") " + formatHex(material.n),
    "(f) the public exponent e  (" + bitLengthBigInt(material.e) + ") " + formatHex(material.e),
    "(g) the privatekey d (" + bitLengthBigInt(material.d) + ") " + formatHex(material.d),
  ];
}

async function generateKeysLocally(bits, iters) {
  const requestedBits = Math.max(64, Math.min(512, bits));
  const rounds = Math.max(8, Math.min(200, iters));
  const e = 65537n;
  const pBits = Math.floor(requestedBits / 2);
  const qBits = requestedBits - pBits;

  let p = 0n;
  let q = 0n;
  let n = 0n;
  let lambda = 0n;

  do {
    p = await generatePrime(pBits, rounds);
    do {
      q = await generatePrime(qBits, rounds);
    } while (q === p);

    n = p * q;
    lambda = lcmBigInt(p - 1n, q - 1n);
  } while (gcdBigInt(e, lambda) !== 1n);

  const d = modInverse(e, lambda);
  const username = "localuser";
  const m = base62ToBigInt(username) % n;
  const s = modPow(m, d, n);

  localKeyMaterial = { p, q, n, e, d, s, username };

  const pubKey = [formatHex(n), formatHex(e), formatHex(s), username, ""].join("\n");
  const privKey = [formatHex(n), formatHex(d), ""].join("\n");

  return {
    success: true,
    pub_key: pubKey,
    priv_key: privKey,
    steps: formatLocalKeygenSteps(localKeyMaterial),
    local: true,
  };
}

function encryptLocally(plaintext, pubKeyRaw) {
  const pub = parsePublicKeyString(pubKeyRaw);
  const bytes = new TextEncoder().encode(plaintext);
  const nBits = bitLengthBigInt(pub.n);
  const bytesPerBlock = Math.max(2, blockSize(nBits));
  const payloadBytes = bytesPerBlock - 1;

  const lines = [];
  for (let i = 0; i < bytes.length; i += payloadBytes) {
    const chunk = bytes.slice(i, i + payloadBytes);
    const packed = new Uint8Array(chunk.length + 1);
    packed[0] = 0xFF;
    packed.set(chunk, 1);

    const m = bytesToBigInt(packed);
    const c = modPow(m, pub.e, pub.n);
    lines.push(formatHex(c));
  }

  return lines.join("\n");
}

function decryptLocally(ciphertext, privKeyRaw) {
  const priv = parsePrivateKeyString(privKeyRaw);
  const lines = (ciphertext || "")
    .split(/\r?\n/)
    .map((line) => line.trim())
    .filter(Boolean);

  const out = [];
  for (const line of lines) {
    const c = parseHexBigInt(line);
    const m = modPow(c, priv.d, priv.n);
    let bytes = bigIntToBytes(m);

    if (bytes.length > 0 && bytes[0] === 0xFF) {
      bytes = bytes.slice(1);
    }

    for (const b of bytes) out.push(b);
  }

  return new TextDecoder().decode(new Uint8Array(out));
}

async function postBackend(path, payload) {
  const response = await fetch(BACKEND_URL + path, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(payload),
  });

  if (!response.ok) {
    throw new Error("Backend request failed (HTTP " + response.status + ").");
  }

  return response.json();
}


// ================================================================
// === PARSE VERBOSE KEYGEN OUTPUT
// Lines look like:
//   (a) username root
//   (b) the signature s  (17)  1A2B3C
//   (c) the first large prime p (67) DEADBEEF...
// ================================================================

function parseKeygenOutput(steps) {
  const vals = { n:"", e:"", d:"", p:"", q:"", s:"", username:"" };

  for (const line of steps) {
    const m = line.match(/\((\w)\)\s+(.+?)\s+\((\d+)\)\s+([0-9A-Fa-f]+)/);
    if (m) {
      const letter = m[1];
      const hexVal = m[4];
      const bits   = parseInt(m[3]);
      if (letter === "b") { vals.s = hexVal; updateKeyRow("s", hexVal, bits); }
      if (letter === "c") { vals.p = hexVal; updateKeyRow("p", hexVal, bits); }
      if (letter === "d") { vals.q = hexVal; updateKeyRow("q", hexVal, bits); }
      if (letter === "e") { vals.n = hexVal; updateKeyRow("n", hexVal, bits); }
      if (letter === "f") { vals.e = hexVal; updateKeyRow("e", hexVal, bits); }
      if (letter === "g") { vals.d = hexVal; updateKeyRow("d", hexVal, bits); }
    }
    // Username line: "(a) username root"
    const um = line.match(/\(a\)\s+username\s+(\S+)/);
    if (um) vals.username = um[1];
  }
  return vals;
}

// Update a key row in the Keys panel
function updateKeyRow(key, hexVal, bits) {
  const valEl  = document.getElementById("val-" + key);
  const bitsEl = document.getElementById("bits-" + key);
  if (valEl)  valEl.textContent  = hexVal || "—";
  if (bitsEl) bitsEl.textContent = bits ? bits + " bits" : "—";
}


// ================================================================
// === KEY GENERATION
// ================================================================

document.getElementById("btnGenKeys").addEventListener("click", async () => {
  const bits  = parseInt(document.getElementById("bitsInput").value)  || 128;
  const iters = parseInt(document.getElementById("itersInput").value) || 50;

  clearLog();
  logStep("// Generating " + bits + "-bit RSA keypair...", true);
  setBusy("Generating keys");

  try {
    let data;

    try {
      data = await postBackend("/rsa/keygen", { bits, iters });
    } catch {
      logStep("// Backend unavailable; switching to in-browser RSA.", true);
      data = await generateKeysLocally(bits, iters);
    }

    if (!data.success) {
      logStep("Error: " + data.error, false, true);
      setStatus("Key generation failed", "error");
      return;
    }

    // Store keys
    currentKeys.pub  = data.pub_key;
    currentKeys.priv = data.priv_key;

    // Update Keys panel
    document.getElementById("pubKeyArea").value  = data.pub_key;
    document.getElementById("privKeyArea").value = data.priv_key;

    // Parse verbose output and populate key rows
    const vals = parseKeygenOutput(data.steps || []);

    // Update stats bar
    const nBits = hexBits(vals.n);
    currentKeys.bits = nBits;
    document.getElementById("statBits").textContent  = nBits || bits;
    document.getElementById("statNLen").textContent  = (vals.n || "").length;
    document.getElementById("statBlock").textContent = blockSize(nBits || bits) + " bytes";

    // Log each step
    (data.steps || []).forEach((s, i) => logStep(s, i === 0));
    logStep(data.local ? "// Keys ready (local mode)." : "// Keys ready.", true);
    setStatus(data.local ? "Keys generated (local)" : "Keys generated", "ok");

  } catch (err) {
    logStep("Error: " + err.message, false, true);
    setStatus("Error", "error");
  } finally {
    clearBusy();
  }
});


// ================================================================
// === ENCRYPT
// ================================================================

document.getElementById("btnEncrypt").addEventListener("click", async () => {
  const plaintext = document.getElementById("plaintext").value;
  if (!plaintext) { setStatus("Enter plaintext first", "error"); return; }
  if (!currentKeys.pub) { setStatus("Generate keys first", "error"); return; }

  clearLog();
  logStep("// Encrypting with public key (n, e)...", true);
  setBusy("Encrypting");

  // Show the math being done
  logStep("1. Read public key (n, e) from rsa.pub");
  logStep("2. Verify username signature: s^e mod n == username");
  logStep("3. Split plaintext into blocks of " + (blockSize(currentKeys.bits) - 1) + " bytes");
  logStep("4. Prepend 0xFF to each block (avoids leading-zero issues)");
  logStep("5. For each block m: compute c = m^e mod n");
  logStep("6. Write each c as hex string...");

  try {
    let data;

    try {
      data = await postBackend("/rsa/encrypt", { plaintext, pub_key: currentKeys.pub });
    } catch {
      logStep("// Backend unavailable; encrypting in-browser.", true);
      data = {
        success: true,
        result: encryptLocally(plaintext, currentKeys.pub),
        local: true,
      };
    }

    if (!data.success) {
      logStep("Error: " + data.error, false, true);
      setStatus("Encryption failed", "error");
      return;
    }

    document.getElementById("ciphertext").value = data.result;
    logStep("// Encryption complete. " + data.result.split("\n").filter(Boolean).length + " block(s).", true);
    setStatus(data.local ? "Encrypted (local)" : "Encrypted", "ok");

  } catch (err) {
    logStep("Error: " + err.message, false, true);
    setStatus("Error", "error");
  } finally {
    clearBusy();
  }
});


// ================================================================
// === DECRYPT
// ================================================================

document.getElementById("btnDecrypt").addEventListener("click", async () => {
  const ciphertext = document.getElementById("ciphertext").value;
  if (!ciphertext) { setStatus("No ciphertext to decrypt", "error"); return; }
  if (!currentKeys.priv) { setStatus("Generate keys first", "error"); return; }

  clearLog();
  logStep("// Decrypting with private key (n, d)...", true);
  setBusy("Decrypting");

  logStep("1. Read private key (n, d) from rsa.priv");
  logStep("2. Read each hex line as integer c");
  logStep("3. For each block c: compute m = c^d mod n");
  logStep("4. mpz_export() converts integer back to bytes");
  logStep("5. Strip the prepended 0xFF byte from each block");
  logStep("6. Write remaining bytes to output...");

  try {
    let data;

    try {
      data = await postBackend("/rsa/decrypt", { ciphertext, priv_key: currentKeys.priv });
    } catch {
      logStep("// Backend unavailable; decrypting in-browser.", true);
      data = {
        success: true,
        result: decryptLocally(ciphertext, currentKeys.priv),
        local: true,
      };
    }

    if (!data.success) {
      logStep("Error: " + data.error, false, true);
      setStatus("Decryption failed", "error");
      return;
    }

    document.getElementById("plaintext").value = data.result;
    logStep("// Decryption complete.", true);
    setStatus(data.local ? "Decrypted (local)" : "Decrypted", "ok");

  } catch (err) {
    logStep("Error: " + err.message, false, true);
    setStatus("Error", "error");
  } finally {
    clearBusy();
  }
});


// ================================================================
// === MANUAL KEY FILE EDITING
// Let the user paste in their own key files
// ================================================================

document.getElementById("pubKeyArea").addEventListener("change", () => {
  currentKeys.pub = document.getElementById("pubKeyArea").value;
  localKeyMaterial = null;
  logStep("// Public key updated from Keys panel.", true);
});

document.getElementById("privKeyArea").addEventListener("change", () => {
  currentKeys.priv = document.getElementById("privKeyArea").value;
  localKeyMaterial = null;
  logStep("// Private key updated from Keys panel.", true);
});


// ================================================================
// === BOOT
// ================================================================

logStep("// RSA Cryptography - backend first, in-browser fallback enabled.");
logStep("// Click 'New Keys' to generate a keypair, then encrypt or decrypt.");
