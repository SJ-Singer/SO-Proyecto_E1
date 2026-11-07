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

// Variables de control de la simulación (Para Shell CLI)
extern bool is_simulation_running;
extern int simulation_speed_ms;


extern bool system_running; // Controla si la aplicación sigue viva (false con exit)
extern bool cpu_running;    // Controla si la CPU está ejecutando ticks (start/pause)

// Funciones thread-safe para manipular la tabla de procesos protegida con mutex
void addProcessToSystem(PCB* proc);

#endif // GLOBALS_H