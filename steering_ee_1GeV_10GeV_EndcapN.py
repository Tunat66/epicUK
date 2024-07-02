from DDSim.DD4hepSimulation import DD4hepSimulation
from g4units import mm, GeV, MeV, deg
SIM = DD4hepSimulation()

#now configure the test run as given in the tutorial
SIM.gun.thetaMin = 3*deg
SIM.gun.thetaMax = 45*deg
SIM.gun.multiplicity = 2
SIM.gun.distribution = "cos(theta)"
SIM.gun.momentumMin = 1*GeV
SIM.gun.momentumMax = 10*GeV
SIM.gun.particle = "e-"
