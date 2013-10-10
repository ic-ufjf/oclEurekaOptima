import csv
from subprocess import Popen, PIPE
import time

EXECUCOES = 5

list = []

def run(value):
    media = 0.0
    for x in range(1, EXECUCOES):	

        p = Popen(['./opencl', value], stdout=PIPE)
        output_time = float(p.stdout.read())
        media+=output_time
        #time.sleep(0.001)
    
    media=media/EXECUCOES
    list.append(media)  
    print value,' = ', media 
    return media    
       	      

for i in range(642,643):
    media = run(str(i))
    with open('log.txt', 'a') as the_file:
	    the_file.write(str(i))
	    the_file.write('\t')
	    the_file.write(str(media))
	    the_file.write('\n')
