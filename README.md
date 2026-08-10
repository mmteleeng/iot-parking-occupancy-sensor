# IoT Parking Occupancy Sensor (ESP32)

Coursework project for the Internet of Things course at Politecnico di Milano. Two parts: a parking occupancy sensor node built and simulated in Wokwi, and a separate exercise on optimizing sink placement in a wireless sensor network.

## Part 1: Parking Occupancy Sensor Node
Built a sensor node using an ESP32 and an HC-SR04 ultrasonic sensor to detect whether a parking spot is occupied. If the measured distance is 50 cm or less, the spot counts as occupied.

The node wakes up every 39 seconds, takes a reading, sends the status to a sink node over ESP-NOW, then goes back into deep sleep to save power. Everything was built and tested in Wokwi's ESP32 simulator.

**What it does:**
- Reads distance from the HC-SR04 (trigger/echo pins)
- Sends occupancy status (FREE/OCCUPIED) via ESP-NOW
- Uses deep sleep between readings to cut power use
- Tracks timing for each phase (sensor read, transmission, sleep) for the energy analysis below

**Energy analysis:**
Measured average power per phase (transmission, sensor reading, deep sleep) and used that to estimate battery life. With a battery budget of about 17,334 J, the node comes out to roughly 2,280 wake cycles, or about 38.6 hours of runtime before needing a battery change. Deep sleep dominates the energy budget even though it's low power, just because it runs so much longer than the active phases.

Full breakdown and charts are in `challenge.pdf`.

**Tools:** ESP32, Wokwi simulator, Arduino (C++), ESP-NOW

## Part 2: WSN Sink Position Optimization
A separate exercise on the same course, about placing a sink node in a wireless sensor network to maximize system lifetime. Started with a fixed sink position, calculated how fast the worst-case sensor would drain its battery, then optimized the sink's position to balance energy use across all sensors — extending lifetime by about 6x compared to a badly placed sink.

Full write-up (formulas, calculations, sink placement trade-offs) is in `sink_position_optimization.pdf`.
