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
  &SpecialTaskWithRandomText,
  &SpecialTaskWithRandomText,
  &SpecialTaskWithRandomText,
};


Private const SpecialTaskFunc priv_special_tasks_guys_array[] =
{
  &SpecialTaskWithRandomText,
  &DrinkTwiceTask,
  &SpecialTaskWithRandomText,
  &SpecialTaskWithRandomText,
  &SpecialTaskWithRandomText,
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
     { "The girl with",      "the longest hair ",      "drinks a shot"     , .counter = 0u  }, /* 5  */
     { "The girl with",      "the highest voice",      "drinks a shot"     , .counter = 0u  }, /* 6  */
     { "All blondes",        "drink 2x ",              NULL                , .counter = 0u  }, /* 7  */
     { "All brunettes",      "drink 2x ",              NULL                , .counter = 0u  }, /* 8  */
     { "The tallest girl",   "drinks 2x ",             NULL                , .counter = 0u  }, /* 9  */
     { "The youngest girl",  "drinks 2x ",             NULL                , .counter = 0u  }, /* 10 */
     { "All girl BESTies",   "drink  2x ",             NULL                , .counter = 0u  }, /* 11 */
     { "The shortest girl",  "drinks 2x ",             NULL                , .counter = 0u  }, /* 12 */
     { "Girls propose",      "the next toast ",        NULL                , .counter = 0u  }, /* 13 */
     { "All redheads",       "drink 2x ",              NULL                , .counter = 0u  }, /* 14 */
     { "All girls",          "with purple hair ",      "drink 2x"          , .counter = 0u  }, /* 15 */
     { "All girls",          "with glasses ",          "drink 2x"          , .counter = 0u  }, /* 16 */
     { "Choose 1 girl",      "that drinks 2x",        NULL                 , .counter = 0u  }, /* 17 */
     { "All girls",          "wearing black",         "drink 2x"           , .counter = 0u  }, /* 18 */
};

/* Easy tasks. */
Private Task_T priv_TextArrayGuysLevel1[] =
{
     {  NULL                    , "Only guys drink",            NULL          , .counter = 0u  }, /* 1  */
     {  "Guys drink"            , "without",                 "using hands"    , .counter = 0u  }, /* 2  */
     {  "The toughest guy"      , "drinks 3x",                  NULL          , .counter = 0u  }, /* 3  */
     {  "The biggest playboy"   , "drinks 3x",                  NULL          , .counter = 0u  }, /* 4  */
     {  NULL                    , "Guys must sing",         "a song together" , .counter = 0u  }, /* 5  */
     {  "Last guy to put his"   , "finger on his nose",        "drinks 2x"    , .counter = 0u  }, /* 6  */
     {  "Choose one guy"        , "who drinks 3x ",             NULL          , .counter = 0u  }, /* 7  */
     {  "All guys"              , "drop and do 10  ",        "pushups"        , .counter = 0u  }, /* 8  */
     {  "All guys with"         , "a six-pack ",             "drink 3x"       , .counter = 0u  }, /* 9  */
     {  "The most wasted"       , "guy drinks",              "water-shot"     , .counter = 0u  }, /* 10 */
     {  "Guys with"             , "hair gel",                "drink 3x"       , .counter = 0u  }, /* 11 */
     {  "Single Guys "          , "drink vodka",             NULL             , .counter = 0u  }, /* 12 */
     {  "Sass proposes",          "the next toast ",         NULL             , .counter = 0u  }, /* 13 */
     {  "The youngest guy"      , "drinks 2x",               NULL             , .counter = 0u  }, /* 14 */
     {  "All guys that"         , "are in the army",         "drink 3x"       , .counter = 0u  }, /* 15 */
     {  "All guys",               "with glasses ",           "drink 2x"       , .counter = 0u  }, /* 16 */
     {  "One guy",                "must say a toast",        "that rhymes"    , .counter = 0u  }, /* 17 */
     { "All guys whose" ,         "name starts with",     "S drinks 2x"       , .counter = 0u  }, /* 18 */
};


Private Task_T priv_TextArrayAlumniLevel1[] =
{
     { "Oldest alumni", 	"drinks 2x", 				NULL 			, .counter = 0u 	},
     { "Youngest alumni", 	"drinks 2x", 				NULL 			, .counter = 0u		},
     { "One alumni", 		"must tell", 				"a joke " 		, .counter = 0u 	},
	 { "All alumni",        "drink 2x", 				NULL			, .counter = 0u		},
	 { "Everybody over 35", "skip 1 round", 			"rest drink 2x"	, .counter = 0u     },
	 { "Alumni with a",   	"young BEST girlfriend", 	"drink 3x" 		, .counter = 0u     },
     { "All alumni who",    "remember GA 2016", 		"drink 3x"		, .counter = 0u 	},
	 { "Alumni who are",    "already drunk",    		"drink 3x"      , .counter = 0u     },
	 { "Alumni who have", 	"been to Ekaterinburg", 	"drink 2x"		, .counter = 0u		},
     { "Alumni board",      "drinks 3x"				    , NULL          , .counter = 0u		},
};

Private Task_T priv_TextArrayCoreTeamLevel1[] =
{
     { "Placeholder", "Coreteam 1", "Level 1 " , .counter = 0u },
     { "Placeholder", "Coreteam 2", "Level 1 " , .counter = 0u },
     { "Placeholder", "Coreteam 3", "Level 1 " , .counter = 0u },
};

Private Task_T priv_TextArraySocRespLevel1[] =
{
     { "Social responsible", "drinks 2x and", "does 10 pushups" 	, .counter = 0u },
     { "Social responsible", "takes off 1 pieces", "of clothing " 	, .counter = 0u },
     { "Social responsible", "tells", "\"your mama\" joke " 		, .counter = 0u },
	 { "Social responsible", "drinks 3x", NULL 						, .counter = 0u },
	 { "Social responsible", "must say the toast", "all drink " 	, .counter = 0u },
	 { "Youngest", 			 "social responsible", "takes a shot "  , .counter = 0u },
	 { "Social responsible", "tells an", 	"embarranssing story"   , .counter = 0u },
	 { "Social responsible", "makes sure", "everyone has a drink"   , .counter = 0u },
	 { "Social responsible", "drinks without", "using hands " 		, .counter = 0u },
	 { "Social responsible", "decides", "who drinks " 				, .counter = 0u },
};

Private Task_T priv_TextArrayKtLevel1[] =
{
     { "Placeholder", "Kt 1", "Level 1 " , .counter = 0u },
     { "Placeholder", "Kt 2", "Level 1 " , .counter = 0u },
     { "Placeholder", "Kt 3", "Level 1 " , .counter = 0u },
};


Private Task_T priv_TextArrayPaxLevel1[] =
{
     { "Placeholder", "Pax 1", "Level 1 " , .counter = 0u },
     { "Placeholder", "Pax 2", "Level 1 " , .counter = 0u },
     { "Placeholder", "Pax 3", "Level 1 " , .counter = 0u },
};

Private Task_T priv_TextArrayBoardLevel1[] =
{
     { "All boardies", 		"drink 2x", 			NULL   , 				.counter = 0u },
     { "Boardies have to", 	"distribute 5 shots", 	"amongst each other" , 	.counter = 0u },
     { "Placeholder", "Board 3", "Level 1 " , 	.counter = 0u },
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
     { "The girls with" ,    "the shortest skirt",   "drinks 2x"           , .counter = 0u  }, /* 8  */
     { "All female" ,        "organisers",           "drink 2x"            , .counter = 0u  }, /* 9  */
     { "Last girl to" ,      "put finger on",        "nose drinks 2x"      , .counter = 0u  }, /* 10 */
     { "All girls that",     "have boyfriends ",     "drink 2x"            , .counter = 0u  }, /* 11 */
     { "All blondes",        "drink vodka ",              NULL             , .counter = 0u  }, /* 12 */
     { "All brunettes",      "drink vodka ",              NULL             , .counter = 0u  }, /* 13 */
     { "Girls who are",      "former virgins",       "drink 2x"            , .counter = 0u  }, /* 14 */
     { "Girls must purr",    "like a kitten",        "after drinking!"     , .counter = 0u  }, /* 15 */
     { "All girls whose" ,   "name starts with",     "L drinks 2x"         , .counter = 0u  }, /* 16 */
     { "All girls who" ,     "kissed a girl",        "today drink 2x"      , .counter = 0u  }, /* 17 */
     { "All girls with" ,    "blue eyes",            "drink 2x"            , .counter = 0u  }, /* 18 */
     { "All girls with" ,    "a pony tail",          "drink 2x"            , .counter = 0u  }, /* 19 */
};

/* Medium tasks. */
Private Task_T priv_TextArrayGuysLevel2[] =
{
     {  "The guy with the"      , "biggest balls",            "drinks vodka"    ,  .counter = 0u  },  /* 1  */
     {  "Guys"                  , "Never have I ever",          NULL            ,  .counter = 0u  },  /* 2  */
     {  "All guys lose"         , "One Item of Clothing",       NULL            ,  .counter = 0u  },  /* 3  */
     {  "All guys whose"        , "name starts with",         "A drinks 2x"     ,  .counter = 0u  },  /* 4  */
     {  "All couples  "         , "drink 2x",                   NULL            ,  .counter = 0u  },  /* 5  */
     {  "All male"              , "organisers",               "drink 2x"        ,  .counter = 0u  },  /* 6  */
     {  "The guy with"          , "the biggest beer",         "belly drinks 2x" ,  .counter = 0u  },  /* 7  */
     {  "All guys with"         , "beards",                   "drink 2x"        ,  .counter = 0u  },  /* 8  */
     {  "Sass"                  , "drinks 3x",                "unless wasted"   ,  .counter = 0u  },  /* 9  */
     {  "Sass"                  , "needs to drink",           "without hands"   ,  .counter = 0u  },  /* 10 */
     {  "All former senors",      "drink vodka",               NULL             ,  .counter = 0u  },  /* 11 */
     {  "All guys who are",       "virgins drink 2x",         "and best of luck!", .counter = 0u  },  /* 12 */
     {  "Guys who have",          "kissed a dude",            "drink vodka!"    ,  .counter = 0u  },  /* 13 */
     {  "Guys must",              "say meow",                 "after drinking!" ,  .counter = 0u  },  /* 14 */
     {  "Guys that have",         "wrestled with",            "Sass drink vodka",  .counter = 0u  },  /* 15 */
     {  "Guys must do",           "10 squats",                "before drinking" ,  .counter = 0u  },  /* 16 */
     {  "Guys that love",         "heavy metal",              "drink vodka"     ,  .counter = 0u  },  /* 17 */
};


Private Task_T priv_TextArrayAlumniLevel2[] =
{
     { "Alumni who lost", 	"virginity in BEST event", 	"drink 3x " 	, 		.counter = 0u },
     { "All drunk alumni",	"lost 1 item", 		      	"of clothing " 	, 		.counter = 0u },
     { "Alumni who were", 	"in GA Estonian", 			"Evening drink 3x ",	.counter = 0u },
	 { "All alumni who", 	"have been to Moscow", 		"drink 2x", 			.counter = 0u },
	 { "Alumni who have", 	"done a Death Round", 		"skip this round!", 	.counter = 0u },
	 { "Alumni who have",   "been senors at",           "Cantus, drink 3x",     .counter = 0u },
     { "Alumni with most",  "flags & numbers", 		 	"drinks 3x", 			.counter = 0u },
	 { "Alumni drink 1x",   "for every coreteam",       "they have been in.",   .counter = 0u },
     { "All alumni that",   "have children", 			"drink 1 vodka shot", 	.counter = 0u },
	 { "One alumni", 		"must tell ", 				"a dirty joke", 		.counter = 0u },
};

Private Task_T priv_TextArrayCoreTeamLevel2[] =
{
     { "Placeholder", "Coreteam 1", "Level 2 " , .counter = 0u },
     { "Placeholder", "Coreteam 2", "Level 2 " , .counter = 0u },
     { "Placeholder", "Coreteam 3", "Level 2 " , .counter = 0u },
};

Private Task_T priv_TextArraySocRespLevel2[] =
{
     { "Placeholder", "Socresp 1", "Level 2 " , .counter = 0u },
     { "Placeholder", "Socresp 2", "Level 2 " , .counter = 0u },
     { "Placeholder", "Socresp 3", "Level 2 " , .counter = 0u },
};

Private Task_T priv_TextArrayKtLevel2[] =
{
     { "Placeholder", "Kt 1", "Level 2 " , .counter = 0u },
     { "Placeholder", "Kt 2", "Level 2 " , .counter = 0u },
     { "Placeholder", "Kt 3", "Level 2 " , .counter = 0u },
};


Private Task_T priv_TextArrayPaxLevel2[] =
{
     { "Placeholder", "Pax 1", "Level 2 " , .counter = 0u },
     { "Placeholder", "Pax 2", "Level 2 " , .counter = 0u },
     { "Placeholder", "Pax 3", "Level 2 " , .counter = 0u },
};

Private Task_T priv_TextArrayBoardLevel2[] =
{
     { "All boardies", "lose 1 item", "of clothing " , .counter = 0u },
     { "Placeholder", "Board 2", "Level 2 " , .counter = 0u },
     { "Placeholder", "Board 3", "Level 2 " , .counter = 0u },
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
     {  "Girls do"             ,   "bodyshots!!!",                     NULL    , .counter = 0u  }, /* 7  */
     {  "All girls must"       ,   "kiss Sass"  ,            "on the cheek"    , .counter = 0u  }, /* 8  */
     {  "Girl showing the"     ,   "least cleavage"  ,           "drinks 3x"   , .counter = 0u  }, /* 9  */
     {  "The last girl"        ,   "to finish shot"  ,    "loses 1 clothing"   , .counter = 0u  }, /* 10 */
     {  "Girl showing the"     ,   "most cleavage"  ,           "drinks 3x"    , .counter = 0u  }, /* 11 */
     {  "Girls can"            ,   "slap one of"  ,              "the guys"    , .counter = 0u  }, /* 12 */
     {  "Girls can"            ,   "slap Sass"  ,                      NULL    , .counter = 0u  }, /* 13 */
     {  "Girls- 1 shot for"    ,  "each guy they slept"  , "with this year"    , .counter = 0u  }, /* 14 */
     {  "All girls with"       ,  "black underwear"      , "drink vodka"       , .counter = 0u  }, /* 15 */
     { " Girls must make"      ,  "a naughty"   ,                "toast"       , .counter = 0u  }, /* 16 */
     { " Girls must make"      ,  "a kinky"   ,                  "toast"       , .counter = 0u  }, /* 17 */
     {  "All girls must",         "drink under",         "the table"           , .counter = 0u  }, /* 18 */
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
     {  "Guys do"              , "bodyshots!!!",                     NULL    , .counter = 0u  }, /* 7  */
     {  "All Guys"             , "with a boner",          "drink 3x"         , .counter = 0u  }, /* 8  */
     {  "The last guy"         , "to finish shot"  ,    "loses 2 clothing"   , .counter = 0u  }, /* 9  */
     {  "All guys drink"       , "while doing"     ,    "pushups"            , .counter = 0u  }, /* 10 */
     {  "Sass must"            , "sing a song"     ,    "during nxt round"   , .counter = 0u  }, /* 11 */
     {  "1 guy must",            "drink while"     ,    "upside down"        , .counter = 0u  }, /* 12 */
     {  "For each inch",         "of his D length",     "guys drink 1 shot"  , .counter = 0u  }, /* 13 */
     {  "All guys that",         "saw boobs today",     "drink vodka"        , .counter = 0u  }, /* 14 */
     {  "All guys lose",         "their shirts",        NULL                 , .counter = 0u  }, /* 15 */
     {  "All guys must",         "drink under",         "the table"          , .counter = 0u  }, /* 16 */
     {  "Army guys",             "do 20 pushups",       NULL                 , .counter = 0u  }, /* 17 */
};


Private Task_T priv_TextArrayAlumniLevel3[] =
{
     { "Alumni with", 		"biggest boobs", 	"drinks 3x " , 		.counter = 0u },
     { "Everybody that"	  , "has met Kark", 	"drinks vodka " , 	.counter = 0u },
     { "Placeholder", "Alumni 3", "Level 3 " , .counter = 0u },
};

Private Task_T priv_TextArrayCoreTeamLevel3[] =
{
     { "Placeholder", "Coreteam 1", "Level 3 " , .counter = 0u },
     { "Placeholder", "Coreteam 2", "Level 3 " , .counter = 0u },
     { "Placeholder", "Coreteam 3", "Level 3 " , .counter = 0u },
};

Private Task_T priv_TextArraySocRespLevel3[] =
{
     { "Placeholder", "Socresp 1", "Level 3 " , .counter = 0u },
     { "Placeholder", "Socresp 2", "Level 3 " , .counter = 0u },
     { "Placeholder", "Socresp 3", "Level 3 " , .counter = 0u },
};

Private Task_T priv_TextArrayKtLevel3[] =
{
     { "Placeholder", "Kt 1", "Level 3 " , .counter = 0u },
     { "Placeholder", "Kt 2", "Level 3 " , .counter = 0u },
     { "Placeholder", "Kt 3", "Level 3 " , .counter = 0u },
};


Private Task_T priv_TextArrayPaxLevel3[] =
{
     { "Placeholder", "Pax 1", "Level 3 " , .counter = 0u },
     { "Placeholder", "Pax 2", "Level 3 " , .counter = 0u },
     { "Placeholder", "Pax 3", "Level 3 " , .counter = 0u },
};

Private Task_T priv_TextArrayBoardLevel3[] =
{
     { "Placeholder", "Board 1", "Level 3 " , .counter = 0u },
     { "Placeholder", "Board 2", "Level 3 " , .counter = 0u },
     { "Placeholder", "Board 3", "Level 3 " , .counter = 0u },
};

/* Hardcore tasks. -> Full Sass mode. */
Private Task_T priv_TextArrayGirlsLevel4[] =
{
     { "1 girl must",       "do a lapdance"       , "to Sass"           , .counter = 0u  }, /*  1  */
     { "All girls",         "lose 1 item"         , "of clothing"       , .counter = 0u  }, /*  2  */
     { "2 girls",           "make out"            , "or drink 3x"       , .counter = 0u  }, /*  3  */
     { "Vodka round!!!" ,   "for girls!!!"        , NULL                , .counter = 0u  }, /*  4  */
     { "All clean",         "shaven girls"        , "drink 1x"          , .counter = 0u  }, /*  5  */
     { "All girls",         "who masturbated"     , "today drink 2x"    , .counter = 0u  }, /*  6  */
     { "Girls must",        "fake an orgasm"      , "or drink 3x"       , .counter = 0u  }, /*  7  */
     { "Girls must",        "take off shirt"      , "or drink 3x"       , .counter = 0u  }, /*  8  */
     { "Girls  sit",        "on guy's laps"       , "for next round"    , .counter = 0u  }, /*  9  */
     { "Everybody who",     "isnt't wearing a"    , "bra drinks vodka"  , .counter = 0u  }, /*  10 */
     { "Guys give a",       "dare for girls"      , "to do next round"  , .counter = 0u  }, /*  12 */
     { "2 Girls must",      "French-kiss"         , "or drink vodka"    , .counter = 0u  }, /*  13 */
     { "Girls who swallow", "drink 1x, others"    , "drink vodka"       , .counter = 0u  }, /*  14 */
     { "Girls must",        "take off bra or"     , "drink vodka"       , .counter = 0u  }, /*  15 */
     { "All girls squeeze", "their boobs"         , "while drinking"    , .counter = 0u  }, /*  16 */ /* God I love this machine :D */
     { "Girl wearing the"  ,"most clothes"        , "drinks 3x"         , .counter = 0u  },  /* 17 */
     { "Only the girl"     ,"with the biggest"    , "boobs drinks"      , .counter = 0u  },  /* 18 */
     { "Girls must make"   ,"a toast with"   ,      "a kinky voice"     , .counter = 0u  },  /* 19 */
     { "Girls that still"  ,"have shirts on" ,      "must drink 3x"     , .counter = 0u  },  /* 20 */
};

/* Hardcore tasks. -> Full Sass mode. */
Private Task_T priv_TextArrayGuysLevel4[] =
{
     { "Sass must do",     "a lapdance"        ,   "to a girl"         , .counter = 0u  },  /*  1  */
     { "Sass must do",     "a lapdance"        ,   "to a guy"          , .counter = 0u  },  /*  2  */
     { "2 guys",           "make out"          ,   "or drink 3x"       , .counter = 0u  },  /*  3  */
     { "Vodka round!!!" ,  "for girls!!!"      ,   NULL                , .counter = 0u  },  /*  4  */
     { "Sass loses",       "3 items"           ,   "of clothing"       , .counter = 0u  },  /*  5  */
     { "All guys",         "who wanked"        ,   "today drink 2x"    , .counter = 0u  },  /*  6  */
     { "Guys must",        "fake an orgasm"    ,   "or drink vodka"    , .counter = 0u  },  /*  7  */
     { "Guys must name",    "10 sex positions" ,   "or drink vodka"    , .counter = 0u  },  /*  8  */
     { "Sass gets handjob", "Just kidding!!!"  ,   "He drinks 3x"      , .counter = 0u  },  /*  9  */
     { "Next round guys",   "do bodyshots"     ,   "from the girls"    , .counter = 0u  },  /*  10 */
     { "Girls give a",     "dare for guys"     ,   "to do next round"  , .counter = 0u  },  /*  11 */
     { "All guys",         "get a spanking"    ,   "from the girls"    , .counter = 0u  },  /*  12 */
     { "Sass",             "gets a spanking"   ,   "from the girls"    , .counter = 0u  },  /*  13 */
     { "Guy who is",       "most wasted"       ,   "does a vodka"      , .counter = 0u  },  /*  14 */
     { "All guys",         "lose"              ,   "their pants"       , .counter = 0u  },  /*  15 */
     { "Guys",             "drink without"     ,   "using hands"       , .counter = 0u  },  /*  16 */
     { "Guys that haven't","eaten pussy in 30"    ,"days drink vodka"  , .counter = 0u  },  /*  17 */
     { "Guy wearing the"  ,"least clothes"        ,"drinks 3x"         , .counter = 0u  },  /*  18 */
     { "Sass does"        ,"20 pushups"           ,NULL                , .counter = 0u  },  /*  19 */
     { "2 guys must"      ,"make out or"          ,"drink vodka"       , .counter = 0u  },  /*  20 */
};


Private Task_T priv_TextArrayAlumniLevel4[] =
{
     { "Placeholder", "Alumni 1", "Level 4 " , .counter = 0u },
     { "Placeholder", "Alumni 2", "Level 4 " , .counter = 0u },
     { "Placeholder", "Alumni 3", "Level 4 " , .counter = 0u },
};

Private Task_T priv_TextArrayCoreTeamLevel4[] =
{
     { "Placeholder", "Coreteam 1", "Level 4 " , .counter = 0u },
     { "Placeholder", "Coreteam 2", "Level 4 " , .counter = 0u },
     { "Placeholder", "Coreteam 3", "Level 4 " , .counter = 0u },
};

Private Task_T priv_TextArraySocRespLevel4[] =
{
     { "Placeholder", "Socresp 1", "Level 4 " , .counter = 0u },
     { "Placeholder", "Socresp 2", "Level 4 " , .counter = 0u },
     { "Placeholder", "Socresp 3", "Level 4 " , .counter = 0u },
};

Private Task_T priv_TextArrayKtLevel4[] =
{
     { "Placeholder", "Kt 1", "Level 4 " , .counter = 0u },
     { "Placeholder", "Kt 2", "Level 4 " , .counter = 0u },
     { "Placeholder", "Kt 3", "Level 4 " , .counter = 0u },
};


Private Task_T priv_TextArrayPaxLevel4[] =
{
     { "Placeholder", "Pax 1", "Level 4 " , .counter = 0u },
     { "Placeholder", "Pax 2", "Level 4 " , .counter = 0u },
     { "Placeholder", "Pax 3", "Level 4 " , .counter = 0u },
};

Private Task_T priv_TextArrayBoardLevel4[] =
{
     { "Placeholder", "Board 1", "Level 4 " , .counter = 0u },
     { "Placeholder", "Board 2", "Level 4 " , .counter = 0u },
     { "Placeholder", "Board 3", "Level 4 " , .counter = 0u },
};

/*****************************************************************/

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
    static U8 test_counter = 0u;

    if (sec == 0u)
    {
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


Public Boolean alumniSpecialTask(U8 sec)
{
    return SpecialTaskWithRandomText(sec, TASK_FOR_ALUMNI);
}


Public Boolean boardSpecialTask(U8 sec)
{
    return SpecialTaskWithRandomText(sec, TASK_FOR_BOARD);
}


Public Boolean socRespSpecialTask(U8 sec)
{
    return SpecialTaskWithRandomText(sec, TASK_FOR_SOCRESP);
}


Public Boolean KtSpecialTask(U8 sec)
{
    return SpecialTaskWithRandomText(sec, TASK_FOR_KT);
}


Public Boolean PaxSpecialTask(U8 sec)
{
    return SpecialTaskWithRandomText(sec, TASK_FOR_PAX);
}


Public Boolean coreTeamSpecialTask(U8 sec)
{
    return SpecialTaskWithRandomText(sec, TASK_FOR_CORETEAM);
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
    Task_T * alumniSelectedArray;
    Task_T * boardSelectedArray;
    Task_T * ktSelectedArray;
    Task_T * paxSelectedArray;
    Task_T * SocRespSelectedArray;
    Task_T * coreTeamSelectedArray;

    U16 number_of_items_girls = 0u;
    U16 number_of_items_guys = 0u;
    U16 number_of_items_alumni = 0u;
    U16 number_of_items_board = 0u;
    U16 number_of_items_kt = 0u;
    U16 number_of_items_pax = 0u;
    U16 number_of_items_socresp = 0u;
    U16 number_of_items_coreteam = 0u;

    switch(taskLevel)
    {
        case 0:
            girlsSelectedArray = priv_TextArrayGirlsLevel1;
            number_of_items_girls = NUMBER_OF_ITEMS(priv_TextArrayGirlsLevel1);

            guysSelectedArray = priv_TextArrayGuysLevel1;
            number_of_items_guys = NUMBER_OF_ITEMS(priv_TextArrayGuysLevel1);

            alumniSelectedArray = priv_TextArrayAlumniLevel1;
            number_of_items_alumni = NUMBER_OF_ITEMS(priv_TextArrayAlumniLevel1);

            boardSelectedArray = priv_TextArrayBoardLevel1;
            number_of_items_board = NUMBER_OF_ITEMS(priv_TextArrayBoardLevel1);

            ktSelectedArray = priv_TextArrayKtLevel1;
            number_of_items_kt = NUMBER_OF_ITEMS(priv_TextArrayKtLevel1);

            paxSelectedArray = priv_TextArrayPaxLevel1;
            number_of_items_pax = NUMBER_OF_ITEMS(priv_TextArrayPaxLevel1);

            SocRespSelectedArray = priv_TextArraySocRespLevel1;
            number_of_items_socresp = NUMBER_OF_ITEMS(priv_TextArraySocRespLevel1);

            coreTeamSelectedArray = priv_TextArrayCoreTeamLevel1;
            number_of_items_coreteam = NUMBER_OF_ITEMS(priv_TextArrayCoreTeamLevel1);
            break;
        case 1:
            girlsSelectedArray = priv_TextArrayGirlsLevel2;
            number_of_items_girls = NUMBER_OF_ITEMS(priv_TextArrayGirlsLevel2);

            guysSelectedArray = priv_TextArrayGuysLevel2;
            number_of_items_guys = NUMBER_OF_ITEMS(priv_TextArrayGuysLevel2);

            alumniSelectedArray = priv_TextArrayAlumniLevel2;
            number_of_items_alumni = NUMBER_OF_ITEMS(priv_TextArrayAlumniLevel2);

            boardSelectedArray = priv_TextArrayBoardLevel2;
            number_of_items_board = NUMBER_OF_ITEMS(priv_TextArrayBoardLevel2);

            ktSelectedArray = priv_TextArrayKtLevel2;
            number_of_items_kt = NUMBER_OF_ITEMS(priv_TextArrayKtLevel2);

            paxSelectedArray = priv_TextArrayPaxLevel2;
            number_of_items_pax = NUMBER_OF_ITEMS(priv_TextArrayPaxLevel2);

            SocRespSelectedArray = priv_TextArraySocRespLevel2;
            number_of_items_socresp = NUMBER_OF_ITEMS(priv_TextArraySocRespLevel2);

            coreTeamSelectedArray = priv_TextArrayCoreTeamLevel2;
            number_of_items_coreteam = NUMBER_OF_ITEMS(priv_TextArrayCoreTeamLevel2);
            break;
        case 2:
            girlsSelectedArray = priv_TextArrayGirlsLevel3;
            number_of_items_girls = NUMBER_OF_ITEMS(priv_TextArrayGirlsLevel3);

            guysSelectedArray = priv_TextArrayGuysLevel3;
            number_of_items_guys = NUMBER_OF_ITEMS(priv_TextArrayGuysLevel3);

            alumniSelectedArray = priv_TextArrayAlumniLevel3;
            number_of_items_alumni = NUMBER_OF_ITEMS(priv_TextArrayAlumniLevel3);

            boardSelectedArray = priv_TextArrayBoardLevel3;
            number_of_items_board = NUMBER_OF_ITEMS(priv_TextArrayBoardLevel3);

            ktSelectedArray = priv_TextArrayKtLevel3;
            number_of_items_kt = NUMBER_OF_ITEMS(priv_TextArrayKtLevel3);

            paxSelectedArray = priv_TextArrayPaxLevel3;
            number_of_items_pax = NUMBER_OF_ITEMS(priv_TextArrayPaxLevel3);

            SocRespSelectedArray = priv_TextArraySocRespLevel3;
            number_of_items_socresp = NUMBER_OF_ITEMS(priv_TextArraySocRespLevel3);

            coreTeamSelectedArray = priv_TextArrayCoreTeamLevel3;
            number_of_items_coreteam = NUMBER_OF_ITEMS(priv_TextArrayCoreTeamLevel3);
            break;
        case 3:
            girlsSelectedArray = priv_TextArrayGirlsLevel4;
            number_of_items_girls = NUMBER_OF_ITEMS(priv_TextArrayGirlsLevel4);

            guysSelectedArray = priv_TextArrayGuysLevel4;
            number_of_items_guys = NUMBER_OF_ITEMS(priv_TextArrayGuysLevel4);

            alumniSelectedArray = priv_TextArrayAlumniLevel4;
            number_of_items_alumni = NUMBER_OF_ITEMS(priv_TextArrayAlumniLevel4);

            boardSelectedArray = priv_TextArrayBoardLevel4;
            number_of_items_board = NUMBER_OF_ITEMS(priv_TextArrayBoardLevel4);

            ktSelectedArray = priv_TextArrayKtLevel4;
            number_of_items_kt = NUMBER_OF_ITEMS(priv_TextArrayKtLevel4);

            paxSelectedArray = priv_TextArrayPaxLevel4;
            number_of_items_pax = NUMBER_OF_ITEMS(priv_TextArrayPaxLevel4);

            SocRespSelectedArray = priv_TextArraySocRespLevel4;
            number_of_items_socresp = NUMBER_OF_ITEMS(priv_TextArraySocRespLevel4);

            coreTeamSelectedArray = priv_TextArrayCoreTeamLevel4;
            number_of_items_coreteam = NUMBER_OF_ITEMS(priv_TextArrayCoreTeamLevel4);
            break;
        default:
            /* Should not happen. */
            girlsSelectedArray = priv_TextArrayGirlsLevel1;
            number_of_items_girls = NUMBER_OF_ITEMS(priv_TextArrayGirlsLevel1);

            guysSelectedArray = priv_TextArrayGuysLevel1;
            number_of_items_guys = NUMBER_OF_ITEMS(priv_TextArrayGuysLevel1);

            alumniSelectedArray = priv_TextArrayAlumniLevel1;
            number_of_items_alumni = NUMBER_OF_ITEMS(priv_TextArrayAlumniLevel1);

            boardSelectedArray = priv_TextArrayBoardLevel1;
            number_of_items_board = NUMBER_OF_ITEMS(priv_TextArrayBoardLevel1);

            ktSelectedArray = priv_TextArrayKtLevel1;
            number_of_items_kt = NUMBER_OF_ITEMS(priv_TextArrayKtLevel1);

            paxSelectedArray = priv_TextArrayPaxLevel1;
            number_of_items_pax = NUMBER_OF_ITEMS(priv_TextArrayPaxLevel1);

            SocRespSelectedArray = priv_TextArraySocRespLevel1;
            number_of_items_socresp = NUMBER_OF_ITEMS(priv_TextArraySocRespLevel1);

            coreTeamSelectedArray = priv_TextArrayCoreTeamLevel1;
            number_of_items_coreteam = NUMBER_OF_ITEMS(priv_TextArrayCoreTeamLevel1);
            break;
    }

    switch(sec)
    {
        case(1u):
            {
               switch(type)
               {
                   case TASK_FOR_GIRLS:
                       {
                           priv_task_str_ptr = getRandomTaskFromArray(girlsSelectedArray, number_of_items_girls);
                       }
                       break;
                   case TASK_FOR_GUYS:
                       {
                           priv_task_str_ptr = getRandomTaskFromArray(guysSelectedArray, number_of_items_guys);
                       }
                       break;
                   case TASK_FOR_ALUMNI:
                       {
                           priv_task_str_ptr = getRandomTaskFromArray(alumniSelectedArray, number_of_items_alumni);
                       }
                       break;
                   case TASK_FOR_BOARD:
                       {
                           priv_task_str_ptr = getRandomTaskFromArray(boardSelectedArray, number_of_items_board);
                       }
                       break;
                   case TASK_FOR_KT:
                       {
                           priv_task_str_ptr = getRandomTaskFromArray(ktSelectedArray, number_of_items_kt);
                       }
                       break;
                   case TASK_FOR_SOCRESP:
                       {
                           priv_task_str_ptr = getRandomTaskFromArray(SocRespSelectedArray, number_of_items_socresp);
                       }
                       break;
                   case TASK_FOR_PAX:
                       {
                           priv_task_str_ptr = getRandomTaskFromArray(paxSelectedArray, number_of_items_pax);
                       }
                       break;
                   case TASK_FOR_CORETEAM:
                       {
                           priv_task_str_ptr = getRandomTaskFromArray(coreTeamSelectedArray, number_of_items_coreteam);
                       }
                       break;
                   default:
                       /* Should not happen. */
                       break;
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

