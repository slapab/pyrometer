import serial
import binascii
import time
import matplotlib.pyplot as plt
import numpy as np

def updatePlot(x, y, col):
    plt.scatter(x,y, color=col)
    plt.draw()

def initPlot(Nsamples, Mms, yData, ymin, ymax):
    xStep = (float(Mms)/1000.0)
    plt.axis([0, Nsamples*xStep, ymin, ymax]) #Nsamples*(float(Mms)/1000.0)
    plt.ylabel("C degree")
    plt.xlabel("time [s]")
    plt.grid(True)
    plt.ion()
    #ax1=plt.axes()
    #plt.show()
    xRange = np.arange(0, Nsamples*xStep, xStep)
    pltLineOne, pltLineTwo = plt.plot(xRange, yData, '-g', xRange, yData, '-r') #get the line and fill the y value with 0

    #init the values on x axis:

    return [pltLineOne, pltLineTwo]

# Param:
# bytes - bytes object with data read from UART
# returns float
def toCdeg(bytes):
    size = len(bytes)

    temp = []

    if size >= 3 and size == 5 :
        temp.append( ((bytes[2] << 8 | bytes[1]) * 0.02) - 273.15 )
    elif size >= 2 and size == 4:
        temp.append(((bytes[1] << 8 | bytes[0]) * 0.02) - 273.15)   # case if start byte was omitted

    if size == 5 :
        temp.append(((bytes[4] << 8 | bytes[3]) * 0.02) - 273.15)
    elif size == 4 :
        temp.append(((bytes[3] << 8 | bytes[2]) * 0.02) - 273.15)   # case if start byte was omitted

    return temp




# Generate the command for reading one time temperature of:
# object if what = "object"
# ambient if what = "ambient"
# both( ambient and object) if what = "both"
# returns cmd bytes and number of expected received bytes
def GenerateOneTimeReadCmd(what):
    cmdByte = 0xAA

    switcher = {
        "ambient" : 0x11,
        "object"  : 0x12,
        "both"    : 0x13
    }
    whichTemp = switcher.get(what.lower(), 0x11)

    sw_bytes = {
        "ambient" : 3,
        "object"  : 3,
        "both"    : 5
    }
    howManyBytesExpect = sw_bytes.get(what.lower(), 0)

    return (bytes([cmdByte, whichTemp]), howManyBytesExpect)


# Read temp.: want N samples each after M[ms]
# what is an string, which should contain: "ambient", "object", "both"
# returns only a cmd bytes
def GenerateMultipleReadCmd(what, Nsamples, MDelay):
    cmdByte = 0xBB

    switcher = {
        "ambient" : 0x11,
        "object"  : 0x12,
        "both"    : 0x13
    }
    whichTemp = switcher.get(what.lower(), 0x11)

    return bytes([cmdByte, whichTemp, Nsamples & 0xFF, (Nsamples >> 8) & 0xFF, MDelay & 0xFF, (MDelay >> 8) & 0xFF])

# Generate save emissivity command
# emissivity must be a double in range 0.1 - 1.0
# returns cmd bytes and number of expected received bytes
def GenerateSaveEmissivityCmd(emissivity):
    cmdByte = 0x55

    emSaveVal = int(round(emissivity * 65535.0))

    return (bytes([cmdByte, emSaveVal & 0xFF, (emSaveVal >> 8) & 0xFF]), 1)


# returns cmd bytes and number of expected received bytes
def GenerateStopCmd():
    return (bytes([0xE8, 0x0A]), 0)

# returns cmd bytes and number of expected received bytes
def GenerateAliveCmd():
    return (bytes([0xEE, 0x0A]), 1)

def PrintHex(bytes):
    print( binascii.hexlify(bytearray(bytes)) )

# Params:
# uart - operational serial handler
# emissivity: float value in range 0.1 - 1.0
# returns:
# True - if saving was ok
# False - if saving failed
def ActionSaveEmissivity(uart, emissivity):
    cmd, byteNo = GenerateSaveEmissivityCmd(0.98)
    uart.flushInput()   # remove all data in input buffer
    uart.write(cmd)
    uart.flush()
    return uart.read(byteNo) == bytes([0x55])    # 0x55 is expected success value


# Params:
# uart -> operational serial handler
# whichTemperature -> string, which should contain: "ambient", "object", "both"
# Returns:
# List with wanted temperature/s. If "ambient" or "object" was requested then return one element list,
#   but when wanted "both", then first element is "Ambient" and second is an "object" temperature.
def ActionReadOneSample(uart, whichTemperature):
    cmd, byteNo = GenerateOneTimeReadCmd(whichTemperature)
    # print( "command will be send: ")
    # PrintHex(cmd)
    uart.flushInput()    # remove all data in input buffer
    uart.write(cmd)
    uart.flush()
    readVal = uart.read(byteNo)
    # print( "received data: ")
    # PrintHex( readVal )
    return toCdeg(readVal)


def PlotData(temp, timePoint):
    print( "y = ", temp, " \nx = ", timePoint)

    updatePlot(timePoint, temp[0], 'r')

    if len(temp) > 1:
        updatePlot(timePoint, temp[1], 'g')

# Params:
# Action -> function which will be called with received temperature data and time point(fox x axi)
def ActionReadMultiple(uart, whichTemperature, Nsamples, Mdealy, Action):

    if Nsamples == 0:
        print( "This function doesnt support Nsamples = 0!" )
        return

    ## stop any running commnd
    cmdStop, byteNo = GenerateStopCmd()
    uart.write(cmdStop)
    uart.flush()

    ## flush input buffer
    uart.flushInput()

    cmd = GenerateMultipleReadCmd(whichTemperature, Nsamples, Mdealy)

    uart.write(cmd)
    uart.flush()

    ## need to receive data
    for i in range(Nsamples):
        data = uart.read() # read byte

        if (data ==  bytes([0xB1])) or (data == bytes([0xB2])):
            values = uart.read(2)

        elif (data ==  bytes([0xB3])):
            values = uart.read(4)
        else:
            continue # unrecognized data hm??

        Action( toCdeg(values), i * (Mdealy/1000.0) )


    ## stop any running command
    uart.write(cmdStop)
    uart.flush()


Nsamples = 100
Mms = 300       # in [ms]

yData = [0] * Nsamples
yData2 = [0] * Nsamples

plotLines = initPlot(Nsamples, Mms, yData, -1, 100)
plotLine = plotLines[0]
plotLine2 = plotLines[1]


for i in range(Nsamples):

    #ymin = float(min(ydata))-10
    #ymax = float(max(ydata))+10
    #plt.ylim([ymin,ymax])
    yData.append(i)
    yData2.append(i - 3)
    del(yData[0])
    del(yData2[0])
    #plotLine.set_xdata(range(Nsamples))
    plotLine.set_ydata(yData)  # update the data
    plotLine2.set_ydata(yData2)
    plt.draw() # update the plot
    time.sleep(50.0/1000.0)

# uart = serial.Serial('/dev/ttyUSB0', 19200, timeout=5)
#
# #print (ActionSaveEmissivity(uart, 0.98)) # save emissivity
#
#
# #print (  ActionReadOneSample(uart, "both") )
#
# ActionReadMultiple(uart, "both", Nsamples, Mms, PlotData)
#
# uart.close()



input("Press Enter to exit ...")