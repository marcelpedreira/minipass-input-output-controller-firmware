# Minipass Input/Output Controller Firmware

The device is based on a PIC 16F877A microcontroller. Its purpose is to add other functionalities to the Minipass Access Control System, such as door interlocking. It can be connected in a bus-type network with other [Minipass Input/Output Controllers](https://github.com/marcelpedreira/minipass-input-output-controller-firmware), [Minipass Door Controllers](https://github.com/marcelpedreira/minipass-door-controller-firmware) and the [Minipass Supervision System](https://github.com/marcelpedreira/minipass-desktop-application). The firmware is developed using C language through the Custom Computer Services PCW compiler for mid-range Microchip microcontrollers.

## Components

- PIC 16F877A microcontroller.
- MAX485 interface for communication using Modbus RTU protocol.
- Dip switches to set the device address on the network.
- Real Time Clock for reports generation.
- External EEPROM memory to store the configuration.
- 8 relay outputs for connecting the actuators.
- 8 digital inputs for connecting the sensors.

<!-- ## General Diagram

![Door Controller Diagram](/assets/ControladorPuerta2.JPG) -->
