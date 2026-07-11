# dudubot

My own LLM bot using C, intended to use with DeepSeek.

You only need a C compiler and `libcurl`. The build script assumes `clang` but
it should be compatible with GCC and maybe MSVC.

## Architectural decisions

### Why C?

Why not? I enjoy coding with C because I have a complete view of every aspect
of what the bot does.

As a recent bonus, the main executable is simple but powerful and extensible.
One can easily wrap it in a fancier UI or add new tools just by writing a
shared library.

### Why DeepSeek?

In theory, it does not need to be DeepSeek. It should work with any
OpenAI-compatible provider.

I personally use DeepSeek because it tends to provide smaller answers and it
seems to follow instructions more precisely. Personal example using OpenCode:
if I ask DeepSeek to read a file, I get it to read the file and explain back to
me; while Claude tends to spin up an entire investigation (tested using the
same config files but different models).

### Why libcurl?

Because I thought it was readily available in both OSX and Windows. It is in
OSX but it is not on Windows. Also, I presume most Linux users either have it
as well or they should be comfortable enough to install it otherwise.

I would love to have the HTTP and SSL code written by myself but SSL is a
complicated beast to tame. I would be able to cut a lot of corners for HTTP
(after all, the request could be a "template" and the response could be ignored
until the double CRLF) but SSL requires a number of security algorithms by
design (even OpenSSL is something odd to use).

### Why clang?

Because it's an easy grab on OSX, Windows and Linux. Therefore I can use the
same compiler, same flags, etc.

### Why a custom build "script"? (i.e. "build.c")

It removes all sort of build-system dependencies. If you got a working C
compiler, then you are good to go. Unlike what would happen if I force you to
have CMake, Ninja, MSVC, Jai, Cargo, whatever.

### Why the interface is so ugly?

Because I like that way. I find existing "Code" assistants (Claude, OpenCode,
etc) to be extremely distracting.

The simple interface enables basic UNIX piping into a "prettier" interface with
some basic parsing (example: detecting the prompt after `finish_reason` etc).

### Why manual session persistence/management?

That is way more powerful than the automatic persistence. Given any point of
the conversation, you can `save` a session, manually edit it, then `load` it in
another process.

That would be similar to "forking" a session (like what some web-based bots
provide) but it converts the whole memory into an human-editable file.

## Note on Windows

Code and experience is currently messy on Windows.

We need `libcurl.dll.a` to compile and `libcurl-x64.dll` to run.

