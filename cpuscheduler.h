// Define las estructuras necesarias para los algoritmos de planificacion
// y los registros de procesos para armar el Gantt
#ifndef CPU_SCHEDULER_H
#define CPU_SCHEDULER_H

#include <string>
#include <vector>
#include "pcb.h"

// Algoritmos de planificación soportados
enum class SchedAlgorithm {
    RR,
    SRTF
};

// Registro histórico para el Diagrama de Gantt
struct GanttSegment {
    int pid;
    std::string process_name;
    int start_tick;
    int end_tick;
};

// Estado global de la configuración del planificador
extern SchedAlgorithm current_algorithm;
extern int current_quantum;
extern std::vector<GanttSegment> gantt_log;

// Configuración y comandos CLI
bool setSchedulerPolicy(const std::string& algo_str, int quantum = 2);

// Rutina principal del hilo motor del procesador
void* cpuThreadRoutine(void* arg);

// Métodos auxiliares para consultas e impresión de métricas
void printGanttDiagram();
void printSystemStats();

#endif // CPU_SCHEDULER_H