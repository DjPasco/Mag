//////////////////////////////////////////////////////////////////////
//
// ScheduledTask.cpp: implementation of the CScheduledTask class.
//
//////////////////////////////////////////////////////////////////////
//
// This is a basic class for adding and deleting tasks from the 
// IE4/IE5/98/NT5 task scheduler.  Written by Michael Dunn, 
// mdunn at inreach dot com.
// You can use it to your heart's content, just give me credit where 
// credit is due.  I'd also appreciate an email telling me if you use
// this code in a publicly-released program, just to satisfy my own
// curiosity about whether folks are really using the code.
//
// Notes:
//  1. You need to call AfxOleInit() before using this class, preferably
//     in your app's InitInstance() function.
//
//////////////////////////////////////////////////////////////////////
//
// Release history:
//  Jan 1, 1999: v1.0 - First release!
//
//  Apr 18, 1999: v1.1 
//      * Fixed compile errors when building for Unicode.  Thanks to Chris
//        Maunder (cmaunder at dynamite dot com dot au) for the fixes.
//      * Fixed SaveTask() to properly check for an existing
//        task.  The change was necessary because I worked around
//        a bug in IE 4, but the bug was fixed in IE 5,
//        resulting in my code breaking when run on IE 5.
//      * Added a #pragma to automatically link with
//        mstask.lib, so you don't have to remember to do that
//        in your app.
//      * Added a check after the CoCreateInstance() call to see if
//        the error is REGDB_E_CLASSNOTREG.  If so, I display a nicer
//        trace message explaining that you don't have the scheduler
//        installed.
//      * Changed the macro __EVERYMONTH to DDS_CST_EVERYMONTH, 
//        because I noticed one day that the MSVC docs say that identifiers
//        beginning with 2 underscores are reserved by the compiler.  "DDS"
//        is my own prefix, and "CST" stands for CScheduledTask.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScheduledTask.h"
#include <afxconv.h>    // For ANSI->Unicode conversion macros.

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#pragma comment(lib, "mstask.lib")

//////////////////////////////////////////////////////////////////////
// Convenience macros

#define DDS_CST_EVERYMONTH \
                        TASK_JANUARY  | TASK_FEBRUARY | TASK_MARCH     | \
                        TASK_APRIL    | TASK_MAY      | TASK_JUNE      | \
                        TASK_JULY     | TASK_AUGUST   | TASK_SEPTEMBER | \
                        TASK_OCTOBER  | TASK_NOVEMBER | TASK_DECEMBER  


//////////////////////////////////////////////////////////////////////
// Construction/Destruction

CScheduledTask::CScheduledTask()
{
    Reset();
}

CScheduledTask::~CScheduledTask()
{
}


//////////////////////////////////////////////////////////////////////
// Data accessor functions

//////////////////////////////////////////////////////////////////////////
//
// Function:    GetStartDateTime()
//
// Description:
//  Returns the task's starting date/time.
//
// Input:
//  time: [out] CTime object that will receive the starting time, if a
//              time has been set.
//
// Returns:
//  TRUE if a starting time has been set and returned, FALSE if no starting
//  time was set.
//
//////////////////////////////////////////////////////////////////////////

BOOL CScheduledTask::GetStartDateTime ( CTime& time ) const
{
BOOL bRet = FALSE;

    if ( 0 != m_timeStart.wYear )
        {
        time = m_timeStart;
        bRet = TRUE;
        }
    else
        {
        TRACE0("CScheduledTask::GetStartDateTime() - no start time set.\n");
        }

    return bRet;
}


//////////////////////////////////////////////////////////////////////////
//
// Function:    SetStartDateTime()
//
// Description:
//  Sets the starting date/time for this task.
//
// Input:
//  time: [in] A CTime object holding the starting date/time.
//  -or-
//        [in] A SYSTEMTIME struct holding the starting date/time.
//
// Returns:
//  Nothing.  
//
// Note:
//  The seconds and milliseconds fields of the time are not used.
//
//////////////////////////////////////////////////////////////////////////

void CScheduledTask::SetStartDateTime ( const CTime& time )
{
    time.GetAsSystemTime ( m_timeStart );

    m_timeStart.wSecond = 0;
    m_timeStart.wMilliseconds = 0;
}

void CScheduledTask::SetStartDateTime ( const SYSTEMTIME& time )
{
    m_timeStart = time;

    m_timeStart.wSecond = 0;
    m_timeStart.wMilliseconds = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Function:    GetEndDate()
//
// Description:
//  Returns the task's ending date.
//
// Input:
//  time: [out] CTime object that will receive the ending date, if a
//              date has been set.
//
// Returns:
//  TRUE if an ending date has been set and returned, FALSE if no ending
//  date was set.
//
// Note:
//   Only the date fields of time are filled in.
//
//////////////////////////////////////////////////////////////////////////

BOOL CScheduledTask::GetEndDate ( CTime& time ) const
{
BOOL bRet = FALSE;

    if ( 0 != m_timeEnd.wYear )
        {
        time = m_timeEnd;
        bRet = TRUE;
        }
    else
        {
        TRACE0("CScheduledTask::GetEndDateTime() - no end time set.\n");
        }

    return bRet;
}


//////////////////////////////////////////////////////////////////////////
//
// Function:    SetEndDate()
//
// Description:
//  Sets the task's ending date.
//
// Input:
//  time: [in] CTime object that holds the ending date.  The time fields are
//             not used and are ignored.
//  -or-
//  time: [in] SYSTEMTIME struct holding the ending date.  The time fields
//             are ignored.
//
// Returns:
//  Nothing.
//
//////////////////////////////////////////////////////////////////////////

void CScheduledTask::SetEndDate ( const CTime& time )
{
    ZeroMemory ( &m_timeEnd, sizeof(SYSTEMTIME) );

    m_timeEnd.wYear  = time.GetYear();
    m_timeEnd.wMonth = time.GetMonth();
    m_timeEnd.wDay   = time.GetDay();
}

void CScheduledTask::SetEndDate ( const SYSTEMTIME& time )
{
    ZeroMemory ( &m_timeEnd, sizeof(SYSTEMTIME) );

    m_timeEnd.wYear  = time.wYear;
    m_timeEnd.wMonth = time.wMonth;
    m_timeEnd.wDay   = time.wDay;
}


//////////////////////////////////////////////////////////////////////////
//
// Function:    GetFrequency()
//
// Description:
//  Returns the frequency of this task.
//
// Input:
//  Nothing.
//
// Returns:
//  The task's frequency (a member of enum ETaskFrequency).  Test the return
//  against freqUnset to determine if a frequency has been set.
//
//////////////////////////////////////////////////////////////////////////

CScheduledTask::ETaskFrequency CScheduledTask::GetFrequency() const
{
    ASSERT ( m_eFreq >= freqFirst  &&  m_eFreq <= freqLast );

    return m_eFreq;
}


//////////////////////////////////////////////////////////////////////////
//
// Function:    SetFrequency()
//
// Description:
//  Sets the frequency of this task.
//
// Input:
//  freq: [in] The frequency for the task.
//
// Returns:
//  Nothing.
//
//////////////////////////////////////////////////////////////////////////

void CScheduledTask::SetFrequency  ( CScheduledTask::ETaskFrequency freq )
{
    ASSERT ( freq >= freqFirst  &&  freq <= freqLast );

    m_eFreq = freq;
}


//////////////////////////////////////////////////////////////////////////
//
// Function:    GetProgram()
//
// Description:
//  Returns the program that this task will run.
//
// Input:
//  Nothing.
//
// Returns:
//  The fully-qualified path to the program to run, or an empty string
//  if no program has been set.
//
//////////////////////////////////////////////////////////////////////////

CString CScheduledTask::GetProgram() const
{
    return m_sProgramPath;
}


//////////////////////////////////////////////////////////////////////////
//
// Function:    SetProgram()
//
// Description:
//  Sets the program that this task will run.
//
// Input:
//  szProgram: [in] The fully-qualified path to the program to run.
//
// Returns:
//  Nothing.
//
//////////////////////////////////////////////////////////////////////////

void CScheduledTask::SetProgram ( LPCTSTR szProgram )
{
    ASSERT ( AfxIsValidString ( szProgram ));

#ifdef _DEBUG
    if ( 0xFFFFFFFF == ::GetFileAttributes ( szProgram ) )
        {
        TRACE1("CScheduledTask::SetProgram() - warning, program <%s> not found.\n",
               szProgram );
        }
#endif

    m_sProgramPath = szProgram;
}


//////////////////////////////////////////////////////////////////////////
//
// Function:    GetParameters()
//
// Description:
//  Returns the parameters for this task.
//
// Input:
//  Nothing.
//
// Returns:
//  The parameters for this task, or an empty string if no parameters have
//  been set.
//
//////////////////////////////////////////////////////////////////////////

CString CScheduledTask::GetParameters() const
{
    return m_sParameters;
}


//////////////////////////////////////////////////////////////////////////
//
// Function:    SetParameters()
//
// Description:
//  Sets the parameters that this task will run.
//
// Input:
//  szParameters: [in] The parameters for this task.
//
// Returns:
//  Nothing.
//
//////////////////////////////////////////////////////////////////////////

void CScheduledTask::SetParameters ( LPCTSTR szParameters )
{
    ASSERT ( AfxIsValidString ( szParameters ));

    m_sParameters = szParameters;
}


//////////////////////////////////////////////////////////////////////////
//
// Function:    GetStartingDir()
//
// Description:
//  Returns the starting directory for the program that this task will run.
//
// Input:
//  Nothing.
//
// Returns:
//  The starting directory.
//
//////////////////////////////////////////////////////////////////////////

CString CScheduledTask::GetStartingDir() const
{
    return m_sStartingDir;
}


//////////////////////////////////////////////////////////////////////////
//
// Function:    SetStartingDir()
//
// Description:
//  Sets the starting directory for the program that this task will run.
//
// Input:
//  szStartingDir: [in] The starting directory.
//
// Returns:
//  Nothing.
//
//////////////////////////////////////////////////////////////////////////

void CScheduledTask::SetStartingDir ( LPCTSTR szStartingDir )
{
    ASSERT ( AfxIsValidString ( szStartingDir ));

    m_sStartingDir = szStartingDir;
}


//////////////////////////////////////////////////////////////////////////
//
// Function:    GetComment()
//
// Description:
//  Returns the comment that is displayed in this task's property page.
//
// Input:
//  Nothing.
//
// Returns:
//  This task's comment.
//
//////////////////////////////////////////////////////////////////////////

CString CScheduledTask::GetComment() const
{
    return m_sComment;
}


//////////////////////////////////////////////////////////////////////////
//
// Function:    SetComment()
//
// Description:
//  Sets the comment that will be displayed in the task's property page
//
// Input:
//  szComment: [in] The comment to set.
//
// Returns:
//  Nothing.
//
//////////////////////////////////////////////////////////////////////////

void CScheduledTask::SetComment ( LPCTSTR szComment )
{
    ASSERT ( AfxIsValidString ( szComment ));

    m_sComment = szComment;
}


//////////////////////////////////////////////////////////////////////////
// Other functions

//////////////////////////////////////////////////////////////////////////
//
// Function:    Reset()
//
// Description:
//  Reset's the object's data to all zero/empty.
//
// Input:
//  Nothing.
//
// Returns:
//  Nothing.
//
//////////////////////////////////////////////////////////////////////////

void CScheduledTask::Reset()
{
    ZeroMemory ( &m_timeStart, sizeof(SYSTEMTIME) );
    ZeroMemory ( &m_timeEnd, sizeof(SYSTEMTIME) );

    m_eFreq = freqUnset;

    m_sProgramPath.Empty();
    m_sParameters.Empty();
    m_sStartingDir.Empty();
    m_sComment.Empty();
}


//////////////////////////////////////////////////////////////////////////
//
// Function:    SaveTask()
//
// Description:
//  Saves the task to the task scheduler.
//
// Input:
//  szTaskName: [in] The name to give the task.  Since the task is saved as a
//                   file, the resulting file name must contain less than
//                   MAX_PATH characters.
//  bFailIfExists: [in] Flag indicating whether the function will fail if a
//                      task with the given name already exists. Defaults
//                      to FALSE.
//
// Returns:
//  S_OK if successful, E_FAIL if the program name or task name weren't set,
//  or an OLE error code if some other error occured along the way.
//
//////////////////////////////////////////////////////////////////////////
// Apr 18, 1999: Changed how the code detects whether a task already exists
//   in the scheduler.  This is because in IE 5, MS fixed 
//   ITaskScheduler::NewWorkItem() to return ERROR_FILE_EXISTS if a task
//   with the given name exists.  In IE 4 it did not return an error.
//   Also changed the cleanup logic to use __try/__finally instead of
//   gotos.  Just 'cuz.
//////////////////////////////////////////////////////////////////////////

HRESULT CScheduledTask::SaveTask ( LPCTSTR szTaskName,
                                   BOOL    bFailIfExists /*=FALSE*/ ) const
{
HRESULT         hr;
ITaskScheduler* pISched    = NULL;
IUnknown*       pIUnk      = NULL;
IPersistFile*   pIFile     = NULL;
ITask*          pITask     = NULL;
ITaskTrigger*   pITaskTrig = NULL;
TASK_TRIGGER    rTrigger;
DWORD           dwTaskFlags;
WORD            wTrigNumber;

    USES_CONVERSION;

    ASSERT ( AfxIsValidString ( szTaskName ));

    // Get an interface to the scheduler.

    hr = ::CoCreateInstance (
               CLSID_CTaskScheduler,
               NULL,
               CLSCTX_INPROC_SERVER,
               IID_ITaskScheduler,
               (void **) &pISched );


    if ( FAILED(hr) )
        {
#ifdef _DEBUG
        TRACE1("CScheduledTask::SaveTask() - failed to create a task scheduler interface. Return = 0x%08X\n",
               (DWORD) hr );
        
        // Check if the error was "class not registered".  If so, you don't
        // have the scheduler installed.  I display a nice long message in
        // the debug window, which hopefully explains what's up. :)

        if ( REGDB_E_CLASSNOTREG == hr )
            {
            TRACE0("    The error was REGDB_E_CLASSNOTREG, meaning you don't have the scheduler installed.\n"
                _T("    If you are running 95 or NT 4 with IE 4, you must install the task scheduler from the\n")
                _T("    IE components install page on MS's web site or the IE CD.\n")
                _T("    If you're on 98, NT 5, or 95/NT 4 with IE 5, then something's wrong with your install\n")
                _T("    because the scheduler should always be installed.\n")
                _T("    Note that this class does *NOT* work with the \"AT\" service, which is the default\n")
                _T("    scheduler on NT 4 and earlier.\n") );
            }
#endif  // _DEBUG

        return hr;
        }


__try
    {
    // Check if a task with the given name already exists in the scheduler.
    // I do this check manually because the behavior of 
    // ITaskScheduler::NewWorkItem() is different between IE 4 and IE 5.
    // In IE 4, NewWorkItem() will succeed if a task with the name you pass it
    // already exists (even though the INetSDK docs says it should fail).
    // In IE 5, NewWorkItem() has been fixed to match the docs.
    // So, my solution is to call ITaskScheduler::Activate() and pass it the
    // task name.  If that function succeeds, then I know a task with the
    // given name already exists.
    // (Note to MS: This _really_ ought to be mentioned in the KB!!)

    hr = pISched->Activate ( T2COLE ( szTaskName ), IID_ITask, &pIUnk );

    if ( SUCCEEDED(hr) )
        {
        // A task with the given name already exists.  Check bFailIfExists
        // to see what we should do.

        pIUnk->Release();               // We don't need this interface.
        pIUnk = NULL;

        if ( bFailIfExists )
            {
            TRACE0("CScheduledTask::SaveTask() - A task with the given name already exists; failing.\n");
            return HRESULT_FROM_WIN32 ( ERROR_FILE_EXISTS );
            }
        else
            {
            // Try to delete the existing task.  If the delete succeeds, then
            // we proceed.  Otherwise, we'll bail out with an error.

            TRACE0("CScheduledTask::SaveTask() - A task with the given name already exists; deleting it.\n");

            hr = CScheduledTask::DeleteTask ( szTaskName );

            if ( FAILED(hr) )
                {
                TRACE1("CScheduledTask::SaveTask() - couldn't delete existing task! Bailing out. Return = 0x%08X\n",
                       (DWORD) hr );
                return hr;
                }
            }
        }


    // Create a new task.

    hr = pISched->NewWorkItem ( T2COLE ( szTaskName ), CLSID_CTask,
                                IID_ITask, &pIUnk );

    if ( FAILED(hr) )
        {
        TRACE1("CScheduledTask::SaveTask() - couldn't create a new work item. Return = 0x%08X\n",
               (DWORD) hr );
        return hr;
        }


    // We now have an IUnknown pointer.  This is queried for an ITask
    // pointer on the work item we just added.
    
    hr = pIUnk->QueryInterface ( IID_ITask, (void **) &pITask );

    if ( FAILED(hr) )
        {
        TRACE1("CScheduledTask::SaveTask() - QI() on IUnknown failed to get an ITask. Return = 0x%08X\n",
               (DWORD) hr );
        return hr;
        }
    

    // Clean up the IUnknown, as we are done with it.
    
    pIUnk->Release();
    pIUnk = NULL;


    // Set the program name.

    hr = pITask->SetApplicationName ( T2COLE( (LPCTSTR) m_sProgramPath ));

    if ( FAILED(hr) )
        {
        TRACE1("CScheduledTask::SaveTask() - failed to set application. Return = 0x%08X\n",
               (DWORD) hr );
        return hr;
        }


    // Set the app's parameters.

    if ( m_sParameters.GetLength() > 0 )
        {
        hr = pITask->SetParameters ( T2COLE( (LPCTSTR) m_sParameters ));

        if ( FAILED(hr) )
            {
            TRACE1("CScheduledTask::SaveTask() - failed to set parameters. Return = 0x%08X\n",
                   (DWORD) hr );
            return hr;
            }
        }        


    // Set the starting directory.

    if ( m_sStartingDir.GetLength() > 0 )
        {
        hr = pITask->SetWorkingDirectory ( T2COLE( (LPCTSTR) m_sStartingDir ));

        if ( FAILED(hr) )
            {
            TRACE1("CScheduledTask::SaveTask() - failed to set starting directory. Return = 0x%08X\n",
                   (DWORD) hr );
            return hr;
            }
        }


    // Set the job comment.

    if ( m_sComment.GetLength() > 0 )
        {
        hr = pITask->SetComment ( T2COLE( (LPCTSTR) m_sComment ));

        if ( FAILED (hr) )
            {
            TRACE1("CScheduledTask::SaveTask() - failed to set task comment. Return = 0x%08X\n",
                   (DWORD) hr );
            return hr;
            }
        }


    // Set the flags on the task object
    
    // The two flags below are the default for events created via the task 
    // scheduler UI.
    // Note that I _don't_ set TASK_FLAG_DELETE_WHEN_DONE.  Setting this flag
    // is bad if you have the "Notify me of missed tasks" option on in the
    // scheduler.  If this flag is set and the event is missed, the scheduler
    // nukes the event without notifying you .  How mean.

    dwTaskFlags = TASK_FLAG_DONT_START_IF_ON_BATTERIES |
                  TASK_FLAG_KILL_IF_GOING_ON_BATTERIES;

    // On NT, set the interactive flag so the user can see it.
    if ( !( GetVersion() & 0x80000000 ) )
        {
        dwTaskFlags |= TASK_FLAG_INTERACTIVE;
        }

    hr = pITask->SetFlags ( dwTaskFlags );

    if ( FAILED (hr) )
        {
        TRACE1("CScheduledTask::SaveTask() - failed to set task flags. Return = 0x%08X\n",
               (DWORD) hr );
        return hr;
        }


    // Now, create a trigger to run the task at our specified time.

    hr = pITask->CreateTrigger ( &wTrigNumber, &pITaskTrig );

    if ( FAILED (hr) )
        {
        TRACE1("CScheduledTask::SaveTask() - failed to create a task trigger. Return = 0x%08X\n",
               (DWORD) hr );
        return hr;
        }


    // Now, fill in the trigger as necessary.  Note that the seconds field of
    // m_timeStart is not used since the scheduler only stores the hour
    // and minute of the starting time.

    ZeroMemory ( &rTrigger, sizeof (TASK_TRIGGER) );

    rTrigger.cbTriggerSize = sizeof (TASK_TRIGGER);
    rTrigger.wBeginYear    = m_timeStart.wYear;
    rTrigger.wBeginMonth   = m_timeStart.wMonth;
    rTrigger.wBeginDay     = m_timeStart.wDay;
    rTrigger.wStartHour    = m_timeStart.wHour;
    rTrigger.wStartMinute  = m_timeStart.wMinute;

    if ( 0 != m_timeEnd.wYear )
        {
        rTrigger.rgFlags   = TASK_TRIGGER_FLAG_HAS_END_DATE;
        rTrigger.wEndYear  = m_timeEnd.wYear;
        rTrigger.wEndMonth = m_timeEnd.wMonth;
        rTrigger.wEndDay   = m_timeEnd.wDay;
        }


    switch ( m_eFreq )
        {
        case freqOnce: 
            rTrigger.TriggerType = TASK_TIME_TRIGGER_ONCE;
        break;
        
        case freqDaily:
            rTrigger.TriggerType = TASK_TIME_TRIGGER_DAILY;

            // Repeat every day.
            rTrigger.Type.Daily.DaysInterval = 1;
        break;

        case freqWeekly:
            rTrigger.TriggerType = TASK_TIME_TRIGGER_WEEKLY;

            rTrigger.Type.Weekly.rgfDaysOfTheWeek = 
                GetDayOfWeekFlag ( m_timeStart );

            // Repeat every week.
            rTrigger.Type.Weekly.WeeksInterval = 1;
        break;

        case freqMonthly:
            rTrigger.TriggerType = TASK_TIME_TRIGGER_MONTHLYDATE;

            rTrigger.Type.MonthlyDate.rgfDays = 1 << ( m_timeStart.wDay - 1 );
            rTrigger.Type.MonthlyDate.rgfMonths = DDS_CST_EVERYMONTH;
        break;

        DEFAULT_UNREACHABLE;
        }


    // Add this trigger to the task using ITaskTrigger::SetTrigger

    hr = pITaskTrig->SetTrigger ( &rTrigger );

    if ( FAILED(hr) )
        {
        TRACE1("CScheduledTask::SaveTask() - failed to add trigger to the task. Return = 0x%08X\n",
               (DWORD) hr );
        return hr;
        }


    // Save the changes with IPersistFile::SaveTask().  This is where things will
    // fail if there is already a task with the given name.

    hr = pITask->QueryInterface ( IID_IPersistFile, (void **) &pIFile );

    if ( FAILED (hr) )
        {
        TRACE1("CScheduledTask::SaveTask() - failed to get an IPersistFile interface on the task. Return = 0x%08X\n",
               (DWORD) hr );
        return hr;
        }

    hr = pIFile->Save ( NULL, FALSE );

    if ( FAILED(hr) )
        {
        TRACE1("CScheduledTask::SaveTask() - error saving task. Return = 0x%08X\n",
               (DWORD) hr );
        return hr;
        }

    }   // end __try
__finally
    {
    // Clean up all the interfaces.
    
    if ( pIFile != NULL )
        pIFile->Release();

    if ( pITaskTrig != NULL )
        pITaskTrig->Release();

    if ( pITask != NULL )
        pITask->Release();

    if ( pISched != NULL )
        pISched->Release();

    }   // end __finally

    return hr;
}


//////////////////////////////////////////////////////////////////////////
//
// Function:    DeleteTask()
//
// Description:
//  Deletes a task with the specified name.
//
// Input:
//  szTaskName: [in] The name of the task to delete.
//
// Returns:
//  S_OK if successful, E_FAIL if the task name was empty,
//  or an OLE error code if some other error occured along the way.
//
//////////////////////////////////////////////////////////////////////////

/*static*/ HRESULT CScheduledTask::DeleteTask ( LPCTSTR szTaskName )
{
HRESULT         hr;
ITaskScheduler* pISched    = NULL;
    
    USES_CONVERSION;

    ASSERT ( AfxIsValidString ( szTaskName ));

    // Validate parameters.

    if ( 0 == lstrlen ( szTaskName ) )
        {
        return E_FAIL;
        }


    // Get an interface to the scheduler.

    hr = ::CoCreateInstance (
               CLSID_CTaskScheduler,
               NULL,
               CLSCTX_INPROC_SERVER,
               IID_ITaskScheduler,
               (void **) &pISched );

 
    if ( FAILED(hr) )
        {
#ifdef _DEBUG
        TRACE1("CScheduledTask::DeleteTask() - failed to create a task scheduler interface. Return = 0x%08X\n",
               (DWORD) hr );

        // Check if the error was "class not registered".  If so, you don't
        // have the scheduler installed.  I display a nice long message in
        // the debug window, which hopefully explains what's up. :)

        if ( REGDB_E_CLASSNOTREG == hr )
            {
            TRACE0("    The error was REGDB_E_CLASSNOTREG, meaning you don't have the scheduler installed.\n"
                _T("    If you are running 95 or NT 4 with IE 4, you must install the task scheduler from the\n")
                _T("    IE components install page on MS's web site or the IE CD.\n")
                _T("    If you're on 98, NT 5, or 95/NT 4 with IE 5, then something's wrong with your install\n")
                _T("    because the scheduler should always be installed.\n")
                _T("    Note that this class does *NOT* work with the \"AT\" service, which is the default\n")
                _T("    scheduler on NT 4 and earlier.\n") );
            }
#endif  // _DEBUG

        return hr;
        }


    // Delete the task.

    hr = pISched->Delete ( T2COLE ( szTaskName ));
  
    if ( FAILED(hr) )
        {
        TRACE1("CScheduledTask::DeleteTask() - failed to delete task. Return = 0x%08X\n",
               (DWORD) hr );
        }


    pISched->Release();

    return hr;
}

bool CScheduledTask::CheckTask(LPCTSTR szTaskName)
{
	HRESULT         hr;
	ITaskScheduler* pISched(NULL);
	IUnknown*       pIUnk(NULL);
	bool bFound(false);

    USES_CONVERSION;

    ASSERT ( AfxIsValidString ( szTaskName ));

    hr = ::CoCreateInstance (
               CLSID_CTaskScheduler,
               NULL,
               CLSCTX_INPROC_SERVER,
               IID_ITaskScheduler,
               (void **) &pISched );


    if(FAILED(hr))
	{
		return false;
	}


	hr = pISched->Activate ( T2COLE ( szTaskName ), IID_ITask, &pIUnk );

	if ( SUCCEEDED(hr) )
	{
		bFound = true;
	}


	if ( pISched != NULL )
		pISched->Release();

	if ( pIUnk != NULL )
		pIUnk->Release();

	return bFound;
}


//////////////////////////////////////////////////////////////////////////
//
// Function:    GetDayOfWeekFlag()
//
// Description:
//  Calculates the day of the week that the given date falls on, and returns
//  a TASK_* flag for use in the scheduler WEEKLY struct.
//
// Input:
//  time: [in] The date to operate on.
//
// Returns:
//  A TASK_* flag that corresponds to the day of the week that the date
//  falls on.
//
//////////////////////////////////////////////////////////////////////////

WORD CScheduledTask::GetDayOfWeekFlag ( const CTime& time ) const
{
static WORD s_wDayFlags[] = { 0, TASK_SUNDAY, TASK_MONDAY, TASK_TUESDAY,
                              TASK_WEDNESDAY, TASK_THURSDAY, TASK_FRIDAY,
                              TASK_SATURDAY };
WORD wRet = 0;
int  nDayOfWeek = time.GetDayOfWeek();

    ASSERT ( nDayOfWeek >= 1  &&  nDayOfWeek <= 7 );

    wRet = s_wDayFlags [ nDayOfWeek ];

    ASSERT ( wRet != 0 );

    return wRet;
}
