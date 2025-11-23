"""
Simple and stupid tool I used to plot out the IMU data
Serves its purpose? Yes!
Ugly? Absolutely!
"""

import matplotlib.pyplot as plt
import getopt, sys

#Acceleration and gyro axes
acc : list[float] = []
gyro : list[float] = []

#Since we're just collecting data from 100 ticks, the time axis is a constant
time : list[int] = range(1, 101)

"""
Separate the acceleration and gyro data, 
keeps the data internally separated by tick
""" 
def parse(filename : str) -> None:
    with open(filename) as file:
        for i in file:
            data : list[str] = i.rstrip().split(",")
            acc.append(data[0:3])
            gyro.append(data[3:])

def plotAcceleration() -> None:
    acc_x : list[float] = []
    acc_y : list[float] = []
    acc_z : list[float] = []

    #Separate the data 
    for sample in acc:
        acc_x.append(float(sample[0]))
        acc_y.append(float(sample[1]))
        acc_z.append(float(sample[2]))
    
    #Everybody loves matplotlib
    fig, ax = plt.subplots()
    ax.plot(time, acc_x, label="x")
    ax.plot(time, acc_y, label="y")
    ax.plot(time, acc_z, label="z") 

    ax.set_ybound(-1, 1.25)
    ax.set_yticks([-1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1, 1.25])

    plt.xlabel("Time")
    plt.ylabel("Acceleration")
    plt.title(sys.argv[1])
    ax.legend()

    plt.show()


def plotGyro() -> None:
    gyro_x : list[float] = []
    gyro_y : list[float] = []
    gyro_z : list[float] = []

    #Separate the data 
    for sample in gyro:
        gyro_x.append(float(sample[0]))
        gyro_y.append(float(sample[1]))
        gyro_z.append(float(sample[2]))
    
    #Everybody loves matplotlib
    fig, ax = plt.subplots()
    ax.plot(time, gyro_x, label="x")
    ax.plot(time, gyro_y, label="y")
    ax.plot(time, gyro_z, label="z") 

    ax.set_ybound(-3, 3)
    ax.set_yticks([-3, -2.5, -2, -1.5, -1, -0.5, 0, 0.5, 1,
     1.5, 2, 2.5, 3])

    plt.xlabel("Time")
    plt.ylabel("Gyro position")
    plt.title(sys.argv[1])

    ax.legend()
    plt.show()

  


    


if __name__ == "__main__":

    parse(sys.argv[1]) 

    match sys.argv[2]:
        case "-a":
            plotAcceleration()
        case "-g":
            plotGyro()