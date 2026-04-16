"""
database.py
-----------
Handles all SQLite interactions.

The database file lives at ../data/portfolio.db relative to this file.
It is created automatically on first run — no setup needed.

Tables:
  visits  — single-row counter of total page visits
  pins    — one row per visitor-dropped map pin
"""

import sqlite3
import os

# ----------------------------------------------------------------
# Path to the database file
# ----------------------------------------------------------------

# __file__ is backend/database.py, so we go up one level to get data/
BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DB_PATH  = os.path.join(BASE_DIR, "data", "portfolio.db")


# ----------------------------------------------------------------
# Connection helper
# ----------------------------------------------------------------

def get_connection() -> sqlite3.Connection:
    """Open a connection to the SQLite database.

    check_same_thread=False is needed because FastAPI runs handlers
    in threads from a thread pool — SQLite is fine with this as long
    as each request uses its own connection, which we ensure by
    opening and closing one per request.
    """
    conn = sqlite3.connect(DB_PATH, check_same_thread=False)
    conn.row_factory = sqlite3.Row  # rows behave like dicts
    return conn


# ----------------------------------------------------------------
# Schema creation (runs once on startup)
# ----------------------------------------------------------------

def init_db():
    """Create tables if they don't already exist."""
    os.makedirs(os.path.dirname(DB_PATH), exist_ok=True)

    conn = get_connection()
    try:
        cursor = conn.cursor()

        # visits: a single row that holds the running total
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS visits (
                id    INTEGER PRIMARY KEY CHECK (id = 1),
                count INTEGER NOT NULL DEFAULT 0
            )
        """)

        # Seed the visits row if it doesn't exist yet
        cursor.execute("""
            INSERT OR IGNORE INTO visits (id, count) VALUES (1, 0)
        """)

        # pins: one row per visitor who opted in to the map
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS pins (
                id         INTEGER PRIMARY KEY AUTOINCREMENT,
                lat        REAL    NOT NULL,
                lng        REAL    NOT NULL,
                label      TEXT    NOT NULL DEFAULT 'A visitor',
                created_at TEXT    NOT NULL DEFAULT (datetime('now'))
            )
        """)

        conn.commit()
    finally:
        conn.close()


# ----------------------------------------------------------------
# Visit counter
# ----------------------------------------------------------------

def increment_visit_count() -> int:
    """Add 1 to the visit counter and return the new total."""
    conn = get_connection()
    try:
        conn.execute("UPDATE visits SET count = count + 1 WHERE id = 1")
        conn.commit()
        row = conn.execute("SELECT count FROM visits WHERE id = 1").fetchone()
        return row["count"]
    finally:
        conn.close()


def get_visit_count() -> int:
    """Return the current visit count without changing it."""
    conn = get_connection()
    try:
        row = conn.execute("SELECT count FROM visits WHERE id = 1").fetchone()
        return row["count"]
    finally:
        conn.close()


# ----------------------------------------------------------------
# Pins
# ----------------------------------------------------------------

def insert_pin(lat: float, lng: float, label: str) -> dict:
    """Save a new pin and return its full record."""
    conn = get_connection()
    try:
        cursor = conn.execute(
            "INSERT INTO pins (lat, lng, label) VALUES (?, ?, ?)",
            (lat, lng, label)
        )
        conn.commit()
        row = conn.execute(
            "SELECT * FROM pins WHERE id = ?", (cursor.lastrowid,)
        ).fetchone()
        return dict(row)
    finally:
        conn.close()


def get_all_pins() -> list[dict]:
    """Return all pins, newest first."""
    conn = get_connection()
    try:
        rows = conn.execute(
            "SELECT * FROM pins ORDER BY created_at DESC"
        ).fetchall()
        return [dict(row) for row in rows]
    finally:
        conn.close()
