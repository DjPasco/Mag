#import "..\Detours\Bin\DetoursHookCenterD.exe"

try
{
		HRESULT hr = CoInitialize(0);

		DetoursHookCenterLib::IFileCheckerPtr		pPtr1(__uuidof(DetoursHookCenterLib::FileChecker));

		if(SUCCEEDED(hr))
		{	
			if(SUCCEEDED(hr))
			{
				VARIANT_BOOL vb_bool;
				hr = pPtr1->CheckFile('C', &vb_bool);
			}
			else
			{
				cout << "CoCreateInstance Failed." << endl;
			}
		}

		CoUninitialize();
}
catch(_com_error &err)
{
	cout << err.ErrorMessage();
}
