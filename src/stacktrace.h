#ifndef STACKTRACE_H
#define STACKTRACE_H

void stacktrace_abort();
bool stacktrace_install();

/**
 * @brief stacktrace_demangle idemangle symbols
 * @param symbol_name the symbol name to demangle
 * @return string must be freed using free()
 */
char *stacktrace_demangle(const char *symbol_name);

#endif // STACKTRACE_H
