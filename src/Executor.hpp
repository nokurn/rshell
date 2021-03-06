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
/// \brief Contains the interface to the \ref rshell::Executor class

#ifndef hpp_rshell_Executor
#define hpp_rshell_Executor

#include "ExecutableCommand.hpp"
#include "ExecutorStreamSet.hpp"
#include "WaitMode.hpp"
#include <memory>

namespace rshell {

// Forward declarations
class ExecutorPipe;
class ExecutorStream;

/// \brief Serves as the abstract base class in the strategy pattern of the
/// execution algorithm
class Executor
{
public:
    /// \brief Destructs the \ref Executor instance
    virtual ~Executor();

    /// \brief Gets a reference to the set of open streams
    /// \return reference to set of open streams
    ExecutorStreamSet& streamSet() noexcept { return _streamSet; }

    /// \brief Gets the stream to replace stdin
    /// \return pointer to input stream
    ExecutorStream* inputStream() const noexcept { return _inputStream; }

    /// \brief Sets the stream to replace stdin
    /// \param inputStream pointer to input stream
    void setInputStream(ExecutorStream* inputStream);

    /// \brief Gets the stream to replace stdout
    /// \return pointer to output stream
    ExecutorStream* outputStream() const noexcept { return _outputStream; }

    /// \brief Sets the stream to replace stdout
    /// \param outputStream pointer to output stream
    void setOutputStream(ExecutorStream* outputStream);

    /// \brief Creates a new pipe on the executor
    /// \return pointer to new pipe
    virtual std::unique_ptr<ExecutorPipe> createPipe() = 0;

    /// \brief Creates a new input file stream on the executor
    /// \param path path to open the stream on
    /// \return pointer to new stream
    virtual std::unique_ptr<ExecutorStream> createInputFileStream(
            const std::string& path) = 0;

    /// \brief Creates a new output file stream on the executor
    /// \param path path to open the stream on
    /// \return pointer to new stream
    virtual std::unique_ptr<ExecutorStream> createOutputFileStream(
            const std::string& path) = 0;

    /// \brief Creates a new append file stream on the executor
    /// \param path path to open the stream on
    /// \return pointer to new stream
    virtual std::unique_ptr<ExecutorStream> createAppendFileStream(
            const std::string& path) = 0;

    /// \brief Executes the abstract command given
    /// \param command command to execute
    /// \param waitMode wait mode to use when executing
    /// \return exit code of the command
    virtual int execute(Command& command, WaitMode waitMode = WaitMode::Wait);

    /// \brief Executes the individual command given
    /// \param command command to execute
    /// \param waitMode wait mode to use when executing
    /// \return exit code of the command
    virtual int execute(ExecutableCommand& command,
            WaitMode waitMode = WaitMode::Wait) = 0;

protected:
    ExecutorStreamSet _streamSet; //!< Set of open streams to close
    ExecutorStream* _inputStream{nullptr}; //!< Stream to replace stdin
    ExecutorStream* _outputStream{nullptr}; //!< Stream to replace stdout
};

} // namespace rshell

#endif // hpp_rshell_Executor
