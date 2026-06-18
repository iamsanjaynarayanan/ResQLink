# Formulae & Thresholds

This document details the mathematical models, sensor data conversions, and firmware threshold logic implemented in the ResQLink embedded system for real-time impact and rollover detection.

---

## 1. MPU6050 Sensor Register & Scale Configuration

To prevent sensor saturation during high-impact vehicular collisions, the MPU6050 Inertial Measurement Unit (IMU) is configured with specific full-scale ranges via its configuration registers.

### 1.1 Accelerometer Sensitivity Configuration
* **Configured Range:** $\pm 4\text{g}$
* **Sensitivity Scale Factor:** $8192 \text{ LSB/g}$

The raw 16-bit signed integers are converted to gravitational force units ($\text{g}$) using the following models:

$$A_x = \frac{Ax_{raw}}{8192}$$

$$A_y = \frac{Ay_{raw}}{8192}$$

$$A_z = \frac{Az_{raw}}{8192}$$

### 1.2 Gyroscope Sensitivity Configuration
* **Configured Range:** $\pm 250^\circ/\text{s}$
* **Sensitivity Scale Factor:** $131 \text{ LSB/}^\circ/\text{s}$

The raw angular velocity values are converted to degrees per second ($^\circ/\text{s}$) using the following models:

$$G_x = \frac{Gx_{raw}}{131}$$

$$G_y = \frac{Gy_{raw}}{131}$$

$$G_z = \frac{Gz_{raw}}{131}$$

---

## 2. Spatial Magnitude Calculations

To achieve orientation-independent crash tracking, the system evaluates the total vector magnitudes rather than isolated axial changes.

### 2.1 Total Acceleration Magnitude
The Euclidean norm calculates the combined acceleration force acting on the vehicle:

$$A_{total} = \sqrt{A_x^2 + A_y^2 + A_z^2}$$

### 2.2 Delta-G (Dynamic Impact Force)
Under static conditions, the vehicle experiences a constant gravitational pull of $1\text{g}$. Dynamic impact force ($\Delta G$) isolates external forces by removing steady-state gravity:

$$\Delta G = |A_{total} - 1|$$

### 2.3 Angular Velocity Magnitude
The net rotational speed of the vehicle across all three rotational axes is defined by:

$$G_{total} = \sqrt{G_x^2 + G_y^2 + G_z^2}$$

---

## 3. Vehicular Orientation Models

Attitude estimation tracks vehicle tilt relative to the ground plane to intercept structural rollovers.

### 3.1 Roll Angle Calculation

$$Roll = \arctan\left(\frac{A_y}{A_z}\right) \times \frac{180}{\pi}$$

### 3.2 Pitch Angle Calculation

$$Pitch = \arctan\left(\frac{-A_x}{\sqrt{A_y^2 + A_z^2}}\right) \times \frac{180}{\pi}$$

---

## 4. Accident Detection Threshold Logic

The firmware uses a strict two-tier verification matrix to confirm severe accidents while filtering out minor road anomalies like potholes, aggressive cornering, or speed bumps.

### 4.1 High-Impact Collision Condition Matrix
An accident event is triggered only if one of the following Boolean conditions evaluates to `true`:

* **Condition Tier 1 (Standard Crash):** High linear impact paired with moderate rotational shift.
  $$\Delta G \ge 2.0\text{g} \quad \land \quad G_{total} \ge 90.0^\circ/\text{s}$$

* **Condition Tier 2 (Severe/Sharp Crash):** Extreme linear impact paired with lower rotational velocity.
  $$\Delta G \ge 2.4\text{g} \quad \land \quad G_{total} \ge 60.0^\circ/\text{s}$$

### 4.2 Structural Rollover Condition
A rollover event is confirmed if the vehicle sustains a critical tilt angle past the stability threshold without recovering within a specific time window ($t_{sustain}$):

$$\text{Threshold Condition: } |Roll| \ge 45^\circ \quad \lor \quad |Pitch| \ge 45^\circ$$

$$\text{Time Validation: } t_{sustain} \ge 1000\text{ ms}$$

---

## 5. GPS Coordinate System Parsing

The NEO-6M GPS receiver outputs geographical positioning text strings in the standard NMEA-0183 standard (`$GPRMC` / `$GNRMC`). The system extracts the raw coordinate geometry format (`ddmm.mmmm`) and transforms it into uniform Decimal Degrees ($DD$) for mapping compatibility.

### 5.1 Conversion Formula

$$Decimal\ Degrees = Degrees + \frac{Minutes}{60}$$

### 5.2 Mathematical Execution Example
Given a raw latitude string parsed from the NMEA sentence as `1101.0080` (representing $11^\circ 01.0080'$):

1. **Isolate Components:** $\text{Degrees} = 11$, $\text{Minutes} = 01.0080$
2. **Apply Conversion:**
   $$DD = 11 + \frac{01.0080}{60}$$
   $$DD = 11 + 0.016800 = 11.016800^\circ$$
