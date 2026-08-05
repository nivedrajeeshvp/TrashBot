from flask import Flask, render_template, Response, request, jsonify
import cv2
import serial
import serial.tools.list_ports
import threading
import time
from ultralytics import YOLO

app = Flask(__name__)

# --- Configuration ---
def find_esp32_port():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if 'USB' in port.description or 'UART' in port.description or 'CP210' in port.description or 'ACM' in port.device:
            return port.device
    return '/dev/ttyUSB0'

SERIAL_PORT = find_esp32_port()
BAUD_RATE = 115200
MODEL_PATH = 'yolov8n.pt'

# --- Global State ---
current_status = "IDLE"
current_dist = 0.0
last_detected = "NONE"
ser = None
patrol_active = False
bio_count = 0
non_bio_count = 0

# Initialize YOLO
print("Loading YOLOv8 on Raspberry Pi 5...")
model = YOLO(MODEL_PATH)

# Initialize Serial
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    print(f"✅ Connected to ESP32 on {SERIAL_PORT}")
except Exception as e:
    print(f"⚠️ Serial connection failed on {SERIAL_PORT}: {e}")

@app.route('/status')
def get_status():
    return jsonify(
        status=current_status,
        dist=current_dist,
        detected=last_detected,
        bio=bio_count,
        non_bio=non_bio_count
    )

# Camera Setup
cap = cv2.VideoCapture(0)

def serial_listener():
    global current_status, current_dist, ser
    while True:
        if ser and ser.in_waiting > 0:
            try:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line == "DETECTED":
                    current_status = "ANALYZING..."
                elif line.startswith("STATUS:"):
                    current_status = line.split(":", 1)[1]
                elif line.startswith("DIST:"):
                    try:
                        current_dist = float(line.split(":")[1])
                    except:
                        pass
            except:
                pass
        time.sleep(0.01)

threading.Thread(target=serial_listener, daemon=True).start()

def gen_frames():
    global current_status, ser, bio_count, non_bio_count, last_detected
    
    bio_classes = ['apple', 'orange', 'broccoli', 'carrot', 'banana', 'sandwich']
    non_bio_classes = ['bottle', 'cup', 'fork', 'knife', 'spoon', 'bowl', 'wine glass']

    while True:
        success, frame = cap.read()
        if not success:
            break
        
        # ONLY RUN AI IF ROBOT IS ANALYZING
        if current_status == "ANALYZING...":
            results = model(frame, verbose=False)
            frame_result = "UNKNOWN"
            detected_names = []

            for r in results:
                for box in r.boxes:
                    x1, y1, x2, y2 = map(int, box.xyxy[0])
                    cls_id = int(box.cls[0])
                    label = model.names[cls_id]
                    conf = float(box.conf[0])
                    detected_names.append(label)

                    color = (255, 255, 255)
                    if label in bio_classes:
                        color = (0, 255, 0); frame_result = "BIO"
                    elif label in non_bio_classes:
                        color = (0, 255, 255); frame_result = "NON-BIO"

                    # Draw BBox only during analysis
                    cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)
                    cv2.putText(frame, f"{label} {conf:.2f}", (x1, y1 - 10), 
                                cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 2)

            current_status = f"DETECTED: {frame_result}"
            last_detected = ", ".join(detected_names[:2]) if detected_names else "NONE"
            
            if frame_result == "BIO": bio_count += 1
            elif frame_result == "NON-BIO": non_bio_count += 1
            elif not detected_names: frame_result = "WALL"
                
            if ser:
                ser.write(f"{frame_result}\r\n".encode('ascii'))
                ser.flush()

        # HUD on Stream (Always Visible)
        cv2.rectangle(frame, (10, 10), (320, 110), (0, 0, 0), -1)
        cv2.rectangle(frame, (10, 10), (320, 110), (0, 255, 0), 2)
        cv2.putText(frame, f"STATUS: {current_status}", (20, 40), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)
        cv2.putText(frame, f"DIST: {current_dist}cm", (20, 70), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)
        cv2.putText(frame, f"BIO: {bio_count} | NON: {non_bio_count}", (20, 95), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 255), 1)

        ret, buffer = cv2.imencode('.jpg', frame)
        frame = buffer.tobytes()
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')

@app.route('/')
def index(): return render_template('index.html')

@app.route('/video_feed')
def video_feed(): return Response(gen_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route('/command/<cmd>')
def send_command(cmd):
    if ser:
        if cmd == 'start': ser.write(b"CMD:START\r\n")
        elif cmd == 'stop': ser.write(b"CMD:STOP\r\n")
        ser.flush()
        return jsonify(status="success")
    return jsonify(status="error")

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=False)
