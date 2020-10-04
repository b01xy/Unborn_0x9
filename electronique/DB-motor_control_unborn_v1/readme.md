# Daughter_board
![](./images/pcb.png)

## Name
[`DB-motor_control_unborn`]()

## Title
Daughter board motor control made for unborn0x9 project

## Author
* [`CTC-dubois_jerome`](), Future Baby Production

## Date
February 2020

## Interfaces
### Input
* [`ITF-A_gnd`]()
* [`ITF-B_5v`]()
* [`ITF-F_12v`]()
* [`ITF-S_3_3v`]()

### Output
* [`ITF-A_gnd`]()
* [`ITF-I_pulse_on`]()
* [`ITF-J_pulse_off`]()
* [`ITF-K_pulse_redpitaya`]()

##External connections
### Input
* GPIO DIO7_N of RedPitaya (connector E1) on P2
* GPIO DIO5_N of RedPitaya (connector E1) on P2
* GPIO DIO4_N of RedPitaya (connector E1) on P2
* GPIO DIO3_N of RedPitaya (connector E1) on P2
* GPIO DIO2_N of RedPitaya (connector E1) on P2
* GPIO DIO1_N of RedPitaya (connector E1) on P2
* GPIO DIO0_N of RedPitaya (connector E1) on P2

### Output
* stepper A1 on P5
* stepper A2 on P5
* stepper B1 on P5
* stepper B2 on P5

## Scheme
![](images/scheme.png)

## Remarks
[BOM](./src/DB-motor_control_v3.csv)

This daughter board is for driving [28BYJ-48-12V] unipolar stepper motor(https://letmeknow.fr/shop/fr/moteurs-et-servo-moteurs/1520-petit-moteur-pas-a-pas-avec-reducteur-28byj-48-12v-700465391773.html) with an ULN2003 driver. We have change from bipolar to unipolar stepper motor, because old driver hash the 12V and produced a lot of noise. This motor has a 1/16 reductor so we have a angular precise enought and with the ULN2003 we can drive the motor without hash. So if we don't moove the motor during measuring acoustic line, it won't generate noise on signal. An arduino is pluged on socket P3 and P4 and the stepper motor is plugged on socket P5.

Socket P2 is used to connect the RedPitaya with this daughter_board such on following image. SN74 are used as level shifter for communication between RedPitaya (3.3 V) and arduino (5 V).

![](images/RPPCB_top_view.png)

The piece on the left of the image can be cut from a matrix board like for the [mother board](../../../modules/hardware/MDL-mother_board/readme.md), the soldering track are on the top of this piece. There are two sockets 1*13 solder on it, don't forget to cut the tracks between these two socket (cut represent by the green line).

## Results

## Pros/Cons/Constraint:

**Pros:** NA

**Cons:** NA

**Constraint:** NA
