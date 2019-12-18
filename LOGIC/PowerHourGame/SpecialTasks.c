/*
 * SpecialTasks.c
 *
 *  Created on: Sep 5, 2017
 *      Author: Joonatan
 */

#include <LOGIC/PowerHourGame/SpecialTasks.h>
#include "bitmaps.h"
#include "font.h"
#include "display_drv.h"
#include "register.h"
#include <stdlib.h>
#include "pot.h"

#define SMALL_SHOT_X 20u
#define SMALL_SHOT_Y 32u
#define SMALL_SHOT_INTERVAL 20u

typedef struct
{
    const char * upper_text;
    const char * middle_text;
    const char * lower_text;
    U8 counter; /* Number of times, this task has been selected. */
} Task_T;


//Private Boolean DrinkTwiceTask(U8 sec, const char * headerWord);
Private Boolean DrinkTwiceTask(U8 sec, SpecialTaskType type);
Private Boolean SpecialTaskWithRandomText(U8 sec, SpecialTaskType type);
Private const Task_T * getRandomTaskFromArray(Task_T * array, U8 array_size);

Private const SpecialTaskFunc priv_special_tasks_girls_array[] =
{
  &SpecialTaskWithRandomText,
  &DrinkTwiceTask,
  &SpecialTaskWithRandomText,
  &SpecialTaskWithRandomText,
  &DrinkTwiceTask,
  &SpecialTaskWithRandomText,
  &SpecialTaskWithRandomText,
};


Private const SpecialTaskFunc priv_special_tasks_guys_array[] =
{
  &SpecialTaskWithRandomText,
  &DrinkTwiceTask,
  &SpecialTaskWithRandomText,
  &SpecialTaskWithRandomText,
  &DrinkTwiceTask,
  &SpecialTaskWithRandomText,
  &SpecialTaskWithRandomText,
};


Private char priv_str_buf[64];
Private SpecialTaskFunc priv_selected_task_ptr;

/* Easy tasks. -> Really softcore.  */
Private Task_T priv_TextArrayGirlsLevel1[] =
{
     { "The girl with ",     "the fanciest clothes"  , "drinks 2x"         , .counter = 0u  }, /* 1  */
     { NULL,                 "Only girls drink"      , NULL                , .counter = 0u  }, /* 2  */
     { "Girls drink",        "without "      ,         "using hands"       , .counter = 0u  }, /* 3  */
     { "Choose one girl",    "who drinks 3x ",         NULL                , .counter = 0u  }, /* 4  */
     { "All bad girls",      "drink 2x ",              NULL                , .counter = 0u  }, /* 5  */
     { "The girl with",      "the longest hair ",      "drinks a shot"     , .counter = 0u  }, /* 6  */
     { "All blondes",        "drink 2x ",              NULL                , .counter = 0u  }, /* 7  */
     { "All brunettes",      "drink 2x ",              NULL                , .counter = 0u  }, /* 8  */
};

/* Easy tasks. */
Private Task_T priv_TextArrayGuysLevel1[] =
{
     {  NULL                    , "Only guys drink",            NULL          , .counter = 0u  }, /* 1  */
     {  "Guys drink"            , "without",                    "using hands" , .counter = 0u  }, /* 2  */
     {  "The toughest guy"      , "drinks 3x",                  NULL          , .counter = 0u  }, /* 3  */
     {  "The biggest playboy"   , "drinks 3x",                  NULL          , .counter = 0u  }, /* 4  */
     {  NULL                    , "Guys must sing",         "a song together" , .counter = 0u  }, /* 7  */
     {  "Last guy to put his"   , "finger on his nose",        "drinks 2x"    , .counter = 0u  }, /* 8  */
     {  "Choose one guy"        , "who drinks 3x ",             NULL          , .counter = 0u  }, /* 9  */
};

/* Medium tasks */
Private Task_T priv_TextArrayGirlsLevel2[] =
{
     { "The girl with ",     "the sexiest voice"     , "drinks 2x "        , .counter = 0u  }, /* 1  */
     { "Girls",              "I have never ever"     , NULL                , .counter = 0u  }, /* 2  */
     { "The girl with  ",    "the largest boobs"     , "drinks 2x"         , .counter = 0u  }, /* 3  */
     { "All couples  ",      "drink 2x"              , NULL                , .counter = 0u  }, /* 4  */
     { "All girls whose" ,   "name starts with",     "S drinks 2x"         , .counter = 0u  }, /* 5  */
     { "All bad girls",      "drink 2x ",              NULL                , .counter = 0u  }, /* 6  */
     { "All good girls",     "drink 2x ",              NULL                , .counter = 0u  }, /* 7  */
};

/* Medium tasks. */
Private Task_T priv_TextArrayGuysLevel2[] =
{
     {  "The guy with the"      , "biggest balls",            "drinks vodka"  , .counter = 0u  }, /* 1  */
     {  "Guys"                  , "Never have I ever",          NULL          , .counter = 0u  }, /* 2  */
     {  "All guys lose"         , "One Item of Clothing",       NULL          , .counter = 0u  }, /* 3  */
     {  "All guys whose"        , "name starts with",         "A drinks 2x"   , .counter = 0u  }, /* 4  */
     {  "All couples  "         , "drink 2x",                   NULL          , .counter = 0u  }, /* 5  */
};

/* Hard tasks. -> Sass mode engaged :D */
Private Task_T priv_TextArrayGirlsLevel3[] =
{
     {  "Girls must"             , "do a ",                      "sexy dance"  , .counter = 0u  }, /* 1  */
     {  "Vodka round!!!",          "for girls!!!"   ,                   NULL   , .counter = 0u  }, /* 2  */
     {  "Most naked"           ,   "girl drinks ",               "2x"          , .counter = 0u  }, /* 3  */
     {  "All girls lose"       ,   "two items of ",              "clothing"    , .counter = 0u  }, /* 4  */
     {  "Girls with"           ,   ">5 flags&numbers",           "drink 3x"    , .counter = 0u  }, /* 5  */
     {  "Girls who've had"     ,   "sex with 1 of the",  "players drink 2x"    , .counter = 0u  }, /* 6  */
};

/* Hard tasks. -> Sass mode engaged : D */
Private Task_T priv_TextArrayGuysLevel3[] =
{
     {  "Guys must"            , "do a ",                      "sexy dance"  , .counter = 0u  }, /* 1  */
     {  "Vodka round!!!",        "for guys!!!"   ,                   NULL    , .counter = 0u  }, /* 2  */
     {  "Most naked"           , "guy drinks ",                "2x"          , .counter = 0u  }, /* 3  */
     {  "All guys lose"        , "two items of ",         "clothing"         , .counter = 0u  }, /* 4  */
     {  "Guys with"            , ">5 flags&numbers",      "drink 3x"         , .counter = 0u  }, /* 5  */
     {  "Guys who want to"     , "sleep with 1 of",       "the players drink", .counter = 0u  }, /* 6  */
};


/* Hardcore tasks. -> Full Sass mode. */
Private Task_T priv_TextArrayGirlsLevel4[] =
{
     { "1 girl must",     "do a lapdance"  , "to Sass"         , .counter = 0u  }, /*  1  */
     { "All girls",       "lose 1 item"    , "of clothing"     , .counter = 0u  }, /*  2  */
     { "2 girls",         "make out"       , "or drink 3x"     , .counter = 0u  }, /*  3  */
     { "Vodka round!!!" , "for girls!!!"   , NULL              , .counter = 0u  }, /*  4  */
     { "All clean",       "shaven girls"   , "drink 1x"        , .counter = 0u  }, /*  5  */
     { "All girls",       "who masturbated", "today drink 2x"  , .counter = 0u  }, /*  6  */
};

/* Hardcore tasks. -> Full Sass mode. */
Private Task_T priv_TextArrayGuysLevel4[] =
{
     { "Sass must do",     "a lapdance"  , "to a girl"         , .counter = 0u  }, /*  1  */
     { "Sass must do",     "a lapdance"  , "to a guy"          , .counter = 0u  }, /*  2  */
     { "2 guys",           "make out"    , "or drink 3x"       , .counter = 0u  }, /*  3  */
     { "Vodka round!!!" ,  "for girls!!!"   , NULL             , .counter = 0u  }, /*  4  */
     { "Sass loses",       "3 items"     ,  "of clothing"      , .counter = 0u  }, /*  5  */
     { "All guys",         "who wanked"  ,  "today drink 2x"   , .counter = 0u  }, /*  6  */
};


Public Boolean girlsSpecialTask(U8 sec)
{
    Boolean res = FALSE;
    static U8 test_counter = 0u;

    /* If sec is 0, then we have just begun. */
    if (sec == 0u)
    {
        priv_selected_task_ptr = priv_special_tasks_girls_array[test_counter];
        test_counter++;
        if(test_counter >= NUMBER_OF_ITEMS(priv_special_tasks_girls_array))
        {
            test_counter = 0u;
        }
    }

    res = priv_selected_task_ptr(sec, TASK_FOR_GIRLS);
    return res;
}


Public Boolean guysSpecialTask(U8 sec)
{
    Boolean res = FALSE;
    static U8 test_counter = 0u; //TODO : This should be reviewed.

    if (sec == 0u)
    {
        //priv_selected_task_ptr = &DrinkTwiceTask;
        priv_selected_task_ptr = priv_special_tasks_guys_array[test_counter];
        test_counter++;
        if(test_counter >= NUMBER_OF_ITEMS(priv_special_tasks_guys_array))
        {
            test_counter = 0u;
        }
    }

    res = priv_selected_task_ptr(sec, TASK_FOR_GUYS);
    return res;
}


Private Boolean DrinkTwiceTask(U8 sec, SpecialTaskType type)
{
    Boolean res = FALSE;

    switch(sec)
    {
    case(1u):
       display_clear();
       if (type == TASK_FOR_GIRLS)
       {
           strcpy(priv_str_buf, "Girls");
       }
       else if(type == TASK_FOR_GUYS)
       {
           strcpy(priv_str_buf, "Guys");
       }

       strcat(priv_str_buf, " drink");
       display_drawStringCenter(priv_str_buf, 64u, 2u, FONT_LARGE_FONT, FALSE);
       display_drawStringCenter("2x", 64u ,20u, FONT_LARGE_FONT, FALSE);
       break;
    case (2u):
       display_drawBitmapCenter(&small_shot_bitmap, 64 - SMALL_SHOT_INTERVAL, SMALL_SHOT_Y, FALSE);
       break;
    case (3u):
        display_drawBitmapCenter(&small_shot_bitmap, 64 + SMALL_SHOT_INTERVAL, SMALL_SHOT_Y, FALSE);
       break;
    case(10u):
       res = TRUE;
       break;
    default:
        break;
    }

    return res;
}

Private const Task_T * priv_task_str_ptr;

/* The sec parameter specifies the current second from the beginning of the task.
 * This function is called cyclically after every second. */
Private Boolean SpecialTaskWithRandomText(U8 sec, SpecialTaskType type)
{
    //This is the simplest special task, currently no bitmaps, but we just display text on screen.
    //Text is three lines and randomly selected from tasks from previous PH controller :)

    Boolean res = FALSE;
    int taskLevel = pot_getSelectedRange();
    Task_T * girlsSelectedArray;
    Task_T * guysSelectedArray;
    U16 number_of_items_girls = 0u;
    U16 number_of_items_guys = 0u;

    switch(taskLevel)
    {
        case 0:
            girlsSelectedArray = priv_TextArrayGirlsLevel1;
            number_of_items_girls = NUMBER_OF_ITEMS(priv_TextArrayGirlsLevel1);
            guysSelectedArray = priv_TextArrayGuysLevel1;
            number_of_items_guys = NUMBER_OF_ITEMS(priv_TextArrayGuysLevel1);
            break;
        case 1:
            girlsSelectedArray = priv_TextArrayGirlsLevel2;
            number_of_items_girls = NUMBER_OF_ITEMS(priv_TextArrayGirlsLevel2);
            guysSelectedArray = priv_TextArrayGuysLevel2;
            number_of_items_guys = NUMBER_OF_ITEMS(priv_TextArrayGuysLevel2);
            break;
        case 2:
            girlsSelectedArray = priv_TextArrayGirlsLevel3;
            number_of_items_girls = NUMBER_OF_ITEMS(priv_TextArrayGirlsLevel3);
            guysSelectedArray = priv_TextArrayGuysLevel3;
            number_of_items_guys = NUMBER_OF_ITEMS(priv_TextArrayGuysLevel3);
            break;
        case 3:
            girlsSelectedArray = priv_TextArrayGirlsLevel4;
            number_of_items_girls = NUMBER_OF_ITEMS(priv_TextArrayGirlsLevel4);
            guysSelectedArray = priv_TextArrayGuysLevel4;
            number_of_items_guys = NUMBER_OF_ITEMS(priv_TextArrayGuysLevel4);
            break;
        default:
            /* Should not happen. */
            girlsSelectedArray = priv_TextArrayGirlsLevel1;
            number_of_items_girls = NUMBER_OF_ITEMS(priv_TextArrayGirlsLevel1);
            guysSelectedArray = priv_TextArrayGuysLevel1;
            number_of_items_guys = NUMBER_OF_ITEMS(priv_TextArrayGuysLevel1);
            break;
    }

    switch(sec)
    {
        case(1u):
            {
               if (type == TASK_FOR_GIRLS)
               {
                   priv_task_str_ptr = getRandomTaskFromArray(girlsSelectedArray, number_of_items_girls);
               }
               else if(type == TASK_FOR_GUYS)
               {
                   priv_task_str_ptr = getRandomTaskFromArray(guysSelectedArray, number_of_items_guys);
               }
            }
            break;
        case (2u):
            {
                display_clear();
                display_drawStringCenter(priv_task_str_ptr->upper_text, 64u, 4u, FONT_MEDIUM_FONT, FALSE);
            }
            break;
        case (3u):
            {
                display_drawStringCenter(priv_task_str_ptr->middle_text, 64u, 23u, FONT_MEDIUM_FONT, FALSE);
            }
            break;
        case(4u):
            {
                display_drawStringCenter(priv_task_str_ptr->lower_text, 64u, 43u, FONT_MEDIUM_FONT, FALSE);
            }
            break;
        case(12u):
            {
                res = TRUE;
            }
            break;
        default:
            break;
    }

    return res;
}


Private const Task_T * getRandomTaskFromArray(Task_T * array, U8 array_size)
{
    U8 ix;
    U8 min_count = 0xffu;

    U8 * index_array;
    U8 unused = 0u;
    U16 result_index;

    index_array = (U8 *)malloc(sizeof(U8) * array_size);

    if (index_array == NULL)
    {
        /* TODO : Review error handling. Currently will be stuck in infinite loop. */
        while(1);
    }

    for (ix = 0u; ix < array_size; ix++)
    {

        if (array[ix].counter < min_count)
        {
            min_count = array[ix].counter;
        }
    }

    for (ix = 0u; ix < array_size; ix++)
    {
        if (array[ix].counter <= min_count)
        {
            /* We can use this item. */
            index_array[unused] = ix;
            unused++;
        }
    }

    if (unused > 0u)
    {
        /* So now index_array should contain all the unused indexes. */
        result_index = index_array[generate_random_number(unused - 1u)];
    }
    else
    {
        /* TODO : Review this case. something has gone seriously wrong... */
        result_index = 0u;
    }

    free(index_array);
    array[result_index].counter++;
    return &array[result_index];
}

