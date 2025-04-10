#include <algorithm>
#include <string>

namespace rcon_server {

class Utils final {
public:
    static inline std::string fixPath(std::string path) {
        std::transform(path.begin(), path.end(), path.begin(), [](unsigned char c) -> int {
            return c == '\\' ? '/' : c;
        });
        return path;
    }
};

} // namespace rcon_server