#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <csignal>
#include <algorithm>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

struct Inventory {
    unsigned long long carbon;
    unsigned long long oxygen;
    unsigned long long hydrogen;
    unsigned long long water;
    unsigned long long carbon_dioxide;
    unsigned long long alcohol;
    unsigned long long glucose;
};

const unsigned long long MAX_ATOMS = 1000000000000000000ULL;

// גלובליים
Inventory *inventory = nullptr; // מיפוי זיכרון למלאי
int inventory_fd = -1;          // תיאור קובץ המלאי
std::string save_file_path;

int tcp_port = -1, udp_port = -1, timeout_sec = 0;
std::string uds_stream_path, uds_dgram_path;

// פונקציות נעילה וקבלת מפת זיכרון
bool lock_inventory_file() {
    if (inventory_fd == -1) return false;
    while (flock(inventory_fd, LOCK_EX) == -1) {
        if (errno != EINTR) return false;
    }
    return true;
}

void unlock_inventory_file() {
    if (inventory_fd != -1) flock(inventory_fd, LOCK_UN);
}

bool load_inventory_from_file() {
    if (inventory_fd == -1) return false;
    if (lseek(inventory_fd, 0, SEEK_SET) == -1) return false;

    ssize_t r = read(inventory_fd, inventory, sizeof(Inventory));
    return r == sizeof(Inventory);
}

bool save_inventory_to_file() {
    if (inventory_fd == -1) return false;
    if (lseek(inventory_fd, 0, SEEK_SET) == -1) return false;

    ssize_t w = write(inventory_fd, inventory, sizeof(Inventory));
    fsync(inventory_fd);
    return w == sizeof(Inventory);
}

void print_inventory() {
    std::cout << "Inventory:" << std::endl;
    std::cout << "CARBON: " << inventory->carbon << std::endl;
    std::cout << "OXYGEN: " << inventory->oxygen << std::endl;
    std::cout << "HYDROGEN: " << inventory->hydrogen << std::endl;
    std::cout << "WATER: " << inventory->water << std::endl;
    std::cout << "CARBON DIOXIDE: " << inventory->carbon_dioxide << std::endl;
    std::cout << "ALCOHOL: " << inventory->alcohol << std::endl;
    std::cout << "GLUCOSE: " << inventory->glucose << std::endl;
    std::cout << std::endl;
}

void handle_alarm(int sig) {
    std::cout << "No activity for " << timeout_sec << " seconds. Shutting down server." << std::endl;
    exit(0);
}

void update_inventory(const std::string &atom, unsigned long long number) {
    if (!lock_inventory_file()) {
        std::cerr << "Failed to lock inventory file!" << std::endl;
        return;
    }

    if (atom == "CARBON") {
        inventory->carbon = std::min(MAX_ATOMS, inventory->carbon + number);
    } else if (atom == "OXYGEN") {
        inventory->oxygen = std::min(MAX_ATOMS, inventory->oxygen + number);
    } else if (atom == "HYDROGEN") {
        inventory->hydrogen = std::min(MAX_ATOMS, inventory->hydrogen + number);
    }

    save_inventory_to_file();
    unlock_inventory_file();
}

void process_tcp_command(const std::string &cmd) {
    std::cout << "Received TCP command: " << cmd << std::endl;
    std::istringstream iss(cmd);
    std::string action, atom;
    unsigned long long number;
    if (iss >> action >> atom >> number && action == "ADD") {
        if (atom != "CARBON" && atom != "OXYGEN" && atom != "HYDROGEN") {
            std::cerr << "Invalid atom type!" << std::endl;
            return;
        }
        update_inventory(atom, number);
        print_inventory();
    } else {
        std::cerr << "Invalid TCP command!" << std::endl;
    }
}

bool process_udp_command(const std::string &cmd) {
    std::cout << "Received UDP command: " << cmd << std::endl;
    std::istringstream iss(cmd);
    std::string action, molecule;
    unsigned long long number;

    if (!(iss >> action >> molecule) || action != "DELIVER") {
        std::cerr << "Invalid UDP command!" << std::endl;
        return false;
    }

    unsigned long long needed_C=0, needed_O=0, needed_H=0;
    std::string molecule_name = molecule;

    if (molecule == "CARBON") {
        std::string second_word;
        if (!(iss >> second_word) || second_word != "DIOXIDE") {
            std::cerr << "Invalid molecule! Expected 'CARBON DIOXIDE'" << std::endl;
            return false;
        }
        molecule_name = "CARBON DIOXIDE";
    }

    if (!(iss >> number)) {
        std::cerr << "Missing number!" << std::endl;
        return false;
    }

    if (molecule_name == "WATER") {
        needed_H=2*number; needed_O=1*number;
    } else if (molecule_name == "ALCOHOL") {
        needed_C=2*number; needed_H=6*number; needed_O=1*number;
    } else if (molecule_name == "GLUCOSE") {
        needed_C=6*number; needed_H=12*number; needed_O=6*number;
    } else if (molecule_name == "CARBON DIOXIDE") {
        needed_C=1*number; needed_O=2*number;
    } else {
        std::cerr << "Invalid molecule!" << std::endl;
        return false;
    }

    if (!lock_inventory_file()) {
        std::cerr << "Failed to lock inventory file!" << std::endl;
        return false;
    }

    if (inventory->carbon >= needed_C && inventory->oxygen >= needed_O && inventory->hydrogen >= needed_H) {
        inventory->carbon -= needed_C;
        inventory->oxygen -= needed_O;
        inventory->hydrogen -= needed_H;

        if (molecule_name == "WATER") inventory->water += number;
        else if (molecule_name == "ALCOHOL") inventory->alcohol += number;
        else if (molecule_name == "GLUCOSE") inventory->glucose += number;
        else if (molecule_name == "CARBON DIOXIDE") inventory->carbon_dioxide += number;

        save_inventory_to_file();
        unlock_inventory_file();

        print_inventory();
        return true;
    }

    unlock_inventory_file();
    std::cerr << "Not enough atoms" << std::endl;
    return false;
}

void process_console_command(const std::string &cmd) {
    unsigned long long w=inventory->water, c=inventory->carbon_dioxide,
            a=inventory->alcohol, g=inventory->glucose;
    if (cmd == "GEN SOFT DRINK")
        std::cout << "SOFT DRINKs: " << std::min({w, c, g}) << std::endl;
    else if (cmd == "GEN VODKA")
        std::cout << "VODKA: " << std::min({w, a, g}) << std::endl;
    else if (cmd == "GEN CHAMPAGNE")
        std::cout << "CHAMPAGNE: " << std::min({w, c, a}) << std::endl;
    else
        std::cerr << "Invalid console command!" << std::endl;
}

int main(int argc, char *argv[]) {
    int opt;
    struct option long_opts[] = {
            {"tcp-port", required_argument, nullptr, 'T'},
            {"udp-port", required_argument, nullptr, 'U'},
            {"stream-path", required_argument, nullptr, 's'},
            {"datagram-path", required_argument, nullptr, 'd'},
            {"carbon", required_argument, nullptr, 'c'},
            {"oxygen", required_argument, nullptr, 'o'},
            {"hydrogen", required_argument, nullptr, 'h'},
            {"timeout", required_argument, nullptr, 't'},
            {"save-file", required_argument, nullptr, 'f'},
            {nullptr, 0, nullptr, 0}
    };

    tcp_port = -1;
    udp_port = -1;
    timeout_sec = 0;
    uds_stream_path.clear();
    uds_dgram_path.clear();
    save_file_path.clear();

    unsigned long long initial_carbon = 0, initial_oxygen = 0, initial_hydrogen = 0;

    while ((opt = getopt_long(argc, argv, "T:U:s:d:c:o:h:t:f:", long_opts, nullptr)) != -1) {
        switch (opt) {
            case 'T': tcp_port = std::stoi(optarg); break;
            case 'U': udp_port = std::stoi(optarg); break;
            case 's': uds_stream_path = optarg; break;
            case 'd': uds_dgram_path = optarg; break;
            case 'c': initial_carbon = std::stoull(optarg); break;
            case 'o': initial_oxygen = std::stoull(optarg); break;
            case 'h': initial_hydrogen = std::stoull(optarg); break;
            case 't': timeout_sec = std::stoi(optarg); break;
            case 'f': save_file_path = optarg; break;
            default:
                std::cerr << "Usage error: " << argv[0]
                          << " [-T tcp_port] [-U udp_port] [-s uds_stream_path] [-d uds_dgram_path]"
                          << " [-c carbon] [-o oxygen] [-h hydrogen] [-t timeout] [-f save_file]" << std::endl;
                return 1;
        }
    }

    // בדיקות התנגשויות בין TCP ו-UDS stream
    if (tcp_port != -1 && !uds_stream_path.empty()) {
        std::cerr << "Error: Cannot specify both TCP port (-T) and UNIX stream socket (-s)." << std::endl;
        return 1;
    }
    // בדיקות התנגשויות בין UDP ו-UDS datagram
    if (udp_port != -1 && !uds_dgram_path.empty()) {
        std::cerr << "Error: Cannot specify both UDP port (-U) and UNIX datagram socket (-d)." << std::endl;
        return 1;
    }
    // חייב להיות לפחות TCP או uds stream
    if (tcp_port == -1 && uds_stream_path.empty()) {
        std::cerr << "Error: Must specify TCP port (-T) or UNIX stream socket (-s)." << std::endl;
        return 1;
    }
    // חייב להיות לפחות UDP או uds datagram
    if (udp_port == -1 && uds_dgram_path.empty()) {
        std::cerr << "Error: Must specify UDP port (-U) or UNIX datagram socket (-d)." << std::endl;
        return 1;
    }

    if (!save_file_path.empty()) {
        // פותח/יוצר את קובץ המלאי
        inventory_fd = open(save_file_path.c_str(), O_RDWR | O_CREAT, 0666);
        if (inventory_fd == -1) {
            perror("open inventory file");
            return 1;
        }

        // נעילה כדי למנוע גישה בו זמנית בעייתית
        if (!lock_inventory_file()) {
            std::cerr << "Failed to lock inventory file for initialization" << std::endl;
            close(inventory_fd);
            return 1;
        }

        // מיפוי הזיכרון של הקובץ (או יצירת מלאי חדש)
        off_t size = lseek(inventory_fd, 0, SEEK_END);
        if (size == sizeof(Inventory)) {
            // קובץ מלאי קיים - טוען את המלאי
            inventory = (Inventory*)mmap(nullptr, sizeof(Inventory), PROT_READ | PROT_WRITE, MAP_SHARED, inventory_fd, 0);
            if (inventory == MAP_FAILED) {
                perror("mmap");
                unlock_inventory_file();
                close(inventory_fd);
                return 1;
            }

            if (!load_inventory_from_file()) {
                std::cerr << "Failed to load inventory from file!" << std::endl;
                unlock_inventory_file();
                munmap(inventory, sizeof(Inventory));
                close(inventory_fd);
                return 1;
            }
        } else {
            // מאתחל מלאי חדש עם ערכים ראשוניים
            if (ftruncate(inventory_fd, sizeof(Inventory)) == -1) {
                perror("ftruncate");
                unlock_inventory_file();
                close(inventory_fd);
                return 1;
            }
            inventory = (Inventory*)mmap(nullptr, sizeof(Inventory), PROT_READ | PROT_WRITE, MAP_SHARED, inventory_fd, 0);
            if (inventory == MAP_FAILED) {
                perror("mmap");
                unlock_inventory_file();
                close(inventory_fd);
                return 1;
            }
            inventory->carbon = initial_carbon;
            inventory->oxygen = initial_oxygen;
            inventory->hydrogen = initial_hydrogen;
            inventory->water = 0;
            inventory->carbon_dioxide = 0;
            inventory->alcohol = 0;
            inventory->glucose = 0;

            if (!save_inventory_to_file()) {
                std::cerr << "Failed to initialize inventory file!" << std::endl;
                unlock_inventory_file();
                munmap(inventory, sizeof(Inventory));
                close(inventory_fd);
                return 1;
            }
        }

        unlock_inventory_file();
    } else {
        // אם לא שומרים לקובץ - משתמשים במלאי בזיכרון רגיל
        static Inventory local_inventory;
        inventory = &local_inventory;
        inventory->carbon = initial_carbon;
        inventory->oxygen = initial_oxygen;
        inventory->hydrogen = initial_hydrogen;
        inventory->water = 0;
        inventory->carbon_dioxide = 0;
        inventory->alcohol = 0;
        inventory->glucose = 0;
    }

    // יצירת sockets לפי ההגדרות
    int tcp_sock = -1;
    if (tcp_port != -1) {
        tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
        int optval = 1;
        setsockopt(tcp_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(tcp_port);
        if (bind(tcp_sock, (sockaddr*)&addr, sizeof(addr)) == -1) {
            perror("bind tcp");
            exit(1);
        }
        if (listen(tcp_sock, MAX_CLIENTS) == -1) {
            perror("listen tcp");
            exit(1);
        }
        std::cout << "TCP socket listening on port " << tcp_port << std::endl;
    }

    int udp_sock = -1;
    if (udp_port != -1) {
        udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(udp_port);
        if (bind(udp_sock, (sockaddr*)&addr, sizeof(addr)) == -1) {
            perror("bind udp");
            exit(1);
        }
        std::cout << "UDP socket listening on port " << udp_port << std::endl;
    }

    int uds_stream_sock = -1;
    if (!uds_stream_path.empty()) {
        unlink(uds_stream_path.c_str());
        uds_stream_sock = socket(AF_UNIX, SOCK_STREAM, 0);
        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, uds_stream_path.c_str(), sizeof(addr.sun_path) - 1);
        addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';
        if (bind(uds_stream_sock, (sockaddr*)&addr, sizeof(addr)) == -1) {
            perror("bind uds_stream");
            exit(1);
        }
        if (listen(uds_stream_sock, MAX_CLIENTS) == -1) {
            perror("listen uds_stream");
            exit(1);
        }
        std::cout << "UNIX stream socket created at: " << uds_stream_path << std::endl;
    }

    int uds_dgram_sock = -1;
    if (!uds_dgram_path.empty()) {
        unlink(uds_dgram_path.c_str());
        uds_dgram_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, uds_dgram_path.c_str(), sizeof(addr.sun_path) - 1);
        addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';
        if (bind(uds_dgram_sock, (sockaddr*)&addr, sizeof(addr)) == -1) {
            perror("bind uds_dgram");
            exit(1);
        }
        std::cout << "UNIX datagram socket created at: " << uds_dgram_path << std::endl;
    }

    fd_set master, read_fds;
    FD_ZERO(&master);
    if (tcp_sock != -1) FD_SET(tcp_sock, &master);
    if (udp_sock != -1) FD_SET(udp_sock, &master);
    if (uds_stream_sock != -1) FD_SET(uds_stream_sock, &master);
    if (uds_dgram_sock != -1) FD_SET(uds_dgram_sock, &master);
    FD_SET(STDIN_FILENO, &master);
    int fdmax = std::max({tcp_sock, udp_sock, uds_stream_sock, uds_dgram_sock, STDIN_FILENO});

    std::cout << "Server started!" << std::endl;
    print_inventory();

    while (true) {
        read_fds = master;
        if (select(fdmax + 1, &read_fds, nullptr, nullptr, nullptr) == -1) {
            perror("select");
            exit(1);
        }
        if (timeout_sec > 0) alarm(timeout_sec);

        for (int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) {
                // UDS Datagram
                if (i == uds_dgram_sock) {
                    char buf[BUFFER_SIZE];
                    sockaddr_un cli_addr{};
                    socklen_t len = sizeof(cli_addr);
                    int n = recvfrom(i, buf, sizeof(buf) - 1, 0, (sockaddr*)&cli_addr, &len);
                    buf[n] = '\0';
                    bool success = process_udp_command(buf);
                    std::string r = success ? "DELIVERED" : "FAILED";
                    sendto(i, r.c_str(), r.size(), 0, (sockaddr*)&cli_addr, len);
                }
                    // UDP
                else if (i == udp_sock) {
                    char buf[BUFFER_SIZE];
                    sockaddr_in cli_addr{};
                    socklen_t len = sizeof(cli_addr);
                    int n = recvfrom(i, buf, sizeof(buf) - 1, 0, (sockaddr*)&cli_addr, &len);
                    buf[n] = '\0';
                    bool success = process_udp_command(buf);
                    std::string r = success ? "DELIVERED" : "FAILED";
                    sendto(i, r.c_str(), r.size(), 0, (sockaddr*)&cli_addr, len);
                }
                    // UDS Stream & TCP (accept)
                else if (i == uds_stream_sock || i == tcp_sock) {
                    sockaddr_storage cli_addr{};
                    socklen_t len = sizeof(cli_addr);
                    int new_fd = accept(i, (sockaddr*)&cli_addr, &len);
                    if (new_fd != -1) {
                        FD_SET(new_fd, &master);
                        if (new_fd > fdmax) fdmax = new_fd;
                        std::cout << "New client connected (fd: " << new_fd << ")" << std::endl;
                    }
                }
                    // STDIN
                else if (i == STDIN_FILENO) {
                    std::string line;
                    std::getline(std::cin, line);
                    if(line =="EXIT"|| line == "exit") {
                        std::cout << "Exit command received. Shutting down server." << std::endl;

                        if (inventory != nullptr) save_inventory_to_file();
                        exit(0);
                    }
                    else if (line == "PRINT") {
                        print_inventory();
                    }
                    else
                    process_console_command(line);
                }
                    // TCP/UDS Stream data
                else {
                    char buf[BUFFER_SIZE];
                    int n = recv(i, buf, sizeof(buf) - 1, 0);
                    if (n <= 0) {
                        close(i);
                        FD_CLR(i, &master);
                    } else {
                        buf[n] = '\0';
                        process_tcp_command(buf);
                    }
                }
            }
        }

    }

    if (inventory != nullptr) {
        munmap(inventory, sizeof(Inventory));
        inventory = nullptr;
    }
    if (inventory_fd != -1) {
        close(inventory_fd);
        inventory_fd = -1;
    }

    return 0;
}
