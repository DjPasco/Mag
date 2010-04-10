clear
echo Detours bibliotekos veikimo testas.
echo RUN
DetoursHookCenter.exe run FileUsage
echo
echo HOOK
DetoursHookCenter.exe hook FileUsage