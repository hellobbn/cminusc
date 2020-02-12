// hpp debug and helper functions

#ifndef CPP_HELPER_HPP
#define CPP_HELPER_HPP

#define DEBUG 3

#ifdef DEBUG_PRINT
#undef DEBUG_PRINT
#endif

#ifdef DEBUG
#define DEBUG_PRINT(x)                                                         \
    std::cout << "\033[33mDEBUG: \033[0m" << __FILE__ << ":" << __LINE__       \
              << ":" << __func__ << "(): " << x
#else
#define Debug(x)
#endif

#if DEBUG >= 1
#define DEBUG_PRINT_1(x)                                                       \
    std::cout << "\033[33mDEBUG: \033[0m" << __FILE__ << ":" << __LINE__       \
              << ":" << __func__ << "(): " << x
#else
#define DEBUG_PRINT_1(x)
#endif

#if DEBUG >= 2
#define DEBUG_PRINT_2(x)                                                       \
    std::cout << "\033[33mDEBUG: \033[0m" << __FILE__ << ":" << __LINE__       \
              << ":" << __func__ << "(): " << x
#else
#define DEBUG_PRINT_2(x)
#endif

#if DEBUG >= 3
#define DEBUG_PRINT_3(x)                                                       \
    std::cout << "\033[33mDEBUG: \033[0m" << __FILE__ << ":" << __LINE__       \
              << ":" << __func__ << "(): " << x
#else
#define DEBUG_PRINT_3(x)
#endif

#endif