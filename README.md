

### About project
This project shows the efforts to run and test the MLX90614ESF-BCC IR sensor. This sensor allow to measure the ambient temperature and temperature of the observed object. It has interesting chips inside, for example it has 17-bit ADC, DSP for digital filtering and temperature compensation, so the accuracy of measurements should be very good.

Also in this attempt I wanted to try use C++ and OOP (btw. I have to admit, it is very hard to use OOP without using heap) to program the STM32F051R8T6 microcontroller. Whole project was created in Eclipse Mars with GCC, GDB and OpenOCD. All sources were developing in Linux.

To communicate with sensor the I2C (SMBus mode) was used. The STM32F051R8T6 provides hardware implementation of SMBus which
was very useful because of hardware calculation of the CRC8 code. MLX90614 class provides access to RAM and EEPROM, but not all addresses
are allowed to perform write operation (see documentation). This class also provides method to save emissivity value, this requires sensor reset - implemented as putting sensor into sleep mode and waking it up.


To perform some automatic temperatures reading from sensor and transfer them to PC  the command - response frame has been implemented.
Connection with PC was made by UART interface. The PC sends one of defined commands, for example, read one temperature of object and when microcontroller recognizes that command it performs temperature reading and raw value (16 bits of data) is sending to the PC.


### Project's sources structure

- **pyrometer** - C++ project with sources for microcontroller
- **UnitTests** - tests project with sources of tests
- **python3_scripts** - scripts written in Python 3 to communicate with STM32F0 via UART - they should get measurements of temperature and show them in chart or save in list in Qt application. 


### Before developing in C++

To save program size I disabled in compiler:
* **RTTI** - without RTTI (Run Time Type Information) we can not use `dynamic_cast`, `typeid` and everything else what requires to know type at runtime.
* **Exceptions** - exceptions also consume lots of memory


### Unit tests

Usually some part of sources in embedded systems are not dependent from hardware so the code should be easly tested. I have small experience with testing and I partially know the GTest framework but I have decided to use it in this project. Unit testing requires special code design to easly set mock or stub some dependencies, so the most of classes has own *interfaces* (pure virtual classes).

Currently, due to lack of time only small part of the code is covered by tests and some of them fails - these were not fixed after refactoring.

To save compilation time of test's project the best way is using precompiled library of gtest and link it to the executive file.
In **UnitTests** directory I have placed the precompiled library of gmock (compiled on Linux).
