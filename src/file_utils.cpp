#include "file_utils.hpp"

namespace file_utils {

bool HasEofbitSet(const std::istream& is) {
  return is.rdstate() & std::ios_base::eofbit;
}

bool HasFailbitSet(const std::istream& is) {
  return is.rdstate() & std::ios_base::failbit;
}

} // namespace file_utils