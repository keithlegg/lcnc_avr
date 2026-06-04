#!/usr/bin/env python3


import serial, time


#   Created: Alexander Richter 2022
# ruined: Keith Legg 2026  

#   This is a "standalone" version of the pythons script that does not need LinuxCNC to work. 
#  Keith made it for testing a WIP version of the firmware that is written in AVR C without the arduino libs 


#   The Send and receive Protocol is <Signal><PinNumber>:<Pin State>
#   To begin Transmitting Ready is send out and expects to receive E: to establish connection. 

#   Afterwards Data is exchanged.
#   Data is only send everytime it changes once.

#   Inputs & Toggle Inputs    = 'I' -write only  -Pin State: 0,1
#   Outputs                   = 'O' -read only   -Pin State: 0,1




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


#connection = '/dev/ttyACM0'      
connection = '/dev/ttyUSB0'


# Set how many Inputs you have programmed in Arduino and which pins are Inputs, Set Inputs = 0 to disable
Inputs = 2 
InPinmap = [8,9] #Which Pins are Inputs?


# Set how many Toggled ("sticky") Inputs you have programmed in Arduino and which pins are Toggled Inputs , Set SInputs = 0 to disable
SInputs = 1
sInPinmap = [10] #Which Pins are SInputs?


# Set how many Outputs you have programmed in Arduino and which pins are Outputs, Set Outputs = 0 to disable
Outputs = 2             #9 Outputs, Set Outputs = 0 to disable
OutPinmap = [11,12] #Which Pins are Outputs?



DEBUG = 1


########  End of Config!  ########

# global Variables for State Saving

olddOutStates= [0]*Outputs



# Inputs and Toggled Inputs are handled the same. 
# For DAU compatiblity we set them up seperately. 
# Here we merge the arrays.

Inputs   = Inputs+ SInputs
InPinmap += sInPinmap


# Storing Variables for counter timing Stuff
counter_last_update = {}
min_update_interval = 100



#setup Serial connection
arduino = serial.Serial(connection, 115200, timeout=1, xonxoff=False, rtscts=False, dsrdtr=True)

######## GlobalVariables ########
firstcom = 0
event    = time.time()
timeout  = 9 #send something after max 9 seconds


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



        




while True:
    try:
        data = arduino.readline().decode('utf-8')  
    
        if (DEBUG):
            print ("I received:{}".format(data))
            #print ("I received: ", data)
            print("-----------\n")

        data = data.split(":",1)

        #?? why ??
        #print() 

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
                
                if (DEBUG):
                    print ("No Command!:{}.".format(cmd))

                if cmd == "I":
                    print("command I")

                    firstcom = 1

                    if value == 1:
                        if(DEBUG):
                            print("I1");
                    if value == 0:
                        if(DEBUG):
                            print("I0");


                elif cmd == "A":
                    print("command A")

                elif cmd == "L":
                    print("command L")

                elif cmd == "K":
                    print("command K")

                elif cmd == "M":
                    print("command M")
                                
                elif cmd == "R":
                    print("command R")

                elif cmd == 'E':
                    arduino.write(b"E0:0\n")
                    if (DEBUG):
                        print("Sending E0:0 to establish contact")
                else: 
                    pass
    

        except: 
            pass
    

    except KeyboardInterrupt:
        if (DEBUG):
            print ("Keyboard Interrupted.. BYE")
        exit()

    except: 
        if (DEBUG):
            print ("I received garbage")
        arduino.flush()
    
    if firstcom == 1: 
        #if ==1: E0:0 has been exchanged, which means Arduino knows that LinuxCNC is running and starts sending and receiving Data
        #managageOutputs()     
        print('debug firstcom ')

    #keep com alive. This is send to help Arduino detect connection loss.
    if keepAlive(event):    
        arduino.write(b"E:\n")
        
        if (DEBUG):
            print("keep alive")
        
        event = time.time()
    
    time.sleep(0.001)   
    
