# Production Deployment Summary

## ✅ Configuration Complete

### Backend (Koyeb Deployment)
**URL**: https://xenophobic-netta-cybergenii-1584fde7.koyeb.app

**Storage**: Cloudinary
- Cloud Name: paypaddy
- Auto-cleanup: 6 hours
- Folder: burglary_alerts/

**Environment Variables Set**:
```bash
CLOUDINARY_API_KEY=148834358482124
CLOUDINARY_API_SECRET=0pdvv7IpwYsCSZLH6NVOa9qAgi0
CLOUDINARY_CLOUD_NAME=paypaddy
IMAGE_RETENTION_HOURS=6
BURGLARY_USERNAME=admin
BURGLARY_PASSWORD=admin123
DEVICE_API_KEY=esp32_device_key_xyz789
```

### ESP32 Devices
**WiFi Configuration**:
- SSID: `cybergenii`
- Password: `12341234`

**ESP32 Main**: [config.h](file:///home/cybergenii/Desktop/codes/embedded/gsm-burglary-alarm/esp32-main/include/config.h)
- Backend URL: `https://xenophobic-netta-cybergenii-1584fde7.koyeb.app`
- Alert endpoint: `/api/v1/burglary/alert/alert`

**ESP32-CAM**: [config.h](file:///home/cybergenii/Desktop/codes/embedded/gsm-burglary-alarm/esp32-cam/include/config.h)
- Backend URL: `https://xenophobic-netta-cybergenii-1584fde7.koyeb.app/api/v1/burglary/image/image`
- Image upload: Direct HTTPS multipart POST

---

## 📦 Next Steps for Deployment

### 1. Install Cloudinary SDK
```bash
cd backend
pip install cloudinary
```

### 2. Deploy Backend Updates to Koyeb
```bash
# Ensure .env has all Cloudinary variables
# Push changes to git
git add .
git commit -m "Add Cloudinary storage integration"
git push origin main

# Koyeb will auto-deploy
```

### 3. Setup Automated Image Cleanup

**Option A: Cron Job** (if you have server access)
```bash
# Run every hour
0 * * * * cd /path/to/backend && python cleanup_task.py
```

**Option B: Koyeb doesn't support cron**
We need to trigger cleanup via an API endpoint that can be called by an external cron service.

Create endpoint in `main.py`:
```python
@app.post("/api/v1/burglary/maintenance/cleanup")
async def trigger_cleanup(db: Session = Depends(get_db)):
    """Cleanup old images (call via external cron)."""
    from burglary_alert.utils.storage import storage
    deleted = storage.cleanup_old_images(db)
    return {"status": "success", "deleted_count": deleted}
```

Then use external service like **cron-job.org** or **EasyCron**:
- URL: `https://xenophobic-netta-cybergenii-1584fde7.koyeb.app/api/v1/burglary/maintenance/cleanup`
- Method: POST
- Schedule: Every hour

### 4. Flash ESP32 Devices
```bash
# ESP32 Main
cd esp32-main
pio run -t upload

# ESP32-CAM
cd esp32-cam
pio run -t upload
```

### 5. Test End-to-End

**Test Alert**:
```bash
curl -X POST "https://xenophobic-netta-cybergenii-1584fde7.koyeb.app/api/v1/burglary/alert/alert" \
  -H "X-API-Key: esp32_device_key_xyz789" \
  -H "Content-Type: application/json" \
  -d '{
    "timestamp": '$(date +%s000)',
    "detection_confidence": 0.95,
    "pir_left": true,
    "pir_middle": true,
    "pir_right": false,
    "network_status": "online"
  }'
```

**Test Image Upload**:
```bash
curl -X POST "https://xenophobic-netta-cybergenii-1584fde7.koyeb.app/api/v1/burglary/image/image" \
  -H "X-API-Key: esp32_device_key_xyz789" \
  -F "file=@test_image.jpg"
```

**Login and View Feeds**:
```bash
# Get token
TOKEN=$(curl -X POST "https://xenophobic-netta-cybergenii-1584fde7.koyeb.app/api/v1/burglary/auth/login" \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"admin123"}' | jq -r '.access_token')

# View alerts
curl -X GET "https://xenophobic-netta-cybergenii-1584fde7.koyeb.app/api/v1/burglary/alert/feeds" \
  -H "Authorization: Bearer $TOKEN"
```

---

## 🔄 Key Changes Made

### Backend Changes
1. **Replaced** `burglary_alert/utils/storage.py` with Cloudinary implementation
2. **Updated** `burglary_alert/routers/images.py` to use Cloudinary upload
3. **Added** `cleanup_task.py` for scheduled cleanup
4. **Added** Cloudinary SDK dependency

### ESP32 Changes
1. **Updated** WiFi credentials to "cybergenii"
2. **Changed** backend URL to Koyeb HTTPS endpoint
3. **Ready to flash** - no code changes needed

### Documentation
- Created [CLOUDINARY_SETUP.md](file:///home/cybergenii/Desktop/codes/embedded/gsm-burglary-alarm/docs/CLOUDINARY_SETUP.md) - Complete guide
- Created deployment summary (this file)

---

## ⚠️ Important Notes

### Cloudinary Free Tier Limits
- **Storage**: 25GB (more than enough)
- **Bandwidth**: 25GB/month
- **Transformations**: 25,000/month
- With 6-hour retention, you'll use ~18MB/month

### Image Retention
- Images stored for **6 hours only**
- After 6 hours: Deleted from Cloudinary + database
- Telegram messages still have images (Telegram stores them)
- Adjust `IMAGE_RETENTION_HOURS` in .env if needed

### HTTPS vs HTTP
- Koyeb provides HTTPS automatically
- ESP32 now uses `https://` URLs
- SSL verification may need to be handled in ESP32 code

### Emergency Phone Number
Don't forget to set in ESP32 main `config.h`:
```cpp
#define EMERGENCY_PHONE "+1234567890"  // Your actual number
#define APN "your.apn.here"  // Your SIM operator's APN
```

---

## 🎯 Deployment Checklist

- [ ] Install Cloudinary SDK: `pip install cloudinary`
- [ ] Push backend changes to git
- [ ] Verify Koyeb deployment successful
- [ ] Setup external cron for cleanup (cron-job.org)
- [ ] Flash ESP32 main with new config
- [ ] Flash ESP32-CAM with new config
- [ ] Test alert posting from ESP32 main
- [ ] Test image upload from ESP32-CAM
- [ ] Verify images appear in Cloudinary dashboard
- [ ] Configure Telegram bot (optional)
- [ ] Test end-to-end motion detection
- [ ] Verify 6-hour cleanup works

---

## 📞 Support

If you encounter issues:
1. Check backend logs on Koyeb dashboard
2. Monitor ESP32 serial outputs
3. Verify Cloudinary dashboard for uploads
4. Test individual endpoints with curl
5. Check cleanup cron is running

Ready to deploy! 🚀
