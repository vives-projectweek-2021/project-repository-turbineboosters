# TurbineBoosters
Welcome to our project repository!


## Choice of motor

To create our windturbine we had to make some choices. The first choice we had to make was wich motor to use as generator.
It soon became clear to us that we had to use a 3 phasse BLDC motor.
We looked around at our options and had 2 motors available, one that generates around 60V:

![afbeelding](https://user-images.githubusercontent.com/71695433/116680587-21b65a80-a9ac-11eb-83ad-507638dc10e1.png)

and another one that generates only around 6V, but has more power:

![afbeelding](https://user-images.githubusercontent.com/71695433/116680641-3397fd80-a9ac-11eb-85da-1a268e85886c.png)

We decided to go for the motor that generates 60V because we lose 1,4V across the diodes of our 3 phase bridge rectifier and that leaves us with an efficiency of 96%, while the other motor would only offer us an efficiency of about 77%.


## Rectifier + first LoRa module

The power generated by our motor is fed to a 3 phase bridge rectifier:

![afbeelding](https://user-images.githubusercontent.com/71695433/116681864-cedda280-a9ad-11eb-8af5-de38272dbf09.png)

## Lorawan receiver 

The data that we recieve over lorawan on the Explorer microconroller is than formated to a byte array. Then the data is send over bluetooth and send to a raspberry pi.

First we wanted to send the data over bleutooth low energy that was on the controller. But the bleutooth HC05 chip doesn't take that much power and is easier to use. 

![afbeelding](https://user-images.githubusercontent.com/71642918/117444555-eda5e100-af39-11eb-9775-9940873c95af.png)

## Raspberry pi

The data that we receive over bleutooth is than but into an influxdb database. This data is than visualized with the use of grafana.

![afbeelding](https://user-images.githubusercontent.com/71642918/117447221-812ce100-af3d-11eb-8843-8e71a5884f23.png)

## Mechanical design

![afbeelding](https://user-images.githubusercontent.com/71695433/117445889-96087500-af3b-11eb-8d13-f27b62f94ad6.png)

We mounted a bicycle wheel on the shaft of our motor, to which we attached 8 PVC-blades via 3D-printed coupling pieces.
On this board we measure the current and voltage via a shunt-resistor of 0,5Ω.
The LoRa module (RN2483A) receives this data and sends it peer-to-peer to another LoRa module:
