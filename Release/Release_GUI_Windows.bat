@rem Copyright (c) MediaArea.net SARL. All Rights Reserved.
@rem
@rem Use of this source code is governed by a GPL v3+ and MPL v2+ license that can
@rem be found in the License.html file in the root of the source tree.
@rem

@rem echo off

@rem --- Clean up ---
del   MediaConch_GUI_Windows.exe

rem --- Installer ---
..\..\..\MediaArea-Utils-Binaries\Windows\NSIS\makensis ..\Source\Install\MediaConch_GUI_Windows.nsi

rem --- Clean up ---
if "%1"=="SkipCleanUp" goto SkipCleanUp
rmdir MediaConch_GUI_Windows\ /S /Q
:SkipCleanUp
