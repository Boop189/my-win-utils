# winRE-utils
Utilities used for reverse engineering windows apps.  

#DLL Injectors - RedTeam (C++ examples)

This repo contains two DLL injectors.
1. The first one is static. The user must specify the name of the process to be injected and the DLL on the host system. 
2. The second one is remote. It make a request to a webserver retrieving payload dll and injects into a predefined process. 
