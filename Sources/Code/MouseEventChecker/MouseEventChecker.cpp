
#include <MSFS/MSFS.h>
#include <MSFS/MSFS_Render.h>
#include <MSFS/Render/nanovg.h>
#include <MSFS/Legacy/gauges.h>
#include <SimConnect.h>

#include <map>
#include <deque>
#include <cstring>
#include <string>
#include <sstream>
#include <fstream>

#define RED_BUTTON_ACTION_EVENTID 0x11043
#define GREEN_BUTTON_ACTION_EVENTID 0x11044

int g_Time = 0;
int g_Font = -1;
std::map<FsContext, NVGcontext*> g_GaugeNVGcontext;
const int cMaxLogLength = 25;
std::deque<std::string> g_EventLog;
char g_OutputStr[1000] = "";

HRESULT hr;
HANDLE hSimConnect = NULL;
void CALLBACK SimConnectDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext);

void MyEventHandler(ID32 event, UINT32 evdata, PVOID userdata)
{
	std::ostringstream strMsg;
	switch (event)
	{
	case GREEN_BUTTON_ACTION_EVENTID:
		strMsg << "T: " << g_Time << " EventHandler GREEN BUTTON " << "data: " << evdata; // Why always evdata == MOUSE_NONE ???
		break;
	case RED_BUTTON_ACTION_EVENTID:
		strMsg << "T: " << g_Time << " EventHandler RED BUTTON " << "data: " << evdata; // Why always evdata == MOUSE_NONE ???
		break;
	default:
		break;
	}
	if (strMsg.str().length() > 0)
		g_EventLog.push_back(strMsg.str());
}

extern "C"
{
	MSFS_CALLBACK bool DISPLAY_gauge_callback(FsContext ctx, int service_id, void* pData)
	{
		switch (service_id)
		{
		case PANEL_SERVICE_PRE_INSTALL:
		{
			sGaugeInstallData* p_install_data = (sGaugeInstallData*)pData;
			register_key_event_handler((GAUGE_KEY_EVENT_HANDLER)MyEventHandler, NULL);
			if (FAILED(SimConnect_Open(&hSimConnect, "MouseEventDisplay", NULL, 0, 0, 0)))
			{
				fprintf(stderr, "SimConnect_Open error.\n");
				return false;
			}
			return true;
		}
		case PANEL_SERVICE_POST_INSTALL:
		{
			NVGparams params;
			params.edgeAntiAlias = true;
			params.userPtr = ctx;
			g_GaugeNVGcontext[ctx] = nvgCreateInternal(&params);
			g_Font = nvgCreateFont(g_GaugeNVGcontext[ctx], "myfont", "./data/Roboto-Regular.ttf");
			if (g_Font == -1)
			{
				fprintf(stderr, "Could not load font Roboto-Regular.ttf.\n");
				return false;
			}
			return true;
		}
		case PANEL_SERVICE_PRE_INITIALIZE:
		{
			g_Time = 0;
			g_EventLog.push_back(std::string("Play with mouse and explore log!"));
			return true;
		}
		case PANEL_SERVICE_POST_INITIALIZE: break;
		case PANEL_SERVICE_PRE_UPDATE:
		{
			hr = SimConnect_CallDispatch(hSimConnect, SimConnectDispatchProc, NULL);
			if (FAILED(hr))
			{
				fprintf(stderr, "Error on call to SimConnect_CallDispatch.");
				return false;
			}

			while (g_EventLog.size() > cMaxLogLength)
			{
				g_EventLog.pop_front();
			}
			strcpy(g_OutputStr, "");
			for (auto it : g_EventLog)
			{
				strcat(g_OutputStr, it.c_str());
				strcat(g_OutputStr, "\n");
			}
			return true;
		}
		case PANEL_SERVICE_POST_UPDATE: break;
		case PANEL_SERVICE_PRE_DRAW:
		{
			sGaugeDrawData* p_draw_data = (sGaugeDrawData*)pData;
			float pxRatio = (float)p_draw_data->fbWidth / (float)p_draw_data->winWidth;
			NVGcontext* nvgctx = g_GaugeNVGcontext[ctx];
			nvgBeginFrame(nvgctx, p_draw_data->winWidth, p_draw_data->winHeight, pxRatio);
			{
				// Black background
				nvgFillColor(nvgctx, nvgRGB(0, 0, 0));
				nvgBeginPath(nvgctx);
				nvgRect(nvgctx, 0, 0, p_draw_data->winWidth, p_draw_data->winHeight);
				nvgFill(nvgctx);

				nvgFillColor(nvgctx, nvgRGB(255, 255, 255));
				nvgFontFaceId(nvgctx, g_Font);
				nvgFontSize(nvgctx, 20.f);
				nvgTextAlign(nvgctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
				nvgTextBox(nvgctx, 0, 20.f, 0.8f * p_draw_data->winWidth, (const char*)g_OutputStr, nullptr);

			}
			nvgEndFrame(nvgctx);
			g_Time = (int)(p_draw_data->t);

			return true;
		}
		case PANEL_SERVICE_POST_DRAW: break;
		case PANEL_SERVICE_PRE_KILL:
		{
			NVGcontext* nvgctx = g_GaugeNVGcontext[ctx];
			nvgDeleteInternal(nvgctx);
			g_GaugeNVGcontext.erase(ctx);
			SimConnect_Close(hSimConnect);
			unregister_key_event_handler((GAUGE_KEY_EVENT_HANDLER)MyEventHandler, NULL);
			return true;
		}
		case PANEL_SERVICE_POST_KILL: break;
		}
		return false;
	}

	MSFS_CALLBACK void DISPLAY_mouse_callback(float fX, float fY, unsigned int iFlags)
	{
		std::ostringstream strMsg;
		if (iFlags == MOUSE_MOVE)
			return; // It works, just ignore for log
		
		strMsg << "T: " << g_Time << " WASM mouse callback ";

		
		switch (iFlags)
		{
		case MOUSE_LEFTSINGLE:
			strMsg << "LEFTSINGLE";
			break;
		case MOUSE_LEFTDRAG:
			strMsg << "LEFTDRAG";
			break;
		case MOUSE_RIGHTDRAG:
			strMsg << "RIGHTDRAG";
			break;
		case MOUSE_MIDDLEDRAG:
			strMsg << "MIDDLEDRAG";
			break;
		case MOUSE_LEFTRELEASE:
			strMsg << "LEFTRELEASE";
			break;
		case MOUSE_RIGHTRELEASE:
			strMsg << "RIGHTRELEASE";
			break;
		case MOUSE_MIDDLERELEASE:
			strMsg << "MIDDLERELEASE";
			break;
		case MOUSE_RIGHTSINGLE:
			strMsg << "RIGHTSINGLE";
			break;
		case MOUSE_MIDDLESINGLE:
			strMsg << "MIDDLESINGLE";
			break;
		case MOUSE_LEFTDOUBLE:
			strMsg << "LEFTDOUBLE";
			break;
		case MOUSE_MIDDLEDOUBLE:
			strMsg << "MIDDLEDOUBLE";
			break;
		case MOUSE_WHEEL_UP:
			strMsg << "MOUSE_WHEEL_UP";
			break;
		case MOUSE_WHEEL_DOWN:
			strMsg << "MOUSE_WHEEL_DOWN";
			break;
		case MOUSE_NONE:
			strMsg << "NONE";
			break;
		}
		
		strMsg << " x: " << fX << " y: " << fY;
		g_EventLog.push_back(strMsg.str());
		
	}
}

std::map<SIMCONNECT_RECV_ID, std::string> cSimConnect_Recv_ID_description = {
	{SIMCONNECT_RECV_ID_NULL, "SIMCONNECT_RECV_ID_NULL"},
	{SIMCONNECT_RECV_ID_EXCEPTION, "SIMCONNECT_RECV_ID_EXCEPTION"},
	{SIMCONNECT_RECV_ID_OPEN, "SIMCONNECT_RECV_ID_OPEN"},
	{SIMCONNECT_RECV_ID_QUIT, "SIMCONNECT_RECV_ID_QUIT"},
	{SIMCONNECT_RECV_ID_EVENT, "SIMCONNECT_RECV_ID_EVENT"},
	{SIMCONNECT_RECV_ID_EVENT_OBJECT_ADDREMOVE, "SIMCONNECT_RECV_ID_EVENT_OBJECT_ADDREMOVE"},
	{SIMCONNECT_RECV_ID_EVENT_FILENAME, "SIMCONNECT_RECV_ID_EVENT_FILENAME"},
	{SIMCONNECT_RECV_ID_EVENT_FRAME, "SIMCONNECT_RECV_ID_EVENT_FRAME"},
	{SIMCONNECT_RECV_ID_SIMOBJECT_DATA, "SIMCONNECT_RECV_ID_SIMOBJECT_DATA"},
	{SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE, "SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE"},
	{SIMCONNECT_RECV_ID_WEATHER_OBSERVATION, "SIMCONNECT_RECV_ID_WEATHER_OBSERVATION"},
	{SIMCONNECT_RECV_ID_CLOUD_STATE, "SIMCONNECT_RECV_ID_CLOUD_STATE"},
	{SIMCONNECT_RECV_ID_ASSIGNED_OBJECT_ID, "SIMCONNECT_RECV_ID_ASSIGNED_OBJECT_ID"},
	{SIMCONNECT_RECV_ID_RESERVED_KEY, "SIMCONNECT_RECV_ID_RESERVED_KEY"},
	{SIMCONNECT_RECV_ID_CUSTOM_ACTION, "SIMCONNECT_RECV_ID_CUSTOM_ACTION"},
	{SIMCONNECT_RECV_ID_SYSTEM_STATE, "SIMCONNECT_RECV_ID_SYSTEM_STATE"},
	{SIMCONNECT_RECV_ID_CLIENT_DATA, "SIMCONNECT_RECV_ID_CLIENT_DATA"},
	{SIMCONNECT_RECV_ID_EVENT_WEATHER_MODE, "SIMCONNECT_RECV_ID_EVENT_WEATHER_MODE"},
	{SIMCONNECT_RECV_ID_AIRPORT_LIST, "SIMCONNECT_RECV_ID_AIRPORT_LIST"},
	{SIMCONNECT_RECV_ID_VOR_LIST, "SIMCONNECT_RECV_ID_VOR_LIST"},
	{SIMCONNECT_RECV_ID_NDB_LIST, "SIMCONNECT_RECV_ID_NDB_LIST"},
	{SIMCONNECT_RECV_ID_WAYPOINT_LIST, "SIMCONNECT_RECV_ID_WAYPOINT_LIST"},
	{SIMCONNECT_RECV_ID_EVENT_MULTIPLAYER_SERVER_STARTED, "SIMCONNECT_RECV_ID_EVENT_MULTIPLAYER_SERVER_STARTED"},
	{SIMCONNECT_RECV_ID_EVENT_MULTIPLAYER_CLIENT_STARTED, "SIMCONNECT_RECV_ID_EVENT_MULTIPLAYER_CLIENT_STARTED"},
	{SIMCONNECT_RECV_ID_EVENT_MULTIPLAYER_SESSION_ENDED, "SIMCONNECT_RECV_ID_EVENT_MULTIPLAYER_SESSION_ENDED"},
	{SIMCONNECT_RECV_ID_EVENT_RACE_END, "SIMCONNECT_RECV_ID_EVENT_RACE_END"},
	{SIMCONNECT_RECV_ID_EVENT_RACE_LAP, "SIMCONNECT_RECV_ID_EVENT_RACE_LAP"}
};

enum NOTIFICATION_GROUP_ID
{
	GROUP0,
};

enum CLIENT_EVENT_ID
{
	GREEN_BUTTON_EVENT_ID,
};

void CALLBACK SimConnectDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext)
{
	switch (pData->dwID)
	{
	case SIMCONNECT_RECV_ID_OPEN:
	{
		SIMCONNECT_RECV_OPEN* pRecvOpen = (SIMCONNECT_RECV_OPEN*)pData;
		
		if (FAILED(SimConnect_MapClientEventToSimEvent(hSimConnect, GREEN_BUTTON_EVENT_ID, "#0x11044")))
		{
			fprintf(stderr, "Unable to Map Event with ID #0x11044\n");
			break;
		}
		SimConnect_AddClientEventToNotificationGroup(hSimConnect, GROUP0, GREEN_BUTTON_EVENT_ID);
		SimConnect_SetNotificationGroupPriority(hSimConnect, GROUP0, SIMCONNECT_GROUP_PRIORITY_HIGHEST);

		break;
	}
	case SIMCONNECT_RECV_ID_EVENT:	// ѕолучена событие
	{
		SIMCONNECT_RECV_EVENT* pRecvEvent = (SIMCONNECT_RECV_EVENT*)pData;
		switch (pRecvEvent->uGroupID)
		{
		case GROUP0:
		{
			switch (pRecvEvent->uEventID)
			{
			case GREEN_BUTTON_EVENT_ID:
			{
				std::ostringstream strMsg;
				strMsg << "T: " << g_Time << " SimConnect Green Button Event " << pRecvEvent->dwData;
				g_EventLog.push_back(strMsg.str());
				break;
			}
			}
			break;
		}
		default:
		{
			fprintf(stderr, "Receive Unknown Event. Group %d ID %d\n", pRecvEvent->uGroupID, pRecvEvent->uEventID);
		}
		}
		break;
	}
	case SIMCONNECT_RECV_ID_NULL: // ничего полезного не получили
	{
		break;
	}
	default:
	{
		fprintf(stderr, "Unknown SimConnect message: %s \n", cSimConnect_Recv_ID_description[(SIMCONNECT_RECV_ID)pData->dwID].c_str());
	}
	}
}
