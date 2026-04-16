# Portfolio Backend

FastAPI backend for the portfolio site.  
Handles the visitor counter and opt-in map pins.

---

## Running Locally

### 1. Install Python dependencies

```bash
cd backend
pip install -r requirements.txt
```

### 2. Start the server

```bash
uvicorn main:app --reload --port 8000
```

The API is now running at `http://localhost:8000`.

### 3. Browse the auto-generated API docs

FastAPI generates interactive documentation automatically.  
Open your browser and go to:

```
http://localhost:8000/docs
```

You can test every endpoint directly from this page — no Postman needed.

### 4. Open the frontend

Open `index.html` in your browser (or serve it with something like
`python -m http.server 5500` from the project root).

The frontend is configured to talk to `http://localhost:8000` by default.

---

## Project Structure

```
backend/
  main.py          ← FastAPI app and all route definitions
  database.py      ← SQLite helpers (connect, create tables, queries)
  requirements.txt ← Python dependencies
  Dockerfile       ← for Google Cloud Run deployment
data/
  portfolio.db     ← SQLite database (auto-created on first run)
```

---

## API Endpoints

| Method | Path      | Description                        |
|--------|-----------|------------------------------------|
| POST   | /visit    | Increment visit counter            |
| GET    | /visits   | Get current visit count            |
| POST   | /pins     | Save an opt-in map pin             |
| GET    | /pins     | Get all map pins                   |

Full interactive docs at `/docs` when the server is running.

---

## Deploying to Google Cloud Run

### Prerequisites

- [Google Cloud SDK](https://cloud.google.com/sdk/docs/install) installed
- A Google Cloud project created
- Docker installed (for building the container image)

### Steps

#### 1. Authenticate with Google Cloud

```bash
gcloud auth login
gcloud config set project YOUR_PROJECT_ID
```

#### 2. Build and push the container image

```bash
cd backend

gcloud builds submit --tag gcr.io/YOUR_PROJECT_ID/portfolio-backend
```

This builds the Docker image using Cloud Build (no local Docker needed)
and pushes it to Google Container Registry.

#### 3. Deploy to Cloud Run

```bash
gcloud run deploy portfolio-backend \
  --image gcr.io/YOUR_PROJECT_ID/portfolio-backend \
  --platform managed \
  --region us-central1 \
  --allow-unauthenticated
```

Cloud Run will give you a URL like:
```
https://portfolio-backend-xxxxxxxxxx-uc.a.run.app
```

#### 4. Update the frontend

In `index.html`, change the `BACKEND_URL` constant from:
```js
const BACKEND_URL = "http://localhost:8000";
```
to your Cloud Run URL:
```js
const BACKEND_URL = "https://portfolio-backend-xxxxxxxxxx-uc.a.run.app";
```

---

## Important: SQLite on Cloud Run

Cloud Run containers are **stateless** — the filesystem resets on every
deployment or cold start. This means the SQLite database file will be
wiped each time.

For persistent storage on Cloud Run you have two options:

### Option A — Google Cloud Firestore (recommended, free tier available)

Replace the SQLite calls in `database.py` with the
[Firestore Python client](https://cloud.google.com/firestore/docs/quickstart-servers).
The structure of `main.py` and all the routes stay exactly the same.

### Option B — Mount a persistent volume

Use [Cloud Run volume mounts](https://cloud.google.com/run/docs/configuring/services/volumes)
with a Cloud Filestore NFS share. The SQLite code stays unchanged —
you just point `DB_PATH` at the mounted volume path.

For a portfolio site, **Option A (Firestore)** is simpler and has a
generous free tier.

---

## CORS in Production

`main.py` currently allows all origins (`"*"`) for convenience during
development. Before going live, replace this with your actual domain:

```python
app.add_middleware(
    CORSMiddleware,
    allow_origins = ["https://yourdomain.com"],
    ...
)
```
