import serial

# Generate save emissivity command
# emissivity must be a double in range 0.1 - 1.0
# returns cmd bytes and number of expected received bytes
def GenerateSaveEmissivityCmd(emissivity):
    cmdByte = 0x55

    emSaveVal = int(round(emissivity * 65535.0))

    return (bytes([cmdByte, emSaveVal & 0xFF, (emSaveVal >> 8) & 0xFF]), 1)

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


# returns cmd bytes and number of expected received bytes
def GenerateStopCmd():
    return (bytes([0xE8, 0x0A]), 0)

# returns cmd bytes and number of expected received bytes
def GenerateAliveCmd():
    return (bytes([0xEE, 0x0A]), 1)


def PrintHex(bytes):
    print( binascii.hexlify(bytearray(bytes)) )



class PirConnManager:
    def __init__(self):
        self.uart = serial.Serial('/dev/ttyUSB0', 19200, timeout=5)

    def __del__(self):
        self.uart.close()
        print(self.__class__.__name__, " deleting")

    # Params:
    # uart -> operational serial handler
    # whichTemperature -> string, which should contain: "ambient", "object", "both"
    # Returns:
    # List with wanted temperature/s. If "ambient" or "object" was requested then return one element list,
    #   but when wanted "both", then first element is "Ambient" and second is an "object" temperature.
    def readOneSample(self, whichTemperature):
        cmd, byteNo = GenerateOneTimeReadCmd(whichTemperature)
        # print( "command will be send: ")
        # PrintHex(cmd)
        self.uart.flushInput()    # remove all data in input buffer
        self.uart.write(cmd)
        self.uart.flush()
        readVal = self.uart.read(byteNo)
        # print( "received data: ")
        # PrintHex( readVal )
        return self.toCdeg(readVal)


    # Params:
    # emissivity: float value in range 0.1 - 1.0
    # returns:
    # True - if saving was ok
    # False - if saving failed
    def saveEmissivity(self, emissivity):
        if not (emissivity >= 0.1 and emissivity <= 1.0):
            return False

        cmd, byteNo = GenerateSaveEmissivityCmd(emissivity)
        self.uart.flushInput()   # remove all data in input buffer
        self.uart.write(cmd)
        self.uart.flush()
        return self.uart.read(byteNo) == bytes([0x55])    # 0x55 is expected success value



    def sendStopCommand(self):
        cmd, bytesNo = GenerateStopCmd()
        self.uart.flushInput()
        self.uart.write(cmd)
        self.uart.flush()


    # Param:
    # bytes - bytes object with data read from UART
    # returns float
    def toCdeg(self, bytes):
        size = len(bytes)

        temp = []

        if size == 3:
            temp.append(((bytes[2] << 8 | bytes[1]) * 0.02) - 273.15)
        elif size == 2:
            temp.append(((bytes[1] << 8 | bytes[0]) * 0.02) - 273.15)  # case if start byte was omitted
        elif size == 5:
            temp.append(((bytes[2] << 8 | bytes[1]) * 0.02) - 273.15)  # first temp
            temp.append(((bytes[4] << 8 | bytes[3]) * 0.02) - 273.15)  # second temp
        elif size == 4:
            temp.append(((bytes[1] << 8 | bytes[0]) * 0.02) - 273.15)  # first temp
            temp.append(((bytes[3] << 8 | bytes[2]) * 0.02) - 273.15)  # case if start byte was omitted

        return temp