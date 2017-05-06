/*
 *    Copyright (C) 1987, 1988 Chuck Simmons
 * 
 * See the file COPYING, distributed with empire, for restriction
 * and warranty information.
 */

/*
edit.c -- Routines to handle edit mode commands.
*/

#include <stdio.h>
#include <string.h>
#include <curses.h>
#include <ctype.h>
#include "empire.h"
#include "extern.h"

void e_move(loc_t *path_start, loc_t loc);
extern int get_piece_name(void);

void
edit(loc_t edit_cursor)
{
    char e_cursor(loc_t *);
    void e_leave(void), e_print(loc_t *), e_random(loc_t);
    void e_stasis(loc_t), e_end(loc_t *, loc_t, int);
    void e_wake(loc_t), e_sleep(loc_t);
    void e_info(loc_t), e_prod(loc_t), e_help(void), e_explore(loc_t);
    void e_fill(loc_t), e_land(loc_t), e_transport(loc_t);
    void e_city_func(loc_t *, loc_t, int *);
    void e_attack(loc_t), e_repair(loc_t);

    loc_t path_start;
    int path_type;
	
    path_start = -1; /* not building a path yet */
	
    comment ("Edit mode...");

    for (;;) { /* until user gives command to leave */
	char e;
	display_loc_u (edit_cursor); /* position cursor */
	e = e_cursor (&edit_cursor); /* handle cursor movement */

	switch (e) {
	case 'B': /* change city production */
	    e_prod (edit_cursor);
	    break;
	case 'F': /* fill */
	    e_fill (edit_cursor);
	    break;
	case 'G': /* explore */
	    e_explore (edit_cursor);
	    break;
	case 'H': /* help */
	    e_help ();
	    break;
	case 'I': /* directional stasis */
	    e_stasis (edit_cursor);
	    break;
	case 'K': /* wake up anything and everything */
	    e_wake (edit_cursor);
	    break;
	case 'L': /* land plane */
	    e_land (edit_cursor);
	    break;
	case 'M': /* start move to location */
	    path_type = NOPIECE;
	    e_move (&path_start, edit_cursor);
	    break;
	case 'N': /* end move to location */
	    e_end (&path_start, edit_cursor, path_type);
	    break;
	case 'O': /* leave edit mode */
	    e_leave ();
	    return;
	case 'P': /* print new sector */
	    e_print (&edit_cursor);
	    break;
	case 'R': /* make piece move randomly */
	    e_random (edit_cursor);
	    break;
	case 'S': /* sleep */
	    e_sleep (edit_cursor);
	    break;
	case 'T': /* transport army */
	    e_transport (edit_cursor);
	    break;
	case 'U': /* repair ship */
	    e_repair (edit_cursor);
	    break;
	case 'V': /* set city function */
	    e_city_func (&path_start, edit_cursor, &path_type);
	    break;
	case 'Y': /* set army func to attack */
	    e_attack (edit_cursor);
	    break;
	case '?': /* request info */
	    e_info (edit_cursor);
	    break;
	case '\014': /* control-L */
	    redraw ();
	    break;
	default: /* bad command? */
	    huh ();
	    break;
	}
    }
}

/*
Get the next command.  We handle cursor movement here.
This routine is an attempt to make cursor movement reasonably
fast.
*/

char
e_cursor(loc_t *edit_cursor)
{
    chtype e;
	
    /* set up terminal */
    (void) crmode ();
    (void) refresh ();
    e = getch ();
    topini (); /* clear any error messages */

    for (;;) {
	int p;
	p = direction (e);
	if (p == -1) break;

	if (!move_cursor (edit_cursor, dir_offset[p]))
	    (void) beep ();
		
	(void) refresh ();
	e = getch ();
    }
    (void) nocrmode (); /* reset terminal */
    return toupper(e);
}

/*
Leave edit mode.
*/

void
e_leave(void)
{
    comment ("Exiting edit mode.");
}

/*
Print new sector.
*/

void
e_print(loc_t *edit_cursor)
{
    int sector;
	
    sector = get_range ("New Sector? ", 0, NUM_SECTORS-1);

    /* position cursor at center of sector */
    *edit_cursor = sector_loc (sector);
    sector_change (); /* allow change of sector */
}

/*
Set the function of a piece.
*/

void
e_set_func(loc_t loc, long func)
{
    piece_info_t *obj;
    obj = find_obj_at_loc (loc);
    if (obj != NULL && obj->owner == USER) {
	obj->func = func;
	return;
    }
    huh (); /* no object here */
}
	
/* Set the function of a city for some piece. */

void
e_set_city_func(city_info_t *cityp, int type, long func)
{
    cityp->func[type] = func;
}

/*
Set a piece to move randomly.
*/

void
e_random(loc_t loc)
{
    e_set_func (loc, RANDOM);
}

void
e_city_random(city_info_t *cityp, int type)
{
    e_set_city_func (cityp, type, RANDOM);
}

/*
Put a ship in fill mode.
*/

void
e_fill(loc_t loc)
{
    if (user_map[loc].contents == 'T' || user_map[loc].contents == 'C')
	e_set_func (loc, FILL);
    else huh ();
}

void
e_city_fill(city_info_t *cityp, int type)
{
    if (type == TRANSPORT || type == CARRIER)
	e_set_city_func (cityp, type, FILL);
    else huh ();
}

/*
Set a piece to explore.
*/

void
e_explore(loc_t loc)
{
    e_set_func (loc, EXPLORE);
}

void
e_city_explore(city_info_t *cityp, loc_t type)
{
    e_set_city_func (cityp, type, EXPLORE);
}

/*
Set a fighter to land.
*/

void
e_land(loc_t loc)
{
    if (user_map[loc].contents == 'F')
	e_set_func (loc, LAND);
    else huh ();
}

/*
Set an army's function to TRANSPORT.
*/

void
e_transport(loc_t loc)
{
    if (user_map[loc].contents == 'A')
	e_set_func (loc, WFTRANSPORT);
    else huh ();
}

/*
Set an army's function to ATTACK.
*/

void
e_attack(loc_t loc)
{
    if (user_map[loc].contents == 'A')
	e_set_func (loc, ARMYATTACK);
    else huh ();
}

void
e_city_attack(city_info_t *cityp, int type)
{
    if (type == ARMY)
	e_set_city_func (cityp, type, ARMYATTACK);
    else huh ();
}

/*
Set a ship's function to REPAIR.
*/

void
e_repair(loc_t loc)
{
    if (strchr ("PDSTBC", user_map[loc].contents))
	e_set_func (loc, REPAIR);
    else huh ();
}

void
e_city_repair(city_info_t *cityp, int type)
{
	if (type == ARMY || type == FIGHTER || type == SATELLITE)
		huh ();
	else e_set_city_func (cityp, type, REPAIR);
}

/*
Set object to move in a direction.
*/

static char dirs[] = "WEDCXZAQ";
 
void
e_stasis(loc_t loc)
{
    if (!isupper (user_map[loc].contents))
	huh (); /* no object here */
    else if (user_map[loc].contents == 'X')
	huh ();
    else {
	char e = get_chx(); /* get a direction */
	char *p = strchr (dirs, e);

	if (p == NULL)
	    huh ();
	else
	    e_set_func (loc, (long)(MOVE_N - (p - dirs)));
    }
}

void
e_city_stasis(city_info_t *cityp, int type)
{
    char e;
    char *p;
	
    e = get_chx(); /* get a direction */
    p = strchr (dirs, e);

    if (p == NULL)
	huh ();
    else
	e_set_city_func (cityp, type, (long)(MOVE_N - (p - dirs)));
}

/*
Wake up anything and everything.
*/

void
e_wake(loc_t loc)
{
    city_info_t *cityp;
    piece_info_t *obj;

    cityp = find_city (loc);
    if (cityp != NULL) {
	int i;
	for (i = 0; i < NUM_OBJECTS; i++)
	    cityp->func[i] = NOFUNC;
    }
    for (obj = map[loc].objp; obj != NULL; obj = obj->loc_link.next)
	obj->func = NOFUNC;
}

void
e_city_wake(city_info_t *cityp, int type)
{
    e_set_city_func (cityp, type, NOFUNC);
}

/*
Set a city's function.  We get the piece type to set, then
the function itself.
*/

void
e_city_func(loc_t *path_start, loc_t loc, int *path_type)
{
    int type;
    char e;
    city_info_t *cityp;

    cityp = find_city (loc);
    if (!cityp || cityp->owner != USER) {
	huh ();
	return;
    }

    type = get_piece_name();
    if (type == NOPIECE) {
	huh ();
	return;
    }
	
    e = get_chx ();
	
    switch (e) {
    case 'F': /* fill */
	e_city_fill (cityp, type);
	break;
    case 'G': /* explore */
	e_city_explore (cityp, type);
	break;
    case 'I': /* directional stasis */
	e_city_stasis (cityp, type);
	break;
    case 'K': /* turn off function */
	e_city_wake (cityp, type);
	break;
    case 'M': /* start move to location */
	*path_type = type;
	e_move (path_start, loc);
	break;
    case 'R': /* make piece move randomly */
	e_city_random (cityp, type);
	break;
    case 'U': /* repair ship */
	e_city_repair (cityp, type);
	break;
    case 'Y': /* set army func to attack */
	e_city_attack (cityp, type);
	break;
    default: /* bad command? */
	huh ();
	break;
    }
}

/*
Beginning of move to location.
*/

void
e_move(loc_t *path_start, loc_t loc)
{
    if (!isupper(user_map[loc].contents)) huh (); /* nothing there? */
    else if (user_map[loc].contents == 'X') huh (); /* enemy city? */
    else *path_start = loc;
}

/*
End of move to location.
*/

void
e_end(loc_t *path_start, loc_t loc, int path_type)
{	
    if (*path_start == -1)
	huh (); /* no path started? */
    else if (path_type == NOPIECE)
	e_set_func (*path_start, loc);
    else {
	city_info_t *cityp = find_city (*path_start);
	ASSERT (cityp);
	e_set_city_func (cityp, path_type, loc);
    }

    *path_start = -1; /* remember no path in progress */
}

/*
Put a piece to sleep.
*/

void
e_sleep(loc_t loc)
{
    if (user_map[loc].contents == 'O') huh (); /* can't sleep a city */
    else e_set_func (loc, SENTRY);
}

/*
Print out information about a piece.
*/

void
e_info(loc_t edit_cursor)
{
    void e_city_info(loc_t), e_piece_info(loc_t edit_cursor, char ab);

    char ab;

    ab = user_map[edit_cursor].contents;

    if (ab == 'O')
	e_city_info (edit_cursor);
    else if (ab == 'X' && debug)
	e_city_info (edit_cursor);
    else if ((ab >= 'A') && (ab <= 'T'))
	e_piece_info (edit_cursor, ab);
    else if ((ab >= 'a') && (ab <= 't') && (debug))
	e_piece_info (edit_cursor, ab);
    else
	huh ();
}

/*
Print info about a piece.
*/

void
e_piece_info(loc_t edit_cursor, char ab)
{
    piece_info_t *obj;
    int type;
    char *p;

    ab = toupper (ab);
    p = strchr (type_chars, ab);
    type = p - type_chars;

    obj = find_obj (type, edit_cursor);
    ASSERT (obj != NULL);
    describe_obj (obj);
}

/*
Display info on a city.
*/

void
e_city_info(loc_t edit_cursor)
{
    piece_info_t *obj;
    city_info_t *cityp;
    int f, s;
    char func_buf[STRSIZE];
    char temp_buf[STRSIZE];
    char junk_buf2[STRSIZE];

    error (""); /* clear line */

    f = 0; /* no fighters counted yet */
    for (obj = map[edit_cursor].objp; obj != NULL;
	 obj = obj->loc_link.next)
	if (obj->type == FIGHTER) f++;

    s = 0; /* no ships counted yet */
    for (obj = map[edit_cursor].objp; obj != NULL;
	 obj = obj->loc_link.next)
	if (obj->type >= DESTROYER) s++;

    if (f == 1 && s == 1) 
	(void) sprintf (jnkbuf, "1 fighter landed, 1 ship docked");
    else if (f == 1)
	(void) sprintf (jnkbuf, "1 fighter landed, %d ships docked", s);
    else if (s == 1)
	(void) sprintf (jnkbuf, "%d fighters landed, 1 ship docked", f);
    else
	(void) sprintf (jnkbuf, "%d fighters landed, %d ships docked", f, s);

    cityp = find_city (edit_cursor);
    ASSERT (cityp != NULL);

    *func_buf = 0; /* nothing in buffer */
    for (s = 0; s < NUM_OBJECTS; s++) { /* for each piece */
	if (cityp->func[s] < 0)
	    (void) sprintf (temp_buf, "%c:%s; ",
			    piece_attr[s].sname,
			    func_name[FUNCI(cityp->func[s])]);
	else (void) sprintf (temp_buf, "%c: %d;",
			     piece_attr[s].sname,
			     loc_disp(cityp->func[s]));
		
	(void) strcat (func_buf, temp_buf);
    }

    (void) sprintf (junk_buf2,
		    "City at location %d will complete %s on round %ld",
		    loc_disp(cityp->loc),
		    piece_attr[(int)cityp->prod].article,
		    date + piece_attr[(int)cityp->prod].build_time - cityp->work);

    info (junk_buf2, jnkbuf, func_buf);
}

/*
Change city production.
*/

void
e_prod(loc_t loc)
{
    city_info_t *cityp;
	
    cityp = find_city (loc);

    if (cityp == NULL) huh (); /* no city? */
    else set_prod (cityp);
}

/*
get help
*/

void
e_help(void)
{
    help (help_edit, edit_lines);
    prompt ("Press any key to continue: ");
    (void) get_chx ();
}

/* end */
