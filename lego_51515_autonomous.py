from spike import PrimeHub, LightMatrix, Button, StatusLight, ForceSensor, MotionSensor, Speaker, ColorSensor, App, DistanceSensor, Motor, MotorPair
from spike.control import wait_for_seconds, wait_until, Timer
from math import *

hub = PrimeHub()

motor_pair = MotorPair('A', 'B')
motor_pair.set_motor_rotation(17.5, 'cm')
motor_pair.set_default_speed(20)
motor_e = Motor('E')
distance = DistanceSensor('F')
colour_c = ColorSensor('C')
colour_d = ColorSensor('D')
timer = Timer()
timer.reset()
motor_pair.start(steering=0)

blnStopped = 0
blockFound = 0
hasBlock = 0
home = 0

def scan():
    global blockFound
    motor_pair.stop()
    motor_pair.move_tank(45, 'degrees', 20, 0)
    count = 0
    blockFound = 0
    while count < 9 and blockFound == 0:
        global blockFound
        motor_pair.move_tank(-10, 'degrees', 20, 0)
        dist = distance.get_distance_cm()
        if  dist == None:
            pass
        elif dist < 15:
            blockFound = 1
        count = count + 1

def closeGrip():
    motor_e.set_default_speed(50)
    motor_e.run_for_rotations(+1.50)

def openGrip():
    motor_e.set_default_speed(50)
    motor_e.run_for_rotations(-1.50)

def checkTableEdge():
    if colour_c.get_reflected_light() < 50 or colour_c.get_reflected_light() < 50:
        global blnStopped
        if blnStopped == 0:
            global blnStopped
            blnStopped = 1
            motor_pair.stop()
            motor_pair.move(-10, 'cm')
            motor_pair.stop()
            motor_pair.move_tank(180, 'degrees', 20, 0)
            motor_pair.stop()
            motor_pair.start(steering=0)
            blnStopped = 0

def checkEnteredHome():
    global hasBlock
    if hasBlock == 1 and (colour_c.get_color() == 'green' or colour_d.get_color() == 'green'):
        motor_pair.stop()
        if colour_c.get_color() == 'green':
            motor_pair.move_tank(135, 'degrees', 20, 0)
            motor_pair.start(steering=0)
        else:
            if colour_d.get_color() == 'green':
                motor_pair.move_tank(-135, 'degrees', 20, 0)
                motor_pair.start(steering=0)

def checkAtHome():
    global hasBlock
    if hasBlock == 1 and colour_c.get_color() == 'green' and colour_d.get_color() == 'green':
        global home
        hub.speaker.beep(60, 0.5)
        motor_pair.stop()
        openGrip()
        home = 1

#openGrip()

while home == 0:
    checkTableEdge()
    checkEnteredHome()
    checkAtHome()
    
    #if timer.now() >= 8 and hasBlock == 0:
    #    scan()
    #    if blockFound == 1:
    #        motor_pair.move(10, 'cm')
    #        closeGrip()
    #        hasBlock = 1
    #    motor_pair.start(steering=0)
    #    timer.reset()
