#include <vector>
#include "sort_performer.hpp"

int main() {
  sort::SortPerformer sort_performer(128 << 20, BUFSIZ);
  sort_performer.Sort("data/input", "data/output");
  return 0;
}
