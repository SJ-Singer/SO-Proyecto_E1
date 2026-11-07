// Define e inicializa las instancias 
// reales de esas variables globales compartidas.

#include "globals.h"

std::vector<PCB*> process_table;
int next_pid = 1;
int clock_ticks = 0;

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