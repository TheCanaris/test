#include <iostream>
#include <fstream>
#include <regex>
#include <map>
#include <vector>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

struct Pose {
    double x = 0, y = 0, z = 0;
    double a = 0, b = 0, c = 0;
    int status = 0;
    int turn = 0;
    bool hasData = false;
};

int main() {
    std::string inputDir = "input_folder";
    std::ofstream out("output.dat");

    if (!out.is_open()) {
        std::cerr << "Cannot open output file\n";
        return 1;
    }

    std::string num = R"([-+]?\d*\.?\d+)";

    std::regex posRegex("^\\s*(pos\\w+)\\.frame\\.pos\\.(x|y|z)\\s*:=\\s*(" + num + ")");
    std::regex oriRegex("^\\s*(pos\\w+)\\.frame\\.ori\\.(a|b|c)\\s*:=\\s*(" + num + ")");
    std::regex statusRegex("^\\s*(pos\\w+)\\.Status\\s*:=\\s*(\\d+)");
    std::regex turnRegex("^\\s*(pos\\w+)\\.Turn\\s*:=\\s*(\\d+)");

    // =========================
    // WRITE HEADER
    // =========================
    out << "&ACCESS RVO1\n";
    out << "&REL 244\n";
    out << "&PARAM DISKPATH = KRC:\\R1\\Program\\Bauteilprogramme\n";
    out << "DEFDAT  SP15_005\n\n";

    out << std::fixed << std::setprecision(3);

    // =========================
    // LOOP FILES
    // =========================
    for (const auto& entry : fs::directory_iterator(inputDir)) {

        if (!entry.is_regular_file()) continue;

        std::ifstream in(entry.path());
        if (!in.is_open()) continue;

        std::map<std::string, Pose> poses;
        std::vector<std::string> order;
        std::string line;

        // 👉 COMMENT WHICH FILE IS BEING PROCESSED
        out << "\n; =========================================\n";
        out << "; FILE: " << entry.path().filename().string() << "\n";
        out << "; =========================================\n";

        while (std::getline(in, line)) {
            std::smatch m;

            if (std::regex_search(line, m, posRegex)) {
                std::string name = m[1];

                if (!poses.count(name)) order.push_back(name);

                auto &p = poses[name];
                p.hasData = true;

                double v = std::stod(m[3]);

                if (m[2] == "x") p.x = v;
                if (m[2] == "y") p.y = v;
                if (m[2] == "z") p.z = v;
            }
            else if (std::regex_search(line, m, oriRegex)) {
                std::string name = m[1];

                if (!poses.count(name)) order.push_back(name);

                auto &p = poses[name];
                p.hasData = true;

                double v = std::stod(m[3]);

                if (m[2] == "a") p.a = v;
                if (m[2] == "b") p.b = v;
                if (m[2] == "c") p.c = v;
            }
            else if (std::regex_search(line, m, statusRegex)) {
                std::string name = m[1];

                if (!poses.count(name)) order.push_back(name);

                auto &p = poses[name];
                p.status = std::stoi(m[2]);
                p.hasData = true;
            }
            else if (std::regex_search(line, m, turnRegex)) {
                std::string name = m[1];

                if (!poses.count(name)) order.push_back(name);

                auto &p = poses[name];
                p.turn = std::stoi(m[2]);
                p.hasData = true;
            }
        }

        // =========================
        // WRITE POSITIONS PER FILE
        // =========================
        for (const auto& name : order) {
            const auto& p = poses[name];

            if (!p.hasData) continue;

            if (p.x == 0 && p.y == 0 && p.z == 0 &&
                p.a == 0 && p.b == 0 && p.c == 0) {
                continue;
            }

            out << "DECL E6POS " << name << "={";
            out << "X " << p.x << ",";
            out << "Y " << p.y << ",";
            out << "Z " << p.z << ",";
            out << "A " << p.a << ",";
            out << "B " << p.b << ",";
            out << "C " << p.c << ",";
            out << "S " << p.status << ",";
            out << "T " << p.turn << ",";
            out << "E1 0.0,E2 0.0,E3 0.0,E4 0.0,E5 0.0,E6 0.0";
            out << "}\n";
        }
    }

    // =========================
    // FOOTER
    // =========================
    out << "\nENDDAT\n";

    std::cout << "Done. Directory processed.\n";
    return 0;
}