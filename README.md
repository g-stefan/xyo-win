# XYO Win

C++ library
- Windows layer for the XYO C++ libraries and applications (Win32 only).
- Windows as objects: `Window` (virtual `windowProcedure`), `Application` / `SimpleApplication`
(window class, main window, single instance, icon), `MessageManager` (message loop, exit code).
- COM automation: `Variant`, `DispatchVariant`, `Function` (call `IDispatch` methods by name),
`Dispatch` + `XYO_WIN_DISPATCH__*` tables (expose C++ methods as `IDispatch`), `Ole::isValid`.
- System helpers: `Registry` (strings, DWORDs, keys), `Shell` (`.lnk` shortcuts, run as another user),
`User` (local accounts through ADSI), `Capture` (desktop / window screenshots to `Bitmap` or PNG),
`Util` (message broadcast).

Built on `xyo-system` and `xyo-pixel32`; used by `proxy-forward`, `xyo-win-service`,
`xyo-win-inject` and `dll-inject`.

## Documentation

- [Overview](docs/README.md) - purpose and design
- [Getting started](docs/getting-started.md) - build, depend on it, namespaces, first program, conventions
- [Windows and the message loop](docs/windows.md) - `Window`, `Application`, `SimpleApplication`, `MessageManager`, notifications
- [COM automation](docs/automation.md) - `Ole`, `Variant`, `DispatchVariant`, `Function`, `Dispatch`
- [System helpers](docs/system.md) - `Registry`, `Shell`, `User`, `Capture`, `Util`
- [API reference](docs/reference.md)

A Claude Code skill for this library is in
[.claude/skills/xyo-win](.claude/skills/xyo-win/SKILL.md).

## License

Copyright (c) 2014-2026 Grigore Stefan
Licensed under the [MIT](LICENSE) license.
