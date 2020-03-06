# winRE-utils
Utilities used for reverse engineering windows apps (Educational).  

This repo contains two DLL injectors.
1. The first one is static. The user must specify the name of the process to be injected into and the path to the DLL on the system. 
2. The second one is remote. It make a request to a webserver retrieving a payload dll and injects into a predefined process. 
