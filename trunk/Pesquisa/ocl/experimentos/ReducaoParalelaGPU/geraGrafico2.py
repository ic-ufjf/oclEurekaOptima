import numpy as np
from matplotlib.pylab import *


t1   = np.genfromtxt("logTwoStageGPU.txt",delimiter="\n")


t = zeros(len(t1))
for i in range(0,len(t)):
    t[i] = i+1
    
title('Reducao paralela - GPU')

plot(t, t1, 'r-')
legend(['Two stage'])
hold('on')



xlabel('tamanho')
ylabel('tempo (ms)')


legend(['Two stage(OpenCL)'])
savefig('reducao_paralela_ts.png')
    
show()





