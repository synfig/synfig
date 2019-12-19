#include <cstdlib>
#include <string>

int main(int argc, char* argv[]) {
  std::string buf(argv[0]);

  std::size_t found = buf.rfind('/'); // looking for a last slash in path
  if (found != std::string::npos) {
    buf = buf.substr(0, found + 1); // get the directory where exe is located
  }

  return system(buf.append("SynfigStudio.sh").c_str());
}
