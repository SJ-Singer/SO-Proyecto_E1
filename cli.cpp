#include "cli.h"
#include "globals.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>

void* shell_loop(void* arg) {
    std::string line;
    
    std::cout << "\n=== Simulador de Kernel - Shell CLI Iniciada ===\n";
    std::cout << "Escriba 'help' para ver la lista de comandos disponibles.\n";

    // Bucle principal del Hilo CLI
    while (true) {
        std::cout << "> ";
        
        // Captura segura: Leemos la línea entera, incluyendo espacios
        if (!std::getline(std::cin, line)) {
            break; // Salida de seguridad si el flujo EOF es forzado
        }

        // Evitar procesar líneas vacías (si el usuario solo presiona Enter)
        if (line.empty()) {
            continue;
        }

        // --- MOTOR DE PARSING ---
        // Utilizamos un stringstream para dividir la línea en palabras (tokens)
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> args;

        while (ss >> token) {
            args.push_back(token);
        }

        if (args.empty()) {
            continue;
        }

        // El primer token siempre es el comando principal
        std::string command = args[0];

// --- VALIDACIÓN Y EJECUCIÓN DE COMANDOS ---
        
        if (command == "help") {
            if (args.size() > 1) {
                std::cout << "[ERROR DE SINTAXIS]: El comando 'help' no recibe parámetros.\n";
                continue;
            }
            std::cout << "\n--- MANUAL DE USUARIO (CLI) ---\n";
            std::cout << "help                             : Muestra este manual.\n";
            std::cout << "exit                             : Libera memoria y cierra el simulador.\n";
            std::cout << "ps                               : Despliega la tabla de procesos.\n";
            std::cout << "pcb <PID>                        : Muestra detalle extendido del PCB.\n";
            std::cout << "create <nombre> <burst> [prio]   : Crea un nuevo proceso.\n";
            std::cout << "start                            : Inicia/reanuda el Motor de CPU.\n";
            std::cout << "pause                            : Pausa el reloj de la CPU.\n";
            std::cout << "set_speed <ms>                   : Ajusta la velocidad del reloj.\n";
            std::cout << "-------------------------------\n";
        } 
        else if (command == "exit") {
            if (args.size() > 1) {
                std::cout << "[ERROR DE SINTAXIS]: El comando 'exit' no recibe parámetros.\n";
                continue;
            }
            std::cout << "Cerrando la simulación y liberando memoria compartida...\n";
            cleanupProcesses(); 
            break; 
        } 
        // --- COMANDOS DE LA FASE 2 ---
        else if (command == "ps") {
            // Validación: ps no debe recibir argumentos adicionales
            if (args.size() > 1) {
                std::cout << "[ERROR DE SINTAXIS]: El comando 'ps' no recibe parámetros.\n";
                continue;
            }

            // SECCIÓN CRÍTICA: Bloqueamos el mutex antes de leer
            pthread_mutex_lock(&system_mutex);

            // Imprimimos el encabezado con el formato exacto requerido
            std::cout << "\n";
            std::cout << std::left << std::setw(15) << "NOMBRE"
                      << std::setw(12) << "ESTADO"
                      << std::setw(8)  << "PID"
                      << std::setw(18) << "BURST RESTANTE"
                      << std::setw(10) << "PRIORIDAD\n";

            for (const PCB* proc : process_table) {
                proc->printSummary();
            }
            std::cout << "\n";

            // FIN SECCIÓN CRÍTICA: Desbloqueamos el mutex
            pthread_mutex_unlock(&system_mutex);
        }
        else if (command == "pcb") {
            // Validación 1: Cantidad de argumentos (comando + PID = 2)
            if (args.size() != 2) {
                std::cout << "[ERROR DE SINTAXIS]: Parámetros insuficientes o excesivos.\n";
                std::cout << "Uso correcto: pcb <PID>\n";
                continue;
            }

            int target_pid = -1;
            
            // Validación 2: Type Safety (Conversión de String a Int segura)
            try {
                target_pid = std::stoi(args[1]);
                if (target_pid <= 0) {
                    throw std::invalid_argument("El PID debe ser positivo"); // Forzamos el catch
                }
            } catch (...) {
                // Atrapa cualquier error si el usuario introduce letras o números inválidos
                std::cout << "[ERROR DE TIPO]: El PID debe ser un número entero positivo.\n";
                continue;
            }

            // SECCIÓN CRÍTICA: Bloqueamos el mutex para buscar e imprimir
            pthread_mutex_lock(&system_mutex);
            
            PCB* proc = findProcessByPID(target_pid);
            if (proc != nullptr) {
                proc->printDetail();
            } else {
                std::cout << "[ERROR]: No se encontró ningún proceso con el PID " << target_pid << " en el sistema.\n";
            }

            // FIN SECCIÓN CRÍTICA: Desbloqueamos el mutex
            pthread_mutex_unlock(&system_mutex);
        }
        // --- COMANDOS DE LA FASE 3 ---
        else if (command == "create") {
            // Validación 1: Cantidad de parámetros
            if (args.size() < 3 || args.size() > 4) {
                std::cout << "[ERROR DE SINTAXIS]: Parámetros insuficientes o excesivos.\n";
                std::cout << "Uso correcto: create <nombre> <burst_time> [prioridad]\n";
                continue;
            }

            std::string name = args[1];
            int burst = 0;
            int priority = 3; // Prioridad por defecto según requerimientos

            // Validación 2: Tipos de datos numéricos y rangos
            try {
                burst = std::stoi(args[2]);
                if (burst <= 0) {
                    throw std::invalid_argument("Burst time debe ser mayor a 0");
                }
                
                if (args.size() == 4) {
                    priority = std::stoi(args[3]);
                    if (priority < 1 || priority > 5) {
                        throw std::invalid_argument("La prioridad debe estar entre 1 y 5");
                    }
                }
            } catch (...) {
                std::cout << "[ERROR DE TIPO]: 'burst' debe ser entero positivo y 'prioridad' un entero entre 1 y 5.\n";
                continue;
            }

            // SECCIÓN CRÍTICA: Escritura en la memoria compartida
            pthread_mutex_lock(&system_mutex);
            
            // Instanciamos el nuevo proceso utilizando el contador global
            PCB* new_process = new PCB(next_pid, name, burst, priority, clock_ticks);
            process_table.push_back(new_process);
            next_pid++;
            
            std::cout << "Proceso '" << name << "' creado con éxito (PID: " << new_process->pid << ").\n";

            pthread_mutex_unlock(&system_mutex);
        }
        else if (command == "start") {
            if (args.size() > 1) {
                std::cout << "[ERROR DE SINTAXIS]: El comando 'start' no recibe parámetros.\n";
                continue;
            }
            pthread_mutex_lock(&system_mutex);
            is_simulation_running = true;
            pthread_mutex_unlock(&system_mutex);
            std::cout << "Motor de Scheduling INICIADO/REANUDADO.\n";
        }
        else if (command == "pause") {
            if (args.size() > 1) {
                std::cout << "[ERROR DE SINTAXIS]: El comando 'pause' no recibe parámetros.\n";
                continue;
            }
            pthread_mutex_lock(&system_mutex);
            is_simulation_running = false;
            pthread_mutex_unlock(&system_mutex);
            std::cout << "Motor de Scheduling PAUSADO.\n";
        }
        else if (command == "set_speed") {
            if (args.size() != 2) {
                std::cout << "[ERROR DE SINTAXIS]: Uso correcto: set_speed <ms_por_tick>\n";
                continue;
            }

            int speed = 0;
            try {
                speed = std::stoi(args[1]);
                if (speed <= 0) {
                    throw std::invalid_argument("La velocidad debe ser positiva");
                }
            } catch (...) {
                std::cout << "[ERROR DE TIPO]: La velocidad debe ser un número entero positivo en milisegundos.\n";
                continue;
            }

            pthread_mutex_lock(&system_mutex);
            simulation_speed_ms = speed;
            pthread_mutex_unlock(&system_mutex);
            
            std::cout << "Velocidad de simulación ajustada a " << speed << " ms por tick.\n";
        }
        else {
            std::cout << "[ERROR]: Comando no reconocido ('" << command << "'). Escriba 'help' para ver el manual.\n";
        }
    }
return nullptr;
}