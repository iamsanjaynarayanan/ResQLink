# ResQLink

### Intelligent Vehicular Emergency Communication System

> An IoT-based vehicle accident detection, emergency alert, and real-time vehicle tracking system using ESP32, MPU6050, NEO-6M GPS and SIM900A GSM.

---

## Overview

Road accidents remain one of the leading causes of fatalities worldwide. In many situations, victims are unable to communicate their location to emergency responders due to injury, unconsciousness, or network delays.

**ResQLink** is designed to address this challenge by automatically detecting vehicle accidents using motion analysis, determining the vehicle's location through GPS, and transmitting emergency alerts through GSM and local host-based communication channels.

The system continuously monitors vehicle movement using an MPU6050 inertial measurement unit (IMU). Upon detecting a significant impact or rollover event, the system immediately sends an emergency SMS containing GPS coordinates and a Google Maps link. Simultaneously, accident information is transmitted to a local host-based backend for dashboard monitoring.

The system also includes a manual SOS button, enabling users to request assistance during emergencies even when no accident is detected.

---

## Objectives

* Detect vehicle accidents automatically
* Detect vehicle rollover events
* Provide manual SOS emergency functionality
* Transmit accident location through GSM SMS
* Send accident information to a local host server
* Enable real-time vehicle location monitoring
* Reduce emergency response time
* Improve road safety through automated communication

---

## Key Features

### Automatic Accident Detection

* Real-time impact monitoring
* Accelerometer and gyroscope fusion
* False trigger reduction using multiple thresholds

### Rollover Detection

* Roll angle monitoring
* Pitch angle monitoring
* Sustained tilt verification

### GPS Location Tracking

* NEO-6M GPS integration
* Live coordinate acquisition
* Google Maps link generation

### GSM Emergency Alert

* Automatic SMS alerts
* GPS coordinates included in message
* Works independently of web dashboard

### Local Host Connectivity

* HTTPS communication
* Backend API integration
* Dashboard notification support

### Manual SOS Feature

* Push-button emergency trigger
* Sends location instantly
* Useful during medical or security emergencies

---

## System Architecture

```text
MPU6050 + GPS + SOS Button
            │
            ▼
         ESP32
      (Main Controller)
            │
    ┌───────┴────────┐
    ▼                ▼
 SIM900A         Wi-Fi Module
    │                │
    ▼                ▼
 Emergency SMS    Backend API
                      │
                      ▼
                Web Dashboard
```

---

## Hardware Components

| Component            | Purpose                     |
| -------------------- | --------------------------- |
| ESP32 Dev Board      | Main Controller             |
| MPU6050              | Impact & Rollover Detection |
| NEO-6M GPS Module    | Location Tracking           |
| SIM900A GSM Module   | SMS Communication           |
| Push Button          | Manual SOS Trigger          |
| 18650 Battery        | GSM Power Supply            |
| 18650 Battery Holder | Battery Mounting            |
| Breadboard           | Prototyping                 |
| Jumper Wires         | Connections                 |
| Micro USB Cable      | ESP32 Programming & Power   |

---

## Software Technologies

| Technology        | Purpose               |
| ----------------- | --------------------- |
| Arduino IDE       | Firmware Development  |
| Embedded C++      | ESP32 Programming     |
| ESP32 Wi-Fi Stack | Internet Connectivity |
| HTTPClient        | HTTPS Communication   |
| GPS NMEA Parsing  | Location Processing   |
| JSON              | Data Exchange Format  |
| REST API          | Backend Communication |
| Ngrok             | Public API Exposure   |
| Postman           | API Testing           |
| GitHub            | Version Control       |

---

## Working Methodology

### 1. Sensor Data Acquisition

The ESP32 continuously reads:

* Accelerometer X, Y, Z values
* Gyroscope X, Y, Z values
* GPS coordinates
* SOS button state

---

### 2. Impact Detection

The system calculates:

* Acceleration magnitude
* Change in acceleration (Delta-G)
* Angular velocity magnitude

An accident is declared when acceleration and rotational motion exceed predefined thresholds.

---

### 3. Rollover Detection

The system calculates:

* Roll angle
* Pitch angle

If either angle exceeds the threshold continuously for 1 second, a rollover event is detected.

---

### 4. Emergency Response

Upon accident detection:

1. GPS location is retrieved
2. Emergency SMS is generated
3. Google Maps link is created
4. SMS is sent through SIM900A
5. Accident details are transmitted to backend server
6. Dashboard receives alert notification

---

## Mathematical Model

The accident detection and rollover detection algorithms are based on accelerometer and gyroscope measurements obtained from the MPU6050 sensor.

The system performs:

- Accelerometer conversion from raw sensor values to g-force
- Gyroscope conversion from raw sensor values to angular velocity
- Acceleration magnitude calculation
- Delta-G computation
- Gyroscope magnitude calculation
- Roll angle estimation
- Pitch angle estimation
- Threshold-based accident classification

Detailed equations, derivations, threshold values, and detection logic are available in:

Documents/Formulae_and_Thresholds.md

## Repository Structure

```text
ResQLink/
│
├── Firmware/
│   └── ResQLink.ino
│
├── Documents/
│   ├── Block_Diagram.png
│   ├── Flow_Diagram.png
│   ├── Hardware_Setup.jpg
│   ├── Prototype.jpg
│   ├── BOM.md
│   ├── Formulae_and_Thresholds.md
│   └── Backend_Architecture.md
│
├── Results/
│   ├── Accident_Detection_Graph.png
│   ├── Rollover_Detection_Graph.png
│   ├── SMS_Alert.png
│   ├── Incoming_Call.png
│   ├── Dashboard_Homepage.png
│   ├── User_Registration.png
│   └── User_Dashboard.png
│
├── Presentation/
│   ├── InnovSense_2026_Presentation.pptx
│   ├── InnovSense_2026_Poster.pdf
│   └── ProtoStudio_Project_Report.pdf
│
├── Media/
│   └── Demonstration.md
│
├── References/
│   ├── References/
│   └── References.md
│
├── LICENSE
└── README.md

```

---

## Results

The prototype successfully demonstrated:

* Impact-based accident detection
* Rollover detection
* GPS location acquisition
* Automated emergency SMS alerts
* Local Host-based accident reporting
* Live location tracking
* Dashboard visualization
* Manual SOS functionality

---

## Future Enhancements

* Integration with emergency response services
* Mobile application support
* Cloud database integration
* Historical route tracking
* Fleet management support
* AI-based accident classification
* LoRa communication for remote regions
* Integration with smart city infrastructure

---

## References

This project was developed after reviewing multiple research papers related to:

* Vehicle accident detection
* IoT-based emergency systems
* GPS vehicle tracking
* Vehicular communication systems
* Emergency response automation

Complete references are available in:

```text
References/References.md
```

---

## Authors

**Sanjay Narayanan V** and 
**Prawin VS**, 
Electronics and Communication Engineering

**Project:** ResQLink - Intelligent Vehicular Emergency Communication System

---

## License

This project is licensed under the MIT License.

---

> *ResQLink aims to reduce emergency response time by automatically detecting accidents, sharing precise location information, and enabling rapid communication between victims and responders.* 🚑📡🌍
