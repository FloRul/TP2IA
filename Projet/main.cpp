#pragma warning (disable:4786)

#include <windows.h>
#include <time.h>
#include <string.h>

#include "constants.h"
#include "misc/utils.h"
#include "time/PrecisionTimer.h"
#include "GameWorld.h"
#include "misc/Cgdi.h"
#include "ParamLoader.h"
#include "resource.h"
#include "misc/WindowUtils.h"

#define DB_OK 1

//--------------------------------- Globals ------------------------------
//
//------------------------------------------------------------------------

char* g_szApplicationName = "Steering Behaviors - Another Big Shoal";
char*	g_szWindowClassName = "MyWindowClass";

GameWorld* g_GameWorld;

HINSTANCE hinst;

UINT nb_leader;
UINT agent_humain;
UINT comportement;
UINT nb_agent;
UINT nb_poursuiveur1;
UINT nb_poursuiveur2;
UINT nb_poursuiveur3;
UINT offset;

BOOL APIENTRY Dialog1Proc(HWND, UINT, WPARAM, LPARAM);

//---------------------------- WindowProc ---------------------------------
//	
//	This is the callback function which handles all the windows messages
//-------------------------------------------------------------------------

LRESULT CALLBACK WindowProc (HWND   hwnd,
                             UINT   msg,
                             WPARAM wParam,
                             LPARAM lParam)
{
   //these hold the dimensions of the client window area
	 static int cxClient, cyClient; 

	 //used to create the back buffer
   static HDC		hdcBackBuffer;
   static HBITMAP	hBitmap;
   static HBITMAP	hOldBitmap;

    switch (msg)
    {
	
		//A WM_CREATE msg is sent when your application window is first
		//created
    case WM_CREATE:
      {
		// initial parameters
		int nb_leader = 0;
		int agent_humain = 0;

		// Ask user to enter informations for the application
		if (DialogBox(hinst, "DIALOG1", hwnd, (DLGPROC)Dialog1Proc) == DB_OK)
			InvalidateRect(hwnd, NULL, TRUE);

         //to get get the size of the client window first we need  to create
         //a RECT and then ask Windows to fill in our RECT structure with
         //the client window size. Then we assign to cxClient and cyClient 
         //accordingly
			   RECT rect;

			   GetClientRect(hwnd, &rect);

			   cxClient = rect.right;
			   cyClient = rect.bottom;

         //seed random number generator
         srand((unsigned) time(NULL));  

         
         //---------------create a surface to render to(backbuffer)

         //create a memory device context
         hdcBackBuffer = CreateCompatibleDC(NULL);

         //get the DC for the front buffer
         HDC hdc = GetDC(hwnd);

         hBitmap = CreateCompatibleBitmap(hdc,
                                          cxClient,
                                          cyClient);

			  
         //select the bitmap into the memory device context
		 hOldBitmap = (HBITMAP)SelectObject(hdcBackBuffer, hBitmap);

         //don't forget to release the DC
         ReleaseDC(hwnd, hdc); 
		 if (comportement == 0)
		 {
			 g_GameWorld = new GameWorld(cxClient, cyClient, 0, agent_humain, 0, nb_agent, nb_poursuiveur1, nb_poursuiveur2, nb_poursuiveur3, offset);
		 }
		 else
		 {
			 g_GameWorld = new GameWorld(cxClient, cyClient, nb_leader, agent_humain,
				 comportement, nb_agent, nb_poursuiveur1, nb_poursuiveur2, nb_poursuiveur3, offset);
		 }
         
         ChangeMenuState(hwnd, IDR_PRIORITIZED, MFS_CHECKED);
         ChangeMenuState(hwnd, ID_VIEW_FPS, MFS_CHECKED);
      }

      break;

    case WM_COMMAND:
    {
      g_GameWorld->HandleMenuItems(wParam, hwnd);
    }

    break;


    case WM_LBUTTONUP:
    {
      g_GameWorld->SetCrosshair(MAKEPOINTS(lParam));
    }
    
    break;

    case WM_KEYUP:
      {
        switch(wParam)
        {
           case VK_ESCAPE:
            {             
              SendMessage(hwnd, WM_DESTROY, NULL, NULL);
            }
          
            break;

          case 'R':
            {
               delete g_GameWorld;
			   // g_GameWorld = new GameWorld(cxClient, cyClient);
               g_GameWorld = new GameWorld(cxClient, cyClient, nb_leader, agent_humain,
				   comportement, nb_agent, nb_poursuiveur1, nb_poursuiveur2, nb_poursuiveur3, offset);
            }

            break;
           

        }//end switch

        //handle any others
        g_GameWorld->HandleKeyPresses(wParam);
        
      }//end WM_KEYUP

      break;

    
    case WM_PAINT:
      {    
         PAINTSTRUCT ps;
          
         BeginPaint (hwnd, &ps);
        //fill our backbuffer with white
         BitBlt(hdcBackBuffer,
                0,
                0,
                cxClient,
                cyClient,
                NULL,
                NULL,
                NULL,
                WHITENESS);

         gdi->StartDrawing(hdcBackBuffer);
         g_GameWorld->Render();
         gdi->StopDrawing(hdcBackBuffer);

         //now blit backbuffer to front
		 BitBlt(ps.hdc, 0, 0, cxClient, cyClient, hdcBackBuffer, 0, 0, SRCCOPY); 
          
         EndPaint (hwnd, &ps);

      }

      break;


          
		 case WM_DESTROY:
			 {

         //clean up our backbuffer objects
         SelectObject(hdcBackBuffer, hOldBitmap);

         DeleteDC(hdcBackBuffer);
         DeleteObject(hBitmap); 

         
         
         // kill the application, this sends a WM_QUIT message  
				 PostQuitMessage (0);
			 }

       break;

     }//end switch

     //this is where all the messages not specifically handled by our 
		 //winproc are sent to be processed
		 return DefWindowProc (hwnd, msg, wParam, lParam);
}

void LeaderFollowingField(HWND hDlg, bool enable)
{
	// Enabled radio buttons
	HWND buttonNbLeader = GetDlgItem(hDlg, ID_ONE_LEADER);
	EnableWindow(buttonNbLeader, enable);
	buttonNbLeader = GetDlgItem(hDlg, ID_TWO_LEADER);
	EnableWindow(buttonNbLeader, enable);

	// Enable TextFields
	HWND TextFieldNbLeader = GetDlgItem(hDlg, ID_NB_AGENT_FOLLOWER1);
	EnableWindow(TextFieldNbLeader, enable);
	TextFieldNbLeader = GetDlgItem(hDlg, ID_NB_AGENT_FOLLOWER2);
	EnableWindow(TextFieldNbLeader, enable);
	TextFieldNbLeader = GetDlgItem(hDlg, ID_NB_AGENT_FOLLOWER3);
	EnableWindow(TextFieldNbLeader, enable);
}

BOOL APIENTRY Dialog1Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		// Settings of the comboBox leader
		SendDlgItemMessage(hDlg, ID_CB, CB_ADDSTRING, 0, (LONG)"Basique");
		SendDlgItemMessage(hDlg, ID_CB, CB_ADDSTRING, 0, (LONG)"LeaderFollowing");
		SendDlgItemMessage(hDlg, ID_CB, CB_ADDSTRING, 0, (LONG)"FlockingV");
		SendDlgItemMessage(hDlg, ID_CB, CB_ADDSTRING, 0, (LONG)"Zombie Escape");
		SendDlgItemMessage(hDlg, ID_CB, CB_SETCURSEL, comportement, 0);
		
		//SendMessage(hDlg, CB_SHOWDROPDOWN, FALSE, 0);

		// Settings des boutons radio
		CheckDlgButton(hDlg, ID_ONE_LEADER, BST_CHECKED);
		CheckDlgButton(hDlg, ID_NOT_HUMAN_AGENT, BST_INDETERMINATE);

		// Settings default values for int
		SetDlgItemText(hDlg, ID_NB_AGENT, std::to_string(Prm.NumAgents).c_str());
		SetDlgItemText(hDlg, ID_NB_AGENT_FOLLOWER1, "0");
		SetDlgItemText(hDlg, ID_NB_AGENT_FOLLOWER2, "0");
		SetDlgItemText(hDlg, ID_NB_AGENT_FOLLOWER3, "0");
		SetDlgItemText(hDlg, ID_OFFSET, "10");
		return TRUE;
	}
	case WM_COMMAND:

		if (HIWORD(wParam) == CBN_SELCHANGE) {
			// Retrieve the choice of method
			int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL,
				(WPARAM)0, (LPARAM)0);
			TCHAR  ListItem[256];
			(TCHAR)SendMessage((HWND)lParam, (UINT)CB_GETLBTEXT,
				(WPARAM)ItemIndex, (LPARAM)ListItem);
			//MessageBox(hDlg, (LPCWSTR)ListItem, TEXT("Item Selected"), MB_OK);

			if (strcmp(ListItem, "LeaderFollowing") == 0)
				LeaderFollowingField(hDlg, true);
			else
				LeaderFollowingField(hDlg, false);
		}

		if (HIWORD(wParam) == BN_CLICKED) {
			switch (LOWORD(wParam)) {
			case ID_ONE_LEADER:
				nb_leader = 1; break;
			case ID_TWO_LEADER:
				nb_leader = 2; break;
			case ID_HUMAN_AGENT:
				agent_humain = 1; break;
			case ID_NOT_HUMAN_AGENT:
				agent_humain = 0; break;
			}
		}
		if (LOWORD(wParam) == DB_OK || LOWORD(wParam) == IDCANCEL)
		{
			// get the behavior wanted
			comportement = SendDlgItemMessage(hDlg, ID_CB, CB_GETCURSEL, 0, 0);
			// get the number of stadard agent 
			nb_agent = GetDlgItemInt(hDlg, ID_NB_AGENT, NULL, FALSE);
			// get the number of pursuiver for the leader1
			nb_poursuiveur1 = GetDlgItemInt(hDlg, ID_NB_AGENT, NULL, FALSE);
			// get the number of pursuiver for the leader2
			nb_poursuiveur2 = GetDlgItemInt(hDlg, ID_NB_AGENT, NULL, FALSE);
			// get the number of pursuiver for the player
			nb_poursuiveur3 = GetDlgItemInt(hDlg, ID_NB_AGENT, NULL, FALSE);
			// get the offset
			offset = GetDlgItemInt(hDlg, ID_OFFSET, NULL, FALSE);

			EndDialog(hDlg, DB_OK);
			return TRUE;
		}
	default:
		return FALSE;
	}
}

//-------------------------------- WinMain -------------------------------
//
//	The entry point of the windows program
//------------------------------------------------------------------------
int WINAPI WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     szCmdLine, 
                    int       iCmdShow)
{
  //handle to our window
  HWND						hWnd;
    
  //our window class structure
  WNDCLASSEX     winclass;
		 
  // first fill in the window class stucture
  winclass.cbSize        = sizeof(WNDCLASSEX);
  winclass.style         = CS_HREDRAW | CS_VREDRAW;
  winclass.lpfnWndProc   = WindowProc;
  winclass.cbClsExtra    = 0;
  winclass.cbWndExtra    = 0;
  winclass.hInstance     = hInstance;
  winclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
  winclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
  winclass.hbrBackground = NULL;
  winclass.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);
  winclass.lpszClassName = g_szWindowClassName;
  winclass.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

  //register the window class
  if (!RegisterClassEx(&winclass))
  {
    MessageBox(NULL, "Registration Failed!", "Error", 0);

    //exit the application
    return 0;
  }

  //create the window and assign its ID to hwnd    
  hWnd = CreateWindowEx (NULL,                 // extended style
                         g_szWindowClassName,  // window class name
                         g_szApplicationName,  // window caption
                         WS_OVERLAPPED | WS_VISIBLE | WS_CAPTION | WS_SYSMENU,
                         GetSystemMetrics(SM_CXSCREEN)/2 - constWindowWidth/2,
                         GetSystemMetrics(SM_CYSCREEN)/2 - constWindowHeight/2,                    
                         constWindowWidth,     // initial x size
                         constWindowHeight,    // initial y size
                         NULL,                 // parent window handle
                         NULL,                 // window menu handle
                         hInstance,            // program instance handle
                         NULL);                // creation parameters

  //make sure the window creation has gone OK
  if(!hWnd)
  {
    MessageBox(NULL, "CreateWindowEx Failed!", "Error!", 0);
  }

     
  //make the window visible
  ShowWindow (hWnd, iCmdShow);
  UpdateWindow (hWnd);

  // Enter the message loop
  bool bDone = false;

  //create a timer
  PrecisionTimer timer;

  timer.SmoothUpdatesOn();

  //start the timer
  timer.Start();

  MSG msg;

  while(!bDone)
  {		
    while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) 
    {
      if( msg.message == WM_QUIT) 
      {
        //stop loop if it's a quit message
	      bDone = true;
      } 

      else 
      {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
      }
    }

    if (msg.message != WM_QUIT )
    {
      //update
      g_GameWorld->Update(timer.TimeElapsed());
      
      //render
      RedrawWindow(hWnd, false);

      Sleep(2);
    }
   					
  }//end while




  delete g_GameWorld;

  UnregisterClass( g_szWindowClassName, winclass.hInstance );

  return msg.wParam;
}


