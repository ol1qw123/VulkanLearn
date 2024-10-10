#include <vulkan/vulkan.hpp>
#include <stdexcept>
#include <iostream>
#include <vector>

#include "classheader.hpp"
int main() {
    try {
        vulkan::run();
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
