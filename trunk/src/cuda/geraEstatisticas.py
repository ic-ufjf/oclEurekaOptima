import csv
from subprocess import Popen, PIPE
import time

EXECUCOES = 70

list = []

def run(value):
    media = 0.0
    for x in range(0, EXECUCOES):	
	p = Popen(['./reduction', value], stdout=PIPE)
        output_time = float(p.stdout.read())
        media+=output_time
        time.sleep(0.01)
    
    media=media/EXECUCOES
    list.append(media)  
    print value,' = ', media 
    return media

for i in range(1, 1025):
    media = run(str(i))
    with open('logCuda.txt', 'a') as the_file:
	    #the_file.write(str(i))
	    #the_file.write('\t')
	    the_file.write(str(media))
	    the_file.write('\n')
