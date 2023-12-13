#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <fstream>

using namespace std;

class Lokasi {
public:
    virtual int jagaLokasi() = 0;
    virtual string jenisLokasi() const = 0;
};

class Jalan : public Lokasi {
public:
    int mulai;
    int akhir;

    Jalan(int m, int a) : mulai(m), akhir(a) {}

    int jagaLokasi() override {
        cout << "Staff menjaga bagian jalan dari titik " << mulai << " ke titik " << akhir << endl;
        return mulai;
    }

    string jenisLokasi() const override {
        return "jalan";
    }
};

class Staf : public Lokasi {
public:
    int titik;
    int kemampuan;

    Staf(int t, int k) : titik(t), kemampuan(k) {}

    int jagaLokasi() override {
        cout << "Staff menjaga lokasi pada titik " << titik << endl;
        return titik;
    }

    string jenisLokasi() const override {
        return "staf";
    }
};

bool bandingkanStaf(const Staf &a, const Staf &b) {
    return a.kemampuan > b.kemampuan;
}

class SistemKeamanan {
private:
    int totalLokasi;
    vector<Lokasi*> lokasi;
    unordered_map<int, vector<string>> penjaga;
    vector<Jalan> jalanan;
    const vector<Staf>& staf;

public:
    SistemKeamanan(int N, const vector<Staf>& staf) : totalLokasi(N), staf(staf) {
        for (int i = 0; i <= N; ++i) {
            lokasi.push_back(nullptr);
        }
    }

    ~SistemKeamanan() {
        for (Lokasi* loc : lokasi) {
            delete loc;
        }
    }

    void setJalanan(const vector<Jalan>& jalanan) {
        this->jalanan = jalanan;
    }

    void tambahLokasi(Lokasi* loc) {
        int indeksLokasi = loc->jagaLokasi();
        lokasi[indeksLokasi] = loc;

        if (loc->jenisLokasi() == "staf") {
            penjaga[indeksLokasi].push_back("Staff dengan kemampuan " + to_string(static_cast<Staf*>(loc)->kemampuan));
        }
    }

    bool sudahOptimal() {
        for (int i = 1; i <= totalLokasi; ++i) {
            if (lokasi[i] == nullptr) {
                return false;
            }
        }
        return true;
    }

    void jalankanPemeriksaanKeamanan() {
        for (Lokasi* loc : lokasi) {
            if (loc != nullptr) {
                loc->jagaLokasi();
            }
        }
    }

    void infoPenjaga() const {
        for (const auto& entry : penjaga) {
            cout << "Titik " << entry.first << ":";
            for (const string& guard : entry.second) {
                cout << " " << guard << ",";
            }
            cout << endl;
        }

        generateGraph("security_system_graph.dot");
    }

    void generateGraph(const string& filename) const {
        ofstream dotFile(filename);
        if (!dotFile.is_open()) {
            cerr << "Error: Unable to open DOT file for writing." << endl;
            return;
        }

        dotFile << "graph SecuritySystem {" << endl;

        // Tambahkan titik dan staff
        for (int i = 1; i <= totalLokasi; ++i) {
            if (lokasi[i] != nullptr) {
                // Menentukan warna untuk staff atau titik
                string color;
                if (lokasi[i]->jenisLokasi() == "staf") {
                    color = "orange"; // Color for staff nodes
                    dotFile << "  " << i << " [label=\"" << i << "\", style=filled, fillcolor=\"" << color << "\"];" << endl;
                }
            }
        }

        // Tambahkan jalur penghubung (edges) untuk jalanan
        for (const Jalan& jalan : jalanan) {
            dotFile << "  " << jalan.mulai << " -- " << jalan.akhir << " [color=\"black\"];" << endl;
        }

        // Tambahkan jalur yang dianggap terjaga oleh staff dengan kemampuan 1
        for (const Staf& staffMember : staf) {
            if (staffMember.kemampuan == 1) {
                int staffLocation = staffMember.titik;
                for (const Jalan& jalan : jalanan) {
                    if (jalan.mulai == staffLocation || jalan.akhir == staffLocation) {
                        dotFile << "  " << staffLocation << " -- " << (jalan.mulai == staffLocation ? jalan.akhir : jalan.mulai) << " [color=\"blue\", dir=\"forward\"];" << endl;
                    }
                }
            }
        }

        // Tambahkan edges untuk lokasi yang dijaga oleh staff dengan kemampuan 0
        for (const auto& entry : penjaga) {
            for (const string& guard : entry.second) {
                size_t pos = guard.find("Titik ");
                if (pos != string::npos) {
                    int guardedLocation = stoi(guard.substr(pos + 6));
                    // Determine the color based on the guarding capability
                    string color;
                    if (guard.find("Staff dengan kemampuan") != string::npos) {
                        color = "blue"; // Color for staff-guarded edges
                    } else {
                        color = "black"; // Color for other guarded edges
                    }
                    // Add an arrow from staff to the guarded location
                    dotFile << "  " << entry.first << " -- " << guardedLocation << " [color=\"" << color << "\", dir=\"forward\"];" << endl;
                }
            }
        }

        dotFile << "}" << endl;
        dotFile.close();

        cout << "Graph visualization saved to " << filename << endl;

        // Generate the graphical output using the dot command
        string command = "neato -Tpng " + filename + " -o " + filename + ".png";
        system(command.c_str());
        cout << "Graphical output saved to " << filename << ".png" << endl;
    }
};

int main() {
    int N, R, M;
    cin >> N >> R >> M;

    vector<Jalan> jalanan;
    vector<Staf> staf;

    for (int i = 0; i < R; ++i) {
        int mulai, akhir;
        cin >> mulai >> akhir;
        Jalan* jalan = new Jalan(mulai, akhir);
        jalanan.push_back(*jalan);
    }

    for (int i = 0; i < M; ++i) {
        int titik, kemampuan;
        cin >> titik >> kemampuan;
        Staf* stafMember = new Staf(titik, kemampuan);
        staf.push_back(*stafMember);
    }

    SistemKeamanan sistemKeamanan(N, staf);

    sistemKeamanan.setJalanan(jalanan);

    sort(staf.begin(), staf.end(), bandingkanStaf);

    for (Staf& staffMember : staf) {
        sistemKeamanan.tambahLokasi(&staffMember);
    }

    sistemKeamanan.jalankanPemeriksaanKeamanan();

    if (sistemKeamanan.sudahOptimal()) {
        cout << "Semua lokasi telah terjaga dengan baik." << endl;
    } else {
        cout << "Beberapa lokasi masih belum terjaga." << endl;
    }

    cout << "Informasi Penjaga:" << endl;
    sistemKeamanan.infoPenjaga();

    return 0;
}
