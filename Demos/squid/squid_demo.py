# squid_demo.py --- 
# 
# Filename: squid_demo.py
# Description: 
# Author: Subhasis Ray
# Maintainer: 
# Created: Wed Feb 22 23:24:21 2012 (+0530)
# Version: 
# Last-Updated: Wed Feb 22 23:45:40 2012 (+0530)
#           By: Subhasis Ray
#     Update #: 43
# URL: 
# Keywords: 
# Compatibility: 
# 
# 

# Commentary: 
# 
# 
# 
# 

# Change log:
# 
# 
# 

# Code:

import moose

from squid import SquidAxon
from electronics import ClampCircuit

class SquidDemo(object):
    def __init__(self):
        self.model_container = moose.Neutral('model')
        self.data_container = moose.Neutral('data')
        self.squid_axon = SquidAxon('model/squid_axon')
        self.clamp_ckt = ClampCircuit('model/electronics', self.squid_axon)
        self.simdt = 0.0

        self.vm_table = moose.Table('/data/vm')
        self.vm_table.stepMode = 3
        moose.connect(self.vm_table, 'requestData', self.squid_axon, 'get_Vm')
        
        
    def schedule(self, simdt, plotdt):
        self.simdt = simdt
        moose.setClock(0, simdt)
        moose.setClock(1, simdt)
        moose.setClock(2, simdt)
        moose.setClock(3, plotdt)
        moose.useClock(0, '%s/#[TYPE=Compartment]' % (self.model_container.path), 'init')
        moose.useClock(0, '%s/##' % (self.clamp_ckt.path), 'process')
        moose.useClock(1, '%s/#[TYPE=Compartment]' % (self.model_container.path), 'process')
        moose.useClock(2, '%s/#[TYPE=HHChannel]' % (self.squid_axon.path), 'process')
        moose.useClock(3, '%s/#[TYPE=Table]' % (self.data_container.path), 'process')
        moose.reinit()
        
    def run(self, clamp_mode, runtime):
        if clamp_mode == 0:
            self.clamp_ckt.do_voltage_clamp(self.simdt)
        else:
            self.clamp_ckt.do_current_clamp()
        moose.start(runtime)

    def save_data(self):
        for child in self.data_container.children:
            tab = moose.Table(child)
            tab.xplot('%s.dat' % (tab.name), tab.name)

if __name__ == '__main__':
    demo = SquidDemo()
    demo.schedule(1e-2, 0.1)
    demo.run(1, 50.0)
    demo.save_data()

# 
# squid_demo.py ends here
