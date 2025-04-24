#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class Interpreter {
   public:
    enum { TapeSize = 30000 };

    Interpreter(std::vector<unsigned char> prog, std::istream& in, std::ostream& out):
        program(prog), input(in), output(out), cells(TapeSize, 0), prog_ptr(0), data_ptr(0) {
    }

    bool run() {
        while (prog_ptr < program.size()) {
            switch (program[prog_ptr]) {
                case '>':
                    ++data_ptr;
                    break;

                case '<':
                    if (data_ptr == 0) {
                        runtime_error("data pointer moved left out of bounds");
                        return false;
                    }
                    --data_ptr;
                    break;

                case '+':
                    ++cells[data_ptr];
                    break;

                case '-':
                    --cells[data_ptr];
                    break;

                case '.':
                    output.put(static_cast<char>(cells[data_ptr]));
                    output.flush();
                    break;

                case ',': {
                    char c;

                    if (input.get(c)) {
                        cells[data_ptr] = static_cast<unsigned char>(c);
                    }

                    break;
                }

                case '[':
                    if (cells[data_ptr] == 0) {
                        if (!skip_loop()) return false;
                    } else {
                        loop_stack.push_back(prog_ptr);
                    }
                    break;

                case ']':
                    if (cells[data_ptr] != 0) {
                        if (loop_stack.empty()) {
                            runtime_error("unmatched ']'");
                            return false;
                        }
                        prog_ptr = loop_stack.back();
                    } else if (!loop_stack.empty()) {
                        loop_stack.pop_back();
                    }
                    break;

                default:
                    break;
            }

            ++prog_ptr;
        }

        return true;
    }

   private:
    bool skip_loop() {
        std::size_t balance = 1;

        while (balance > 0) {
            ++prog_ptr;

            if (prog_ptr >= program.size()) {
                runtime_error("Unmatched '['");
                return false;
            }

            if (program[prog_ptr] == '[')
                ++balance;
            else if (program[prog_ptr] == ']')
                --balance;
        }
        return true;
    }

    void runtime_error(const std::string& msg) {
        std::cerr << "runtime error: " << msg << '\n';
    }

    std::vector<unsigned char> program;
    std::istream& input;
    std::ostream& output;

    std::vector<unsigned char> cells;
    std::vector<std::size_t> loop_stack;

    std::size_t prog_ptr;
    std::size_t data_ptr;
};

static bool read_program(const std::string& path, std::vector<unsigned char>& out) {
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file) return false;

    char c;
    while (file.get(c)) {
        switch (c) {
            case '>':
            case '<':
            case '+':
            case '-':
            case '.':
            case ',':
            case '[':
            case ']':
                out.push_back(static_cast<unsigned char>(c));
                break;
            default:
                break;
        }
    }
    return true;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: brainfck <file>\n";
        return 1;
    }

    std::string filename(argv[1]);

    std::vector<unsigned char> program;
    if (!read_program(filename, program)) {
        std::cerr << "error reading '" << filename << "'\n";
        return 1;
    }

    Interpreter interpreter(program, std::cin, std::cout);
    if (!interpreter.run()) {
        return 1;
    }

    return 0;
}
