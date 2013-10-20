import numpy as np
from matplotlib.pylab import *

t1   = np.genfromtxt("logGruposGPU.txt",delimiter="\n")
t2   = np.genfromtxt("logTwoStageGPU1.txt",delimiter="\n")
t3   = np.genfromtxt("logCUDA.txt", delimiter = "\n");

print len(t1), len(t3)

print t1


t = zeros(len(t1))
for i in range(0,len(t)):
    t[i] = i+1
    
title('Reducao paralela - GPU')

plot(t, t1, 'r-')
legend(['Grupos'])
hold('on')

plot(t, t2, 'b-')
hold('on')

plot(t, t3, 'g-')
hold('on')


xlabel('tamanho')
ylabel('tempo (ms)')


legend(['Grupos (OpenCL)','Two stage(OpenCL)', 'Grupos (CUDA)'])
savefig('reducao_paralela_gpus.png')
    
show()





