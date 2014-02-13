#ifndef SRC_FORK_H_
#define SRC_FORK_H_

#include <string>
#include <vector>

namespace runas {

std::vector<char*> StringVectorToCharStarVector(
    const std::string& command,
    const std::vector<std::string>& args);

bool Fork(const std::string& command,
          const std::vector<std::string>& args,
          const std::string& std_input,
          int options,
          int* exit_code);

}  // namespace runas

#endif  // SRC_FORK_H_
