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

#include "Parser.hpp"
#include "AppendRedirectionCommand.hpp"
#include "ConjunctiveCommand.hpp"
#include "DisjunctiveCommand.hpp"
#include "ExecutableCommand.hpp"
#include "ExitBuiltinCommand.hpp"
#include "InputRedirectionCommand.hpp"
#include "OutputRedirectionCommand.hpp"
#include "PipeCommand.hpp"
#include "SequentialCommand.hpp"
#include "TestBuiltinCommand.hpp"
#include "utility/make_unique.hpp"
#include <cassert>
#include <stdexcept>

using utility::make_unique;

namespace rshell {

Parser::Parser(const std::vector<Token>& tokens)
    : _tokens(tokens)
{
}

std::unique_ptr<Command> Parser::apply()
{
    // Reset the state of the parser to a known, empty state
    _root.release();
    _current = &_root;
    _scopes = {};

    // Dispatch each token to its appropriate parsing method
    for (auto&& token : _tokens) {
        switch (token.type) {
            case Token::Type::Word: parseWord(token); break;
            case Token::Type::Sequence: parseSequence(token); break;
            case Token::Type::Conjunction: parseConjunction(token); break;
            case Token::Type::Disjunction: parseDisjunction(token); break;
            case Token::Type::Pipe: parsePipe(token); break;
            case Token::Type::InputRedirection: parseInputRedirection(token); break;
            case Token::Type::OutputRedirection: parseOutputRedirection(token); break;
            case Token::Type::AppendRedirection: parseAppendRedirection(token); break;
            case Token::Type::OpenScope: parseOpenScope(token); break;
            case Token::Type::CloseScope: parseCloseScope(token); break;
            case Token::Type::None: break;
        }
    }

    return std::move(_root);
}

void Parser::parseWord(const Token& token)
{
    assert(token.type == Token::Type::Word);

    // If we receive a word without having a command at the current pointer,
    // we must instantiate a new command
    if (*_current == nullptr) {
        // We can determine the type of command from the word, which will
        // represent the command program.  First determine if the command is
        // builtin, creating the appropriate command where necessary.  If the
        // command is not builtin, assume it is an executable command
        if (token.text == "exit") {
            *_current = make_unique<ExitBuiltinCommand>();
        }
        else if (token.text == "test" || token.text == "[") {
            *_current = make_unique<TestBuiltinCommand>();
        }
        else {
            *_current = make_unique<ExecutableCommand>();
        }
    }

    auto didAccept =
        parseExecutableWord(token)
        || parseInputRedirectionWord(token)
        || parseOutputRedirectionWord(token)
        || parseAppendRedirectionWord(token);
    if (!didAccept) {
        throw std::runtime_error{"unexpected word encountered"};
    }
}

bool Parser::parseExecutableWord(const Token& token)
{
    // Ensure the current command is an executable
    auto command = dynamic_cast<ExecutableCommand*>(_current->get());
    if (command == nullptr) {
        return false;
    }

    // If there is no program in the command yet, take the word as the
    // program.  Otherwise, take it as another argument
    if (command->program.empty()) {
        command->program = token.text;
    }
    else {
        command->arguments.push_back(token.text);
    }

    return true;
}

bool Parser::parseInputRedirectionWord(const Token& token)
{
    // Ensure the current command is an input redirection
    auto command = dynamic_cast<InputRedirectionCommand*>(_current->get());
    if (command == nullptr) {
        return false;
    }

    // If there is no name in the path yet, take the word as the path
    if (command->path.empty()) {
        command->path = token.text;
    }
    else {
        // "foo < bar baz" is invalid
        throw std::runtime_error{"too many words after redirection"};
    }

    return true;
}

bool Parser::parseOutputRedirectionWord(const Token& token)
{
    // Ensure the current command is an output redirection
    auto command = dynamic_cast<OutputRedirectionCommand*>(_current->get());
    if (command == nullptr) {
        return false;
    }

    // If there is no name in the path yet, take the word as the path
    if (command->path.empty()) {
        command->path = token.text;
    }
    else {
        // "foo > bar baz" is invalid
        throw std::runtime_error{"too many words after redirection"};
    }

    return true;
}

bool Parser::parseAppendRedirectionWord(const Token& token)
{
    // Ensure the current command is an append redirection
    auto command = dynamic_cast<AppendRedirectionCommand*>(_current->get());
    if (command == nullptr) {
        return false;
    }

    // If there is no name in the path yet, take the word as the path
    if (command->path.empty()) {
        command->path = token.text;
    }
    else {
        // "foo >> bar baz" is invalid
        throw std::runtime_error{"too many words after redirection"};
    }

    return true;
}

void Parser::parseSequence(const Token& token)
{
    assert(token.type == Token::Type::Sequence);

    // If we come across a sequence delimiter outside of a scope, we must be
    // at the root level and we should replace the root with a sequential
    // command, making the current root the first command in the new root
    // sequence
    if (_scopes.empty()) {
        auto scope = make_unique<SequentialCommand>();
        if (_root != nullptr) {
            scope->sequence.push_back(std::move(_root));
        }

        ScopePair pair;
        pair.first = scope.get();
        pair.second = _current;
        _scopes.push(pair);
        _root = std::move(scope);
        _isRootSequence = true;
    }

    // Create an empty command at the back of the current scope sequence and
    // make it current
    auto scope = _scopes.top().first;
    scope->sequence.push_back(nullptr);
    _current = &scope->sequence.back();
}

void Parser::parseConjunction(const Token& token)
{
    assert(token.type == Token::Type::Conjunction);

    // "&& foo" is an invalid command, as is "foo; && bar"
    if (*_current == nullptr) {
        throw std::runtime_error{"conjunction must follow command"};
    }

    // Extract the current command from the tree, replace it with a
    // conjunctive command, and make the previous current command the primary
    // command of the conjunction
    auto current = make_unique<ConjunctiveCommand>();
    auto connective = current.get();
    current->primary = std::move(*_current);
    *_current = std::move(current);
    _current = &connective->secondary;
}

void Parser::parseDisjunction(const Token& token)
{
    assert(token.type == Token::Type::Disjunction);

    // "|| foo" is an invalid command, as is "foo; || bar"
    if (*_current == nullptr) {
        throw std::runtime_error{"disjunction must follow command"};
    }

    // Extract the current command from the tree, replace it with a
    // disjunctive command, and make the previous current command the primary
    // command of the disjunction
    auto current = make_unique<DisjunctiveCommand>();
    auto connective = current.get();
    current->primary = std::move(*_current);
    *_current = std::move(current);
    _current = &connective->secondary;
}

void Parser::parsePipe(const Token& token)
{
    assert(token.type == Token::Type::Pipe);

    // "| foo" is an invalid command, as is "foo; | bar"
    if (*_current == nullptr) {
        throw std::runtime_error{"pipe must follow command"};
    }

    // Extract the current command from the tree, replace it with a pipe
    // command, and make the previous current command the primary command
    // of the pipe
    auto current = make_unique<PipeCommand>();
    auto connective = current.get();
    current->primary = std::move(*_current);
    *_current = std::move(current);
    _current = &connective->secondary;
}

void Parser::parseInputRedirection(const Token& token)
{
    assert(token.type == Token::Type::InputRedirection);

    // "< foo" is an invalid command, as is "foo; < bar"
    if (*_current == nullptr) {
        throw std::runtime_error{"input redirection must follow command"};
    }

    // Extract the current command from the tree, replace it with an input
    // redirection command, and make the previous current command the primary
    // command of the redirection
    auto current = make_unique<InputRedirectionCommand>();
    current->primary = std::move(*_current);
    *_current = std::move(current);
}

void Parser::parseOutputRedirection(const Token& token)
{
    assert(token.type == Token::Type::OutputRedirection);

    // "> foo" is an invalid command, as is "foo; > bar"
    if (*_current == nullptr) {
        throw std::runtime_error{"output redirection must follow command"};
    }

    // Extract the current command from the tree, replace it with an output
    // redirection command, and make the previous current command the primary
    // command of the redirection
    auto current = make_unique<OutputRedirectionCommand>();
    current->primary = std::move(*_current);
    *_current = std::move(current);
}

void Parser::parseAppendRedirection(const Token& token)
{
    assert(token.type == Token::Type::AppendRedirection);

    // ">> foo" is an invalid command, as is "foo; >> bar"
    if (*_current == nullptr) {
        throw std::runtime_error{"append redirection must follow command"};
    }

    // Extract the current command from the tree, replace it with an output
    // redirection command, and make the previous current command the primary
    // command of the redirection
    auto current = make_unique<AppendRedirectionCommand>();
    current->primary = std::move(*_current);
    *_current = std::move(current);
}

void Parser::parseOpenScope(const Token& token)
{
    assert(token.type == Token::Type::OpenScope);

    // "foo (bar)" is an invalid command
    if (*_current != nullptr) {
        throw std::runtime_error{"scope must not follow command"};
    }

    // Create a new sequential command for the scope
    auto scope = make_unique<SequentialCommand>();
    scope->sequence.push_back(nullptr);

    // Create the contextual scope pair and push it into the stack
    ScopePair pair;
    pair.first = scope.get();
    pair.second = _current;
    _scopes.push(pair);

    // Make the inside of the new sequence the current command
    *_current = std::move(scope);
    _current = &pair.first->sequence.back();
}

void Parser::parseCloseScope(const Token& token)
{
    assert(token.type == Token::Type::CloseScope);

    // Detect imbalanced scopes
    if (_scopes.empty() || (_scopes.size() == 1 && _isRootSequence)) {
        throw std::runtime_error{"unbalanced closing parenthesis"};
    }

    // Exit the scope by making its stored command current
    _current = _scopes.top().second;
    _scopes.pop();
}

} // namespace rshell
