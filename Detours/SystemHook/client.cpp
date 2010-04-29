#include "stdafx.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

// Settings common to both executables: determines whether
// we're using TCP/IP or real DDE.
#include "ipcsetup.h"

#include "client.h"


//void MyFrame::OnExecute(wxCommandEvent& WXUNUSED(event))
//{
//    if (m_client->IsConnected())
//    {
//        wxString s = _T("Date");
//
//        m_client->GetConnection()->Execute((wxChar *)s.c_str());
//        m_client->GetConnection()->Execute((wxChar *)s.c_str(), (s.Length() + 1) * sizeof(wxChar));
//        char bytes[3];
//        bytes[0] = '1'; bytes[1] = '2'; bytes[2] = '3';
//        m_client->GetConnection()->Execute((wxChar *)bytes, 3, wxIPC_PRIVATE);
//    }
//}
//
//void MyFrame::OnRequest(wxCommandEvent& WXUNUSED(event))
//{
//    if (m_client->IsConnected())
//    {
//        int size;
//        m_client->GetConnection()->Request(_T("Date"));
//        m_client->GetConnection()->Request(_T("Date+len"), &size);
//        m_client->GetConnection()->Request(_T("bytes[3]"), &size, wxIPC_PRIVATE);
//    }
//}
static MyConnection     *m_connection = NULL;
// ----------------------------------------------------------------------------
// MyClient
// ----------------------------------------------------------------------------
MyClient::MyClient() : wxClient()
{
    //m_connection = NULL;
}

bool MyClient::Connect(const wxString& sHost, const wxString& sService, const wxString& sTopic)
{
    // suppress the log messages from MakeConnection()
    wxLogNull nolog;

    m_connection = (MyConnection *)MakeConnection(sHost, sService, sTopic);

    return m_connection    != NULL;
}

wxConnectionBase *MyClient::OnMakeConnection()
{
    return new MyConnection;
}

void MyClient::Disconnect()
{
    if (m_connection)
    {
        m_connection->Disconnect();
        delete m_connection;
        m_connection = NULL;
    }
}

MyClient::~MyClient()
{
    Disconnect();
}

// ----------------------------------------------------------------------------
// MyConnection
// ----------------------------------------------------------------------------

bool MyConnection::OnDisconnect()
{
    return true;
}

bool MyConnection::Execute(const wxChar *data, int size, wxIPCFormat format)
{
    return wxConnection::Execute(data, size, format);;
}

wxChar *MyConnection::Request(const wxString& item, int *size, wxIPCFormat format)
{
    wxChar *data =  wxConnection::Request(item, size, format);
    return data;
}

bool MyConnection::Poke(const wxString& item, wxChar *data, int size, wxIPCFormat format)
{
	return true;
}

bool MyConnection::OnAdvise(const wxString& topic, const wxString& item, wxChar *data, int size, wxIPCFormat format)
{
	return true;
}

bool MyClient::IsConnected() { return m_connection != NULL; };
MyConnection *MyClient::GetConnection() { return m_connection; };

