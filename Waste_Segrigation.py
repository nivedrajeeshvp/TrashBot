import cv2
import time
import serial
from ultralytics import YOLO
import numpy as np

# ---------- SERIAL SETUP ----------
PORT = '/dev/ttyUSB0'     # confirmed working
BAUD = 115200

ser = serial.Serial(PORT, BAUD, timeout=1)
ser.setDTR(False)   # prevent ESP32 reset
ser.setRTS(False)
time.sleep(2)
ser.reset_input_buffer()

print("Serial Connected on", PORT)

last_sent = ""

# ---------- YOLOv8 SETUP ----------
model = YOLO("yolov8n.pt")   # nano model (fast)

# -------- WASTE CLASSES --------
non = [
    "bottle",
    "cup",
    "fork",
    "knife",
    "spoon",
    "bowl",
    "cell phone",
    "book"
]

bio = [
    "banana",
    "apple",
    "orange",
    "broccoli",
    "carrot",
    "pizza",
    "sandwich",
    "cake"
]

vs = cv2.VideoCapture(0)

while True:

    ret, frame = vs.read()
    if not ret:
        break

    results = model(frame)

    current_waste = ""

    for r in results:

        boxes = r.boxes

        for box in boxes:

            conf = float(box.conf[0])
            cls = int(box.cls[0])

            class_name = model.names[cls]

            # -------- FILTER ONLY WASTE CLASSES --------
            if class_name not in bio and class_name not in non:
                continue

            x1,y1,x2,y2 = map(int,box.xyxy[0])

            if class_name in bio:
                current_waste = "A"    # BIO
                color = (0,255,0)
                label_text = "BIO WASTE"

            elif class_name in non:
                current_waste = "B"    # NON
                color = (0,0,255)
                label_text = "NON-BIO"

            cv2.rectangle(frame,(x1,y1),(x2,y2),color,2)

            cv2.putText(
                frame,
                label_text,
                (x1, y1 - 10),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.6,
                color,
                2
            )

    # -------- SERIAL SEND (UNCHANGED) --------
    if current_waste != "" and current_waste != last_sent:
        ser.write(current_waste.encode())   # single byte
        ser.flush()
        print("Sent:", current_waste)
        last_sent = current_waste

    cv2.imshow("Waste Segregation", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

vs.release()
ser.close()
cv2.destroyAllWindows()