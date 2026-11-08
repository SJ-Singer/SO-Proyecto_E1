#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include "pcb.h"
#include "globals.h"
#include "cpuscheduler.h"
#include "cli.h"

//Elimine la parte de la rutina de los hilos pq deberia estar en los archivos correspondientes
// no en el main, el main es quien los llama
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