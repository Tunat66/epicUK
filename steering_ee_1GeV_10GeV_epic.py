from DDSim.DD4hepSimulation import DD4hepSimulation
from g4units import mm, GeV, MeV, deg
SIM = DD4hepSimulation()

#now configure the test run as given in the tutorial
SIM.gun.thetaMin = 3*deg
SIM.gun.thetaMax = 180*deg
SIM.gun.multiplicity = 2
SIM.gun.distribution = "cos(theta)"
SIM.gun.momentumMin = 0.001*MeV
SIM.gun.momentumMax = 1000*MeV 
SIM.gun.particle = "mu-"
SIM.gun.position = (0, 0, 0)
#SIM.gun.position = (0, 0, 0)
