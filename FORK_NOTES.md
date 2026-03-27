# Fork Notes

This fork exists to carry a small set of terminal-emulation patches that add:

- ECMA-48 `SGR 2` (`faint` / `dim`)
- ECMA-48 `SGR 9` (`strikethrough`)
- OSC 52 clipboard parsing that accepts tmux-style variants

Why this fork exists
--------------------

Upstream `mosh` currently has an open issue for `SGR 2` support and an
unmerged PR for `faint`/`strikethrough` support. Upstream clipboard
support in `1.4.0` handles plain `OSC 52;c;...`, but tmux-over-mosh may
still need the more liberal OSC 52 parsing from upstream PR `#1054`.

This fork keeps the patch minimal so it is easy to rebase onto upstream.

Files changed
-------------

- `src/terminal/terminalframebuffer.h`
- `src/terminal/terminalframebuffer.cc`
- `src/terminal/terminalfunctions.cc`
- `src/tests/emulation-attributes.test`
- `src/tests/Makefile.am`
- `src/tests/osc52-clipboard.cc`

Homebrew tap
------------

This fork is distributed via:

- source fork: `https://github.com/antonme/mosh`
- tap repo: `https://github.com/antonme/homebrew-mosh`
- formula: `antonme/mosh/mosh-dim`

Install:

```sh
brew tap antonme/mosh
brew install antonme/mosh/mosh-dim
```

Upgrade:

```sh
brew update
brew upgrade antonme/mosh/mosh-dim
```

The formula installs the normal binaries:

- `mosh`
- `mosh-client`
- `mosh-server`

Build notes for macOS
---------------------

On macOS, prefer Apple's archive tools when building:

```sh
AR=/usr/bin/ar RANLIB=/usr/bin/ranlib ./configure --prefix="$PWD/dist"
make -j"$(sysctl -n hw.ncpu)"
make check
make install
```

Using Homebrew's GNU `ar`/`ranlib` can produce archives that Apple's
linker rejects.

Testing alternate binaries
--------------------------

You can test the fork without replacing your system `mosh`:

```sh
dist/bin/mosh --client="$PWD/dist/bin/mosh-client" --server="$PWD/dist/bin/mosh-server" host
```

Notes
-----

- For a normal `mosh` deployment, patch both client and server.
- The new attribute coverage is exercised through the existing
  `emulation-attributes.test` suite.
- The OSC 52 clipboard parser coverage is exercised through
  `osc52-clipboard`.
