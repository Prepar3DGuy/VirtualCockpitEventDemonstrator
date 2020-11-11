
#include <MSFS/MSFS.h>
#include <MSFS/MSFS_Render.h>
#include <MSFS/Render/nanovg.h>
#include <MSFS/Legacy/gauges.h>

#include <map>
#include <deque>
#include <cstring>
#include <string>
#include <sstream>
#include <fstream>

int g_Time = 0;
int g_Font = -1;
std::map<FsContext, NVGcontext*> g_GaugeNVGcontext;
const int cMaxLogLength = 25;
std::deque<std::string> g_EventLog;
char g_OutputStr[1000] = "";

extern "C"
{
	MSFS_CALLBACK bool DISPLAY_gauge_callback(FsContext ctx, int service_id, void* pData)
	{
		switch (service_id)
		{
		case PANEL_SERVICE_PRE_INSTALL:
		{
			sGaugeInstallData* p_install_data = (sGaugeInstallData*)pData;
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
		}
		
		strMsg << " x: " << fX << " y: " << fY;
		g_EventLog.push_back(strMsg.str());
		
	}
}