# macOS DS4 port work in progress

This directory contains an experimental native macOS bridge for the GIMX DS4
USB path. It is not the full GIMX application. The user confirmed the PS4
responded to the Move in an initial hardware test; sustained operation and all
individual button mappings remain to be verified.

`bridge.c` connects the PS Move, DS4 and GIMX adapter using the upstream DS4
report builder. It forwards feature GET/SET requests through `ds4_feature.c`,
which rejects unsupported requests. The bridge is separate from GIMX's `gusb`
and `controller.c` event loop. It sends neutral input if Move reports stop and
resets the adapter on exit. It is an experimental limited-scope bridge.

`../shared/gimxserial/src/posix/gserial.c` has an experimental macOS tty
branch using `IOSSIOSPEED`, including `/dev/cu.*` paths. It compiles but has
not been exercised with a USB-to-TTL adapter.

Run `make test` for request-validation and upstream DS4 report tests. With a USB DS4 and the workspace's
`psmoveapi/build/libpsmoveapi.dylib`, `make hardware-test` performs a read-only
`0xF2` feature-report request; it may require device access outside a sandbox.
This single operation is not a PS4 authentication test. The diagnostic opens
the first matching DS4; disconnect other DS4 devices to ensure it targets USB.

Build with `make build/bridge` and run `./build/bridge "$SERIAL_PORT"` with
`SERIAL_PORT` set to the adapter's serial device path,
with the DS4 and USB-to-TTL connected to the Mac, Pro Micro connected to the
PS4, and PS Move connected over Bluetooth. The bridge maps Move PS, Cross,
Circle, Square, Triangle, Select and Start to the corresponding DS4 controls.
Stop with Ctrl-C. The serial port may differ on another Mac.

The source snapshot is GIMX commit `e44a1cc9823939165d3d92418e9d679c66496ee1`
with its pinned submodules, copied from upstream; see `../LICENSE` and the
individual source notices. See `../../GIMX_MACOS_PORT_SURVEY.md` for status.
