"""
main.py
-------
FastAPI backend for the portfolio site.

Endpoints:
  POST /visit      — record a page visit, returns new total
  GET  /visits     — get the current visit count
  POST /pins       — save an opt-in map pin
  GET  /pins       — get all map pins

Run locally:
  uvicorn main:app --reload --port 8000

The frontend (index.html) talks to this server at http://localhost:8000.
When deployed to Google Cloud Run the URL changes but nothing else does.
"""

from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel, Field
import database

# ----------------------------------------------------------------
# App setup
# ----------------------------------------------------------------

app = FastAPI(
    title       = "Portfolio Backend",
    description = "Visitor counter and opt-in map pins for the portfolio site.",
    version     = "1.0.0",
)

# ----------------------------------------------------------------
# CORS
#
# CORS (Cross-Origin Resource Sharing) controls which websites are
# allowed to call this API from the browser.
#
# During local development the frontend is served from a different
# port than the backend (e.g. file:// or localhost:5500 vs :8000),
# so we need to allow those origins.
#
# In production, replace the origins list with your actual domain.
# ----------------------------------------------------------------

app.add_middleware(
    CORSMiddleware,
    allow_origins     = ["*"],   # replace with your domain in production
    allow_credentials = True,
    allow_methods     = ["*"],
    allow_headers     = ["*"],
)

# ----------------------------------------------------------------
# Startup — initialise the database on first run
# ----------------------------------------------------------------

@app.on_event("startup")
def on_startup():
    """Create database tables if they don't exist yet."""
    database.init_db()


# ----------------------------------------------------------------
# Pydantic models
#
# Pydantic validates incoming request bodies automatically.
# If a request is missing a field or sends the wrong type,
# FastAPI returns a clear 422 error without any extra code.
# ----------------------------------------------------------------

class PinCreate(BaseModel):
    lat   : float = Field(..., ge=-90,  le=90,  description="Latitude")
    lng   : float = Field(..., ge=-180, le=180, description="Longitude")
    label : str   = Field("A visitor", max_length=100)


# ----------------------------------------------------------------
# Routes — Visits
# ----------------------------------------------------------------

@app.post("/visit", summary="Record a page visit")
def record_visit():
    """Increment the visit counter by 1 and return the new total.

    Called once each time a user loads the portfolio homepage.
    No personal data is recorded — just a number.
    """
    new_count = database.increment_visit_count()
    return { "count": new_count }


@app.get("/visits", summary="Get total visit count")
def read_visits():
    """Return the current total visit count."""
    count = database.get_visit_count()
    return { "count": count }


# ----------------------------------------------------------------
# Routes — Pins
# ----------------------------------------------------------------

@app.post("/pins", summary="Drop an opt-in map pin", status_code=201)
def create_pin(pin: PinCreate):
    """Save a visitor's map pin.

    Only called when the user explicitly clicks 'Drop My Pin'
    and grants browser geolocation permission. Coordinates are
    already jittered ~5km on the frontend before being sent here,
    so no exact home addresses are stored.
    """
    saved = database.insert_pin(
        lat   = pin.lat,
        lng   = pin.lng,
        label = pin.label,
    )
    return saved


@app.get("/pins", summary="Get all map pins")
def read_pins():
    """Return all stored pins so the map can render them."""
    pins = database.get_all_pins()
    return { "pins": pins }


# ----------------------------------------------------------------
# RSA Cryptography Routes
# These compile and run the C programs in rsa_src/ via subprocess.
# Each request uses unique temp files so concurrent calls don't collide.
# ----------------------------------------------------------------

import subprocess, tempfile, os, shutil, uuid

# Path to the compiled RSA binaries (relative to backend/)
RSA_BIN = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "rsa_src")

class RSARequest(BaseModel):
    plaintext  : str = ""
    ciphertext : str = ""
    pub_key    : str = ""   # contents of rsa.pub
    priv_key   : str = ""   # contents of rsa.priv

class RSAResponse(BaseModel):
    success    : bool
    result     : str = ""
    pub_key    : str = ""
    priv_key   : str = ""
    steps      : list = []
    error      : str = ""

def run_cmd(cmd, cwd, timeout=30):
    """Run a shell command, return (stdout, stderr, returncode)."""
    r = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, timeout=timeout)
    return r.stdout, r.stderr, r.returncode

@app.post("/rsa/keygen", response_model=RSAResponse, summary="Generate RSA keypair")
def rsa_keygen():
    """
    Generates a fresh RSA keypair using the C keygen binary.
    Returns the public and private key file contents plus verbose key details.
    """
    tmpdir = tempfile.mkdtemp()
    try:
        # Run keygen with verbose output so we capture all key details
        stdout, stderr, rc = run_cmd(
            [os.path.join(RSA_BIN, "keygen"), "-b", "128", "-v",
             "-n", os.path.join(tmpdir, "rsa.pub"),
             "-d", os.path.join(tmpdir, "rsa.priv")],
            cwd=tmpdir
        )
        if rc != 0:
            return RSAResponse(success=False, error=stderr or "keygen failed")

        pub  = open(os.path.join(tmpdir, "rsa.pub")).read()  if os.path.exists(os.path.join(tmpdir, "rsa.pub"))  else ""
        priv = open(os.path.join(tmpdir, "rsa.priv")).read() if os.path.exists(os.path.join(tmpdir, "rsa.priv")) else ""

        # Parse verbose output into steps for the UI
        steps = [line.strip() for line in stdout.splitlines() if line.strip()]

        return RSAResponse(success=True, pub_key=pub, priv_key=priv, steps=steps)
    except Exception as ex:
        return RSAResponse(success=False, error=str(ex))
    finally:
        shutil.rmtree(tmpdir, ignore_errors=True)


@app.post("/rsa/encrypt", response_model=RSAResponse, summary="Encrypt plaintext with RSA public key")
def rsa_encrypt_route(req: RSARequest):
    """
    Encrypts plaintext using the provided RSA public key.
    Writes temp files, runs the C encrypt binary, returns ciphertext.
    """
    if not req.plaintext:
        return RSAResponse(success=False, error="No plaintext provided")
    if not req.pub_key:
        return RSAResponse(success=False, error="No public key provided")

    tmpdir = tempfile.mkdtemp()
    try:
        pub_path  = os.path.join(tmpdir, "rsa.pub")
        in_path   = os.path.join(tmpdir, "plain.txt")
        out_path  = os.path.join(tmpdir, "cipher.txt")

        open(pub_path, 'w').write(req.pub_key)
        open(in_path,  'w').write(req.plaintext)

        stdout, stderr, rc = run_cmd(
            [os.path.join(RSA_BIN, "encrypt"),
             "-n", pub_path, "-i", in_path, "-o", out_path],
            cwd=tmpdir
        )
        if rc != 0:
            return RSAResponse(success=False, error=stderr or "encrypt failed")

        ciphertext = open(out_path).read() if os.path.exists(out_path) else ""

        # Build step-by-step explanation
        steps = [
            "1. Read public key (n, e) from rsa.pub",
            "2. Verify username signature: s^e mod n == username",
            f"3. Plaintext converted to integer blocks (prepend 0xFF to each block)",
            "4. Each block: c = m^e mod n  (modular exponentiation)",
            "5. Ciphertext written as hex strings, one per line",
        ]
        return RSAResponse(success=True, result=ciphertext, steps=steps)
    except Exception as ex:
        return RSAResponse(success=False, error=str(ex))
    finally:
        shutil.rmtree(tmpdir, ignore_errors=True)


@app.post("/rsa/decrypt", response_model=RSAResponse, summary="Decrypt ciphertext with RSA private key")
def rsa_decrypt_route(req: RSARequest):
    """
    Decrypts ciphertext using the provided RSA private key.
    Writes temp files, runs the C decrypt binary, returns plaintext.
    """
    if not req.ciphertext:
        return RSAResponse(success=False, error="No ciphertext provided")
    if not req.priv_key:
        return RSAResponse(success=False, error="No private key provided")

    tmpdir = tempfile.mkdtemp()
    try:
        priv_path = os.path.join(tmpdir, "rsa.priv")
        in_path   = os.path.join(tmpdir, "cipher.txt")
        out_path  = os.path.join(tmpdir, "plain.txt")

        open(priv_path, 'w').write(req.priv_key)
        open(in_path,   'w').write(req.ciphertext)

        stdout, stderr, rc = run_cmd(
            [os.path.join(RSA_BIN, "decrypt"),
             "-n", priv_path, "-i", in_path, "-o", out_path],
            cwd=tmpdir
        )
        if rc != 0:
            return RSAResponse(success=False, error=stderr or "decrypt failed")

        plaintext = open(out_path).read() if os.path.exists(out_path) else ""

        steps = [
            "1. Read private key (n, d) from rsa.priv",
            "2. Read each hex line of ciphertext as integer c",
            "3. Each block: m = c^d mod n  (modular exponentiation)",
            "4. mpz_export() converts integer back to bytes",
            "5. Strip the prepended 0xFF byte from each block",
            "6. Write remaining bytes to output",
        ]
        return RSAResponse(success=True, result=plaintext, steps=steps)
    except Exception as ex:
        return RSAResponse(success=False, error=str(ex))
    finally:
        shutil.rmtree(tmpdir, ignore_errors=True)
