# MINI Cooper JCW (F56, 2016) instrument panel cluster (STM32G431KB)
The part number is [9383559 (VIN: WMWXM920802D57158)](https://allegro.pl/oferta/mini-cooper-f56-f57-jcw-licznik-zegary-9383559-uk-17026759798). The code is also tested with [8707226](https://www.ebay.pl/sch/i.html?_nkw=8707226). The IPC uses payload CRC8 (in addition to frame CRC15) and rolling/alive counters within numerous CAN frames - it probably wouldn't be possible for me to reverse engineer communication in a reasonable timeframe without access to a working car to sniff the frames. Luckily, we have [CarCluster](https://github.com/r00li/CarCluster) :slightly_smiling_face:

![MINI Cooper IPC in action](/Assets/Images/mini_cooper_ipc_in_action.jpg)
![MINI Cooper IPC setup](/Assets/Images/mini_cooper_ipc_birds_eye_view.jpg)
![MINI Cooper IPC OLED display](/Assets/Images/mini_cooper_ipc_oled.jpg)
![MINI Cooper IPC frames](/Assets/Images/mini_cooper_ipc_frames.JPG)

# Pinout
* [Wiring for BMW F series cluster](https://github.com/r00li/CarCluster/blob/main/Misc/README_WIRING_BMW_F.md) (Andrej Rolih)

# Missing files?
Don't worry :slightly_smiling_face: Just log in to MyST and hit Alt-K to generate /Drivers/CMCIS/ and /Drivers/STM32G4xx_HAL_Driver/ based on the .ioc file. After a couple of seconds your project will be ready for building.

# Readings
* [CarCluster](https://github.com/r00li/CarCluster) (Andrej Rolih)
* [CAN Part 9 - CRCs and Incrementing Counters (Bucket Filling Technique in XNet)](https://hooovahh.blogspot.com/2017/07/can-part-9-crcs-and-incrementing.html) (Hooovahh's Blog)
* [A detailed explanation of Checksum and Rolling Counter verification](https://en.eeworld.com.cn/mp/aes/a382631.jspx) (EEWORLD)

# Call to action
Create your own [home laboratory/workshop/garage](http://ufnalski.edu.pl/control_engineering_for_hobbyists/2025_dzien_otwarty_we/Dzien_Otwarty_WE_2025_Control_Engineering_for_Hobbyists.pdf)! Get inspired by [ControllersTech](https://www.youtube.com/@ControllersTech), [DroneBot Workshop](https://www.youtube.com/@Dronebotworkshop), [Andreas Spiess](https://www.youtube.com/@AndreasSpiess), [GreatScott!](https://www.youtube.com/@greatscottlab), [bitluni's lab](https://www.youtube.com/@bitluni), [ElectroBOOM](https://www.youtube.com/@ElectroBOOM), [Phil's Lab](https://www.youtube.com/@PhilsLab), [atomic14](https://www.youtube.com/@atomic14), [That Project](https://www.youtube.com/@ThatProject), [Paul McWhorter](https://www.youtube.com/@paulmcwhorter), [Max Imagination](https://www.youtube.com/@MaxImagination), [Nikodem Bartnik](https://www.youtube.com/@nikodembartnik), and many other professional hobbyists sharing their awesome projects and tutorials! Shout-out/kudos to all of them!

> [!WARNING]
> Automotive reverse engineering - do try this at home :exclamation:

200+ challenges to start from: [Control Engineering for Hobbyists at the Warsaw University of Technology](http://ufnalski.edu.pl/control_engineering_for_hobbyists/Control_Engineering_for_Hobbyists_list_of_challenges.pdf).

Stay tuned :sunglasses:
