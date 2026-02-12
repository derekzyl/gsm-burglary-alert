# Backend Testing and Deployment Guide

## Development Server

Start the backend development server:

```bash
cd backend
uvicorn main:app --reload --host 0.0.0.0 --port 8000
```

Access the API documentation at: http://localhost:8000/docs

## Testing Authentication

1. **Login Endpoint**:
```bash
curl -X POST "http://localhost:8000/api/v1/burglary/auth/login" \
  -H "Content-Type: application/json" \
  -d '{"username": "admin", "password": "admin123"}'
```

2. **Get Alerts (requires authentication)**:
```bash
# Replace TOKEN with the access_token from login
curl -X GET "http://localhost:8000/api/v1/burglary/alert/feeds" \
  -H "Authorization: Bearer TOKEN"
```

##Testing Device Endpoints

**Send Alert from ESP32** (simulated):
```bash
curl -X POST "http://localhost:8000/api/v1/burglary/alert/alert" \
  -H "Content-Type: application/json" \
  -H "X-API-Key: esp32_device_key_xyz789" \
  -d '{
    "timestamp": 1707756000000,
    "detection_confidence": 0.95,
    "pir_left": true,
    "pir_middle": true,
    "pir_right": false,
    "network_status": "online"
  }'
```

## Environment Variables

Required in `.env`:
```bash
BURGLARY_USERNAME=admin
BURGLARY_PASSWORD=admin123
BURGLARY_SECRET_KEY=your-secret-key-change-in-production-please
DEVICE_API_KEY=esp32_device_key_xyz789
DATABASE_URL="postgresql://..."
```

## Production Deployment

1. Install dependencies:
```bash
pip install -r requirements_burglary.txt
```

2. Set strong passwords in production `.env`

3. Use production WSGI server:
```bash
gunicorn main:app --workers 4 --worker-class uvicorn.workers.UvicornWorker --bind 0.0.0.0:8000
```

4. Setup Nginx reverse proxy for HTTPS
