import numpy as np
from matplotlib.pylab import *

t1   = np.genfromtxt("logGrupos.dat",delimiter="\n")
t2 = np.genfromtxt("logTwoStage.dat",delimiter="\n")

t = zeros(len(t2))
for i in range(0,len(t)):
    t[i] = i+1

title('Reducao paralela - CPU')

plot(t, t1, 'r-')
legend(['Grupos'])
hold('on')
plot(t, t2, 'b-')
hold('on')


xlabel('tamanho')
ylabel('tempo (ms)')


legend(['Grupos','Two stage'])
savefig('tempo.png')
    
show()






print tempoGrupos
print tempoTwoStage



