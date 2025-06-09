
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unordered_map>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <algorithm>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

std::unordered_map<std::string, unsigned long long> atom_inventory;
const unsigned long long MAX_ATOMS = 1000000000000000000ULL;

void print_inventory()
{
    std::cout << "Inventory:" << std::endl;
    for (const auto &pair : atom_inventory)
        std::cout << pair.first << ": " << pair.second << std::endl;

    std::cout << std::endl;
}

void process_tcp_command(const std::string &cmd)
{
    std::istringstream iss(cmd);
    std::string action, item;
    unsigned long long number;

    if (iss >> action >> item >> number && action == "ADD")
    {
        if (item != "CARBON" && item != "OXYGEN" && item != "HYDROGEN")
        {
            std::cerr << "Invalid atom !" << std::endl;
            return;
        }
        // אפשר להוסיף אטומים וגם מולקולות ישירות
        if (atom_inventory[item] + number <= MAX_ATOMS)
        {
            atom_inventory[item] += number;
        }
        else
        {
            atom_inventory[item] = MAX_ATOMS;
        }
        print_inventory();
    }
    else
    {
        std::cerr << "Invalid TCP command!" << std::endl;
    }
}

/**
 * Processes UDP commands to deliver molecules.
 * Format: DELIVER <MOLECULE> <NUMBER>
 * Returns true if the delivery was successful.
 */
bool process_udp_command(const std::string &cmd)
{
    std::istringstream iss(cmd);
    std::string action, molecule;
    unsigned long long number;

    if (!(iss >> action))
    {
        std::cerr << "Invalid UDP command!" << std::endl;
        return false;
    }

    if (action != "DELIVER")
    {
        std::cerr << "Invalid action! need to start with DELIVER" << std::endl;
        return false;
    }

    // קרא את המולקולה (או מילה ראשונה של המולקולה)
    if (!(iss >> molecule))
    {
        std::cerr << "Invalid UDP command!" << std::endl;
        return false;
    }

    unsigned long long needed_C = 0, needed_O = 0, needed_H = 0;
    std::string molecule_name = molecule;

    // בדיקה האם זו מולקולה עם שתי מילים (CARBON DIOXIDE)
    if (molecule == "CARBON")
    {
        std::string second_word;
        if (!(iss >> second_word))
        {
            std::cerr << "Incomplete CARBON DIOXIDE command!" << std::endl;
            return false;
        }
        if (second_word != "DIOXIDE")
        {
            std::cerr << "Invalid molecule!" << std::endl;
            return false;
        }
        molecule_name = "CARBON DIOXIDE";
        if (!(iss >> number))
        {
            std::cerr << "Missing number!" << std::endl;
            return false;
        }
        needed_C = 1 * number;
        needed_O = 2 * number;
    }
    else if (molecule == "WATER" || molecule == "ALCOHOL" || molecule == "GLUCOSE")
    {
        if (!(iss >> number))
        {
            std::cerr << "Missing number!" << std::endl;
            return false;
        }
        if (molecule == "WATER")
        {
            needed_H = 2 * number;
            needed_O = 1 * number;
        }
        else if (molecule == "ALCOHOL")
        {
            needed_C = 2 * number;
            needed_H = 6 * number;
            needed_O = 1 * number;
        }
        else if (molecule == "GLUCOSE")
        {
            needed_C = 6 * number;
            needed_H = 12 * number;
            needed_O = 6 * number;
        }
    }
    else
    {
        std::cerr << "Invalid molecule!" << std::endl;
        return false;
    }

    // Check if there are enough atoms
    if (atom_inventory["CARBON"] >= needed_C &&
        atom_inventory["OXYGEN"] >= needed_O &&
        atom_inventory["HYDROGEN"] >= needed_H)
    {
        // הורד את האטומים מהמלאי
        atom_inventory["CARBON"] -= needed_C;
        atom_inventory["OXYGEN"] -= needed_O;
        atom_inventory["HYDROGEN"] -= needed_H;

        // הוסף את המולקולה למלאי!
        atom_inventory[molecule_name] += number;

        print_inventory();
        return true;
    }
    else
    {
        std::cerr << "Not enough atoms" << std::endl;
        return false;
    }
}

void process_console_command(const std::string &cmd)
{
    unsigned long long water = atom_inventory["WATER"];
    unsigned long long carbon_dioxide = atom_inventory["CARBON DIOXIDE"];
    unsigned long long alcohol = atom_inventory["ALCOHOL"];
    unsigned long long glucose = atom_inventory["GLUCOSE"];

    if (cmd == "GEN SOFT DRINK")
    {
        unsigned long long count = std::min({water, carbon_dioxide, glucose});
        std::cout << "SOFT DRINKs available: " << count << std::endl;
    }
    else if (cmd == "GEN VODKA")
    {
        unsigned long long count = std::min({water, alcohol, glucose});
        std::cout << "VODKA drinks available: " << count << std::endl;
    }
    else if (cmd == "GEN CHAMPAGNE")
    {
        unsigned long long count = std::min({water, carbon_dioxide, alcohol});
        std::cout << "CHAMPAGNE drinks available: " << count << std::endl;
    }
    else
    {
        std::cerr << "Invalid console command!" << std::endl;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <tcp_port> <udp_port>" << std::endl;
        return 1;
    }
    int tcp_port = std::stoi(argv[1]);
    int udp_port = std::stoi(argv[2]);

    // TCP socket setup
    int tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(tcp_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in tcp_addr{};
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_addr.s_addr = INADDR_ANY;
    tcp_addr.sin_port = htons(tcp_port);
    bind(tcp_sock, (sockaddr *)&tcp_addr, sizeof(tcp_addr));
    listen(tcp_sock, MAX_CLIENTS);

    // UDP socket setup
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in udp_addr{};
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(udp_port);
    bind(udp_sock, (sockaddr *)&udp_addr, sizeof(udp_addr));

    fd_set master, read_fds;
    FD_ZERO(&master);
    FD_SET(tcp_sock, &master);
    FD_SET(udp_sock, &master);
    FD_SET(STDIN_FILENO, &master);
    int fdmax = std::max({tcp_sock, udp_sock, STDIN_FILENO});

    std::cout << "Drinks Bar Server started on TCP port " << tcp_port
              << " and UDP port " << udp_port << std::endl;

    while (true)
    {
        read_fds = master;
        if (select(fdmax + 1, &read_fds, nullptr, nullptr, nullptr) == -1)
        {
            perror("select");
            exit(1);
        }

        for (int i = 0; i <= fdmax; i++)
        {
            if (FD_ISSET(i, &read_fds))
            {
                if (i == tcp_sock)
                {
                    sockaddr_in cli_addr{};
                    socklen_t cli_len = sizeof(cli_addr);
                    int new_fd = accept(tcp_sock, (sockaddr *)&cli_addr, &cli_len);
                    if (new_fd != -1)
                    {
                        FD_SET(new_fd, &master);
                        if (new_fd > fdmax)
                            fdmax = new_fd;
                        std::cout << "New TCP client connected." << std::endl;
                    }
                }
                else if (i == udp_sock)
                {
                    char buf[BUFFER_SIZE];
                    sockaddr_in client_addr{};
                    socklen_t len = sizeof(client_addr);
                    int n = recvfrom(udp_sock, buf, sizeof(buf) - 1, 0, (sockaddr *)&client_addr, &len);
                    buf[n] = '\0';
                    std::string response = process_udp_command(buf) ? "DELIVERED" : "FAILED";
                    sendto(udp_sock, response.c_str(), response.size(), 0, (sockaddr *)&client_addr, len);
                }
                else if (i == STDIN_FILENO)
                {
                    std::string line;
                    std::getline(std::cin, line);
                    if (line == "exit" || line == "EXIT")
                    {
                        std::cout << "Shutting down server..." << std::endl;
                        close(tcp_sock);
                        close(udp_sock);
                        return 0;
                    }
                    process_console_command(line);
                }
                else
                {
                    char buf[BUFFER_SIZE];
                    int nbytes = recv(i, buf, sizeof(buf) - 1, 0);
                    if (nbytes <= 0)
                    {
                        std::cout << "TCP client disconnected." << std::endl;
                        close(i);
                        FD_CLR(i, &master);
                    }
                    else
                    {
                        buf[nbytes] = '\0';
                        process_tcp_command(buf);
                    }
                }
            }
        }
    }
    return 0;
}
=======
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unordered_map>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <algorithm>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

std::unordered_map<std::string, unsigned long long> atom_inventory;
const unsigned long long MAX_ATOMS = 1000000000000000000ULL;

void print_inventory()
{
    std::cout << "Inventory:" << std::endl;
    for (const auto &pair : atom_inventory)
        std::cout << pair.first << ": " << pair.second << std::endl;

    std::cout << std::endl;
}

void process_tcp_command(const std::string &cmd)
{
    std::istringstream iss(cmd);
    std::string action, item;
    unsigned long long number;

    if (iss >> action >> item >> number && action == "ADD")
    {
        if (item != "CARBON" && item != "OXYGEN" && item != "HYDROGEN")
        {
            std::cerr << "Invalid atom !" << std::endl;
            return;
        }
        // אפשר להוסיף אטומים וגם מולקולות ישירות
        if (atom_inventory[item] + number <= MAX_ATOMS)
        {
            atom_inventory[item] += number;
        }
        else
        {
            atom_inventory[item] = MAX_ATOMS;
        }
        print_inventory();
    }
    else
    {
        std::cerr << "Invalid TCP command!" << std::endl;
    }
}

/**
 * Processes UDP commands to deliver molecules.
 * Format: DELIVER <MOLECULE> <NUMBER>
 * Returns true if the delivery was successful.
 */
bool process_udp_command(const std::string &cmd)
{
    std::istringstream iss(cmd);
    std::string action, molecule;
    unsigned long long number;

    if (!(iss >> action))
    {
        std::cerr << "Invalid UDP command!" << std::endl;
        return false;
    }

    if (action != "DELIVER")
    {
        std::cerr << "Invalid action! need to start with DELIVER" << std::endl;
        return false;
    }

    // קרא את המולקולה (או מילה ראשונה של המולקולה)
    if (!(iss >> molecule))
    {
        std::cerr << "Invalid UDP command!" << std::endl;
        return false;
    }

    unsigned long long needed_C = 0, needed_O = 0, needed_H = 0;
    std::string molecule_name = molecule;

    // בדיקה האם זו מולקולה עם שתי מילים (CARBON DIOXIDE)
    if (molecule == "CARBON")
    {
        std::string second_word;
        if (!(iss >> second_word))
        {
            std::cerr << "Incomplete CARBON DIOXIDE command!" << std::endl;
            return false;
        }
        if (second_word != "DIOXIDE")
        {
            std::cerr << "Invalid molecule!" << std::endl;
            return false;
        }
        molecule_name = "CARBON DIOXIDE";
        if (!(iss >> number))
        {
            std::cerr << "Missing number!" << std::endl;
            return false;
        }
        needed_C = 1 * number;
        needed_O = 2 * number;
    }
    else if (molecule == "WATER" || molecule == "ALCOHOL" || molecule == "GLUCOSE")
    {
        if (!(iss >> number))
        {
            std::cerr << "Missing number!" << std::endl;
            return false;
        }
        if (molecule == "WATER")
        {
            needed_H = 2 * number;
            needed_O = 1 * number;
        }
        else if (molecule == "ALCOHOL")
        {
            needed_C = 2 * number;
            needed_H = 6 * number;
            needed_O = 1 * number;
        }
        else if (molecule == "GLUCOSE")
        {
            needed_C = 6 * number;
            needed_H = 12 * number;
            needed_O = 6 * number;
        }
    }
    else
    {
        std::cerr << "Invalid molecule!" << std::endl;
        return false;
    }

    // Check if there are enough atoms
    if (atom_inventory["CARBON"] >= needed_C &&
        atom_inventory["OXYGEN"] >= needed_O &&
        atom_inventory["HYDROGEN"] >= needed_H)
    {
        // הורד את האטומים מהמלאי
        atom_inventory["CARBON"] -= needed_C;
        atom_inventory["OXYGEN"] -= needed_O;
        atom_inventory["HYDROGEN"] -= needed_H;

        // הוסף את המולקולה למלאי!
        atom_inventory[molecule_name] += number;

        print_inventory();
        return true;
    }
    else
    {
        std::cerr << "Not enough atoms" << std::endl;
        return false;
    }
}

void process_console_command(const std::string &cmd)
{
    unsigned long long water = atom_inventory["WATER"];
    unsigned long long carbon_dioxide = atom_inventory["CARBON DIOXIDE"];
    unsigned long long alcohol = atom_inventory["ALCOHOL"];
    unsigned long long glucose = atom_inventory["GLUCOSE"];

    if (cmd == "GEN SOFT DRINK")
    {
        unsigned long long count = std::min({water, carbon_dioxide, glucose});
        std::cout << "SOFT DRINKs available: " << count << std::endl;
    }
    else if (cmd == "GEN VODKA")
    {
        unsigned long long count = std::min({water, alcohol, glucose});
        std::cout << "VODKA drinks available: " << count << std::endl;
    }
    else if (cmd == "GEN CHAMPAGNE")
    {
        unsigned long long count = std::min({water, carbon_dioxide, alcohol});
        std::cout << "CHAMPAGNE drinks available: " << count << std::endl;
    }
    else
    {
        std::cerr << "Invalid console command!" << std::endl;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <tcp_port> <udp_port>" << std::endl;
        return 1;
    }
    int tcp_port = std::stoi(argv[1]);
    int udp_port = std::stoi(argv[2]);

    // TCP socket setup
    int tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(tcp_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in tcp_addr{};
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_addr.s_addr = INADDR_ANY;
    tcp_addr.sin_port = htons(tcp_port);
    bind(tcp_sock, (sockaddr *)&tcp_addr, sizeof(tcp_addr));
    listen(tcp_sock, MAX_CLIENTS);

    // UDP socket setup
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in udp_addr{};
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(udp_port);
    bind(udp_sock, (sockaddr *)&udp_addr, sizeof(udp_addr));

    fd_set master, read_fds;
    FD_ZERO(&master);
    FD_SET(tcp_sock, &master);
    FD_SET(udp_sock, &master);
    FD_SET(STDIN_FILENO, &master);
    int fdmax = std::max({tcp_sock, udp_sock, STDIN_FILENO});

    std::cout << "Drinks Bar Server started on TCP port " << tcp_port
              << " and UDP port " << udp_port << std::endl;

    while (true)
    {
        read_fds = master;
        if (select(fdmax + 1, &read_fds, nullptr, nullptr, nullptr) == -1)
        {
            perror("select");
            exit(1);
        }

        for (int i = 0; i <= fdmax; i++)
        {
            if (FD_ISSET(i, &read_fds))
            {
                if (i == tcp_sock)
                {
                    sockaddr_in cli_addr{};
                    socklen_t cli_len = sizeof(cli_addr);
                    int new_fd = accept(tcp_sock, (sockaddr *)&cli_addr, &cli_len);
                    if (new_fd != -1)
                    {
                        FD_SET(new_fd, &master);
                        if (new_fd > fdmax)
                            fdmax = new_fd;
                        std::cout << "New TCP client connected." << std::endl;
                    }
                }
                else if (i == udp_sock)
                {
                    char buf[BUFFER_SIZE];
                    sockaddr_in client_addr{};
                    socklen_t len = sizeof(client_addr);
                    int n = recvfrom(udp_sock, buf, sizeof(buf) - 1, 0, (sockaddr *)&client_addr, &len);
                    buf[n] = '\0';
                    std::string response = process_udp_command(buf) ? "DELIVERED" : "FAILED";
                    sendto(udp_sock, response.c_str(), response.size(), 0, (sockaddr *)&client_addr, len);
                }
                else if (i == STDIN_FILENO)
                {
                    std::string line;
                    std::getline(std::cin, line);
                    if (line == "exit" || line == "EXIT")
                    {
                        std::cout << "Shutting down server..." << std::endl;
                        close(tcp_sock);
                        close(udp_sock);
                        return 0;
                    }
                    process_console_command(line);
                }
                else
                {
                    char buf[BUFFER_SIZE];
                    int nbytes = recv(i, buf, sizeof(buf) - 1, 0);
                    if (nbytes <= 0)
                    {
                        std::cout << "TCP client disconnected." << std::endl;
                        close(i);
                        FD_CLR(i, &master);
                    }
                    else
                    {
                        buf[nbytes] = '\0';
                        process_tcp_command(buf);
                    }
                }
            }
        }
    }
    return 0;
}
