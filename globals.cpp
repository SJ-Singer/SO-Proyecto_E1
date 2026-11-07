// Define e inicializa las instancias 
// reales de esas variables globales compartidas.

#include "globals.h"

std::vector<PCB*> process_table;
int next_pid = 1;
int clock_ticks = 0;
// Variables de control de la simulación (Para Shell CLI)
bool is_simulation_running = false; // El motor inicia pausado
int simulation_speed_ms = 1000;     // Velocidad por defecto: 1000 ms (1 seg)

// Inicialización estática del Cerrojo Mutex POSIX
pthread_mutex_t system_mutex = PTHREAD_MUTEX_INITIALIZER;

PCB* findProcessByPID(int pid) {
    for (PCB* proc : process_table) {
        if (proc->pid == pid) {
            return proc;
        }
    }
    return nullptr;
}

void cleanupProcesses() {
    for (PCB* proc : process_table) {
        delete proc;
    }
    process_table.clear();
}

// Configuración de Hilos POSIX y Cerrojos Mutex
bool system_running = true;
bool cpu_running = false;

void addProcessToSystem(PCB* proc) {
    pthread_mutex_lock(&system_mutex);
    process_table.push_back(proc);
    pthread_mutex_unlock(&system_mutex);
}