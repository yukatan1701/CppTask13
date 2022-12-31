#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <functional>
#include <vector>

struct Args {
    enum Mode {
        Unknown,
        Compress,
        Decompress
    };
    Mode Mode;
    std::string Input;
    std::string Output;
    Args() : Mode(Unknown) {}
};

class Neighbour : public std::pair<uint32_t, uint8_t> {
public:
    Neighbour(uint32_t N, uint8_t W) {
        this->first = N;
        this->second = W;
    }
    
    bool operator<(const Neighbour &N) const {
        return this->first < N.first;
    }

    friend std::ostream& operator<<(std::ostream &OS, const Neighbour &N);
};

std::ostream &operator<<(std::ostream &OS, const Neighbour &N)
{
    OS << "<" << N.first << ", " << int(N.second) << ">";
    return OS;
}

bool cmpNeighbours(Neighbour &N1, Neighbour &N2) {
    if (N1.first == N2.first)
        return N1.second < N2.second;
    return N1.first < N2.first;
}

typedef std::set<Neighbour> NeighbourSet;

Args parseArguments(int argc, char **argv) {
    Args Args;
    for (int I = 1; I < argc; I++) {
        std::string Key(argv[I]);
        if (Key == "-s") {
            Args.Mode = Args::Mode::Compress;
        } else if (Key == "-d") {
            Args.Mode = Args::Mode::Decompress;
        } else if (Key == "-i") {
            if (I == argc - 1) {
                std::cerr << "Filename expected for -i." << std::endl;
                exit(1);
            }
            Args.Input = std::string(argv[I+1]);
            I++;
        } else if (Key == "-o") {
            if (I == argc - 1) {
                std::cerr << "Filename expected for -o." << std::endl;
                exit(1);
            }
            Args.Output = std::string(argv[I+1]);
            I++;
        } else {
            std::cerr << "Invalid key: " << Key << std::endl;
            exit(1);
        }
    }
    if (Args.Mode == Args::Mode::Unknown) {
        std::cerr << "Unknown mode." << std::endl;
        exit(1);
    }
    if (Args.Input.empty()) {
        std::cerr << "Unknown input filename." << std::endl;
        exit(1);
    }
    if (Args.Output.empty()) {
        std::cerr << "Unknown output filename." << std::endl;
        exit(1);
    }
    return Args;
}

typedef std::map<uint32_t, NeighbourSet> GraphMap;

#ifdef DEBUG
void dump(const GraphMap &GM) {
    for (auto Itr = GM.begin(); Itr != GM.end(); Itr++) {
        std::cerr << "[" << Itr->first << "] ";
        std::cerr << "{ ";
        for (auto &Neigh : Itr->second) {
            std::cerr << Neigh << " ";
        }
        std::cerr << "}" << std::endl;
    }
}
#endif

void compress(const Args &Args) {
    GraphMap GM;
    uint32_t N1, N2;
    uint8_t W;
    std::ifstream Input(Args.Input);
    std::string Line;
    while (std::getline(Input, Line)) {
        std::sscanf(Line.c_str(), "%d\t%d\t%hhd", &N1, &N2, &W);
        #ifdef DEBUG
        std::cerr << "N1: " << N1 << ", N2: " << N2 << ", W: " << int(W) << std::endl;
        #endif
        auto Min = std::min(N1, N2);
        auto Max = std::max(N1, N2);
        NeighbourSet NS;
        auto ElemItr = GM.insert(std::make_pair(Min, NS));
        ElemItr.first->second.insert(Neighbour(Max, W));
    }
    Input.close();
    #ifdef DEBUG
    dump(GM);
    #endif
    std::ofstream Out(Args.Output, std::ios::binary);
    uint32_t TotalN = GM.size();
    Out.write(reinterpret_cast<const char *>(&TotalN), sizeof(TotalN));
    for (auto Itr = GM.begin(); Itr != GM.end(); Itr++) {
        uint32_t Key = Itr->first;
        uint32_t N = Itr->second.size();
        Out.write(reinterpret_cast<const char *>(&Key), sizeof(Key));
        Out.write(reinterpret_cast<const char *>(&N), sizeof(N));
        for (auto NItr = Itr->second.begin(); NItr != Itr->second.end(); NItr++) {
            uint32_t V = NItr->first;
            uint8_t W = NItr->second;
            Out.write(reinterpret_cast<const char *>(&V), sizeof(V));
            Out.write(reinterpret_cast<const char *>(&W), sizeof(W));
        }
    }
    Out.close();
}

void decompress(const Args &Args) {
    std::ifstream In(Args.Input, std::ios::binary);
    std::ofstream Out(Args.Output);
    uint32_t TotalN;
    In.read(reinterpret_cast<char*>(&TotalN), sizeof(TotalN));
    for (int I = 0; I < TotalN; I++) {
        uint32_t Key, N;
        In.read(reinterpret_cast<char*>(&Key), sizeof(Key));
        In.read(reinterpret_cast<char*>(&N), sizeof(N));
        uint32_t V;
        uint8_t W;
        for (int J = 0; J < N; J++) {
            In.read(reinterpret_cast<char*>(&V), sizeof(V));
            In.read(reinterpret_cast<char*>(&W), sizeof(W));
            Out << Key << "\t" << V << "\t" << int(W) << std::endl;
        }
    }
    Out.close();
    In.close();
}

int main(int argc, char **argv) {
    auto Args = parseArguments(argc, argv);
    if (Args.Mode == Args::Mode::Compress)
        compress(Args);
    else if (Args.Mode == Args::Mode::Decompress)
        decompress(Args);
    else
        __builtin_unreachable();
    return 0;
}