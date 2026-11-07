//Declara con la palabra clave extern la lista/tabla global de procesos (std::vector<PCB*>),
// el reloj discreto del sistema 
//(clock_ticks) y el cerrojo global pthread_mutex_t.

#ifndef GLOBALS_H
#define GLOBALS_H

#include <vector>
#include <pthread.h>
#include "pcb.h"

// Lista global que mantendrá el registro de todos los procesos
extern std::vector<PCB*> process_table;

// Contador autoincremental de PID
extern int next_pid;

// Reloj discreto del sistema (Clock Ticks)
extern int clock_ticks;

// Mutex de exclusión mutua para proteger el acceso a process_table
extern pthread_mutex_t system_mutex;

// Función de búsqueda auxiliar por PID
PCB* findProcessByPID(int pid);

// Función para liberar la memoria dinámica al cerrar el programa
void cleanupProcesses();

#endif // GLOBALS_H