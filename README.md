# ♻️ TrashBot - AI Powered Intelligent Waste Collection & Segregation Robot

![Python](https://img.shields.io/badge/Python-3.x-blue?logo=python)
![YOLOv8](https://img.shields.io/badge/YOLOv8-Ultralytics-green)
![Raspberry Pi](https://img.shields.io/badge/Raspberry%20Pi-5-red?logo=raspberrypi)
![ESP32](https://img.shields.io/badge/ESP32-IoT-orange)
![Flask](https://img.shields.io/badge/Flask-Web%20Dashboard-black?logo=flask)
![OpenCV](https://img.shields.io/badge/OpenCV-Computer%20Vision-blue?logo=opencv)
![License](https://img.shields.io/badge/License-MIT-yellow)

---

## 📖 Overview

**TrashBot** is an AI-powered autonomous waste collection and segregation robot designed for indoor environments such as educational institutions, offices, and hospitals.

The system combines embedded systems, robotics, computer vision, and machine learning to automatically detect, classify, collect, and segregate waste with minimal human intervention.

---

## ✨ Features

- 🤖 Autonomous indoor navigation
- 👀 Real-time waste detection using YOLOv8
- ♻️ Waste classification into biodegradable and non-biodegradable categories
- 🦾 3D-printed robotic arm for pick-and-place operation
- 📷 Raspberry Pi Camera based vision system
- ⚙️ ESP32-based embedded control
- 🌐 Flask dashboard for monitoring and control
- 🔋 Battery-powered portable system

---

# 🏗️ System Architecture

```
                 Raspberry Pi 5
                        │
        ┌───────────────┼───────────────┐
        │               │               │
   Pi Camera         LiDAR/ToF      Flask Dashboard
        │               │
        │               │
    YOLOv8 Object Detection
        │
Waste Classification
        │
ESP32 Controller
        │
Servo Motors + DC Motors
        │
Robotic Arm
        │
Waste Bin
```

---

# ⚙️ Hardware Used

- Raspberry Pi 5
- ESP32
- Raspberry Pi Camera
- LiDAR / ToF Sensor
- L298N Motor Driver
- MG996R Servo Motors
- DC Gear Motors
- Li-ion Battery
- Buck Converter
- 3D Printed Robotic Arm

---

# 💻 Software Stack

- Python
- OpenCV
- YOLOv8 (Ultralytics)
- Flask
- Embedded C
- Arduino IDE
- Git & GitHub

---

# 📂 Repository Structure

```
TrashBot/
│
├── docs/
│   └── Project_Report.pdf
│
├── images/
│
├── raspberry_pi/
│   ├── main.py
│   ├── detection.py
│   ├── navigation.py
│   ├── motor_controller.py
│   └── camera.py
│
├── esp32/
│   └── firmware.ino
│
├── flask_dashboard/
│   ├── app.py
│   ├── templates/
│   └── static/
│
├── requirements.txt
├── README.md
└── LICENSE
```

---

# 🔄 Working Flow

1. Robot patrols the environment.
2. LiDAR/ToF detects nearby objects.
3. Camera captures an image.
4. YOLOv8 classifies the object.
5. ESP32 controls the robotic arm.
6. Waste is picked up.
7. Waste is placed into the correct bin.

---

# 🚀 Installation

Clone the repository

```bash
git clone https://github.com/nivedrajeeshvp/TrashBot.git
```

Install dependencies

```bash
pip install -r requirements.txt
```

Run the application

```bash
python raspberry_pi/main.py
```

---

# 📸 Project Images

Add screenshots here after uploading them to the `images/` folder.

Example:

```markdown
![Robot](images/robot.jpg)

![Robotic Arm](images/robotic_arm.jpg)

![Dashboard](images/dashboard.png)
```

---

# 📈 Future Improvements

- Outdoor autonomous navigation
- GPS integration
- Mobile application
- Cloud monitoring
- Multi-robot coordination
- Smart city integration

---

# 👨‍💻 Team

Government Engineering College Barton Hill

Department of Electronics and Communication Engineering

- Archita R
- Hari Nanda B
- Nived Rajeesh V P
- Yadhuna V S

---

# 📄 Project Report

The complete project report is available in:

```
docs/Project_Report.pdf
```

---

# 📜 License

This project is licensed under the MIT License.

---

⭐ If you found this project interesting, consider giving it a **Star**!
