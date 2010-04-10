clear
echo Detours bibliotekos paleidimo testas.
echo Testas bus kartojamas 10 kartu.

for ((  i = 1 ;  i <= 10;  i++  ))
do
	echo
	echo
	echo Bandymas Nr.: $i
	echo RUN
	time DetoursHookCenter.exe run HookDllLoadingTest
	echo
	echo HOOK
	time DetoursHookCenter.exe hook HookDllLoadingTest
done