## Linux Process Synchronization

This repository contains a program that creates, manages, and synchronizes access to a shared variable v1 by n processes using the reverse order of their arrival.

### Run The Program

```
gcc pgme_processus.c -o pgme_processus

gcc pgme_principal.c -o pgme_principal

./pgme_principal
```