#include "neon_engine.h"

// Main code
int main(int, char**) {
    NeonEngine* neon_engine = NeonEngine::get_instance();
    neon_engine->run();

    return 0;
}
