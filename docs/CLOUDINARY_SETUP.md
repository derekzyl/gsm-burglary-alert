# Cloudinary Image Storage Setup

## Overview
The burglary alert system now uses Cloudinary for image storage instead of local filesystem. Images are automatically deleted after 6 hours to save storage space.

## Configuration

### Backend Environment Variables
Already configured in `.env`:
```bash
CLOUDINARY_API_KEY=148834358482124
CLOUDINARY_API_SECRET=0pdvv7IpwYsCSZLH6NVOa9qAgi0
CLOUDINARY_CLOUD_NAME=paypaddy
IMAGE_RETENTION_HOURS=6
```

### ESP32 Devices
Both ESP32 main and ESP32-CAM have been configured with:
- **WiFi SSID**: cybergenii
- **WiFi Password**: 12341234
- **Backend URL**: https://xenophobic-netta-cybergenii-1584fde7.koyeb.app

## How It Works

### Image Upload Flow
1. ESP32-CAM captures image
2. Uploads to backend via HTTPS
3. Backend receives image and uploads to Cloudinary
4. Cloudinary returns:
   - Full image URL (1600x1200)
   - Thumbnail URL (200x150, auto-generated)
5. Backend stores URLs in database
6. Image forwarded to Telegram (if configured)

### Automatic Cleanup
Images older than 6 hours are automatically deleted from:
- Cloudinary cloud storage
- Database records

## Cleanup Task Setup

### Manual Cleanup
Run manually:
```bash
cd backend
python cleanup_task.py
```

### Automated Cleanup (Recommended)

**Option 1: Cron Job (Linux/Mac)**
```bash
# Edit crontab
crontab -e

# Add this line to run every hour
0 * * * * cd /path/to/backend && python cleanup_task.py >> /var/log/burglary_cleanup.log 2>&1
```

**Option 2: Systemd Timer (Linux)**
Create `/etc/systemd/system/burglary-cleanup.service`:
```ini
[Unit]
Description=Burglary Alert Image Cleanup
After=network.target

[Service]
Type=oneshot
User=your_username
WorkingDirectory=/path/to/backend
ExecStart=/usr/bin/python3 cleanup_task.py
Environment="PATH=/path/to/venv/bin:/usr/bin"
```

Create `/etc/systemd/system/burglary-cleanup.timer`:
```ini
[Unit]
Description=Run burglary cleanup every hour

[Timer]
OnCalendar=hourly
Persistent=true

[Install]
WantedBy=timers.target
```

Enable and start:
```bash
sudo systemctl enable burglary-cleanup.timer
sudo systemctl start burglary-cleanup.timer
```

**Option 3: FastAPI Background Task**
Add to `main.py` (already in your backend):
```python
from fastapi_utils.tasks import repeat_every

@app.on_event("startup")
@repeat_every(seconds=3600)  # Every hour
async def cleanup_images():
    from burglary_alert.utils.storage import storage
    from database import get_db
    
    db = next(get_db())
    try:
        storage.cleanup_old_images(db)
    finally:
        db.close()
```

## Installation

Install Cloudinary SDK:
```bash
cd backend
pip install cloudinary
# OR
pip install -r requirements_cloudinary.txt
```

## Verification

### Test Upload
```bash
# Upload test image
curl -X POST "https://xenophobic-netta-cybergenii-1584fde7.koyeb.app/api/v1/burglary/image/image" \
  -H "X-API-Key: esp32_device_key_xyz789" \
  -F "file=@test_image.jpg"
```

### Check Cloudinary Dashboard
1. Login to Cloudinary: https://cloudinary.com
2. Navigate to Media Library
3. Look for `burglary_alerts` folder
4. Verify images are uploaded

### Monitor Cleanup
```bash
# Check cleanup logs
tail -f /var/log/burglary_cleanup.log

# OR check systemd logs
journalctl -u burglary-cleanup.service
```

## Storage Limits

Cloudinary Free Tier:
- **Storage**: 25GB
- **Bandwidth**: 25GB/month
- **Transformations**: 25,000/month

With 6-hour retention:
- Average ~150KB per image
- Max ~4 images/day = ~600KB/day
- Monthly storage: ~18MB (well within limits)

## Benefits

✅ No local disk space issues
✅ Automatic thumbnail generation
✅ CDN delivery (faster image loading)
✅ Automatic cleanup (no manual intervention)
✅ Reliable cloud storage
✅ Easy to scale

## Troubleshooting

**Upload fails:**
- Check Cloudinary credentials in `.env`
- Verify account is active
- Check storage quota

**Cleanup not running:**
- Check cron/systemd timer is active
- Verify Python script has correct path to database
- Check logs for errors

**Images not deleted:**
- Verify `IMAGE_RETENTION_HOURS` in `.env`
- Check Cloudinary API key has delete permissions
- Manual cleanup: `python cleanup_task.py`

## Next Steps

After testing image upload:
1. Configure Telegram bot for notifications
2. Flash ESP32 devices with updated config
3. Test end-to-end flow
4. Enable automated cleanup (choose one option above)
