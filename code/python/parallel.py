from subprocess import Popen
from os import mkdir

argfile = open('commandline.txt')
for number, line in enumerate(argfile):    
    newpath = 'scatter.%03i' % number 
    mkdir(newpath)
    cmd = '../abc.py ' + line.strip()
    print 'Running %r in %r' % (cmd, newpath)
    Popen(cmd, shell=True, cwd=newpath)