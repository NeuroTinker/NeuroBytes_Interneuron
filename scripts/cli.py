import serial
import matplotlib.pyplot as plt
import sys
import select
import re
import threading
import _thread
import queue
import time
import hipsterplot

ser = serial.Serial('/dev/ttyACM0')
command_buffer = queue.Queue()
plt.axis = ([0,40,0,150])
plt.ion()
data = 10*[40*[0]]
axis = ([0,40,0,150])
graph_flag = 10*[0]
channel_ids = [0, 0b100, 0b101, 0b110, 0b111]
nid_in_count = 0


def main():
    print('Connected on ' + ser.name)
    t = 0
    #command_buffer = []
    #_thread.start_new_thread(input_thread, (command_buffer,))
    #_thread.start_new_thread(input_thread)
    quit_lock = threading.Lock()
    quit_event = threading.Event()
    quit_flag = 0
    command_thread = threading.Thread(target=command_loop, args=(quit_flag,))
    command_thread.start()
    nid_in_count = 0
    while True:
        # print(command_in)
        #command_in = command_buffer[0]
        #command_in = command_buffer.get()
        while not command_buffer.empty():
            command_in = command_buffer.get()
            if re.search('blink', command_in):
                blink()
            elif re.search('identify', command_in):
                channel = command_in[9]
                if isinstance(int(channel), int):
                    identify(int(channel))
                else:
                    print("Error: use syntax 'identify X' where X is channel number")
            elif re.search('quit', command_in):
                quit_flag = 1
                break
        #time.sleep(0.01)
        if quit_flag == 1:
            #input_thread.join()
            print('exiting')
            break
        if ser.in_waiting > 0:
            nid_in = int(ser.readline(ser.inWaiting()).decode('ascii'))
            if (nid_in > 300):
                nid_in = nid_in - 65536
            data[0].pop(0)
            data[0].append(nid_in)
            hipsterplot.plot(data[0])
            #plt.clf()
            #plt.plot(range(40), data[0])
            #plt.pause(0.01)
            #print(nid_in)

def command_loop(quit_flag):
    while quit_flag == 0:
        for line in sys.stdin:
            command_buffer.put(line)
        pass
    #command_in = input()
    #command_buffer.append(command_in)

def blink():
    ser.write(b'1')

def identify(channel):
    ser.write(b'2')
    #ser.write(channel_ids[channel])

if __name__ == "__main__":
    main()