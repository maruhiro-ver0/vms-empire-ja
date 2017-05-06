/*
 *    Copyright (C) 1987, 1988 Chuck Simmons
 * 
 * See the file COPYING, distributed with empire, for restriction
 * and warranty information.
 */

/*
attack.c -- handle an attack between two pieces.  We do everything from
fighting it out between the pieces to notifying the user who won and
killing off the losing object.  Somewhere far above, our caller is
responsible for actually removing the object from its list and actually
updating the player's view of the world.

Find object being attacked.  If it is a city, attacker has 50% chance
of taking city.  If successful, give city to attacker.  Otherwise
kill attacking piece.  Tell user who won.

If attacking object is not a city, loop.  On each iteration, select one
piece to throw a blow.  Damage the opponent by the strength of the blow
thrower.  Stop looping when one object has 0 or fewer hits.  Kill off 
the dead object.  Tell user who won and how many hits her piece has left,
if any.
*/

#include "empire.h"
#include "extern.h"

void
attack_city(piece_info_t *att_obj, loc_t loc)
{
    city_info_t *cityp;
    int att_owner, city_owner;

    cityp = find_city (loc);
    ASSERT (cityp);
	
    att_owner = att_obj->owner;
    city_owner = cityp->owner;

    if (irand (2) == 0) { /* attack fails? */
	if (att_owner == USER) {
	    comment ("都市の防衛隊があなたの部隊を退けた。");
	    ksend ("都市の防衛隊があなたの部隊を退けた。\n"); //kermyt
	}
	else if (city_owner == USER) {
	    ksend ("支配下の都市%dが攻撃された。\n",loc_disp(cityp->loc)); //kermyt
	    comment ("支配下の都市%dが攻撃された。",loc_disp(cityp->loc));
	}
	kill_obj (att_obj, loc);
    }
    else { /* attack succeeded */
	kill_city (cityp);
	cityp->owner = att_owner;
	kill_obj (att_obj, loc);

	if (att_owner == USER) {
	    ksend ("都市%dを支配下に置いた!\n",loc_disp(cityp->loc)); //kermyt
	    error ("都市%dを支配下に置いた!",loc_disp(cityp->loc));

	    extra ("占領のため部隊は解散した。");
	    ksend ("占領のため部隊は解散した。\n");
	    set_prod (cityp);
	}
	else if (city_owner == USER) {
	    ksend("都市%dを失った!\n",loc_disp(cityp->loc)); //kermyt
	    comment ("都市%dを失った!",loc_disp(cityp->loc));
	}
    }
    /* let city owner see all results */
    if (city_owner != UNOWNED) scan (MAP(city_owner), loc);
}

/*
Attack a piece other than a city.  The piece could be anyone's.
First we have to figure out what is being attacked.
*/

void
attack_obj(piece_info_t *att_obj, loc_t loc)
{
    void describe(piece_info_t *, piece_info_t *, loc_t);
    void survive(piece_info_t *, loc_t);

    piece_info_t *def_obj; /* defender */
    int owner;

    def_obj = find_obj_at_loc (loc);
    ASSERT (def_obj != NULL); /* can't find object to attack? */
	
    if (def_obj->type == SATELLITE) return; /* can't attack a satellite */

    while (att_obj->hits > 0 && def_obj->hits > 0) {
	if (irand (2) == 0) /* defender hits? */
	    att_obj->hits -= piece_attr[def_obj->type].strength;
	else def_obj->hits -= piece_attr[att_obj->type].strength;
    }

    if (att_obj->hits > 0) { /* attacker won? */
	describe (att_obj, def_obj, loc);
	owner = def_obj->owner;
	kill_obj (def_obj, loc); /* kill loser */
	survive (att_obj, loc); /* move attacker */
    }
    else { /* defender won */
	describe (def_obj, att_obj, loc);
	owner = att_obj->owner;
	kill_obj (att_obj, loc);
	survive (def_obj, loc);
    }
    /* show results to first killed */
    scan (MAP(owner), loc);
}

void
attack(piece_info_t *att_obj, loc_t loc)
{
    if (map[loc].contents == MAP_CITY) /* attacking a city? */
	attack_city (att_obj, loc);
    else attack_obj (att_obj, loc); /* attacking a piece */
}

/*
Here we look to see if any cargo was killed in the attack.  If
a ships contents exceeds its capacity, some of the survivors
fall overboard and drown.  We also move the survivor to the given
location.
*/

void
survive(piece_info_t *obj, loc_t loc)
{
    while (obj_capacity (obj) < obj->count)
	kill_obj (obj->cargo, loc);
		
    move_obj (obj, loc);
}

void
describe(piece_info_t *win_obj, piece_info_t *lose_obj, loc_t loc)
{
    char buf[STRSIZE];
    char buf2[STRSIZE];
	
    *buf = '\0';
    *buf2 = '\0';
	
    if (win_obj->owner != lose_obj->owner) {
	if (win_obj->owner == USER) {
	    int diff;
	    user_score += piece_attr[lose_obj->type].build_time; 
	    ksend ("敵の%s%dを破壊した。\n",piece_attr[lose_obj->type].name,loc_disp(loc)); //kermyt
	    topmsg (1, "敵の%s%dを破壊した。",piece_attr[lose_obj->type].name,loc_disp(loc));
	    ksend ("あなたの%sの残りヒットポイントは%dだ。\n",piece_attr[win_obj->type].name,win_obj->hits); //kermyt
	    topmsg (2, "あなたの%sの残りヒットポイントは%dだ。", piece_attr[win_obj->type].name, win_obj->hits);
				
	    diff = win_obj->count - obj_capacity (win_obj);
	    if (diff > 0) switch (win_obj->cargo->type) {
		case ARMY:
		    ksend("攻撃で%d個の地上部隊が海に落ちておぼれた。\n",diff); //kermyt
		    topmsg (3,"攻撃で%d個の地上部隊が海に落ちておぼれた。",diff);
		    break;
		case FIGHTER:
		    ksend("攻撃で%d機の戦闘機が海に落ちて失われた。\n",diff); //kermyt
		    topmsg (3,"攻撃で%dの戦闘機が海に落ちて失われた。",diff);
		    break;
		}
	}
	else {
	    comp_score += piece_attr[lose_obj->type].build_time;
	    ksend ("あなたの%s%dは破壊された。\n",piece_attr[lose_obj->type].name,loc_disp(loc)); //kermyt
	    topmsg (3, "あなたの%s%dは破壊された。",piece_attr[lose_obj->type].name,loc_disp(loc));
	}
	set_need_delay ();
    }
}

/* end */
