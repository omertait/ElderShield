# IOT-Final-Project- Fall Detection


## Overview

ElderShield is an innovative IoT solution designed to enhance the safety and well-being of elderly individuals living alone. By leveraging the power of modern technology and the Internet, ElderShield acts as a virtual guardian, providing peace of mind to both the elderly and their loved ones. This project combines an ESP32 microcontroller, various sensors, and internet connectivity via Blynk and Make to create a wearable and adaptable system that ensures timely assistance in critical situations.

## Features

- **Fall Detection**: Utilizes an MPU6050 accelerometer and gyroscope sensor to detect sudden falls, automatically sending an alert to predefined emergency contacts.
- **Shower Monitoring**: An embedded humidity sensor monitors the duration of shower activities. If an unusually long shower time is detected, indicating a potential issue, it triggers an alert to emergency contacts.
- **Interactive SOS Button**: Allows the user to manually signal for help through a simple and accessible interface.
- **Remote Check-Ins**: Loved ones can send "poke" messages via the internet, prompting the user with visual (LED) and auditory (speaker) signals. The user can respond using physical package sensors, reassuring their safety.
- **Internet-Connected**: Through Blynk and Make, ElderShield maintains a constant connection for real-time monitoring and alerts.

## Hardware Requirements

- ESP32 Microcontroller
- MPU6050 Accelerometer and Gyroscope Sensor
- Humidity Sensor
- LEDs
- Speaker
- Power Supply/Battery
- Waterproof Enclosure for Shower Use

## Software Requirements

- Arduino IDE for ESP32 programming
- Blynk App for remote connectivity and alerts
- Make (formerly Integromat) for integrating webhooks and custom logic flows
- Custom ElderShield Firmware (provided in this repository)

## Setup and Installation

1. **Hardware Assembly**: Follow the hardware connection diagrams provided in the `diagrams` folder to assemble your ElderShield device.
2. **Software Configuration**: 
   - Install the Arduino IDE and add the ESP32 board manager.
   - Clone this repository and open the ElderShield firmware in the Arduino IDE.
   - Configure your WiFi credentials and Blynk token in the firmware.
3. **Blynk App Setup**:
   - Download the Blynk app and create a new project.
   - Add buttons and value displays as needed for fall detection, shower monitoring, and SOS alerts.
   - Note your Blynk Auth Token and enter it into the ElderShield firmware.
4. **Make Integration**:
   - Set up a Make scenario for additional logic, such as sending SMS or emails to emergency contacts.
   - Use webhooks provided by Make in the ElderShield firmware for integration.
5. **Deployment**:
   - Upload the ElderShield firmware to the ESP32.
   - Securely attach the device to the user, ensuring comfort and proper sensor alignment.
   - Place the device in a suitable location during showers for humidity monitoring.

## Usage

- Wear the ElderShield device as instructed. Ensure it remains charged and within WiFi range.
- In an emergency, press the SOS button or rely on automatic alerts through fall detection and shower monitoring.
- Respond to "poke" messages from loved ones by interacting with the device's sensors.

## Contributing

We welcome contributions from the community to make ElderShield even better. Whether it's feature enhancements, bug fixes, or documentation improvements, please feel free to fork this repository and submit your pull requests.

## License

ElderShield is released under the MIT License. See the LICENSE file for more details.

## Contact

For support, feature requests, or to get involved with the ElderShield project, please contact us at [your contact information].
