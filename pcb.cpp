// Contiene la lógica del constructor del PCB, funciones para formatear los estados 
// a texto y las rutinas de impresión de las tablas ps y pcb <PID>

#include "pcb.h"
#include <iostream>
#include <iomanip>

std::string stateToString(ProcessState state) {
    switch (state) {
        case ProcessState::NEW:        return "NEW";
        case ProcessState::READY:      return "READY";
        case ProcessState::RUNNING:    return "RUNNING";
        case ProcessState::PAUSED:     return "PAUSED";
        case ProcessState::TERMINATED: return "TERMINATED";
        default:                       return "UNKNOWN";
    }
}

PCB::PCB(int id, std::string proc_name, int burst, int prio, int arrival)
    : pid(id), name(proc_name), state(ProcessState::NEW), priority(prio),
      initial_burst(burst), remaining_burst(burst), arrival_time(arrival),
      first_run_time(-1), completion_time(-1), waiting_time(0),
      turnaround_time(0), response_time(0) {}

// Salida tipo tabla para comando 'ps'
void PCB::printSummary() const {
    std::cout << std::left << std::setw(15) << name
              << std::setw(12) << stateToString(state)
              << std::setw(8)  << pid
              << std::setw(18) << (std::to_string(remaining_burst) + "s")
              << std::setw(10) << priority << "\n";
}

// Salida detallada para comando 'pcb <PID>'
void PCB::printDetail() const {
    std::cout << "=========================================\n";
    std::cout << "      CONTROL BLOCK DE PROCESO (PCB)     \n";
    std::cout << "=========================================\n";
    std::cout << "Nombre del Proceso : " << name << "\n";
    std::cout << "PID                : " << pid << "\n";
    std::cout << "Estado Actual      : " << stateToString(state) << "\n";
    std::cout << "Prioridad          : " << priority << "\n";
    std::cout << "Tiempo de Llegada  : " << arrival_time << " ticks\n";
    std::cout << "Ráfaga Inicial     : " << initial_burst << " ticks\n";
    std::cout << "Ráfaga Restante    : " << remaining_burst << " ticks\n";
    std::cout << "Tiempo de Espera   : " << waiting_time << " ticks\n";
    
    if (state == ProcessState::TERMINATED) {
        std::cout << "Tiempo de Retorno  : " << turnaround_time << " ticks\n";
    } else {
        std::cout << "Tiempo de Retorno  : N/A (En ejecución/pendiente)\n";
    }
    
    if (first_run_time != -1) {
        std::cout << "Tiempo de Respuesta: " << response_time << " ticks\n";
    } else {
        std::cout << "Tiempo de Respuesta: N/A (No iniciado)\n";
    }
    std::cout << "=========================================\n";
}