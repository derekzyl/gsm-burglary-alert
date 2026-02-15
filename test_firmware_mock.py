import time

import requests

# Config matches what is in config.h (assumed)
BASE_URL = "https://xenophobic-netta-cybergenii-1584fde7.koyeb.app/api/v1/burglary"
API_KEY = "esp32_device_key_xyz789"


def test_alert_upload():
    print("\n--- Testing Alert Upload (ESP32 Main Mock) ---")
    url = f"{BASE_URL}/alert/alert"
    headers = {"X-API-Key": API_KEY}

    # Simulate data sent by ESP32 Main
    payload = {
        "timestamp": int(time.time() * 1000),
        "detection_confidence": 0.95,
        "pir_left": True,
        "pir_middle": False,
        "pir_right": True,
        "network_status": "online",
    }

    try:
        print(f"Sending POST to {url}")
        response = requests.post(url, json=payload, headers=headers, timeout=10)
        print(f"Status: {response.status_code}")
        print(f"Response: {response.text}")

        if response.status_code == 200:
            return response.json().get("alert_id")
    except Exception as e:
        print(f"Error: {e}")
    return None


def test_image_upload():
    print("\n--- Testing Image Upload (ESP32-CAM Mock) ---")
    url = f"{BASE_URL}/image/image"
    headers = {"X-API-Key": API_KEY}

    # Create dummy image
    dummy_image = b"\xff\xd8\xff\xe0" + b"\x00" * 100  # Minimal JPEG header
    files = {"file": ("mock_cam_capture.jpg", dummy_image, "image/jpeg")}

    try:
        print(f"Sending POST to {url}")
        response = requests.post(url, files=files, headers=headers, timeout=20)
        print(f"Status: {response.status_code}")
        print(f"Response: {response.text}")
    except Exception as e:
        print(f"Error: {e}")


if __name__ == "__main__":
    test_alert_upload()
    test_image_upload()
