# dudubot

My own LLM bot using C, intended to use with DeepSeek.

You only need a C compiler and `libcurl`. The build script assumes `clang` but
it should be compatible with GCC and maybe MSVC.

## Building and using

Currently, this is source-only. You have to build it manually to use it.

1. Build `build.c`
2. Run `build` (or `build.exe`)
3. Setup an environment variable `DUDUBOT_API_KEY` with your API key
4. (Optional) Setup an environment variable `DUDUBOT_URL` if you want to use an
   LLM provider other than DeepSeek
5. Run `dudubot`

The bot is a very lean REPL. You write some text, that text is evaluated by the
LLM and the response is streamed back to you.

There are some hidden options to control the session. These might change in the
future:

```
> tool x
```

Adds access of tool `x` to the bot. A dynamic library named "x" (`libx.so`,
`x.dll` or `libx.dylib`) is expected to exist in the same directory as
`dudubot` containing a symbol named `dudubot_tool`. That symbol will be used to
setup the tool data (description, arguments, etc).

```
> save x
```

These save the current session to a file. The format is a bit finicky, so it's
best to run a couple of sessions and see how they work.

```
> load x
```

Loads a saved session from a file. This is additive - so if you load "x" then
"y", the final session will be the contents of "x" followed by "y". This is
useful to create compositions (example: "x" could be a tool list, "y" could be
a system prompt).

```
> a
<contents>
.
```

This is just a quick `ed`-like way to add multiple lines to the same prompt,
like when you need to paste contents from the clipboard.

Just like stream editors, a line containing only a period (`.`) ends the
command.

```
> .
```

Starts another round. This is useful if the LLM returns without finishing a
round or if you want to `load` a previous session and run it.

## Future plans

These are large refactorings I want to think about it, then execute them when I
have a clear plan.

They are in no particular order.

* Support to read sessions ("chats") from the "chats" folder
* Search paths for chats and tools (should enable user-level customisations and
  easier third-party bundling on Linux)
* A tool returning instructions (almost a skill). This is more a
  proof-of-concept than a real thing, to be honest.
* Maybe support for MCP? This has very low priority because it requires a
  jsonp-based asynchronous stack.

## Won't dos

In the world of AI chatbots, these are taken for granted but there are no plans
to support them here.

* "Memory files" (`CLAUDE.md` etc). Manual session load/save works as fine.
  Also, you should avoid adding editor-specific files to your repositories. A
well-written `README` would be as good for your AI tooling as an automatically
managed "memory file";
* "Skills". These are just like tools but they return instructions to the LLM.
  They might also contain more scripts and more instructions. Just like
`CLAUDE.md` these could be better implemented in different ways, as we should
not trust scripts just because they came with a "skill" and extra instructions
could be refactored as documentation or other skills.

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

### Why no binaries?

Because distribution methods varies a lot by platform:

* Windows would be the easiest with a ZIP containing all files;
* Linux varies with the distro. Maybe it is possible to use a static-linked
  executables but I don't have capacity to implement and (more importantly)
maintain that; 
* OSX requires bundles and signing for a decent experience. Solutions like
  HomeBrew kinda work but they are sketchy for an unknown piece of software
like this.

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

### Why all files (dyn-libs and chats) are relative to executable?

Because it's the easiest way to consume them when you are running this from
a source build, and it works in the same way for every platform. Unfortunately,
each platform (Mac/Win/Lin) uses entirely different ways for native
experiences.

Better support will be added when time allows.

## Note on Windows

Code and experience is currently messy on Windows.

We need `libcurl.dll.a` to compile and `libcurl-x64.dll` to run.

## Note on Linux

Tests for Linux were done in an Alpine container. Roughly like this:

```
$ apk add clang gcc curl curl-dev binutils
...
$ clang -o build build.c && ./build
$ ./test-tool view_local_file '{"path":"tll.h"}'
...
$ export DUDUBOT_API_KEY=...
$ ./dudubot
> Hello
...
>
```

