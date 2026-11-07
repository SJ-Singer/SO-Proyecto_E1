//Contiene la enumeración de los estados y la estructura del PCB 
// con sus atributos y métodos.

#ifndef PCB_H
#define PCB_H

#include <string>

// Estados del ciclo de vida del proceso (Exigidos en Sección 2 de la guía)
enum class ProcessState {
    NEW,
    READY,
    RUNNING,
    PAUSED,
    TERMINATED
};

// Convierte el enum del estado a una cadena de texto para la CLI
std::string stateToString(ProcessState state);

// Estructura del Process Control Block (PCB)
struct PCB {
    int pid;                    // Identificador único autoincremental
    std::string name;           // Nombre del proceso (sin espacios)
    ProcessState state;         // Estado actual del proceso
    int priority;               // Prioridad (1 a 5, por defecto 3)

    // Tiempos de Ráfaga de CPU
    int initial_burst;          // Ráfaga total de CPU solicitada (ticks)
    int remaining_burst;        // Ráfaga restante por consumir (ticks)

    // Métricas de Tiempo de Sistema (medidas en clock_ticks)
    int arrival_time;           // Momento en que llegó al sistema
    int first_run_time;         // Instante de primera ejecución (-1 si no ha ejecutado)
    int completion_time;        // Instante en que pasó a TERMINATED (-1 si no ha terminado)
    int waiting_time;           // Tiempo total acumulado en espera (READY)
    int turnaround_time;        // Tiempo total de retorno (Completion - Arrival)
    int response_time;          // Tiempo de respuesta (First_run - Arrival)

    // Constructor
    PCB(int id, std::string proc_name, int burst, int prio = 3, int arrival = 0);

    // Salidas formateadas requeridas por la consola
    void printSummary() const;  // Formato para el comando 'ps'
    void printDetail() const;   // Formato para el comando 'pcb <PID>'
};

#endif // PCB_H