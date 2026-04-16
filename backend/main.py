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
