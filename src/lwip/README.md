## README.md

This folder contains a patch of lwIP SNMP app.

The patch modifies handling of snmp system mibs and let them be managed by  
profinet. The lwip files are copied into lwip mw during prebuild step. They  
are not intended to be built in the context of the the uphy (this) mw and  
therefore excluded using .cyignore.
