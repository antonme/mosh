# Fork Notes

This repository is no longer just a minimal dim/strikethrough patch on top of
upstream `mosh`. The current fork release line also carries:

- ECMA-48 `SGR 2` (`faint` / `dim`) support
- ECMA-48 `SGR 9` (`strikethrough`) support
- SSH agent forwarding support, integrated from
  `mobile-shell/mosh#1297`


## Build Notes For macOS

On macOS, prefer Apple's archive tools when building:

```sh
AR=/usr/bin/ar RANLIB=/usr/bin/ranlib ./configure --prefix="$PWD/dist"
make -j"$(sysctl -n hw.ncpu)"
make check
make install
```

Using Homebrew's GNU `ar`/`ranlib` can produce archives that Apple's linker
rejects.

## Testing Alternate Binaries

You can test the fork without replacing your system `mosh`:

```sh
dist/bin/mosh --client="$PWD/dist/bin/mosh-client" --server="$PWD/dist/bin/mosh-server" host
```

## Notes

- For a normal `mosh` deployment, patch both client and server.
- The terminal attribute coverage is exercised through
  `src/tests/emulation-attributes.test`.
- The local integration test may need to run outside restrictive sandboxes
  because `mosh-server` needs to bind localhost UDP sockets.
