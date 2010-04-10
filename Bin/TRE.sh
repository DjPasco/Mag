clear
echo EasyHook bibliotekos paleidimo testas.
echo Testas bus kartojamas 10 kartu.

for ((  i = 1 ;  i <= 10;  i++  ))
do
	echo
	echo
	echo Bandymas Nr.: $i
	echo RUN
	time FileMon.exe run HookDllLoadingTest.exe
	echo
	echo HOOK
	time FileMon.exe hook HookDllLoadingTest.exe
done