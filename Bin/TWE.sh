clear
echo EasyHook bibliotekos veikimo testas.
echo RUN
FileMon.exe run FileUsage.exe
echo
echo HOOK
FileMon.exe hook FileUsage.exe