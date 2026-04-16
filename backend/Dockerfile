# Dockerfile
# ----------
# Used by Google Cloud Run to containerise the FastAPI backend.
# Cloud Run expects the app to listen on the port in $PORT (default 8080).

FROM python:3.12-slim

# Set working directory inside the container
WORKDIR /app

# Install dependencies first (cached layer — only rebuilds when
# requirements.txt changes, not on every code change)
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy backend source code
COPY . .

# Create the data directory for the SQLite database
# Note: on Cloud Run you'll want to replace SQLite with Cloud Firestore
# or mount a persistent volume — see README.md for details.
RUN mkdir -p /data

# Start the server on the port Cloud Run provides via $PORT
CMD ["sh", "-c", "uvicorn main:app --host 0.0.0.0 --port ${PORT:-8080}"]
