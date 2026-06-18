# VADRS Mathematical Model

## 1. Accelerometer Conversion

MPU6050 configured for ±4g range.

\[
A_x = \frac{Ax_{raw}}{8192}
\]

\[
A_y = \frac{Ay_{raw}}{8192}
\]

\[
A_z = \frac{Az_{raw}}{8192}
\]

Unit: g

---

## 2. Gyroscope Conversion

MPU6050 configured for ±250°/s range.

\[
G_x = \frac{Gx_{raw}}{131}
\]

\[
G_y = \frac{Gy_{raw}}{131}
\]

\[
G_z = \frac{Gz_{raw}}{131}
\]

Unit: °/s

---

## 3. Total Acceleration Magnitude

\[
A_{total} = \sqrt{A_x^2 + A_y^2 + A_z^2}
\]

---

## 4. Delta-G (Impact Detection)

\[
\Delta G = |A_{total} - 1|
\]

---

## 5. Gyroscope Magnitude

\[
G_{total} = \sqrt{G_x^2 + G_y^2 + G_z^2}
\]

---

## 6. Roll Angle

\[
Roll = \arctan\left(\frac{A_y}{A_z}\right)\times\frac{180}{\pi}
\]

---

## 7. Pitch Angle

\[
Pitch = \arctan\left(\frac{-A_x}{\sqrt{A_y^2+A_z^2}}\right)\times\frac{180}{\pi}
\]

---

## 8. Impact Detection Condition

Condition 1:

\[
(\Delta G \ge 2) \land (G_{total} \ge 90)
\]

OR

\[
(\Delta G \ge 2.4) \land (G_{total} \ge 60)
\]

---

## 9. Rollover Detection

\[
|Roll| \ge 45^\circ
\]

OR

\[
|Pitch| \ge 45^\circ
\]

for

\[
t \ge 1 \text{ second}
\]

---

## 10. GPS Coordinate Conversion

GPS NMEA Format:

ddmm.mmmm

Conversion:

\[
Decimal = Degrees + \frac{Minutes}{60}
\]

Example:

1256.1234

\[
12 + \frac{56.1234}{60}
\]

= 12.93539°