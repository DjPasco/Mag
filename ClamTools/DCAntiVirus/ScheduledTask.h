/////////////////////////////////////////////////////////////////////////////
//
// ScheduledTask.h: interface for the CScheduledTask class.
//
/////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCHEDULEDTASK_H__A00D015F_9061_11D2_8D3B_FC17F33B2809__INCLUDED_)
#define AFX_SCHEDULEDTASK_H__A00D015F_9061_11D2_8D3B_FC17F33B2809__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <mstask.h>

class CScheduledTask  
{
// Construction
public:
	CScheduledTask();


// Attributes
    enum ETaskFrequency 
        {
        freqUnset = 0,                  // for debugging - used in asserts
        freqFirst = 0,                  // for debugging
        freqOnce,
        freqDaily,
        freqWeekly,
        freqMonthly,
        freqLast = freqMonthly          // for debugging
        };

    // Accessor functions.  End-of-line comments are for the VC 6 popup
    // list tooltip thingies.

    BOOL  GetStartDateTime ( CTime& ) const;        // get the task's starting date and time
    void  SetStartDateTime ( const CTime& );        // set the task's starting date and time
    void  SetStartDateTime ( const SYSTEMTIME& );   // set the task's starting date and time

    BOOL  GetEndDate ( CTime& ) const;              // get the task's ending date
    void  SetEndDate ( const CTime& );              // set the task's ending date
    void  SetEndDate ( const SYSTEMTIME& );         // set the task's ending date

    ETaskFrequency GetFrequency() const;            // get the task's frequency
    void           SetFrequency ( ETaskFrequency ); // set the task's frequency
    
    CString GetProgram() const;                     // get the program to be run
    void    SetProgram ( LPCTSTR szProgram );       // set the program to be run

    CString GetParameters() const;                  // get the parameters for the program
    void    SetParameters ( LPCTSTR szParams );     // set the parameters for the program

    CString GetStartingDir() const;                 // get the program's starting directory
    void    SetStartingDir ( LPCTSTR szDir );       // set the program's starting directory

    CString GetComment() const;                     // get the comment displayed in the scheduler UI
    void    SetComment ( LPCTSTR szComment );       // set the comment displayed in the scheduler UI

    // Variables
protected:
    SYSTEMTIME m_timeStart;
    SYSTEMTIME m_timeEnd;

    ETaskFrequency m_eFreq;

    CString    m_sProgramPath;
    CString    m_sParameters;
    CString    m_sStartingDir;
    CString    m_sComment;


// Operations
public:
    void Reset();   // reset all fields to zero

    HRESULT SaveTask ( LPCTSTR szTaskName, 
                       BOOL bFailIfExists = FALSE ) const;  // save the task to the scheduler
    
    static HRESULT DeleteTask ( LPCTSTR szTaskName );   // delete a task from the scheduler

	static bool CheckTask(LPCTSTR szTaskName);


// Implementation
public:
	virtual ~CScheduledTask();

protected:
    WORD GetDayOfWeekFlag ( const CTime& ) const;
};

#endif // !defined(AFX_SCHEDULEDTASK_H__A00D015F_9061_11D2_8D3B_FC17F33B2809__INCLUDED_)
