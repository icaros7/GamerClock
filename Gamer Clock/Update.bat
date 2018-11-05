@echo off
echo Task kill Older Gamer Clock for OSD...
taskkill -f -im Gamer_Clock.exe
echo done.
echo Downloading Last Version Gamer Clock for OSD...
powershell "(new-object Net.WebClient).DownloadFile('https://github.com/icaros7/GamerClock/raw/master/Gamer%20Clock/Gamer_Clock.exe', 'Gamer_Clock.exe')"
echo done
echo.
echo If didn't see any error then
echo SuccessfulLy Update.
echo Press any key to launch Gamer Clock for OSD
pause
Gamer_Clock.exe