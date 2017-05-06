/*
 *    Copyright (C) 1987, 1988 Chuck Simmons
 * 
 * See the file COPYING, distributed with empire, for restriction
 * and warranty information.
 */

/*
Static data.

One of our hopes is that changing the types of pieces that
exist in the game is mostly a matter of modifying this file.
However, see also the help routine, empire.h, and empire.6.
*/

#include "empire.h"

/*
Piece attributes.  Notice that the Transport is allowed only one hit.
In the previous version of this game, the user could easily win simply
by building armies and troop transports.  We attempt to alleviate this
problem by making transports far more fragile.  We have also increased
the range of a fighter from 20 to 30 so that fighters will be somewhat
more useful.
*/

piece_attr_t piece_attr[] = {
	{'A', /* character for printing piece */
	 "地上部隊", /* name of piece */ 
	 "地上部隊", /* nickname */
	 "地上部隊", /* name with preceding article */
	 "地上部隊", /* plural */
	 "+", /* terrain */
	 5, /* units to build */
	 1, /* strength */
	 1, /* max hits */
	 1, /* movement */
	 0, /* capacity */
	 INFINITY}, /* range */

	/*
	 For fighters, the range is set to an even multiple of the speed.
	 This allows user to move fighter, say, two turns out and two
	 turns back.
	*/
	 
	{'F', "戦闘機", "戦闘機", "戦闘機", "戦闘機",
		".+", 10, 1,  1, 8, 0, 32},

	{'P', "哨戒艇", "哨戒艇", "哨戒艇", "哨戒艇",
		".",  15, 1,  1, 4, 0, INFINITY},
		
	{'D', "駆逐艦", "駆逐艦", "駆逐艦", "駆逐艦",
		".",  20, 1,  3, 2, 0, INFINITY},

	{'S', "潜水艦", "潜水艦", "潜水艦", "潜水艦",
		".",  20, 3,  2, 2, 0, INFINITY},

	{'T', "輸送艦", "輸送艦", "輸送艦", "輸送艦",
		".",  30, 1,  1, 2, 6, INFINITY},

	{'C', "空母", "空母", "空母", "空母",
		".",  30, 1,  8, 2, 8, INFINITY},

	{'B', "戦艦", "戦艦", "戦艦", "戦艦",
		".",  40, 2, 10, 2, 0, INFINITY},
		
	{'Z', "人工衛星", "人工衛星", "人工衛星", "人工衛星",
		".+", 50, 0, 1, 10, 0, 500}
};

/* Direction offsets. */

int dir_offset [] = {-MAP_WIDTH, /* north */
		     -MAP_WIDTH+1, /* northeast */
		     1, /* east */
		     MAP_WIDTH+1, /* southeast */
		     MAP_WIDTH, /* south */
		     MAP_WIDTH-1, /* southwest */
		     -1, /* west */
		     -MAP_WIDTH-1}; /* northwest */

/* Names of movement functions. */

char *func_name[] = {"なし", "ランダム", "防衛", "搭載", "帰還",
			"探索", "load", "攻撃", "load", "修理",
			"搭乗",
			"W", "E", "D", "C", "X", "Z", "A", "Q"};

/* The order in which pieces should be moved. */
int move_order[] = {SATELLITE, TRANSPORT, CARRIER, BATTLESHIP, 
		    PATROL, SUBMARINE, DESTROYER, ARMY, FIGHTER};

/* types of pieces, in declared order */
char type_chars[] = "AFPDSTCBZ";

/* Lists of attackable objects if object is adjacent to moving piece. */

char tt_attack[] = "T";
char army_attack[] = "O*TACFBSDP";
char fighter_attack[] = "TCFBSDPA";
char ship_attack[] = "TCBSDP";

/* Define various types of objectives */

move_info_t tt_explore = { /* water objectives */
	COMP, /* home city */
	" ", /* objectives */
	{1} /* weights */
};
move_info_t tt_load = { /* land objectives */
	COMP, "$",           {1}
};

/*
Rationale for 'tt_unload':

     Any continent with four or more cities is extremely attractive,
and we should grab it quickly.  A continent with three cities is
fairly attractive, but we are willing to go slightly out of our
way to find a better continent.  Similarily for two cities on a
continent.  At one city on a continent, things are looking fairly
unattractive, and we are willing to go quite a bit out of our way
to find a better continent.

     Cities marked with a '0' are on continents where we already
have cities, and these cities will likely fall to our armies anyway,
so we don't need to dump armies on them unless everything else is
real far away.  We would prefer to use unloading transports for
taking cities instead of exploring, but we are willing to explore
if interesting cities are too far away.

     It has been suggested that continents containing one city
are not interesting.  Unfortunately, most of the time what the
computer sees is a single city on a continent next to lots of
unexplored territory.  So it dumps an army on the continent to
explore the continent and the army ends up attacking the city
anyway.  So, we have decided we might as well send the tt to
the city in the first place and increase the speed with which
the computer unloads its tts.
*/

move_info_t tt_unload     = {
	COMP, "9876543210 ", {1, 1, 1, 1, 1, 1, 11, 21, 41, 101, 61}
};

/*
 '$' represents loading tt must be first
 'x' represents tt producing city
 '0' represnets explorable territory
*/
move_info_t army_fight    = { /* land objectives */
	COMP, "O*TA ",       {1, 1, 1, 1, 11}
};
move_info_t army_load     = { /* water objectives */
	COMP, "$x",          {1, W_TT_BUILD}
};

move_info_t fighter_fight = {
	COMP, "TCFBSDPA ",   {1, 1, 5, 5, 5, 5, 5, 5, 9}
};
move_info_t ship_fight    = {
	COMP, "TCBSDP ",     {1, 1, 3, 3, 3, 3, 21}
};
move_info_t ship_repair   = {
	COMP, "X",           {1}
};

move_info_t user_army        = {
	USER, " ",   {1}
};
move_info_t user_army_attack = {
	USER, "*Xa ", {1, 1, 1, 12}
};
move_info_t user_fighter     = {
	USER, " ",   {1}
};
move_info_t user_ship        = {
	USER, " ",   {1}
};
move_info_t user_ship_repair = {
	USER, "O",   {1}
};

/*
Various help texts.
*/

char *help_cmd[] = {
	"コマンドモード",
	"Auto:     移動モードに移行する",
	"City:     コンピュータに都市を与える",
	"Date:     ラウンドを表示する",
	"Examine:  マップを調査する",
	"File:     マップを表示する",
	"Give:     コンピュータに手番を渡す",
	"Help:     このテキストを表示する",
	"J:        エディットモードに移行する",
	"Move:     自分のユニットを動かす",
	"N:        コンピュータにN手渡す",
	"Print:    セクターを表示する",
	"Quit:     ゲームを終了する",
	"Restore:  ゲームを復帰させる",
	"Save:     ゲームを保存する",
	"Trace:    empmovie.datに棋譜を保存する",
	"Watch:    棋譜を再生する",
	"Zoom:     縮小されたマップを見る",
	"<ctrl-L>: 画面を再描画する"
};
int cmd_lines = 19;

char *help_user[] = {
	"移動モード",
	"QWE",
	"A D       移動方向",
	"ZXC",
	"<space>:  何もしない",
	"Build:    都市の生産を変える",
	"Fill:     任務を搭載にする",
	"Grope:    任務を探索にする",
	"Help:     このテキストを表示する",
	"I <方向>: 任務を指定方向への移動にする",
	"J:        エディットモードに移行する",
	"Kill:     任務を解除する",
	"Land:     任務を着地にする",
	"Out:      移動モードを終了する",
	"Print:    画面を表示する",
	"Random:   任務をランダムにする",
	"Sentry:   任務を防衛にする",
	"Transport:任務を搭乗にする",
	"Upgrade:  任務を修理にする",
	"V <piece> <func>: 都市の任務を設定する",
	"Y:        任務を攻撃にする",
	"<ctrl-L>: 画面を再描画する",
	"?:        ユニットの情報を表示する"
};
int user_lines = 22;
	
char *help_edit[] = {
	"エディットモード",
	"QWE",
	"A D       移動方向",
	"ZXC",
	"Build:    都市の生産を変える",
	"Fill:     任務を搭載にする",
	"Grope:    任務を探索にする",
	"Help:     このテキストを表示する",
	"I <方向>: 任務を指定方向への移動にする",
	"Kill:     任務を解除する",
	"Land:     任務を着地にする",
	"Mark:     ユニットをマークする",
	"N:        マークしたユニットの移動先",
	"Out:      エディットモードを終了する",
	"Print:    セクターを表示する",
	"Random:   任務をランダムにする",
	"Sentry:   任務を防衛にする",
	"Upgrade:  任務を修理にする",
	"V <ユニット> <任務>: 都市の任務を設定",
	"Y:        任務を攻撃にする",
	"<ctrl-L>: 画面を再描画する",
	"?:        ユニットの情報を表示する"
};
int edit_lines = 22;
