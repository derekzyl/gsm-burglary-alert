# Daily Image Cleanup Configuration

## ✅ Updates Made

### 1. Dedicated Cloudinary Folder
All burglary alert images are stored in:
```
cloudinary://paypaddy/burglary_alerts/
```

This folder is **completely separate** from your other projects. Images are organized as:
```
burglary_alerts/
├── 20260212_193000_capture_1707763800.jpg
├── 20260212_194500_capture_1707764700.jpg
└── ...
```

### 2. Daily Cleanup Schedule
- **Retention**: 24 hours (configurable)
- **Schedule**: Automatic daily cleanup via background task
- **First run**: 24 hours after backend starts
- **Subsequent runs**: Every 24 hours

### 3. Background Task Implementation
Added automated cleanup to `main.py`:
```python
@app.on_event("startup")
async def startup_event():
    # Runs cleanup every 24 hours automatically
    asyncio.create_task(daily_cleanup())
```

No external cron service needed! ✨

---

## 📋 Configuration

### Environment Variables (.env)
```bash
# Cloudinary Configuration
CLOUDINARY_CLOUD_NAME=paypaddy
CLOUDINARY_API_KEY=148834358482124
CLOUDINARY_API_SECRET=0pdvv7IpwYsCSZLH6NVOa9qAgi0

# Burglary Alert Settings
CLOUDINARY_FOLDER=burglary_alerts       # Dedicated folder
IMAGE_RETENTION_HOURS=24                 # Daily cleanup
```

### Customization Options

**Change retention period**:
```bash
IMAGE_RETENTION_HOURS=12   # Every 12 hours
IMAGE_RETENTION_HOURS=48   # Every 2 days
IMAGE_RETENTION_HOURS=168  # Weekly
```

**Change folder name** (if needed):
```bash
CLOUDINARY_FOLDER=my_security_system
```

---

## 🔄 How It Works

### Startup
1. Backend starts
2. Background task initialized
3. Waits 24 hours
4. Runs cleanup

### Daily Cleanup Process
```
Every 24 hours:
├── Find images older than 24 hours
├── Delete from Cloudinary (burglary_alerts folder)
├── Delete database records
└── Log: "Daily cleanup complete: X images deleted"
```

### Folder Isolation
```
Your Cloudinary Account:
├── burglary_alerts/        ← This project (isolated)
│   ├── image1.jpg
│   └── image2.jpg
├── other_project/          ← Other projects (separate)
│   └── photo.jpg
└── another_folder/         ← Not affected
    └── document.pdf
```

---

## 🧪 Testing

### Manual Test
```bash
# Trigger cleanup immediately (for testing)
cd backend
python cleanup_task.py
```

### Check Logs
Backend will log cleanup activity:
```
🧹 Running daily image cleanup...
✅ Daily cleanup complete: 3 images deleted
```

### Verify Cloudinary
1. Login to https://cloudinary.com
2. Go to Media Library
3. Check `burglary_alerts` folder
4. Old images (>24h) should be gone

---

## 🎯 Benefits

✅ **Isolated storage** - Won't interfere with other projects
✅ **Automatic cleanup** - No manual intervention needed
✅ **Daily schedule** - Runs every 24 hours
✅ **Built-in** - No external cron service required
✅ **Configurable** - Easy to adjust retention time
✅ **Reliable** - Retries on errors

---

## ⚙️ Advanced: Manual Cleanup Endpoint

If you want to trigger cleanup manually via API:

```python
# Add to main.py
@app.post("/api/v1/burglary/maintenance/cleanup")
async def manual_cleanup(db: Session = Depends(get_db)):
    """Manually trigger cleanup."""
    from burglary_alert.utils.storage import storage
    deleted = storage.cleanup_old_images(db)
    return {"status": "success", "deleted": deleted}
```

Call it:
```bash
curl -X POST "https://your-backend.koyeb.app/api/v1/burglary/maintenance/cleanup"
```

---

## 📊 Storage Estimate

With daily cleanup (24-hour retention):
- Average: 4 detections/day
- Image size: ~150KB each
- Daily storage: ~600KB
- **Total Cloudinary usage**: <1MB (well within free tier)

---

## 🔧 Monitoring

Check cleanup is working:
```bash
# View backend logs
# On Koyeb: Dashboard → App → Logs

# Should see every 24 hours:
# "🧹 Running daily image cleanup..."
# "✅ Daily cleanup complete: X images deleted"
```

---

## 🚀 Deployment

When you deploy to Koyeb:
1. Push updated code (already done locally)
2. Koyeb auto-deploys
3. Background task starts automatically
4. Cleanup runs daily

No additional setup needed! The task is built into your backend.

---

All set! Your images will now:
- ✅ Store in dedicated `burglary_alerts` folder
- ✅ Auto-delete after 24 hours
- ✅ Clean up daily without manual intervention
