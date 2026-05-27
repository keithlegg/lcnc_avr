#!/usr/bin/env python3


import serial, time, hal

#   Created: Alexander Richter 2022
# ruined: Keith Legg 2026  

#   This Software is used as IO Expansion for LinuxCNC. Here i am using a Mega 2560.

#   It is NOT intended for timing and security relevant IO's. Don't use it for Emergency Stops or Endstop switches!



#   The Send and receive Protocol is <Signal><PinNumber>:<Pin State>
#   To begin Transmitting Ready is send out and expects to receive E: to establish connection. 
# Afterwards Data is exchanged.
#   Data is only send everytime it changes once.

#   Inputs & Toggle Inputs    = 'I' -write only  -Pin State: 0,1
#   Outputs                           = 'O' -read only   -Pin State: 0,1




#   Command 'E0:0' is used for connectivity checks and is send every 5 seconds as keep alive signal

#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#   See the GNU General Public License for more details.
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


c = hal.component("avrsimple")  #name that we will cal pins from in hal
connection = '/dev/ttyACM0'       #this is the port your Arduino is connected to. You can check with ""sudo dmesg | grep tty"" in Terminal


# Set how many Inputs you have programmed in Arduino and which pins are Inputs, Set Inputs = 0 to disable
Inputs = 2 
InPinmap = [8,9] #Which Pins are Inputs?


# Set how many Toggled ("sticky") Inputs you have programmed in Arduino and which pins are Toggled Inputs , Set SInputs = 0 to disable
SInputs = 1
sInPinmap = [10] #Which Pins are SInputs?


# Set how many Outputs you have programmed in Arduino and which pins are Outputs, Set Outputs = 0 to disable
Outputs = 2             #9 Outputs, Set Outputs = 0 to disable
OutPinmap = [11,12] #Which Pins are Outputs?




#only works when this script is run from halrun in Terminal. "halrun","loadusr arduino" now Debug info will be displayed.
DEBUG = 0


########  End of Config!  ########

# global Variables for State Saving

olddOutStates= [0]*Outputs



# Inputs and Toggled Inputs are handled the same. 
# For DAU compatiblity we set them up seperately. 
# Here we merge the arrays.

Inputs = Inputs+ SInputs
InPinmap += sInPinmap


# Storing Variables for counter timing Stuff
counter_last_update = {}
min_update_interval = 100


######## SetUp of HalPins ########

# setup Input halpins
for port in range(Inputs):
    c.newpin("din.{}".format(InPinmap[port]), hal.HAL_BIT, hal.HAL_OUT)
    c.newparam("din.{}-invert".format(InPinmap[port]), hal.HAL_BIT, hal.HAL_RW)

# setup Output halpins
for port in range(Outputs):
    c.newpin("dout.{}".format(OutPinmap[port]), hal.HAL_BIT, hal.HAL_IN)
    olddOutStates[port] = 0




c.ready()



#setup Serial connection
arduino = serial.Serial(connection, 115200, timeout=1, xonxoff=False, rtscts=False, dsrdtr=True)

######## GlobalVariables ########
firstcom = 0
event = time.time()
timeout = 9 #send something after max 9 seconds


######## Functions ########

def keepAlive(event):
    return event + timeout < time.time()

def readinput(input_str):
    for i in range(50):

        if input_str:
            string = input_str.decode()  # convert the byte string to a unicode string
            print (string)
            num = int(string) # convert the unicode string to an int
    return num


def extract_nbr(input_str):
    if input_str is None or input_str == '':
        return 0

    out_number = ''
    for i, ele in enumerate(input_str):
        if ele.isdigit() or (ele == '-' and i+1 < len(input_str) and input_str[i+1].isdigit()):
            out_number += ele
    return int(out_number)


def managageOutputs():
    for port in range(Outputs):
        State = int(c["dout.{}".format(OutPinmap[port])])
        
        #check if states have changed
        if olddOutStates[port] != State:    
            Sig = 'O'
            Pin = int(OutPinmap[port])
            command = "{}{}:{}\n".format(Sig,Pin,State)
            arduino.write(command.encode())
            if (DEBUG):print ("Sending:{}".format(command.encode()))
            olddOutStates[port]= State
            time.sleep(0.01)
        




while True:
    try:
        data = arduino.readline().decode('utf-8')                   #read Data received from Arduino and decode it
        if (DEBUG):print ("I received:{}".format(data))
        data = data.split(":",1)

        try:
            cmd = data[0][0]
            if cmd == "":
                if (DEBUG):print ("No Command!:{}".format(cmd))
            
            else:
                if not data[0][1]:
                    io = 0
                else:
                    io = extract_nbr(data[0])

                value = extract_nbr(data[1])
                if (DEBUG):print ("No Command!:{}.".format(cmd))

                if cmd == "I":
                    firstcom = 1
                    if value == 1:
                        if c["din.{}-invert".format(io)] == 0:
                            c["din.{}".format(io)] = 1
                            if(DEBUG):print("din{}:{}".format(io,1))
                        else: 
                            c["din.{}".format(io)] = 0
                            if(DEBUG):print("din{}:{}".format(io,0))
                        
                    if value == 0:
                        if c["din.{}-invert".format(io)] == 0:
                            c["din.{}".format(io)] = 0
                            if(DEBUG):print("din{}:{}".format(io,0))
                        else: 
                            c["din.{}".format(io)] = 1
                            if(DEBUG):print("din{}:{}".format(io,1))
                    else:
                        pass

                elif cmd == "A":
                    pass

                elif cmd == "L":
                    pass

                elif cmd == "K":
                    pass

                elif cmd == "M":
                    pass
                                
                elif cmd == "R":
                    pass

                elif cmd == 'E':
                        arduino.write(b"E0:0\n")
                        if (DEBUG):print("Sending E0:0 to establish contact")
                else: pass
    

        except: pass
    

    except KeyboardInterrupt:
        if (DEBUG):print ("Keyboard Interrupted.. BYE")
        exit()
    except: 
        if (DEBUG):print ("I received garbage")
        arduino.flush()
    
    if firstcom == 1: managageOutputs()     #if ==1: E0:0 has been exchanged, which means Arduino knows that LinuxCNC is running and starts sending and receiving Data

    if keepAlive(event):    #keep com alive. This is send to help Arduino detect connection loss.
        arduino.write(b"E:\n")
        if (DEBUG):print("keepAlive")
        event = time.time()
    
    time.sleep(0.001)   
    
