# System helpers

Free functions over Win32 / COM, in nested namespaces of `XYO::Win`. With
`using namespace XYO::Win;` write `XYO::Win::Registry::` and
`XYO::Win::Shell::` in full (see [Getting started](getting-started.md#3-include-and-namespaces)).

## Registry

Every function opens the key, does one thing and closes it. `masterkey` is
an open key or a root (`HKEY_CURRENT_USER`, `HKEY_LOCAL_MACHINE`, ...),
`key` a subkey path, `reg` the value name (`NULL` or `""` = the default
value). Each function has an ANSI version and a wide `...W` version with
the same rules. Results are `BOOL`: `TRUE` on success.

| Function | Does |
|----------|------|
| `createKey(masterkey, key)` | create (or open) the key and its missing parents, `KEY_ALL_ACCESS` |
| `createKeyVolatile(masterkey, key)` | same, the new key is volatile (in memory, gone at reboot); an existing key is only opened and stays as it is; a volatile key can not get non-volatile subkeys |
| `writeString(masterkey, key, reg, str)` | `REG_SZ`; the key must exist; `NULL` writes `""` |
| `writeDWord(masterkey, key, reg, value)` | `REG_DWORD`; the key must exist |
| `readString(masterkey, key, reg, buffer, size, def)` | `REG_SZ` or `REG_EXPAND_SZ` (not expanded) into `buffer`, always terminated. `size` is the buffer size **in bytes** (also for `readStringW`). Missing value, wrong type or a too small buffer: copies `def` (`NULL` = `""`, truncated to the buffer) and returns `FALSE` |
| `readDWord(masterkey, key, reg, &value, def)` | `REG_DWORD`; else `value = def`, `FALSE` |
| `getStringLength(masterkey, key, reg, &bytes)` | the stored size in **bytes**, including the terminator when it was stored (`writeString` stores it); allocate that many bytes for `readString` |
| `deleteKey(masterkey, key, reg, TRUE)` | delete the **value** `reg` of `key` |
| `deleteKey(masterkey, key, reg, FALSE)` | delete the **subkey** `reg` of `key`; it may have values but no subkeys |

```cpp
namespace Win = XYO::Win;

const char *key = "Software\\XYO\\MyTool";
char folder[MAX_PATH];
unsigned long int port;

Win::Registry::createKey(HKEY_CURRENT_USER, key);
Win::Registry::writeString(HKEY_CURRENT_USER, key, "folder", "C:\\Data");
Win::Registry::readString(HKEY_CURRENT_USER, key, "folder", folder, sizeof(folder), "C:\\");
Win::Registry::readDWord(HKEY_CURRENT_USER, key, "port", &port, 8080);   // default when missing
Win::Registry::deleteKey(HKEY_CURRENT_USER, key, "folder", TRUE);         // the value
Win::Registry::deleteKey(HKEY_CURRENT_USER, "Software\\XYO", "MyTool", FALSE); // the key
```

Notes: the registry view is the process's own (a 32 bit program sees
`WOW6432Node` under `HKLM\SOFTWARE`); writing under `HKEY_LOCAL_MACHINE`
needs elevation; `createKey` asks for `KEY_ALL_ACCESS`, so it fails on an
existing key you may only read.

## Shell

### Shortcuts (.lnk)

```cpp
bool createLink(const char *outputFile, const char *workingDirectory, const char *path,
                const char *arguments, const char *iconPath, int iconIndex, bool runAsAdministrator);
bool createLinkW(const wchar_t *outputFile, ...);   // same, UTF-16
```

Writes the shortcut `outputFile` (full path or relative to the current
folder, normally ending in `.lnk`) to `path`. The ANSI version takes
**UTF-8**. `workingDirectory`, `arguments` and `iconPath` may be `NULL`
(not set); `outputFile` and `path` are required. `runAsAdministrator` sets
the "Run as administrator" flag (`SLDF_RUNAS_USER`). OLE is initialized
on the calling thread when needed (a thread already in the multithreaded
apartment works too). An existing file is overwritten.

```cpp
// Desktop shortcut of this program
char exe[MAX_PATH];
char desktop[MAX_PATH];
GetModuleFileNameA(NULL, exe, MAX_PATH);       // ANSI: fine for ASCII paths
SHGetFolderPathA(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, desktop);
String link = String(desktop) + "\\My Tool.lnk";
XYO::Win::Shell::createLink(link, NULL, exe, "--start", exe, 0, false);
```

For non ASCII paths get them as UTF-16 (`GetModuleFileNameW`,
`SHGetKnownFolderPath`) and call `createLinkW`.

### Run as another user

```cpp
bool runAs(const char *username, const char *password, const char *command);   // UTF-8
bool runAsW(const wchar_t *username, const wchar_t *password, const wchar_t *command);
```

Starts `command` (a full command line, the program is its first token)
with `CreateProcessWithLogonW(..., LOGON_WITH_PROFILE, ...)` as
`username`, which may be `user`, `DOMAIN\user` or `user@domain`. Returns
`true` once the process started; it does not wait for it. `username` and
`command` are required. Wrong credentials fail (and count against the
account lockout policy, once per call).

## User accounts (ADSI)

These manage local (or domain) accounts through ADSI with the `WinNT://`
provider. They change the machine and need an elevated process. OLE is
initialized on the calling thread when needed. Strings are wide and not
modified.

| Function | Does |
|----------|------|
| `createUserAccountAsCurrentUserPrivilegeOnLocalComputer(user, password, fullName, description)` | create the local account `user`, enabled, password required and not expiring, and add it to **every group the current user is in** (so an administrator creates another administrator) |
| `createHiddenUserAccountAsCurrentUserPrivilegeOnLocalComputer(...)` | the same, then `hideUserFromWelcomeScreen` |
| `deleteUserAccountOnLocalComputer(user)` | `showUserInWelcomeScreen`, remove it from its groups, delete it |
| `hideUserFromWelcomeScreen(user)` | `HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon\SpecialAccounts\UserList`, value `user` = `0` |
| `showUserInWelcomeScreen(user)` | delete that value |
| `createUserAccountOnAD(user, fullName, description, password, containerPath, newUserPath, templateUserPath)` | `HRESULT`; the general form: create `user` in the container ADsPath (`L"WinNT://HOST,Computer"`), then add `newUserPath` (`L"WinNT://HOST/user,User"`) to the groups of `templateUserPath` (`L"WinNT://DOMAIN/someone,User"`) |
| `removeUserAccountOnAD(userPath)` | `HRESULT`; delete the user with that ADsPath (`E_INVALIDARG` if it is not a user) |
| `addToUserGroupsUser(IADsUser *, BSTR memberPath)` / `removeUserFromItsGroups(IADsUser *, BSTR member)` | add / remove a member to / from every group of an `IADsUser`; the first failure is returned, the rest still run |

The local functions build the ADsPaths from
`GetComputerNameExW(ComputerNameDnsFullyQualified)` and
`GetUserNameExW(NameSamCompatible)`. They return `bool`; use the `...OnAD`
forms for the `HRESULT`.

## Capture

```cpp
TPointer<Bitmap> captureDesktop();                       // all monitors
bool captureDesktopToPNGFile(const char *fileName);
TPointer<Bitmap> captureWindow(HWND hwnd);               // client area
bool captureWindowToPNGFile(HWND hwnd, const char *fileName);
```

- `captureDesktop` copies the whole **virtual screen** (every monitor,
  `SM_XVIRTUALSCREEN` ... `SM_CYVIRTUALSCREEN`, layered windows included)
  into a new 32 bit `Bitmap` (`xyo-pixel32`, a BMP in memory, bottom-up,
  BGRA). The alpha bytes of a screen copy are undefined: call
  `bitmap->setAlpha32(255)` before `Process::imageFromBitmap(bitmap)` or
  the `Image` is transparent.
- `captureWindow` copies the client area of `hwnd` from its device
  context, as shown on screen: parts covered by other windows or off
  screen may not be right, a minimized window has an empty client area
  (`nullptr`). An invalid window gives `nullptr`.
- `...ToPNGFile` capture, make the image opaque and save it as an RGBA
  PNG; `false` on any failure.
- Sizes are in the process's DPI coordinates: a program that is not DPI
  aware gets scaled (virtualized) sizes on high DPI screens. Declare DPI
  awareness in the manifest (or `SetProcessDpiAwarenessContext`) for
  pixel exact captures.

```cpp
TPointer<Bitmap> screen = Capture::captureDesktop();
if (screen) {
	screen->setAlpha32(255);
	TPointer<Image> image = Process::imageFromBitmap(screen);
	TPointer<Image> half = Process::resize(image, image->width / 2, image->height / 2);
	Process::pngSave(half, "screen-half.png");
};
```

## Util

| Function | Does |
|----------|------|
| `postMessageToProcessWindows(id, wParam, lParam)` | `PostMessage` to every top-level window of the current process (`EnumWindows`) |
| `sendMessageToAllChildWindowsIE(parent, id, wParam, lParam)` | `SendMessage` to every child of `parent` whose class is `Internet Explorer_Server` (the MSHTML document window) |
