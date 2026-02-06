#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <limits>
#include <cmath>
#include <fstream>

namespace core {

    int readInt(const std::string& prompt, int minV, int maxV) {
        int v;
        while (true) {
            std::cout << prompt;
            std::cin >> v;

            if (!std::cin.fail() && v >= minV && v <= maxV) {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return v;
            }

            std::cout << "  [Error] Enter an integer from " << minV << " to " << maxV << ".\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    double readDouble(const std::string& prompt, double minV, double maxV) {
        double v;
        while (true) {
            std::cout << prompt;
            std::cin >> v;

            if (!std::cin.fail() && v >= minV && v <= maxV) {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return v;
            }

            std::cout << "  [Error] Enter a number from " << minV << " to " << maxV << ".\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    std::string readLine(const std::string& prompt) {
        std::cout << prompt;
        std::string s;
        std::getline(std::cin, s);
        return s;
    }

    bool yesNo(const std::string& prompt) {
        while (true) {
            std::cout << prompt << " (y/n): ";
            std::string s;
            std::getline(std::cin, s);
            if (s == "y" || s == "Y") return true;
            if (s == "n" || s == "N") return false;
            std::cout << "  [Error] Please type y or n.\n";
        }
    }

    void pause() {
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }

} // namespace core

namespace units {
    constexpr double BTU_PER_HR_PER_KW = 3412; // 1 kW = 3412.142 BTU/hr
    constexpr double BTU_PER_HR_PER_TON = 12000.0; // 1 refrigeration ton = 12,000 BTU/hr

    double btuhr_to_kw(double btuhr) { return btuhr / BTU_PER_HR_PER_KW; }
    double kw_to_btuhr(double kw) { return kw * BTU_PER_HR_PER_KW; }
    double btuhr_to_ton(double btuhr) { return btuhr / BTU_PER_HR_PER_TON; }
    double ton_to_btuhr(double ton) { return ton * BTU_PER_HR_PER_TON; }

} // namespace units

struct LoadItem {
    std::string name;
    std::string method;
    double btu_per_hr = 0.0;
};

namespace calcs {

    // Qs (BTU/hr) = 1.08 * CFM * ΔT(F)
    double air_sensible_btuhr(double cfm, double deltaT_F) {
        return 1.08 * cfm * deltaT_F;
    }

    // Q (BTU/hr) = 500 * GPM * ΔT(F)
    double hydronic_btuhr(double gpm, double deltaT_F) {
        return 500.0 * gpm * deltaT_F;
    }

    // Q (BTU/hr) = U * A * ΔT(F)
    double conduction_btuhr(double U, double area_ft2, double deltaT_F) {
        return U * area_ft2 * deltaT_F;
    }

    // CFM = ACH * Volume(ft³) / 60
    double cfm_from_ach(double ach, double volume_ft3) {
        return (ach * volume_ft3) / 60.0;
    }

} // namespace calcs

namespace ui {

    void printHeader() {
        std::cout << "=============================================\n";
        std::cout << " HEAT LOAD CALCULATOR (Console) - Imperial\n";
        std::cout << " Methods: Air Sensible | Hydronic | Conduction | ACH\n";
        std::cout << "---------------------------------------------\n";
        std::cout << " Notes:\n";
        std::cout << "  - Quick-calcs intended for preliminary sizing.\n";
        std::cout << "  - Verify assumptions, code requirements, and design standards.\n";
        std::cout << "=============================================\n\n";
    }

    void printItemTable(const std::vector<LoadItem>& items) {
        std::cout << "\n------------------ PROJECT LOAD SUMMARY ------------------\n";
        std::cout << std::left
            << std::setw(4) << "#"
            << std::setw(28) << "Name"
            << std::setw(14) << "Method"
            << std::right
            << std::setw(14) << "BTU/hr"
            << std::setw(12) << "kW"
            << std::setw(10) << "Tons"
            << "\n";

        std::cout << std::string(82, '-') << "\n";

        double total = 0.0;
        for (size_t i = 0; i < items.size(); ++i) {
            total += items[i].btu_per_hr;
            std::cout << std::left
                << std::setw(4) << (std::to_string(i + 1) + ")")
                << std::setw(28) << items[i].name.substr(0, 27)
                << std::setw(14) << items[i].method.substr(0, 13)
                << std::right
                << std::setw(14) << std::fixed << std::setprecision(1) << items[i].btu_per_hr
                << std::setw(12) << std::fixed << std::setprecision(3) << units::btuhr_to_kw(items[i].btu_per_hr)
                << std::setw(10) << std::fixed << std::setprecision(3) << units::btuhr_to_ton(items[i].btu_per_hr)
                << "\n";
        }

        std::cout << std::string(82, '-') << "\n";
        std::cout << std::right
            << std::setw(46) << "TOTAL:"
            << std::setw(14) << std::fixed << std::setprecision(1) << total
            << std::setw(12) << std::fixed << std::setprecision(3) << units::btuhr_to_kw(total)
            << std::setw(10) << std::fixed << std::setprecision(3) << units::btuhr_to_ton(total)
            << "\n";
        std::cout << "----------------------------------------------------------\n\n";
    }

    void exportCSV(const std::vector<LoadItem>& items, const std::string& path) {
        std::ofstream out(path);
        if (!out) {
            std::cout << "  ***Error*** Could not write file: " << path << "\n";
            return;
        }

        out << "Index,Name,Method,BTU_per_hr,kW,Tons\n";
        double total = 0.0;

        for (size_t i = 0; i < items.size(); ++i) {
            total += items[i].btu_per_hr;
            out << (i + 1) << ","
                << "\"" << items[i].name << "\","
                << "\"" << items[i].method << "\","
                << std::fixed << std::setprecision(1) << items[i].btu_per_hr << ","
                << std::fixed << std::setprecision(3) << units::btuhr_to_kw(items[i].btu_per_hr) << ","
                << std::fixed << std::setprecision(3) << units::btuhr_to_ton(items[i].btu_per_hr)
                << "\n";
        }

        out << ",\"TOTAL\",\"\","
            << std::fixed << std::setprecision(1) << total << ","
            << std::fixed << std::setprecision(3) << units::btuhr_to_kw(total) << ","
            << std::fixed << std::setprecision(3) << units::btuhr_to_ton(total) << "\n";

        std::cout << "  Saved: " << path << "\n";
    }

} // namespace ui

// ------------------------ ITEM BUILDERS ------------------------

LoadItem buildAirSensibleItem() {
    LoadItem item;
    item.method = "AirSens";

    item.name = core::readLine("Name (e.g., Supply air, Zone vent): ");
    if (item.name.empty()) item.name = "Air Sensible Load";

    double cfm = core::readDouble("CFM: ", 0.0, 1e9);
    double dT = core::readDouble("Delta-T (F): ", -200.0, 200.0);

    item.btu_per_hr = calcs::air_sensible_btuhr(cfm, dT);

    std::cout << "Result: Qs = 1.08 * " << cfm << " * " << dT
        << " = " << std::fixed << std::setprecision(1) << item.btu_per_hr << " BTU/hr\n";
    return item;
}

LoadItem buildHydronicItem() {
    LoadItem item;
    item.method = "Hydronic";

    item.name = core::readLine("Name (e.g., HW coil, baseboard loop): ");
    if (item.name.empty()) item.name = "Hydronic Load";

    double gpm = core::readDouble("GPM: ", 0.0, 1e9);
    double dT = core::readDouble("Delta-T (F): ", -200.0, 200.0);

    item.btu_per_hr = calcs::hydronic_btuhr(gpm, dT);

    std::cout << "Result: Q = 500 * " << gpm << " * " << dT
        << " = " << std::fixed << std::setprecision(1) << item.btu_per_hr << " BTU/hr\n";
    return item;
}

LoadItem buildConductionItem() {
    LoadItem item;
    item.method = "Cond(UA)";

    item.name = core::readLine("Name (e.g., Exterior wall, Roof, Glass): ");
    if (item.name.empty()) item.name = "Conduction Load";

    std::cout << "\nChoose input form:\n";
    std::cout << "  1) U-value directly (BTU/hr·ft^2·F)\n";
    std::cout << "  2) R-value (hr·ft^2·F/BTU)  -> U = 1/R\n";
    int mode = core::readInt("Select: ", 1, 2);

    double area = core::readDouble("Area (ft^2): ", 0.0, 1e12);
    double dT = core::readDouble("Delta-T (F): ", -200.0, 200.0);

    double U = 0.0;
    if (mode == 1) {
        U = core::readDouble("U-value: ", 0.0, 1e6);
    }
    else {
        double R = core::readDouble("R-value: ", 0.000001, 1e12);
        U = 1.0 / R;
        std::cout << "Computed U = 1/R = " << std::fixed << std::setprecision(6) << U << "\n";
    }

    item.btu_per_hr = calcs::conduction_btuhr(U, area, dT);

    std::cout << "Result: Q = U * A * dT = " << std::fixed << std::setprecision(6) << U
        << " * " << std::setprecision(1) << area << " * " << dT
        << " = " << std::setprecision(1) << item.btu_per_hr << " BTU/hr\n";
    return item;
}

LoadItem buildACHItem() {
    LoadItem item;
    item.method = "ACH->Air";

    item.name = core::readLine("Name (e.g., Infiltration, Ventilation): ");
    if (item.name.empty()) item.name = "ACH Air Load";

    double volume = core::readDouble("Zone volume (ft^3): ", 0.0, 1e18);
    double ach = core::readDouble("ACH (air changes per hour): ", 0.0, 1e6);
    double dT = core::readDouble("Delta-T (F): ", -200.0, 200.0);

    double cfm = calcs::cfm_from_ach(ach, volume);
    item.btu_per_hr = calcs::air_sensible_btuhr(cfm, dT);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "CFM = ACH * Volume / 60 = " << ach << " * " << volume << " / 60 = " << cfm << "\n";
    std::cout << "Qs  = 1.08 * CFM * dT   = 1.08 * " << cfm << " * " << dT
        << " = " << std::setprecision(1) << item.btu_per_hr << " BTU/hr\n";
    return item;
}

// ------------------------ MENUS ------------------------

void conversionsMenu() {
    while (true) {
        std::cout << "\n=============================\n";
        std::cout << " CONVERSIONS\n";
        std::cout << "=============================\n";
        std::cout << "1) BTU/hr -> kW & Tons\n";
        std::cout << "2) kW -> BTU/hr\n";
        std::cout << "3) Tons -> BTU/hr\n";
        std::cout << "0) Back\n";

        int c = core::readInt("Select: ", 0, 3);
        if (c == 0) return;

        if (c == 1) {
            double btu = core::readDouble("BTU/hr: ", -1e18, 1e18);
            std::cout << std::fixed << std::setprecision(3)
                << "kW   = " << units::btuhr_to_kw(btu) << "\n"
                << "Tons = " << units::btuhr_to_ton(btu) << "\n";
            core::pause();
        }
        else if (c == 2) {
            double kw = core::readDouble("kW: ", -1e18, 1e18);
            std::cout << std::fixed << std::setprecision(1)
                << "BTU/hr = " << units::kw_to_btuhr(kw) << "\n";
            core::pause();
        }
        else if (c == 3) {
            double ton = core::readDouble("Tons: ", -1e18, 1e18);
            std::cout << std::fixed << std::setprecision(1)
                << "BTU/hr = " << units::ton_to_btuhr(ton) << "\n";
            core::pause();
        }
    }
}

void projectMenu(std::vector<LoadItem>& items) {
    while (true) {
        std::cout << "\n=============================\n";
        std::cout << " PROJECT MODE (Build & Sum)\n";
        std::cout << "=============================\n";
        std::cout << "1) Add Air Sensible (CFM, dT)\n";
        std::cout << "2) Add Hydronic (GPM, dT)\n";
        std::cout << "3) Add Conduction (U/R, A, dT)\n";
        std::cout << "4) Add ACH Air Load (Vol, ACH, dT)\n";
        std::cout << "5) View Summary\n";
        std::cout << "6) Remove Item\n";
        std::cout << "7) Export CSV\n";
        std::cout << "8) Clear Project\n";
        std::cout << "0) Back\n";

        int c = core::readInt("Select: ", 0, 8);
        if (c == 0) return;

        try {
            if (c == 1) items.push_back(buildAirSensibleItem());
            else if (c == 2) items.push_back(buildHydronicItem());
            else if (c == 3) items.push_back(buildConductionItem());
            else if (c == 4) items.push_back(buildACHItem());
            else if (c == 5) {
                if (items.empty()) std::cout << "\n(No items yet.)\n";
                else ui::printItemTable(items);
                core::pause();
            }
            else if (c == 6) {
                if (items.empty()) {
                    std::cout << "\n(No items to remove.)\n";
                    core::pause();
                    continue;
                }
                ui::printItemTable(items);
                int idx = core::readInt("Remove which item #? ", 1, static_cast<int>(items.size()));
                items.erase(items.begin() + (idx - 1));
                std::cout << "Removed.\n";
                core::pause();
            }
            else if (c == 7) {
                if (items.empty()) {
                    std::cout << "\n(No items to export.)\n";
                    core::pause();
                    continue;
                }
                std::string path = core::readLine("CSV file path (e.g., heat_load.csv): ");
                if (path.empty()) path = "heat_load.csv";
                ui::exportCSV(items, path);
                core::pause();
            }
            else if (c == 8) {
                if (core::yesNo("Clear all items?")) {
                    items.clear();
                    std::cout << "Cleared.\n";
                }
                core::pause();
            }
        }
        catch (...) {
            std::cout << "  [Error] Unexpected issue. Inputs were not applied.\n";
            core::pause();
        }
    }
}

void quickCalcMenu() {
    while (true) {
        std::cout << "\n=============================\n";
        std::cout << " QUICK CALCS\n";
        std::cout << "=============================\n";
        std::cout << "1) Air Sensible (CFM, dT)\n";
        std::cout << "2) Hydronic (GPM, dT)\n";
        std::cout << "3) Conduction (U/R, A, dT)\n";
        std::cout << "4) ACH Air Load (Vol, ACH, dT)\n";
        std::cout << "0) Back\n";

        int c = core::readInt("Select: ", 0, 4);
        if (c == 0) return;

        LoadItem item;
        if (c == 1) item = buildAirSensibleItem();
        else if (c == 2) item = buildHydronicItem();
        else if (c == 3) item = buildConductionItem();
        else if (c == 4) item = buildACHItem();

        std::cout << "\n--- Output (Quick) ---\n";
        std::cout << std::fixed << std::setprecision(1)
            << "BTU/hr: " << item.btu_per_hr << "\n";
        std::cout << std::fixed << std::setprecision(3)
            << "kW:     " << units::btuhr_to_kw(item.btu_per_hr) << "\n"
            << "Tons:   " << units::btuhr_to_ton(item.btu_per_hr) << "\n";
        core::pause();
    }
}

int main() {
    ui::printHeader();
    std::vector<LoadItem> projectItems;

    while (true) {
        std::cout << "\n=============================\n";
        std::cout << " MAIN MENU\n";
        std::cout << "=============================\n";
        std::cout << "1) Quick Calcs\n";
        std::cout << "2) Project Mode (Add + Sum)\n";
        std::cout << "3) Conversions\n";
        std::cout << "0) Exit\n";

        int choice = core::readInt("Select: ", 0, 3);
        if (choice == 0) {
            std::cout << "\nGoodbye.\n";
            return 0;
        }
        else if (choice == 1) {
            quickCalcMenu();
        }
        else if (choice == 2) {
            projectMenu(projectItems);
        }
        else if (choice == 3) {
            conversionsMenu();
        }
    }
}
