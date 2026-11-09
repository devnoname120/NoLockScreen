# NoLockScreen

NoLockScreen is a taiHEN user plugin that bypasses the PlayStation Vita lock screen at boot and after standby.

This standalone version resolves the required `SceShell` call sites at runtime instead of relying on firmware-specific module NIDs and hard-coded offsets. It rejects missing or ambiguous matches and applies neither patch unless both sites resolve uniquely.

## Compatibility

- Firmware 3.60 and newer
- Retail, testkit, devkit, and debug/internal system variants
- Hardware-tested on retail firmware 3.60 and a devkit running firmware 3.65
- Signature resolver validated against 43 available `SceShell` images spanning retail 3.60–3.74, external testkit/devkit builds, internal CEX/DEX/devtool/QAF builds, and available 4.00 images

## Installation

1. Download `nolockscreen.suprx` from the latest release.
2. Copy it to `ur0:tai/nolockscreen.suprx`.
3. Add the following entry under `*main` in the active taiHEN configuration:

```text
*main
ur0:tai/nolockscreen.suprx
```

4. Reboot the system.

Use the corresponding path when your active taiHEN directory is on another storage device.

## Build

Install [VitaSDK](https://vitasdk.org/) and set `VITASDK`, then run:

```sh
cmake -S . -B build
cmake --build build
```

The release module is written to `build/nolockscreen.suprx`.

## Tests

The host-side tests cover signature uniqueness and validation, exact configuration transformation, and synchronized self-disarm I/O behavior:

```sh
./tests/run.sh
```

Set `SANITIZE=1` to run the same tests with AddressSanitizer and UndefinedBehaviorSanitizer.

## Development safety build

`NOLOCKSCREEN_SELF_DISARM=ON` creates a one-shot development build that disables the exact `ur0:/tai/nolockscreen.suprx` entry before attempting runtime patching. This variant is intended for controlled hardware testing and is not the release artifact.

```sh
cmake -S . -B build-self-disarm -DNOLOCKSCREEN_SELF_DISARM=ON
cmake --build build-self-disarm
```

## Credits

The original NoLockScreen plugin was created by TheFloW as part of [VitaTweaks](https://github.com/TheOfficialFloW/VitaTweaks).

## License

GNU General Public License v3.0. See [LICENSE](LICENSE).
