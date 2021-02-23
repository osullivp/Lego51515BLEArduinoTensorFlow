from spike import PrimeHub, LightMatrix, Button, StatusLight, ForceSensor, MotionSensor, Speaker, ColorSensor, App, DistanceSensor, Motor, MotorPair
from spike.control import wait_for_seconds, wait_until, Timer
from math import *

hub = PrimeHub()

motor_pair = MotorPair('A', 'B')
motor_pair.set_motor_rotation(17.5, 'cm')
motor_pair.set_default_speed(20)
colour_c = ColorSensor('C')
distance = DistanceSensor('D')
motor_e = Motor('E')

def closeGrip():
    motor_e.set_default_speed(50)
    motor_e.run_for_rotations(+1.50)

def openGrip():
    motor_e.set_default_speed(50)
    motor_e.run_for_rotations(-1.50)

moveRobot = True

if moveRobot == False:
    openGrip()
else:    
    motor_pair.move_tank(-46, 'degrees', 20, 0)
    motor_pair.move(15, 'cm')
    closeGrip()
