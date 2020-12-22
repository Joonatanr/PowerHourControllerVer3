/*
 * clockDisplay.c
 *
 *  Created on: Aug 27, 2017
 *      Author: Joonatan
 */


#include <LOGIC/PowerHourGame/clockDisplay.h>
#include <LOGIC/PowerHourGame/ShotGlassAnimation.h>
#include <LOGIC/PowerHourGame/SpecialTasks.h>
#include "display_drv.h"
#include "buzzer.h"
#include "tumbler.h"
#include "register.h"

#define BEERSHOT_X 86
#define BEERSHOT_Y 1


//Text box definitions.
#define UPPER_TEXT_XLOC 2u
#define UPPER_TEXT_YLOC 2u
#define UPPER_TEXT_HEIGHT 13u
#define UPPER_TEXT_WIDTH 80u
#define UPPER_TEXT_FONT FONT_MEDIUM_FONT

#define LOWER_TEXT_XLOC 2u
#define LOWER_TEXT_YLOC 51u
#define LOWER_TEXT_HEIGHT 13u
#define LOWER_TEXT_WIDTH 80u
#define LOWER_TEXT_FONT FONT_MEDIUM_FONT

#define CLOCK_FONT FONT_NUMBERS_HUGE

/*****************************************************************************************************
 *
 * Private type definitions
 *
 *****************************************************************************************************/

typedef enum
{
    CONTROLLER_INIT,
    CONTROLLER_COUNTING,
    CONTROLLER_OVERRIDDEN,
    CONTROLLER_FINAL,
    CONTROLLER_NUMBER_OF_STATES
} controllerState;

typedef enum
{
    BEERSHOT_NO_ACTION,         //No action with bitmap.
    BEERSHOT_EMPTY,             //Draw initial beershot bitmap.
    BEERSHOT_BEGIN_FILLING,     //Initiate filling animation.
    BEERSHOT_FULL,              //Draw full beershot.
    BEERSHOT_BEGIN_EMPTYING,    //Initiate emptying animation.

    OVERRIDE_FUNCTION,          //Overrides ordinary function, displays bitmap instead.
    RESTORE_FUNCTION,           //Restore normal handling.
} beerShotAction;

typedef enum
{
    BEERSHOT_FROZEN,    //Nothing currently to draw.
    BEERSHOT_FILLING,
    BEERSHOT_EMPTYING
} beershotState;


typedef struct
{
    U8 second;                      // When the event should be triggered.
    const char * upperText;         // Text to be written to upper part of display.
    const char * lowerText;         // Text to be written to lower part of display.
    beerShotAction shot_action;     // Action to be performed on bitmap.
    OverrideFunc    func;           // Optional parameter - This function will override all behaviour, used for special actions.
} ControllerEvent;

typedef enum
{
    TASK_CATEGORY_GIRLS,
    TASK_CATEGORY_GUYS,
    TASK_CATEGORY_BOARD,
    TASK_CATEGORY_CORETEAM,
    TASK_CATEGORY_ALUMNI,
    TASK_CATEGORY_PAX,
    TASK_CATEGORY_KT,
    TASK_CATEGORY_SOC,
    NUMBER_OF_TASK_CATEGORIES
} SchedulerTaskCategory;

typedef struct
{
    const ControllerEvent * event_array;
    U8                        event_cnt;
    Tumbler_T             enable_tumblr;
} SchedulerTaskConf_T;

typedef struct
{
    U16 counter;
    Boolean is_enabled;
} SchedulerTaskState_T;

//Just an idea.
typedef struct
{
    const Bitmap * bmp_ptr;
    U8 bmp_x;
    U8 bmp_y;

    const char * text_str;
    U8 text_x;
    U8 text_y;
    FontType text_font;
    Boolean isInverted;
} IntroSequence;


/*****************************************************************************************************
 *
 * Private function Prototypes
 *
 *****************************************************************************************************/

Private void incrementTimer(void);
Private void convertTimerString(timekeeper_struct * t, char * dest_str);
Private void drawBeerShot(beerShotAction action);
Private Boolean girlsSpecialIntro(U8 sec);
Private Boolean guysSpecialIntro(U8 sec);

Private void doFinalAction(void);
Private U8 selectRandomTaskIndex(void);

Private timekeeper_struct priv_timekeeper;
Private controllerState priv_timer_state;

Private U8 getScheduledSpecialTask(const ControllerEvent ** event_ptr);
Private void setUpperText(const char * str);
Private void setLowerText(const char * str);

Private void clearUpperText(void);
Private void clearLowerText(void);


Private void drawTimer(void);


/*****************************************************************************************************
 *
 * Private variable declarations.
 *
 *****************************************************************************************************/

Private const ControllerEvent priv_initial_event =
{
 .second = 1u,
 .upperText = "Begin!",
 .lowerText = NULL,
 .shot_action = BEERSHOT_EMPTY
};


Private const ControllerEvent priv_normal_minute_events[] =
{
     { .second = 7u,  .upperText = "",              .lowerText = "",        .shot_action = BEERSHOT_EMPTY            , .func = NULL },
     { .second = 20u, .upperText = "Fill shots",    .lowerText = NULL,      .shot_action = BEERSHOT_BEGIN_FILLING    , .func = NULL },
     { .second = 45u, .upperText = "Ready",         .lowerText = NULL,      .shot_action = BEERSHOT_FULL             , .func = NULL },
     { .second = 59u, .upperText = "Proosit!",      .lowerText = "Cheers!", .shot_action = BEERSHOT_BEGIN_EMPTYING   , .func = NULL },
};

Private const ControllerEvent priv_guys_drink_events[] =
{
     { .second = 7u,  .upperText = "",              .lowerText = "",                    .shot_action = OVERRIDE_FUNCTION         , .func = &guysSpecialIntro   },
     { .second = 20u, .upperText = "Fill shots",    .lowerText = "Guys' round",         .shot_action = BEERSHOT_BEGIN_FILLING    , .func = NULL },
     { .second = 45u, .upperText = "Ready",         .lowerText = NULL,                  .shot_action = BEERSHOT_FULL             , .func = NULL },
     { .second = 59u, .upperText = "Proosit!",      .lowerText = "Cheers guys!",        .shot_action = OVERRIDE_FUNCTION         , .func = &guysSpecialTask    },
};

Private const ControllerEvent priv_girls_drink_events[] =
{
     { .second = 7u,  .upperText = "",              .lowerText = "",                    .shot_action = OVERRIDE_FUNCTION         , .func = &girlsSpecialIntro   },
     { .second = 20u, .upperText = "Fill shots",    .lowerText = "Girls round",         .shot_action = BEERSHOT_BEGIN_FILLING    , .func = NULL },
     { .second = 45u, .upperText = "Ready",         .lowerText = NULL,                  .shot_action = BEERSHOT_FULL             , .func = NULL },
     { .second = 59u, .upperText = "Proosit!",      .lowerText = "Cheers girls!",       .shot_action = OVERRIDE_FUNCTION         , .func = &girlsSpecialTask    },
};

Private const ControllerEvent priv_board_drink_events[] =
{
     { .second = 7u,  .upperText = "",              .lowerText = "",                    .shot_action = BEERSHOT_EMPTY            , .func = NULL                 }, /* TODO */
     { .second = 20u, .upperText = "Fill shots",    .lowerText = "Boardies Round",      .shot_action = BEERSHOT_BEGIN_FILLING    , .func = NULL                 },
     { .second = 45u, .upperText = "Ready",         .lowerText = NULL,                  .shot_action = BEERSHOT_FULL             , .func = NULL                 },
     { .second = 59u, .upperText = "Proosit!",      .lowerText = "Cheers boardies!",    .shot_action = BEERSHOT_BEGIN_EMPTYING   , .func = NULL                 }, /* TODO */
};

Private const ControllerEvent priv_cteam_drink_events[] =
{
     { .second = 7u,  .upperText = "",              .lowerText = "",                    .shot_action = BEERSHOT_EMPTY            , .func = NULL                 }, /* TODO */
     { .second = 20u, .upperText = "Fill shots",    .lowerText = "Core Team Round",     .shot_action = BEERSHOT_BEGIN_FILLING    , .func = NULL                 },
     { .second = 45u, .upperText = "Ready",         .lowerText = NULL,                  .shot_action = BEERSHOT_FULL             , .func = NULL                 },
     { .second = 59u, .upperText = "Proosit!",      .lowerText = "Cheers Core Team!",   .shot_action = BEERSHOT_BEGIN_EMPTYING   , .func = NULL                 }, /* TODO */
};

Private const ControllerEvent priv_alumni_drink_events[] =
{
     { .second = 7u,  .upperText = "",              .lowerText = "",                    .shot_action = BEERSHOT_EMPTY            , .func = NULL                 }, /* TODO */
     { .second = 20u, .upperText = "Fill shots",    .lowerText = "Alumni Round",        .shot_action = BEERSHOT_BEGIN_FILLING    , .func = NULL                 },
     { .second = 45u, .upperText = "Ready",         .lowerText = NULL,                  .shot_action = BEERSHOT_FULL             , .func = NULL                 },
     { .second = 59u, .upperText = "Proosit!",      .lowerText = "Cheers Alumni!",      .shot_action = BEERSHOT_BEGIN_EMPTYING   , .func = NULL                 }, /* TODO */
};

Private const ControllerEvent priv_pax_drink_events[] =
{
     { .second = 7u,  .upperText = "",              .lowerText = "",                    .shot_action = BEERSHOT_EMPTY            , .func = NULL                 }, /* TODO */
     { .second = 20u, .upperText = "Fill shots",    .lowerText = "PAX Round",           .shot_action = BEERSHOT_BEGIN_FILLING    , .func = NULL                 },
     { .second = 45u, .upperText = "Ready",         .lowerText = NULL,                  .shot_action = BEERSHOT_FULL             , .func = NULL                 },
     { .second = 59u, .upperText = "Proosit!",      .lowerText = "Cheers PAX!",         .shot_action = BEERSHOT_BEGIN_EMPTYING   , .func = NULL                 }, /* TODO */
};

Private const ControllerEvent priv_kt_drink_events[] =
{
     { .second = 7u,  .upperText = "",              .lowerText = "",                    .shot_action = BEERSHOT_EMPTY            , .func = NULL                 }, /* TODO */
     { .second = 20u, .upperText = "Fill shots",    .lowerText = "KT Round",            .shot_action = BEERSHOT_BEGIN_FILLING    , .func = NULL                 },
     { .second = 45u, .upperText = "Ready",         .lowerText = NULL,                  .shot_action = BEERSHOT_FULL             , .func = NULL                 },
     { .second = 59u, .upperText = "Proosit!",      .lowerText = "Paragrahv 5!",        .shot_action = BEERSHOT_BEGIN_EMPTYING   , .func = NULL                 }, /* TODO */
};

Private const ControllerEvent priv_soc_drink_events[] =
{
     { .second = 7u,  .upperText = "",              .lowerText = "",                    .shot_action = BEERSHOT_EMPTY            , .func = NULL                 }, /* TODO */
     { .second = 20u, .upperText = "Fill shots",    .lowerText = "Soc. Resp. Round",    .shot_action = BEERSHOT_BEGIN_FILLING    , .func = NULL                 },
     { .second = 45u, .upperText = "Ready",         .lowerText = NULL,                  .shot_action = BEERSHOT_FULL             , .func = NULL                 },
     { .second = 59u, .upperText = "Proosit!",      .lowerText = "Cheers Soc!",         .shot_action = BEERSHOT_BEGIN_EMPTYING   , .func = NULL                 }, /* TODO */
};


/* This is a scheduler for special minutes.
 * It contains data about the frequency and offset of special minutes as well
 * as links to their respective actions. */
Private const SchedulerTaskConf_T priv_scheduler_conf[NUMBER_OF_TASK_CATEGORIES] =
{
     {.event_array = priv_girls_drink_events,   .event_cnt = NUMBER_OF_ITEMS(priv_girls_drink_events),  .enable_tumblr = TUMBLER_UPPER_2  },       /*TASK_CATEGORY_GIRLS     */
     {.event_array = priv_guys_drink_events,    .event_cnt = NUMBER_OF_ITEMS(priv_guys_drink_events),   .enable_tumblr = TUMBLER_UPPER_3  },       /*TASK_CATEGORY_GUYS      */
     {.event_array = priv_board_drink_events,   .event_cnt = NUMBER_OF_ITEMS(priv_board_drink_events),  .enable_tumblr = TUMBLER_UPPER_0  },       /*TASK_CATEGORY_BOARD     */
     {.event_array = priv_cteam_drink_events,   .event_cnt = NUMBER_OF_ITEMS(priv_cteam_drink_events),  .enable_tumblr = TUMBLER_UPPER_1  },       /*TASK_CATEGORY_CORETEAM  */
     {.event_array = priv_alumni_drink_events,  .event_cnt = NUMBER_OF_ITEMS(priv_alumni_drink_events), .enable_tumblr = TUMBLER_LOWER_0  },       /*TASK_CATEGORY_ALUMNI    */
     {.event_array = priv_pax_drink_events,     .event_cnt = NUMBER_OF_ITEMS(priv_pax_drink_events),    .enable_tumblr = TUMBLER_LOWER_1  },       /*TASK_CATEGORY_PAX       */
     {.event_array = priv_kt_drink_events,      .event_cnt = NUMBER_OF_ITEMS(priv_kt_drink_events),     .enable_tumblr = TUMBLER_LOWER_2  },       /*TASK_CATEGORY_KT        */
     {.event_array = priv_soc_drink_events,     .event_cnt = NUMBER_OF_ITEMS(priv_soc_drink_events),    .enable_tumblr = TUMBLER_LOWER_3  },       /*TASK_CATEGORY_SOC       */
};

Private SchedulerTaskState_T priv_scheduler_state[NUMBER_OF_TASK_CATEGORIES];

Private beershotState priv_beer_state;
Private Rectangle priv_timer_rect;
Private OverrideFunc priv_override_ptr;
Private char priv_timer_str[10];
Private U8 priv_override_counter;
Private U8 priv_task_frequency = 3u; /* Default value is a task every 3 minutes. */


/*****************************************************************************************************
 *
 * Public function definitions
 *
 *****************************************************************************************************/


Public void clockDisplay_init(void)
{
    U8 font_height;

    priv_timekeeper.minute = 0u;
    priv_timekeeper.second = 0u;

    font_height = font_getFontHeight(CLOCK_FONT);

    priv_timer_rect.location.x = 1u;

    priv_timer_rect.location.y = NUMBER_OF_ROWS >> 1u; //Divide by 2.
    priv_timer_rect.location.y -= (font_height >> 1u);

    priv_timer_rect.size.height = font_height;
    priv_timer_rect.size.width = 15u * 5u;

    priv_timer_state = CONTROLLER_INIT;

}


Public void clockDisplay_start(void)
{
    U8 ix;

    display_clear();
    //We start counting.
    priv_timer_state = CONTROLLER_COUNTING;

    for (ix = 0u; ix < NUMBER_OF_TASK_CATEGORIES; ix++)
    {
        priv_scheduler_state[ix].counter = 0u;
    }
}


/* Should return all variables to their initial states. */
Public void clockDisplay_stop(void)
{
    priv_timer_state = CONTROLLER_INIT;
    priv_timekeeper.minute = 0u;
    priv_timekeeper.second = 0u;
}


Public void clockDisplay_cyclic1000msec(void)
{
    U8 ix;
    const ControllerEvent * event_ptr = NULL;
    beerShotAction action = BEERSHOT_NO_ACTION;
    static Boolean isFirstRun = TRUE;

    static const ControllerEvent * currentMinuteEvents_ptr = priv_normal_minute_events;
    static U8 controllerEvents_cnt = NUMBER_OF_ITEMS(priv_normal_minute_events);

    if ((priv_timekeeper.second == 0u) && (priv_timekeeper.minute > 0u))
    {
        controllerEvents_cnt = getScheduledSpecialTask(&currentMinuteEvents_ptr);
    }

    /* TODO : This should be changed to a better implementation. */
    if (priv_timekeeper.second == 59u)
    {
        //buzzer_playBuzzer(10u);
        buzzer_playBeeps(3u);
    }

    //Game ends and we enter final state.
    if (priv_timekeeper.minute == 60u)
    {
        doFinalAction();
        priv_timer_state = CONTROLLER_FINAL;
    }

    switch(priv_timer_state)
    {
    case CONTROLLER_INIT:
        //We do not do anything here.
        break;
    case CONTROLLER_COUNTING:
        incrementTimer();
        if (isFirstRun)
        {
            event_ptr = &priv_initial_event;
            isFirstRun = FALSE;
        }
        else
        {
            for (ix = 0u; ix < controllerEvents_cnt; ix++)
            {
                event_ptr = &currentMinuteEvents_ptr[ix];
                if (event_ptr->second == priv_timekeeper.second)
                {
                    break;
                }
                event_ptr = NULL;
            }
        }

        if (event_ptr != NULL)
        {
           if (event_ptr->upperText != NULL)
           {
               setUpperText(event_ptr->upperText);
           }

           if (event_ptr->lowerText != NULL)
           {
               setLowerText(event_ptr->lowerText);
           }

           action = event_ptr->shot_action;
        }

        //Currently we still finish this cycle and special handling begins on the next.
        if (action == OVERRIDE_FUNCTION)
        {
            priv_timer_state = CONTROLLER_OVERRIDDEN;
            priv_override_ptr = event_ptr->func;
            priv_override_counter = 0u;
        }

        drawBeerShot(action);

        drawTimer();

        break;
    case CONTROLLER_OVERRIDDEN:
        //We still increment timer.
        incrementTimer();

        if (priv_override_ptr(priv_override_counter))
        {
            display_clear();
            drawTimer();
            drawBeerShot(BEERSHOT_EMPTY);
            priv_timer_state = CONTROLLER_COUNTING;
        }
        priv_override_counter++;
        break;
    case CONTROLLER_FINAL:
    default:
        break;
    }
}

Public void clockDisplay_setTaskFrequency(U8 freq)
{
    priv_task_frequency = freq;
}

Public U16 clockDisplay_getTaskFrequency(void)
{
    return (U16)priv_task_frequency;
}

/********* Private function definitions **********/
Private void doFinalAction(void)
{
    display_clear();
    display_drawString("Game Over!", 20u, 15u, FONT_LARGE_FONT, FALSE);
    display_drawString("  Congratulations! \n You are now drunk", 5u, 37u, FONT_MEDIUM_FONT, FALSE);
}


//Increments timekeeper with 1 second.
Private void incrementTimer(void)
{
    priv_timekeeper.second++;
    if (priv_timekeeper.second >= 60u)
    {
        priv_timekeeper.second = 0u;
        priv_timekeeper.minute++;
    }
}

Private void convertTimerString(timekeeper_struct * t, char * dest_str)
{
    dest_str[0] = '0' + (t->minute / 10u);
    dest_str[1] = '0' + (t->minute % 10u);
    dest_str[2] = ':';
    dest_str[3] = '0' + (t->second / 10u);
    dest_str[4] = '0' + (t->second % 10u);
    dest_str[5] = 0;
}


Private void drawBeerShot(beerShotAction action)
{
   const Bitmap * bmp_ptr = NULL;

   switch(action)
   {
       case BEERSHOT_EMPTY:
           bmp_ptr = ShotGlassAnimation_GetFirst();
           priv_beer_state = BEERSHOT_FROZEN;
           break;
       case BEERSHOT_FULL:
           bmp_ptr = ShotGlassAnimation_GetLast();
           priv_beer_state = BEERSHOT_FROZEN;
           break;
       case BEERSHOT_BEGIN_FILLING:
           bmp_ptr = ShotGlassAnimation_GetFirst();
           priv_beer_state = BEERSHOT_FILLING;
           break;
       case BEERSHOT_BEGIN_EMPTYING:
           bmp_ptr = ShotGlassAnimation_GetLast();
           priv_beer_state = BEERSHOT_EMPTYING;
           break;
       case BEERSHOT_NO_ACTION:
       default:
           break;
   }

   switch(priv_beer_state)
   {
       case(BEERSHOT_FROZEN):
           //Nothing new to draw.
           break;
       case(BEERSHOT_EMPTYING):
           if(bmp_ptr == NULL)
           {
               bmp_ptr = ShotGlassAnimation_GetPrev();
           }
           break;
       case(BEERSHOT_FILLING):
           if(bmp_ptr == NULL)
           {
               bmp_ptr = ShotGlassAnimation_GetNext();
           }
           break;
       default:
           break;
   }

   //If bmp_ptr is not NULL, then we have something to draw.
   if (bmp_ptr != NULL)
   {
       display_drawBitmap(bmp_ptr, BEERSHOT_X, BEERSHOT_Y, FALSE);
   }
}


//Return default normal cycle if no special events are scheduled.
Private U8 getScheduledSpecialTask(const ControllerEvent ** event_ptr)
{
    U8 ix;
    U8 res;

    Boolean is_any_enabled = FALSE;

    /* First we have to check if we have any tasks enabled at all... */

    for (ix = 0u; ix < NUMBER_OF_TASK_CATEGORIES; ix++)
    {
        if (tumbler_getState(priv_scheduler_conf[ix].enable_tumblr))
        {
            priv_scheduler_state[ix].is_enabled = TRUE;
            is_any_enabled = TRUE;
        }
        else
        {
            priv_scheduler_state[ix].is_enabled = FALSE;
        }
    }

    if (((priv_timekeeper.minute % priv_task_frequency) == 0u) && (is_any_enabled == TRUE))
    {
        ix = selectRandomTaskIndex();
        *event_ptr = priv_scheduler_conf[ix].event_array;
        res = priv_scheduler_conf[ix].event_cnt;
    }
    else
    {
        *event_ptr = priv_normal_minute_events;
        res = NUMBER_OF_ITEMS(priv_normal_minute_events);
    }

    return res;
}


/* It returns a random enabled task. However the function should prefer tasks that have not yet been displayed or have been displayed less than others. */
Private U8 selectRandomTaskIndex(void)
{
    U8 max_count = 0u;
    U8 min_count = 0xffu;
    U8 ix;
    U8 res = 0u;

    U8 index_array[NUMBER_OF_TASK_CATEGORIES];
    U8 index_length = 0u;

    /* Lets first establish the MAX and MIN count that we have. */
    for (ix = 0u; ix < NUMBER_OF_TASK_CATEGORIES; ix++)
    {
        max_count = MAX(priv_scheduler_state[ix].counter, max_count);
        min_count = MIN(priv_scheduler_state[ix].counter, min_count);
    }

    if (max_count == min_count)
    {
        /* We have no preference in this case. We add all indexes that are enabled to the array. */
        for (ix = 0u; ix < NUMBER_OF_TASK_CATEGORIES; ix++)
        {
            if (priv_scheduler_state[ix].is_enabled)
            {
                index_array[index_length] = ix;
                index_length++;
            }
        }

        max_count++;
    }
    else
    {
        /* We prefer tasks that have not been selected yet. */
        for (ix = 0u; ix < NUMBER_OF_TASK_CATEGORIES; ix++)
        {
            if (priv_scheduler_state[ix].is_enabled && (priv_scheduler_state[ix].counter < max_count))
            {
                index_array[index_length] = ix;
                index_length++;
            }
        }
    }

    res = index_array[generate_random_number(index_length - 1u)];


    /* We set the selected count to the max. */
    priv_scheduler_state[res].counter = max_count;

    return res;
}


Private void drawTimer(void)
{
    convertTimerString(&priv_timekeeper, priv_timer_str);

    display_fillRectangle(priv_timer_rect.location.x,
                          priv_timer_rect.location.y,
                          priv_timer_rect.size.height,
                          priv_timer_rect.size.width,
                          PATTERN_WHITE);

    display_drawString(priv_timer_str,
                       priv_timer_rect.location.x,
                       priv_timer_rect.location.y,
                       CLOCK_FONT,
                       FALSE);
}



Private void setUpperText(const char * str)
{
    clearUpperText();
    display_drawString(str, UPPER_TEXT_XLOC, UPPER_TEXT_YLOC, UPPER_TEXT_FONT, FALSE);
}

Private void clearUpperText(void)
{
    display_fillRectangle(UPPER_TEXT_XLOC, UPPER_TEXT_YLOC, UPPER_TEXT_HEIGHT, UPPER_TEXT_WIDTH, PATTERN_WHITE);
}

Private void setLowerText(const char * str)
{
    clearLowerText();
    display_drawString(str, LOWER_TEXT_XLOC, LOWER_TEXT_YLOC, LOWER_TEXT_FONT, FALSE);
}

Private void clearLowerText(void)
{
    display_fillRectangle(LOWER_TEXT_XLOC, LOWER_TEXT_YLOC, LOWER_TEXT_HEIGHT, LOWER_TEXT_WIDTH, PATTERN_WHITE);
}


/*****************************************************************************************************
 *
 * Intro functions.
 *
 *****************************************************************************************************/
Private const IntroSequence priv_guys_intros[] =
{
     {.bmp_ptr = &strong_dude_bitmap, .bmp_x = 0u, .bmp_y = 0u, .text_str = "Guys Round!", .text_x = 58u, .text_y = 4u, .text_font = FONT_MEDIUM_FONT , .isInverted = FALSE },
     {.bmp_ptr = &chad_bitmap,        .bmp_x = 0u, .bmp_y = 0u, .text_str = "Guys Round!", .text_x = 50u, .text_y = 4u, .text_font = FONT_MEDIUM_FONT , .isInverted = FALSE },
     {.bmp_ptr = &man3_bitmap,        .bmp_x = 0u, .bmp_y = 0u, .text_str = "Guys Round!", .text_x = 20u, .text_y = 4u, .text_font = FONT_MEDIUM_FONT , .isInverted = TRUE  },
     {.bmp_ptr = &dude4_bitmap,       .bmp_x = 0u, .bmp_y = 0u, .text_str = "Guys Round!", .text_x = 50u, .text_y = 4u, .text_font = FONT_MEDIUM_FONT , .isInverted = TRUE  },
};

Private Boolean guysSpecialIntro(U8 sec)
{
    Boolean res = FALSE;

    static const IntroSequence * intro_ptr = &priv_guys_intros[0];
    static U8 intro_ix;

    switch(sec)
    {
    case(1u):
        display_clear();

        intro_ptr = &priv_guys_intros[intro_ix];
        intro_ix++;
        if (intro_ix >= NUMBER_OF_ITEMS(priv_guys_intros))
        {
            intro_ix = 0u;
        }

        display_drawBitmap(intro_ptr->bmp_ptr, intro_ptr->bmp_x, intro_ptr->bmp_y, intro_ptr->isInverted);
        break;
    case(2u):
        display_drawString(intro_ptr->text_str, intro_ptr->text_x, intro_ptr->text_y, intro_ptr->text_font, FALSE);
        break;
    case(10u):
        res = TRUE;
    break;
    default:
        break;
    }

    return res;
}


Private const IntroSequence priv_girl_intros[] =
{
     { .bmp_ptr = &girl_1_bitmap, .bmp_x = 0u, .bmp_y = 0u, .text_str = "Girls round!", .text_x = 50u, .text_y = 4u, .text_font = FONT_MEDIUM_FONT , .isInverted = FALSE},
     { .bmp_ptr = &girl_2_bitmap, .bmp_x = 6u, .bmp_y = 0u, .text_str = "Girls round!", .text_x = 50u, .text_y = 4u, .text_font = FONT_MEDIUM_FONT , .isInverted = FALSE},
     { .bmp_ptr = &girl_3_bitmap,        .bmp_x = 6u, .bmp_y = 0u, .text_str = "Girls round!", .text_x = 50u, .text_y = 4u, .text_font = FONT_MEDIUM_FONT, .isInverted = TRUE },
     { .bmp_ptr = &girl_9_bitmap,        .bmp_x = 0u, .bmp_y = 0u, .text_str = "Girls round!", .text_x = 50u, .text_y = 4u, .text_font = FONT_MEDIUM_FONT, .isInverted = TRUE },
     { .bmp_ptr = &girl_Sasha_bitmap,    .bmp_x = 0u, .bmp_y = 0u, .text_str = "Girls round!", .text_x = 10u, .text_y = 4u, .text_font = FONT_MEDIUM_FONT, .isInverted = TRUE },
};

//We start displaying a special task.
Private Boolean girlsSpecialIntro(U8 sec)
{
    Boolean res = FALSE;
    static const IntroSequence * intro_ptr = &priv_girl_intros[0];
    static U8 intro_ix;

    switch(sec)
    {
    case(1u):
        display_clear();
        intro_ptr = &priv_girl_intros[intro_ix];
        intro_ix++;
        if (intro_ix >= NUMBER_OF_ITEMS(priv_girl_intros))
        {
            intro_ix = 0u;
        }

        //display_drawBitmap(&test_girl_bitmap, 0u, 0u);
        display_drawBitmap(intro_ptr->bmp_ptr, intro_ptr->bmp_x, intro_ptr->bmp_y, intro_ptr->isInverted);
        break;
    case(2u):
        //display_drawString("Girls Round!", 50, 4, FONT_MEDIUM_FONT);
        display_drawString(intro_ptr->text_str, intro_ptr->text_x, intro_ptr->text_y, intro_ptr->text_font, FALSE);
        break;
    case(10u):
        res = TRUE;
    break;
    default:
        break;
    }

    return res;
}


