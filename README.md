# sludge-sensor

> omg a soil moisture sensor of things

![screenshot](https://cldup.com/BwBRipOWqa.jpg)

## Notes

- Used [Adafruit Feather HUZZAH](https://www.adafruit.com/products/2821)
- Install libs and toolchain via [PlatformIO](http://platformio.org)
- Used DFRobot's [capacitive soil moisture sensor](https://www.dfrobot.com/index.php?route=product/product&path=156_36&product_id=1385) after noting how quickly resistive sensors tend to corrode
- Due to the 1.0V cap of the ESP8266's ADC pin--and 3.0V being the maximum voltage the sensor will produce--had to use a voltage divider (*R1* 2.2kΩ, *R2* 1.0kΩ and 100Ω in series; used metal film) and multiply the pin reading by 3
- After uploading firmware (`pio run -t upload`) and SPIFFS (`pio run -t uploadfs`) find **Sludge Sensor** AP and connect.
- Hit `http://192.168.1.1` and give it a new SSID/pass to connect to existing WiFi network; this is stored in EEPROM
- Thing will then restart and connect to new network

## TODO

- Add configurable pins
- Add configurable timing
- Add value fudging
- OTA updates
- mDNS

## License

:copyright: 2016 [Christopher Hiller](https://boneskull.com).  Licensed MIT.
