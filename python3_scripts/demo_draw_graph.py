import serial
import time
import matplotlib.pyplot as plt

def updatePlot(x, y, col):
    plt.scatter(x,y, color=col)
    plt.draw()

def initPlot(Nsamples, Mms, ymin, ymax):
    plt.axis([0, Nsamples*Mms, ymin, ymax])
    plt.ylabel("C degree")
    plt.xlabel("time [ms]")
    plt.grid(True)
    plt.ion()
    plt.show()


def toCdeg(line):
    return (float(int((line[0]<<8) | line[1])) * 0.02) - 273.15




Nsamples = 200
Mms = 300.0 # in [ms]

initPlot(Nsamples, Mms, 10, 40)

plotX = []
plotY = []
with serial.Serial('/dev/ttyUSB0', 19200, timeout=1) as dev:

    for i in range(Nsamples+1):
        dev.write(b'a') #init response
        # time.sleep(10.0/1000.0)
        ambient = dev.readline() #read ambient temp
        object =  dev.readline() #read object temp
        time.sleep(50.0/1000.0)
        x = i*Mms
        valObj = toCdeg(object)
        valAmb = toCdeg(ambient)
        updatePlot(x, valObj, 'r')
        updatePlot(x, valAmb, 'g')
        time.sleep(Mms/1000.0)

    dev.close()

plt.plot(plotX, plotY, '-')


input("Press Enter to exit ...")