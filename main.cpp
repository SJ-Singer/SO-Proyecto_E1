#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include "pcb.h"
#include "globals.h"

// Rutina del Hilo CLI (Interfaz de Usuario)
void* cliThreadRoutine(void* arg) {
    while (system_running) {
        // El Miembro 2 implementará aquí la lectura y validación de la Shell
        usleep(100000); 
    }
    return nullptr; // <--- Retorno explícito para evitar -Wreturn-type
}

// Rutina del Hilo Motor (CPU Scheduler)
void* cpuThreadRoutine(void* arg) {
    while (system_running) {
        if (cpu_running) {
            pthread_mutex_lock(&system_mutex);
            // El Miembro 3 implementará aquí la lógica de consumo de ráfagas
            clock_ticks++;
            pthread_mutex_unlock(&system_mutex);
        }
        // Retardo por tick (Sección 6: set_speed por defecto 1000ms = 1s)
        usleep(1000000); 
    }
    return nullptr; // <--- Retorno explícito para evitar -Wreturn-type
}

int main() {
    pthread_t cli_thread, cpu_thread;

    std::cout << "[KERNEL] Iniciando Simulador de Sistema Operativo...\n";

    // Creación de los dos hilos POSIX obligatorios
    if (pthread_create(&cli_thread, NULL, cliThreadRoutine, NULL) != 0) {
        std::cerr << "Error al crear Hilo CLI\n";
        return 1;
    }

    if (pthread_create(&cpu_thread, NULL, cpuThreadRoutine, NULL) != 0) {
        std::cerr << "Error al crear Hilo CPU\n";
        return 1;
    }

    // Esperar a que los hilos finalicen ordenadamente antes de salir
    pthread_join(cli_thread, NULL);
    pthread_join(cpu_thread, NULL);

    // Destrucción limpia del cerrojo mutex y liberación de memoria
    pthread_mutex_destroy(&system_mutex);
    cleanupProcesses();

    std::cout << "[KERNEL] Simulación finalizada limpiamente.\n";
    return 0;
}