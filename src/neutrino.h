/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef __neutrino__
#define __neutrino__

#include <configfile.h>

#include <neutrinoMessages.h>
#include "driver/framebuffer.h"
#include "system/setting_helpers.h"
#include "system/configure_network.h"
#include "daemonc/remotecontrol.h"    /* st_rmsg      */
#include "gui/channellist.h"          /* CChannelList */
#include "gui/personalize.h"
#include "gui/rc_lock.h"
#include "gui/user_menue.h"
#include "gui/timerlist.h"
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
#include <gui/psisetup.h>
#include <gui/3dsetup.h>
#endif

#include <string>

#define widest_number "2"

#define ANNOUNCETIME (1 * 60)

/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  main run-class                                             *
*                                                                                     *
**************************************************************************************/

extern const unsigned char genre_sub_classes[];            /* epgview.cpp */
extern const neutrino_locale_t * genre_sub_classes_list[]; /* epgview.cpp */

class CNeutrinoApp : public CMenuTarget, CChangeObserver
{
public:
	enum
	{
		RECORDING_OFF    = 0,
		RECORDING_SERVER = 1,
		RECORDING_VCR    = 2,
		RECORDING_FILE   = 3
	};
	
	

private:
	CFrameBuffer * frameBuffer;

	CConfigFile			configfile;
	CScanSettings			scanSettings;
	CPersonalizeGui			personalize;
	CUserMenu 			usermenu;
	int                             network_dhcp;
	int                             network_automatic_start;

	int				mode;
	int				lastMode;
	bool				softupdate;
//	bool				fromflash;
	bool 				init_cec_setting;
	int				lastChannelMode;
	struct timeval                  standby_pressed_at;

	int				current_muted;

	bool				skipShutdownTimer;
	bool 				skipSleepTimer;
	bool                            lockStandbyCall;
	bool 				pbBlinkChange;
	int tvsort[LIST_MODE_LAST];
	int radiosort[LIST_MODE_LAST];

	CMoviePluginChangeExec 		*MoviePluginChanger;

	void SDT_ReloadChannels();
	void setupNetwork( bool force= false );
	void setupNFS();

	void tvMode( bool rezap = true );
	void radioMode( bool rezap = true );
	void scartMode( bool bOnOff );
	void standbyMode( bool bOnOff, bool fromDeepStandby = false );
	void getAnnounceEpgName(CTimerd::RecordingInfo * eventinfo, std::string &name);

#if HAVE_COOL_HARDWARE
	void ExitRun(const bool write_si = true, int retcode = 0);
#endif
	void RealRun(CMenuWidget &mainSettings);
	void InitZapper();
	void InitTimerdClient();
	void InitZapitClient();
	void InitSectiondClient();

	//menues
	void InitMenu();
 	void InitMenuMain();
	void InitMenuSettings();
	void InitMenuService();

	void SetupFrameBuffer();
	void CmdParser(int argc, char **argv);
	void Cleanup();
	CNeutrinoApp();

public:
	enum
	{
		mode_unknown = -1,
		mode_tv = 1,
		mode_radio = 2,
		mode_scart = 3,
		mode_standby = 4,
		mode_audio = 5,
		mode_pic = 6,
		mode_ts = 7,
		mode_off = 8,
		mode_mask = 0xFF,
		norezap = 0x100
	};

	void saveSetup(const char * fname);
	int loadSetup(const char * fname);
	void loadKeys(const char * fname = NULL);
	void saveKeys(const char * fname = NULL);
	void SetupTiming();
	void SetupFonts();
	void setupRecordingDevice(void);

	~CNeutrinoApp();
	CScanSettings& getScanSettings() {
		return scanSettings;
	};

	CChannelList			*TVchannelList;
	CChannelList			*RADIOchannelList;
	CChannelList			*channelList;
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	CPSISetup			*chPSISetup;
	C3DSetup			*threeDSetup;
#endif

	static CNeutrinoApp* getInstance();

	void channelsInit(bool bOnly = false);
	int run(int argc, char **argv);

	//callback stuff only....
	int exec(CMenuTarget* parent, const std::string & actionKey);

// 	//onchange
 	bool changeNotify(const neutrino_locale_t OptionName, void *);

	int handleMsg(const neutrino_msg_t msg, neutrino_msg_data_t data);

	int getMode() {
		return mode;
	}
	int getLastMode() {
		return lastMode;
	}
	void switchTvRadioMode(const int prev_mode = mode_unknown);
	void switchClockOnOff();
	
	bool isMuted() {return current_muted; }
	void setCurrentMuted(int m) { current_muted = m; }
	int recordingstatus;
	void MakeSectionsdConfig(CSectionsdClient::epg_config& config);
	void SendSectionsdConfig(void);
	int GetChannelMode(void) {
		return lastChannelMode;
	};
	void SetChannelMode(int mode);
	void MarkChannelListChanged(void) { g_channel_list_changed = true; };
	void quickZap(int msg);
	void numericZap(int msg);
	void StopSubtitles();
	void StartSubtitles(bool show = true);
	bool StartPip(const t_channel_id channel_id);
	void SelectSubtitles();
	void showInfo(void);
	CConfigFile* getConfigFile() {return &configfile;};
	bool 		SDTreloadChannels;
	bool 		g_channel_list_changed;
	void saveEpg(bool cvfd_mode);
	void stopDaemonsForFlash();
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	enum {
		SHUTDOWN,
		REBOOT
	};
	void ExitRun(const bool write_si = true, int retcode = SHUTDOWN);
#endif
};
#endif


