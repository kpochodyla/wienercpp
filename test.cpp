#include <fstream>
#include <vector>
#include <sstream>
#include <iostream>

void split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}


int main(int argc, char *argv[])
{
    std::ifstream test_values ("test_values.txt");
    std::string line;

    while (std::getline(test_values, line))
    {
        std::vector<std::string> line_values;
        std::string e;
        std::string d;
        std::string N;
        split(line, '\t', line_values);
        if (line_values[0] == "bin_size") // pomiń nagłówek pliku test_values.txt
        {
            continue;
        }
        
        e = line_values[5];
        N = line_values[3];

        // for (auto v: line_values)
        // {
        //     std::cout << v << " ";
        // }
        // std::cout << std::endl; 

        std::stringstream stream;
        std::cout << "bin_len = " << line_values[0] << "[b]" << std::endl;
        stream << "~/Documents/Programowanie/inzynierka/wiener/wiener" << " " << e << " " << N;

        system(stream.str().c_str());

    }
    return 0;
}