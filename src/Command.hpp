// rshell
// Copyright (c) Jeremiah Griffin <jgrif007@ucr.edu>
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE NEGLIGENCE OR OTHER TORTIOUS ACTION,
// ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
// SOFTWARE.

/// \file
/// \brief Contains the interface to the \ref rshell::Command class

#ifndef hpp_rshell_Command
#define hpp_rshell_Command

#include "WaitMode.hpp"

namespace rshell {

// Forward declarations
class Executor;

/// \brief Serves as the abstract base class in the composite pattern of the
/// command structure
class Command
{
public:
    /// \brief Destructs the \ref Command instance
    virtual ~Command();

    /// \brief Executes the command using the given executor
    /// \param executor executor to use for execution
    /// \param waitMode wait mode to use when executing
    /// \return exit code of the command
    virtual int execute(Executor& executor, WaitMode waitMode) = 0;
};

} // namespace rshell

#endif // hpp_rshell_Command
