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
/// \brief Contains the interface to the \ref rshell::PosixExecutorPipe class

#ifndef hpp_rshell_PosixExecutorPipe
#define hpp_rshell_PosixExecutorPipe

#include "ExecutorPipe.hpp"
#include "PosixExecutorPipeStream.hpp"

namespace rshell {

/// \brief Serves as the abstract base class in the strategy pattern of the
/// piped execution algorithm
class PosixExecutorPipe : public ExecutorPipe
{
public:
    /// \brief Constructs a new instance of the \ref PosixExecutorPipe class
    explicit PosixExecutorPipe();

    /// \brief Destructs the \ref PosixExecutorPipe instance
    virtual ~PosixExecutorPipe();

    /// \brief Gets a reference to the read end of the stream
    /// \return reference to input stream
    virtual ExecutorStream& inputStream() { return _inputStream; }

    /// \brief Gets a reference to the write end of the stream
    /// \return reference to output stream
    virtual ExecutorStream& outputStream() { return _outputStream; }

    /// \brief Closes the pipe
    void close();

protected:
    int _files[2]; //!< File descriptors for the pipe streams
    PosixExecutorPipeStream _inputStream; //!< Stream for read end of pipe
    PosixExecutorPipeStream _outputStream; //!< Stream for write end of pipe
};

} // namespace rshell

#endif // hpp_rshell_PosixExecutorPipe
