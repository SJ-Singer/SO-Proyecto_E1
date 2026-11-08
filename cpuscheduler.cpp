#include "CPUScheduler.h"
#include "globals.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <algorithm>

// Configuración global del Planificador
SchedAlgorithm current_algorithm = SchedAlgorithm::RR;
int current_quantum = 2;
std::vector<GanttSegment> gantt_log;

// Auxiliar para convertir enum del algoritmo a texto
std::string algorithmToString(SchedAlgorithm algo) {
    return (algo == SchedAlgorithm::RR) ? "Round Robin (RR)" : "SRTF (Apropiativo)";
}

// Validación y cambio en caliente del algoritmo de planificación
bool setSchedulerPolicy(const std::string& algo_str, int quantum) {
    if (algo_str == "RR") {
        if (quantum <= 0) return false;
        current_algorithm = SchedAlgorithm::RR;
        current_quantum = quantum;
        return true;
    } else if (algo_str == "SRTF") {
        current_algorithm = SchedAlgorithm::SRTF;
        if (quantum > 0) current_quantum = quantum; // Quantum es opcional/ignorado en SRTF puro
        return true;
    }
    return false;
}

// Función auxiliar para registrar tramos en el diagrama de Gantt
void recordGantt(int pid, const std::string& name, int start, int end) {
    if (start == end) return;
    if (!gantt_log.empty() && gantt_log.back().pid == pid) {
        gantt_log.back().end_tick = end;
    } else {
        gantt_log.push_back({pid, name, start, end});
    }
}

// Rutina del Hilo Motor (CPU Scheduling Engine)
void* cpuThreadRoutine(void* arg) {
    PCB* current_process = nullptr;
    int quantum_counter = 0;

    while (system_running) {
        int sleep_duration = 1000;

        pthread_mutex_lock(&system_mutex);
        sleep_duration = simulation_speed_ms;

        if (is_simulation_running) {
            // 1. Transición de procesos NEW a READY que hayan llegado en el tick actual
            for (PCB* proc : process_table) {
                if (proc->state == ProcessState::NEW && proc->arrival_time <= clock_ticks) {
                    proc->state = ProcessState::READY;
                }
            }

            // 2. Selección de proceso si la CPU está ociosa (IDLE) o evaluación de Apropiación (SRTF)
            if (current_algorithm == SchedAlgorithm::SRTF) {
                // Seleccionar el proceso en READY o RUNNING con el menor remaining_burst
                PCB* shortest = current_process;

                for (PCB* proc : process_table) {
                    if (proc->state == ProcessState::READY) {
                        if (shortest == nullptr || proc->remaining_burst < shortest->remaining_burst) {
                            shortest = proc;
                        }
                    }
                }

                // Desalojo / Precepción si se encontró un proceso con menor tiempo restante
                if (shortest != nullptr && shortest != current_process) {
                    if (current_process != nullptr && current_process->state == ProcessState::RUNNING) {
                        current_process->state = ProcessState::READY;
                    }
                    current_process = shortest;
                    current_process->state = ProcessState::RUNNING;
                }
            } 
            else if (current_algorithm == SchedAlgorithm::RR) {
                // Si no hay proceso ejecutándose, extraemos el primero en READY
                if (current_process == nullptr || current_process->state != ProcessState::RUNNING) {
                    for (PCB* proc : process_table) {
                        if (proc->state == ProcessState::READY) {
                            current_process = proc;
                            current_process->state = ProcessState::RUNNING;
                            quantum_counter = 0;
                            break;
                        }
                    }
                }
            }

            // 3. Ejecución de un Tick de CPU
            if (current_process != nullptr && current_process->state == ProcessState::RUNNING) {
                // Registrar tiempo de primera respuesta (RT)
                if (current_process->first_run_time == -1) {
                    current_process->first_run_time = clock_ticks;
                    current_process->response_time = current_process->first_run_time - current_process->arrival_time;
                }

                // Consumo de ráfaga
                current_process->remaining_burst--;
                quantum_counter++;

                // Actualizar diagrama de Gantt
                recordGantt(current_process->pid, current_process->name, clock_ticks, clock_ticks + 1);

                // Incrementar tiempo de espera (WT) a todos los demás procesos en estado READY
                for (PCB* proc : process_table) {
                    if (proc != current_process && proc->state == ProcessState::READY) {
                        proc->waiting_time++;
                    }
                }

                // Verificación de Finalización del Proceso
                if (current_process->remaining_burst == 0) {
                    current_process->state = ProcessState::TERMINATED;
                    current_process->completion_time = clock_ticks + 1;
                    current_process->turnaround_time = current_process->completion_time - current_process->arrival_time;
                    current_process = nullptr;
                    quantum_counter = 0;
                }
                // Verificación de Agotamiento de Quantum (Solo para Round Robin)
                else if (current_algorithm == SchedAlgorithm::RR && quantum_counter >= current_quantum) {
                    current_process->state = ProcessState::READY;
                    
                    // Rotar en la cola: mover el proceso al final de la lista de evaluación
                    // (manteniendo la concordancia de orden de llegada en la cola READY)
                    auto it = std::find(process_table.begin(), process_table.end(), current_process);
                    if (it != process_table.end()) {
                        process_table.erase(it);
                        process_table.push_back(current_process);
                    }

                    current_process = nullptr;
                    quantum_counter = 0;
                }
            } else {
                // CPU Ociosa (IDLE)
                recordGantt(-1, "IDLE", clock_ticks, clock_ticks + 1);
            }

            // Incrementar el reloj discreto del sistema (+1 tick)
            clock_ticks++;
        }

        pthread_mutex_unlock(&system_mutex);

        // Retardo controlado en tiempo real para simulación en cámara lenta
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_duration));
    }

    return nullptr;
}

// Despliegue del Diagrama de Gantt acumulado
void printGanttDiagram() {
    pthread_mutex_lock(&system_mutex);

    if (gantt_log.empty()) {
        std::cout << "\nDIAGRAMA DE GANTT (0 - 0 ticks):\n[VACIO]\n";
        pthread_mutex_unlock(&system_mutex);
        return;
    }

    std::cout << "\nDIAGRAMA DE GANTT (0 - " << clock_ticks << " ticks):\n";
    for (size_t i = 0; i < gantt_log.size(); ++i) {
        const auto& seg = gantt_log[i];
        std::cout << "[" << seg.start_tick << "-" << seg.end_tick << "s: ";
        if (seg.pid == -1) {
            std::cout << "IDLE]";
        } else {
            std::cout << seg.process_name << "]";
        }

        if (i < gantt_log.size() - 1) {
            std::cout << " -> ";
        }
    }
    std::cout << "\n";

    pthread_mutex_unlock(&system_mutex);
}

// Cálculo e impresión de estadísticas globales del sistema
void printSystemStats() {
    pthread_mutex_lock(&system_mutex);

    int total_processes = process_table.size();
    int terminated_processes = 0;
    double total_wt = 0.0;
    double total_tat = 0.0;
    double total_rt = 0.0;
    int active_busy_ticks = 0;

    for (const PCB* proc : process_table) {
        if (proc->state == ProcessState::TERMINATED) {
            terminated_processes++;
            total_wt += proc->waiting_time;
            total_tat += proc->turnaround_time;
            total_rt += proc->response_time;
        }
    }

    for (const auto& seg : gantt_log) {
        if (seg.pid != -1) {
            active_busy_ticks += (seg.end_tick - seg.start_tick);
        }
    }

    double cpu_utilization = (clock_ticks > 0) ? ((double)active_busy_ticks / clock_ticks) * 100.0 : 0.0;

    std::cout << "\nESTADÍSTICAS DEL SISTEMA:\n";
    std::cout << "Procesos Finalizados      : " << terminated_processes << "/" << total_processes << "\n";

    if (terminated_processes > 0) {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Tiempo Promedio de Espera  : " << (total_wt / terminated_processes) << " ticks\n";
        std::cout << "Tiempo Promedio de Retorno : " << (total_tat / terminated_processes) << " ticks\n";
        std::cout << "Tiempo Promedio de Respuesta: " << (total_rt / terminated_processes) << " ticks\n";
    } else {
        std::cout << "Tiempo Promedio de Espera  : N/A\n";
        std::cout << "Tiempo Promedio de Retorno : N/A\n";
        std::cout << "Tiempo Promedio de Respuesta: N/A\n";
    }

    std::cout << std::fixed << std::setprecision(1);
    std::cout << "Uso de CPU                 : " << cpu_utilization << "%\n\n";

    pthread_mutex_unlock(&system_mutex);
}