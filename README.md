# **WouoUI for ESP32-S3 (128x128)**

This is an **ESP32-S3** ported version based on [RQNG/WouoUI](https://github.com/RQNG/WouoUI). This project mimics the MonoUI style created by Zhihui Jun, implementing an ultra-smooth monochrome OLED menu UI, controlled by an EC11 rotary encoder.

The current version is fully adapted and optimized primarily for **128x128 resolution** OLED screens.

## **Porting Notes & Features**

Based on the excellent original UI framework, the following adaptations were made specifically for the hardware features of the ESP32-S3:

* **Hardware SPI Acceleration**: Uses the ESP32-S3 hardware SPI to drive the OLED (bus frequency set to 10MHz), ensuring ultra-high refresh rates and smooth animations on the 128x128 screen.  
* **Non-Volatile Storage Adaptation**: Rewrote the parameter storage logic, utilizing the ESP32 EEPROM.commit() mechanism to save parameters after power-off.  
* **Interrupt Optimization**: Tailored for the ESP32-S3 architecture, the IRAM\_ATTR attribute was added to the EC11 encoder interrupt handler to prevent crashes caused by code executing from Flash.  
* **Development Environment**: Fully migrated to the **PlatformIO** platform for modern dependency management and one-click compilation and flashing.  
* **Inherits all core UI features from the original**:  
  * Non-linear smooth easing animations (lists, pop-up windows, progress bars, tile icons).  
  * Free switching between Light and Dark mode.  
  * Independent pop-up module with background blur support.  
  * Menu loop scrolling and fully adjustable animation speeds.

## **Hardware Wiring Guide**

This project uses the **ESP32-S3** by default. Please refer to the following configuration for hardware wiring (defined in lib/WouoUI/WouoUI.h):

### **OLED Screen (SPI Interface)**

| OLED Pin | ESP32-S3 Pin | Description |
| :---- | :---- | :---- |
| **SCL (CLK)** | GPIO 15 | Hardware SPI Clock Line |
| **SDA (MOSI)** | GPIO 7 | Hardware SPI Data Line |
| **RES (RST)** | GPIO 4 | Reset Pin |
| **DC** | GPIO 6 | Data/Command Select Pin |
| **CS** | GPIO 5 | Chip Select Pin |

### **EC11 Rotary Encoder**

| EC11 Pin | ESP32-S3 Pin | Description |
| :---- | :---- | :---- |
| **A (CLK)** | GPIO 18 | Rotation trigger pin, external interrupt enabled |
| **B (DT)** | GPIO 8 | Rotation direction pin |
| **SW (BTN)** | GPIO 17 | Button pin, internal pull-up enabled |

**Note:** The ADC pins for the voltage measurement page are currently configured to default placeholder pins. In actual use, please remap them yourself in WouoUI\_Data.h and WouoUI.cpp according to the ESP32-S3 ADC limitations (e.g., ADC2 is unavailable when WiFi is enabled).

## **Quick Start**

This project is developed using **PlatformIO**.

1. Install [VSCode](https://code.visualstudio.com/) and the [PlatformIO Extension](https://platformio.org/).  
2. Clone or download the source code of this project and open it in VSCode.  
3. PlatformIO will automatically read platformio.ini and download the U8g2 dependency library.  
4. Connect your ESP32-S3 to the computer, and click the **Upload** button at the bottom (or use the shortcut Ctrl+Alt+U) to compile and flash the program.

## **Operation Instructions**

* **Sleep Mode**: The program enters sleep mode by default upon startup.  
* **Wake Up & Enter Menu**: In sleep mode, **long press** the knob to wake up and enter the main menu.  
* **Menu Navigation**:  
  * **Rotate**: Switch the selected item.  
  * **Short Press**: Enter the selected item / Confirm modification.  
  * **Long Press**: Return to the previous menu level.  
* **Save Parameters**: After modifying parameters in the settings page, the system will automatically write the parameters to the EEPROM for saving when returning to sleep mode.

## **Current Porting Progress & Known Limitations**

* \[x\] Core UI framework ported (lists, tiles, pop-ups)  
* \[x\] 128x128 OLED hardware SPI adaptation  
* \[x\] Encoder debouncing and interrupt reading optimization  
* \[x\] EEPROM parameter storage adaptation  
* \[ \] **USB HID Function**: The ESP32-S3 supports native USB, but the keyboard shortcuts/multimedia controls (like adjusting volume or simulating keystrokes) in the current code are not yet integrated with the USBHID library. If you need to use this feature, you will need to add the relevant Keyboard API calls manually.

## **Acknowledgments & References**

* **Original UI Project**: [RQNG/WouoUI](https://github.com/RQNG/WouoUI) (Special thanks to the original author RQNG for the excellent algorithms and UI design)  
* **U8g2 Graphics Library**: [olikraus/u8g2](https://github.com/olikraus/u8g2)  
* **UI Inspiration**: Zhihui Jun's MonoUI / UltraLink and other similar designs.

Since the original project does not specify an open-source license, this project also does not impose any special open-source license restrictions. Feel free to modify and adapt it as needed. Users are advised to be mindful of computer security.
