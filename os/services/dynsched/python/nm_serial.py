"""
Network Manager Application - Dynamic Scheduling with TSCH
"""

import threading
import serial
import time
import re

ser0=serial.Serial('/dev/ttyUSB0', 115200, timeout=10)
threads = []
quitThread = 0
sch_len = 4 #DYNSCHED_TSCH_SCHEDULE_DEFAULT_LENGTH
MAX_LINKS = 4 #FRAME802154E_IE_MAX_LINKS
schedulesArray = []
estring = ""

'''
A 2-d Array to construct schedule objects
schArr[x][0] = Timeslot
schArr[x][1] = Channel Offset
schArr[x][2] = Link Option (1=TX/2=RX)
schArr[x][3] = NodeID

'''
schArr1 = [[0, 0, 1, 1],
	  [1, 0, 1, 2],
	  [2, 0, 1, 3],
	  [3, 0, 1, 4]]

schArr2 = [[0, 0, 1, 1],
	  [1, 0, 1, 3],
	  [2, 0, 1, 2],
	  [3, 0, 1, 4]]

schArr3 = [[0, 0, 1, 1],
	  [1, 0, 1, 4],
	  [2, 0, 1, 2],
	  [3, 0, 1, 3]]

schArr4 = [[0, 0, 1, 1],
		[1, 0, 1, 2],
		[2, 0, 1, 4]]

schArr5 = [[0, 0, 1, 1],
		[1, 0, 1, 3],
		[2, 0, 1, 4]]

schArr6 = [[0, 0, 1, 2],
		[1, 0, 1, 3],
		[2, 0, 1, 1]]



class link_info:
	"""
	Initialise link_info object with the link parameters
	:params ts, choff: Timeslot, Channel offset
	:params lo, nid: Link Option, NodeID
	:return: link_info object to be a part of the schedule
	"""
	def __init__(self, ts, choff, lo, nid):
		self.timeslot = ts
		self.channel_offset = choff
		self.linkopt = lo
		self.nodeid = nid

class schedule:
	"""
	Initialize a schedule with the number of links
	:param num: Number of links in the schedule
	:return: Schedule object
	"""
	def __init__(self, num):
		self.num_links = num
		self.link_info_links = []

def create_schedule_from_array(schArr):
	"""
    Create and return the schedule object
    :return: schedule object
    """
	numlinks = len(schArr)
	sch = schedule(numlinks)
	for i in range(numlinks):
		sch.link_info_links.append(link_info(schArr[i][0], schArr[i][1], 											schArr[i][2], schArr[i][3]))
	return sch

def print_schedule_obj(sch):
	"""
	Print the details of passed schedule object
	"""
	#print ('\n********* Printing Schedule object *********\n')	
	print ('\n---------------------------------------------\n')
	print ('Number of links: ' + str(sch.num_links))
	for i in range(sch.num_links):
		print('Link {}:  Timeslot:{}  Choff:{} Linkopt:{} NodeID:{}'
				.format(i, sch.link_info_links[i].timeslot,
						sch.link_info_links[i].channel_offset,
						sch.link_info_links[i].linkopt,
						sch.link_info_links[i].nodeid))
	print ('\n---------------------------------------------\n')
	return


def print_schedule_choices():
	"""
	Prints the Schedule choices present in schedulesArray
	"""	
	print ('\n*************************************************')
	print ('	      Choice of Schedules 	    ')
	#print ('[Timeslot, Channel Offset, Link option, Node ID]\n')
	for i in range(len(schedulesArray)):
		print('Choice ' + str(i+1) + ': ')
		print_schedule_obj(schedulesArray[i])
	print ('*************************************************\n')
	return


def format_schedule_obj(in_schedule):
	"""
	Formats the schedule object to a string of suitable format
	:return: string constructed from the input schedule
	"""
	sch_str = ""
	sch_str+= "N" + str(in_schedule.num_links) + " "
	for i in range(in_schedule.num_links):
		sch_str+= "L" + str(i) + " "
		sch_str+= str(in_schedule.link_info_links[i].timeslot) + ','
		sch_str+= str(in_schedule.link_info_links[i].channel_offset) + ','
		sch_str+= str(in_schedule.link_info_links[i].linkopt) + ','
		sch_str+= str(in_schedule.link_info_links[i].nodeid) + " "
	return sch_str

def dynamic_scheduler(num):
	"""
	Accepts choice of schedules, picks the chosen schedule
	from schedulesArray, formats the schedule 
    in a string and sends the same to the Coordinator 
	connected to the serial port.
	"""
	global quitThread
	while not quitThread:
		print_schedule_choices();
		raw = raw_input('Enter your schedule choice:\n')
		try:
			choice = int(raw)
			print('\nChoice is ' + str(choice) + '\n')
			#Use choice to select schedules
			if ((choice > len(schedulesArray)) or (choice == 0)):
				print("\n!!!! Enter a Valid choice !!!!\n")
				continue
			scheduleChosen = schedulesArray[choice - 1]
			fmt_sch = format_schedule_obj(scheduleChosen)
			print('formatted schedule: {}'.format(fmt_sch))
			write_to_serialport(fmt_sch)
		except ValueError:
			quitThread = 1	
	print("\n******* Quitting scheduler thread********\n")	
	return

def write_to_serialport(sch):
	"""
	Writes the schedule onto the serial port
	:param sch: Schedule to write to the serial port
	# Improvement: 
	# Send schedules until an Ack is received
	# indicating a valid schedule
	"""
	ser0.write(sch + '\r\n')
	return	

def read_from_serialport(num):
	"""
	Reads out logs from serial port
	"""
	resultArr = []
	while not quitThread:
		readOut = ser0.readline()
		print("Received:" + readOut)
		matchObj = re.match('\[WARN: DYNSCHED .* Diff= (.*) ticks', readOut)
		if (matchObj):
			print("\nFound!!! Diff is " + matchObj.group(1) + '\n')
			resultArr.append(matchObj.group(1))
	print("Result Array is " + str(resultArr))
	print("\n******* Quitting read thread********\n")
	return

#Populate schedulesArray 
sch1 = create_schedule_from_array(schArr1)
schedulesArray.append(sch1)
sch2 = create_schedule_from_array(schArr2)
schedulesArray.append(sch2)
sch3 = create_schedule_from_array(schArr3)
schedulesArray.append(sch3)
sch4 = create_schedule_from_array(schArr4)
schedulesArray.append(sch4)
sch5 = create_schedule_from_array(schArr5)
schedulesArray.append(sch5)
sch6 = create_schedule_from_array(schArr6)
schedulesArray.append(sch6)

rthread = threading.Thread(target=read_from_serialport, args=(1,))
threads.append(rthread)
rthread.start()

sthread = threading.Thread(target=dynamic_scheduler, args=(1,))
threads.append(sthread)
sthread.start()

rthread.join()
sthread.join()

