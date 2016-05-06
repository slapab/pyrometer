import serial
import binascii
import time
import matplotlib.pyplot as plt
import numpy as np
import threading
import queue


#ploting thread
class plotingThread( threading.Thread ):
    def __init__(self, Queue, Event, Nsamples, Mms, ymin, ymax):
        threading.Thread.__init__(self)
        #init params if any
        self.q = Queue  # data queue
        self.runningFlag = Event  # event -> to run or exit -> signaling from main process
        self.Nsamples = Nsamples
        self.Mms = Mms
        self.ymin = ymin
        self.ymax = ymax

        #create Y data
        self.yDataLineOne = [0] * self.Nsamples
        self.yDataLineTwo = [0] * self.Nsamples


    def initPlt(self):
        xStep = (float(self.Mms) / 1000.0)
        plt.axis([0, self.Nsamples * xStep, self.ymin, self.ymax])  # Nsamples*(float(Mms)/1000.0)
        plt.ylabel("C degree")
        plt.xlabel("time [s]")
        plt.grid(True)
        plt.ion()

        xRange = np.arange(0, self.Nsamples * xStep, xStep)
        pltLineOne, pltLineTwo = plt.plot(xRange, self.yDataLineOne, '-g',
                                          xRange, self.yDataLineTwo, '-r')  # get the line and fill the y value with 0
        plt.show()

        # draw the figure so the animations will work
        # fig = plt.gcf()
        # fig.show()
        # fig.canvas.draw()
        #
        # # store plot handlers
        # self.fig = fig
        self.pltLineOne = pltLineOne
        self.pltLineTwo = pltLineTwo

    def refreshPlot(self):
        if len(plt.get_fignums()) > 0:
            plt.pause(0.001)

    def closePlotWindow(self):
        if len(plt.get_fignums()) > 0:
            plt.close('all')

    def updatePlot(self, dataList):
        # ymin = float(min(ydata))-10
        # ymax = float(max(ydata))+10
        # plt.ylim([ymin,ymax])


        # plotLine.set_xdata(range(Nsamples))

        # if plot window is closed
        if 0 == len(plt.get_fignums()):
            return

        self.yDataLineOne.append(dataList[0]) # data 1 form queue
        del (self.yDataLineOne[0])
        self.pltLineOne.set_ydata(self.yDataLineOne)  # update the data

        ## for second line
        if len(dataList) == 2:
            self.yDataLineTwo.append(dataList[1]) # data 2 from queue
            del (self.yDataLineTwo[0])
            self.pltLineTwo.set_ydata(self.yDataLineTwo)


        plt.draw()  # update the plot
        self.refreshPlot()

        # plt.show()
        #self.fig.canvas.draw()

    def run(self):
        print("Thread has been started and is running")
        self.initPlt()

        ## get from the queue data
        while(self.runningFlag.is_set()):
            try:
                data = self.q.get(timeout=0.01) #get list of Temp values [temp1, temp2] or just [temp]
                self.updatePlot(data)
            except queue.Empty:
                self.refreshPlot()
                continue


        ## exiting
        self.closePlotWindow()

        print("Plotting thread: Exiting.")




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

    xRange = np.arange(0, Nsamples*xStep, xStep)
    pltLineOne, pltLineTwo = plt.plot(xRange, yData, '-g', xRange, yData, '-r') #get the line and fill the y value with 0
    #plt.show()

    # draw the figure so the animations will work
    fig = plt.gcf()
    fig.show()
    fig.canvas.draw()

    return (fig, [pltLineOne, pltLineTwo])

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

# in one thread -> not used now
def PlotData(temp, timePoint):
    print( "y = ", temp, " \nx = ", timePoint)

    updatePlot(timePoint, temp[0], 'r')

    if len(temp) > 1:
        updatePlot(timePoint, temp[1], 'g')

# This was a testing function to read data form device and
# plot in the same thread. Not used now.
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


def ActionReadMultipleMultithread(uart, whichTemp, Nsamples, Mdelay):
    dataQueue = queue.Queue(10)  # for passing temp data to the plotting thread
    isPlotThRunning = threading.Event()
    isPlotThRunning.set()  # set to True -> plotting thread will be running


    # create and start plotting thread:
    plotThread = plotingThread(Queue=dataQueue, Event=isPlotThRunning, Nsamples=Nsamples, Mms=Mdelay,
                               ymin=-1, ymax=100)
    plotThread.start()



    ## stop any running command on embedded
    cmdStop, byteNo = GenerateStopCmd()
    uart.write(cmdStop)
    uart.flush()

    ## flush input buffer
    uart.flushInput()

    # send command
    cmd = GenerateMultipleReadCmd(whichTemp, Nsamples, Mdelay)
    uart.write(cmd)
    uart.flush()

    ##read data from embedded device
    for i in range(Nsamples):
        data = uart.read()  # read byte

        if (data == bytes([0xB1])) or (data == bytes([0xB2])):
            values = uart.read(2)

        elif (data == bytes([0xB3])):
            values = uart.read(4)
        else:
            continue  # unrecognized data hm??

        tempdata = toCdeg(values) #get list of temperatures in Celsius degree
        dataQueue.put(tempdata)  # pass data to the plotting thread


    # finishing
    isPlotThRunning.clear()  # tell plotting thread to finish its job and exit

    ## stop any running command on embedded
    uart.write(cmdStop)
    uart.flush()

    # wait for end of plotting thread
    plotThread.join()




Nsamples = 50
Mms = 300       # in [ms]

dataQueue = queue.Queue(10) # for passing temp data to the plotting thread
isPlotThRunning = threading.Event()
isPlotThRunning.set() # set to True -> plotting thread will be running

#create and start plotting thread:
plotThread = plotingThread(Queue=dataQueue, Event=isPlotThRunning, Nsamples=Nsamples, Mms=Mms,
                           ymin=-1, ymax=100)
plotThread.start()
for i in range(Nsamples):

    plotdata = [i, i - 3]  # data passing to the plotting thread
    dataQueue.put(plotdata) # pass data to the plotting thread

    time.sleep(50.0/1000.0)



# yData = [0] * Nsamples
# yData2 = [0] * Nsamples
# fig, plotLines = initPlot(Nsamples, Mms, yData, -1, 100)
# plotLine = plotLines[0]
# plotLine2 = plotLines[1]
## UART communication
# uart = serial.Serial('/dev/ttyUSB0', 19200, timeout=5)
#
# #print (ActionSaveEmissivity(uart, 0.98)) # save emissivity
#
# #print (  ActionReadOneSample(uart, "both") )
#
# ActionReadMultiple(uart, "both", Nsamples, Mms, PlotData)
#
# uart.close()




input("Press Enter to exit ...")

isPlotThRunning.clear() # tell plotting thread to finish its job and exit

# wait for end of plotting thread
plotThread.join()
