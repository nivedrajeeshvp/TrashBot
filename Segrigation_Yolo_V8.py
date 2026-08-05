import cv2
import time
import serial
from ultralytics import YOLO

# ---------- SERIAL SETUP ----------
PORT = '/dev/ttyUSB0'
BAUD = 115200

ser = serial.Serial(PORT, BAUD, timeout=1)
ser.setDTR(False)
ser.setRTS(False)
time.sleep(2)
ser.reset_input_buffer()

print("Serial Connected on", PORT)

# ---------- LOAD YOLOv8 ----------
model = YOLO("yolov8n.pt")

# -------- WASTE CLASSES --------
non = [
    "bottle","cup","fork","knife","spoon",
    "bowl","cell phone","book"
]

bio = [
    "banana","apple","orange","broccoli",
    "carrot","pizza","sandwich","cake"
]

# ---------- CAMERA ----------
vs = cv2.VideoCapture(0)

while True:

    ret, frame = vs.read()
    if not ret:
        break

    results = model(frame)

    detected_class = ""

    for r in results:

        boxes = r.boxes

        for box in boxes:

            cls = int(box.cls[0])
            class_name = model.names[cls]

            x1, y1, x2, y2 = map(int, box.xyxy[0])

            # -------- BIO --------
            if class_name in bio:
                detected_class = "A"
                color = (0,255,0)
                label_text = "BIO WASTE"

            # -------- NON BIO --------
            elif class_name in non:
                detected_class = "B"
                color = (0,0,255)
                label_text = "NON-BIO"

            # -------- OTHER OBJECT --------
            else:
                detected_class = "C"
                color = (0,255,255)
                label_text = "OTHER OBJECT"

            # Draw box
            cv2.rectangle(frame,(x1,y1),(x2,y2),color,2)

            # Label
            cv2.putText(
                frame,
                label_text,
                (x1,y1-10),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.6,
                color,
                2
            )

    # ---------- SERIAL SEND ----------
    if detected_class != "":
        ser.write(detected_class.encode())
        ser.flush()
        print("Sent:", detected_class)

        time.sleep(0.5)

    cv2.imshow("Waste Segregation", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

vs.release()
ser.close()
cv2.destroyAllWindows()