#pragma once

#include "stringlist.h"
#include <path.h>

#ifdef WIN32
#else
#include <unistd.h>
#endif

namespace os_support {
struct Process
{
    #ifdef WIN32
    void*   handle;
    void*   stderr;
    #else
    pid_t   handle;
     int    stderr;
    #endif

    static Process spawn(
        const char* exe_name,
        const StringList& args,
        const StringListKV& env,
        const ::helpers::path& logfile,
        const ::helpers::path& working_directory={},
        bool print_command=true,
        bool capture_stderr=false
        );
    bool wait();

    static bool handle_status(int status);
};

enum class TerminalColour {
    Default,
    Red,    // ANSI 1
    Green  // ANSI 2
};
void set_console_colour(std::ostream& os, TerminalColour colour);

void mkdir(const helpers::path& path);

const helpers::path& get_mrustc_path();

}