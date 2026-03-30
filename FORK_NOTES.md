# Fork Notes

This fork exists to carry a minimal terminal-emulation patch that adds:

- ECMA-48 `SGR 2` (`faint` / `dim`)
- ECMA-48 `SGR 9` (`strikethrough`)

Why this fork exists
--------------------

Upstream `mosh` currently has an open issue for `SGR 2` support and an
unmerged PR for `faint`/`strikethrough` support. The change itself is
small, but the upstream PR is stale and no clean upstream release
currently includes it.

This fork keeps the patch minimal so it is easy to rebase onto upstream.

Files changed
-------------

- `src/terminal/terminalframebuffer.h`
- `src/terminal/terminalframebuffer.cc`
- `src/tests/emulation-attributes.test`

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
