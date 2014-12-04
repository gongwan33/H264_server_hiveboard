/*
 * protocol.h
 *  the wileless commuicate protocol
 *  jgfntu@163.com
 */

/* 
 * operation protocols 
 */

/* user -> camera */
struct login_req {
	/* opcode = 0 */
	u32 ch_num1;			/* ch_num means challenge number */
	u32 ch_num2;
	u32 ch_num3;
	u32 ch_num4;
}__attribute__((packed));

/* camera -> user */
struct login_resp {
	/* opcode = 1 */
	u16 result;				/* 0:ok; 2:connection down */

	/* exist when result =0 */
	u8 camera_ID[16];
	u8 reserve1[4];
	u8 reserve2[4];
	u8 camera_version[4];

	u32 au_num1;			/* au_num means authentication number */
	u32 au_num2;
	u32 au_num3;
	u32 au_num4;

	u32 ch_num1;			/* challenge num - camera to user */
	u32 ch_num2;
	u32 ch_num3;
	u32 ch_num4;
	/* -------------------- */
}__attribute__((packed));

/* user -> camera */
struct verify_req {
	/* opcode = 2 */
	u32 au_num1;			/* authentication num - user to camera */
	u32 au_num2;
	u32 au_num3;
	u32 au_num4;
}__attribute__((packed));

/* camera -> user */
struct verify_resp {
	/* opcode = 3 */
	u16 result;				/* 0:verify correct; 1:incorretc */

	/* exist when result =0 */
	u8  reserve;
	/* -------------------- */
}__attribute__((packed));

/* user <-> camera */
struct keep_alive {
	/* opcode = 255 */
	/* has no text */
}__attribute__((packed));

/* camera <-> user */
struct keep_alive_resp {
	/* opcode = 254 */
	/* has no text */
}__attribute__((packed));
/* -------------------------- */

/* user -> camera */
struct video_start_req {
	/* opcode = 4 */
	u8 reserve;				/* default:1 */
}__attribute__((packed));

/* camera -> user */
struct video_start_resp {
	/* opcode = 5 */
	u16 result;				/* 0:agree; 2:refuse */

	/* exist when result = 0 and first used */
	u32 data_con_ID;
}__attribute__((packed));

/* user -> camera */
struct video_end {
	/* opcode = 6 */
	/* has no text */
}__attribute__((packed));

/* user -> camera */
struct video_framInterval {
	/* opcode = 7 */
	u32 fram_interval;		/* per 10, default:0*/
}__attribute__((packed));

/* -------------------------- */

/* user -> camera */
struct audio_start_req {
	/* opcode = 8 */
	u8 reserve;				/* default:1 */
}__attribute__((packed));

/* camera -> user */
struct audio_start_resp {
	/* opcode = 9 */
	u16 result;				/* 0:agree 2:refuse 7:no support */

	/* exist when result = 0 and first used */
	u32 data_con_ID;
}__attribute__((packed));

/* user -> camera */
struct audio_end {
	/* opcode = 10 */
	/* has no text */
}__attribute__((packed));

/* -------------------------- */

/* user -> camera */
struct talk_start_req {
	/* opcode = 11 */
	u8 buffer_time;			/* >= 1 */
}__attribute__((packed));

/* camera -> user */
struct talk_start_resp {
	/* opcode = 12 */
	u16 result;				/* 0:agree; 2:refuse */

	/* exist when result = 0 and first used */
	u32 data_con_ID;
}__attribute__((packed));

/* user -> camera */
struct talk_end {
	/* opcode = 13 */
	/* has no text */
}__attribute__((packed));

/* camera -> user */
struct talk_end_resp {
	/* opcode = 22 */
	/* has no text */
}__attribute__((packed));

/* -------------------------- */

/* user -> camera */
struct decoder_control_req {
	/* opcode = 14 */
	u8 cmd_code;			/* from 0-255 */
}__attribute__((packed));

/* music_played_over */
struct music_played_over{
	/* opcode = 15 */
}__attribute__((packed));

/* user -> camera */
struct camera_params_fetch_req {
	/* opcode = 16 */
	/* has no text */
}__attribute__((packed));

/* camera -> user */
struct camera_params_fetch_resp {
	/* opcode = 17 */
	u8 resolution;			/* 2:160*120 8:320*240 32:640*480 */
	u8 brightness;			/* 0-255 */
	u8 contrast;			/* 0-6 */
	u8 mode;				/* 0:50Hz 1:60Hz 2:outdoor */
	u8 reserve1;
	u8 rotate;				/* 0:original 1:vertical 2:horizontal 3:vertical + horizontal */
	u8 reserve2;
	u8 reserve3;
}__attribute__((packed));

/* camera -> user */
struct camera_params_changed_notify {
	/* opcode = 18 */
	u8 params_type;			/* 0:resolution 1:brightness 2:contrast 3:mode 4:reserve 5:rotate 6:reserve */
	u8 params;
}__attribute__((packed));

/* user -> camera */
struct camera_params_set_req {
	/* opcode = 19 */
	u8 params_type;			/* 0:resolution 1:brightness 2:contrast 3:mode 4:reserve 5:rotate 6:reserve */
	u8 params;
}__attribute__((packed));

/* -------------------------- */

/* user -> camera */
struct device_control_req {
	/* opcode = 250 */
	u8 params_type;			/* 0:moto1 ... */
	u8 params;
}__attribute__((packed));

/* user -> camera */
struct fetch_battery_power_req {
	/* opcode = 251 */
	/* has no text */
}__attribute__((packed));

/* camera -> user */
struct fetch_battery_power_resp {
	/* opcode = 252 */
	u8 battery;				/* 0-100 */
}__attribute__((packed));

/* camera -> user */
struct alarm_notify {
	/* opcode = 25 */
	u8  alarm_type;			/* 0:alarm stop 1:move detection 2:external alarm 3:step motor stop alarm*/
	u16 reserve1;
	u16 reserve2;
	u16 reserve3;
	u16 reserve4;
}__attribute__((packed));

/* ------------------------------------------------------ */

/*temperature and relative humidity data*/
struct tem_rh_data {
	/* opcode = 21 */
	u8 tem_integer;
	u8 tem_decimal;
	u8 rh;
}__attribute__((packed));

/* TODO */
/*
 *
 *
 *
 *
 *
 *
 *
 *
 */

/* ------------------------------------------------------ */

/* 
 * AVdata protocols 
 */

/* user -> camera */
struct data_login_req {
	/* opcode = 0 */
	u8 data_ID[4];				/* if it was wrong, it will be disconnected */
}__attribute__((packed));

/* camera -> user */
struct video_data {
	/* opcode = 1 */
	u32 time_stamp;				/* (10ms) */
	u32 frame_time;				/* sampling time (s) */
	u8  reserve;
	u32 pic_len;				/* picture length */
	u8  pic_data[0];			/* picture data */
}__attribute__((packed));

/* camera -> user */
struct audio_data {
	/* opcode = 2 */
	u32 time_stamp;				/* (10ms) */
	u32 pac_num;				/* increase from 0 */
	u32 sam_time;				/* sampling time (s) */
	u8  format;					/* 0 : adpcm */
	u32 ado_len;				/* data length is 160 */
	u8  ado_data[0];				/* data text */
	//u16 decode_para1;			/* adpcm decode parameter : sample */
	//u8  decode_para2;			/* adpcm decode parameter : index */
}__attribute__((packed));

/* user -> camera */
struct talk_data {
	/* opcode = 3 */
	u32 time_stamp;				/* (10ms) */
	u32 pac_num;				/* increase from 0 */
	u32 sam_time;				/* sampling time (s) */
	u8  format;					/* 0 : adpcm */
	u32 talk_len;				/* data length is 160 */
	u8  talk_data[0];				/* data text */
}__attribute__((packed));


/* ------------------------------------------------------ */

union context {
	/* control protocols */
	struct login_req login_req;
	struct login_resp login_resp;
	struct verify_req verify_req;
	struct verify_resp verify_resp;
	struct audio_start_resp audio_start_resp;
    struct keep_alive_resp keep_alive_resp;
	struct video_start_resp video_start_resp;
    struct video_framInterval video_framInterval;
	struct talk_start_resp talk_start_resp;
	struct talk_end_resp talk_end_resp;
	struct fetch_battery_power_resp fetch_battery_power_resp;
    struct alarm_notify alarm_notify;
	struct tem_rh_data tem_rh_data;
	struct music_played_over music_played_over;
	/* AVdata protocols */
	struct video_data video_data;
	struct audio_data audio_data;
}__attribute__((packed));


/*
 * command format 
 */
struct command {
	u8  protocol_head[4];	/* MO_O : MO_V */
	u16 opcode;				/* diff opprotols has diff opcode */
	u8  reserve1;			/* default:0 */
	u8  reserve2[8];
	u32 text_len;			/* protocol texts length*/
	u32 reserve3;
	union context text[0];	/* protocol texts */
}__attribute__((packed));

