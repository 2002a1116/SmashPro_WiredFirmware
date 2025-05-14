# SmashPro_WiredFirmware
 
This is SmashPro wired firmware for Ch32v103 mcu.

Supported basic function,gyroscope,HD Rumble,addressable rgb(ws2812 led),web usb driver config and usb firmware update.

## todo:

~~1.increase imu sample rate for better precision.~~(imu sample gaps are configable in the driver now);

~~2.firmware shared by both boardtype --> we make GPIO_BUTTON_XX a lookup index for actual gpio pin instead of gpio pin macro.~~(i unified mcu pin definition for both types of pcb,now pcb type only influence indicate rgb and joystick direction，which can be configed by driver.)

3.finish imu quaternion mode.
