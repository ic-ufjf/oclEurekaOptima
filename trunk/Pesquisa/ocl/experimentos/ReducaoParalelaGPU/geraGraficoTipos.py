import numpy as np
from matplotlib.pylab import *

t1   = np.genfromtxt("logFloatGPUGrupos.txt",delimiter="\n")
t2   = np.genfromtxt("logDoubleGPUGrupos.txt",delimiter="\n")
t3   = np.genfromtxt("logTwoStageGPU1.txt", delimiter = "\n");
t4   = np.genfromtxt("logTwoStageDoubleGPU.txt", delimiter = "\n");

t = zeros(len(t1))
for i in range(0,len(t)):
    t[i] = i+1
    
title('Reducao paralela - GPU (OpenCL)')

plot(t, t1, 'r-')
legend(['Grupos'])
hold('on')

plot(t, t2, 'b-')
hold('on')

plot(t, t3, 'g-')
hold('on')

plot(t, t4, 'y-')
hold('on')


xlabel('tamanho')
ylabel('tempo (ms)')


legend(['Grupos (float)', 'Grupos (double)', 'Two stage(float)', 'Two stage (double)', ])
savefig('reducao_paralela_gpus_datatypes.png')
    
show()





