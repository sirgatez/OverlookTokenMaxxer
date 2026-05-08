#include <Keyboard.h>
#include <Mouse.h>
#include <avr/pgmspace.h>

/*
  Overlook Winter Caretaker

  A harmless USB HID demo for ATmega32U4 boards such as Arduino Leonardo,
  Micro, or USB Beetle. It types response-blind notes into a local chat window
  as Jack tries to keep working on his book while caretaking an empty mansion.

  Jack cannot read the response. The sketch guesses what might have been said,
  then continues the conversation while sliding between sane, concerned,
  paranoid, manic, and recovered states.

  Safety constraints:
  - No terminal interaction
  - No app launching
  - No modifier-key shortcuts
  - No command strings
  - Mouse clicks are disabled by default
*/

const unsigned long STARTUP_DELAY_MS = 15000UL;
const bool USE_SHORT_DEMO_TIMING = true;

// true: type one note and press Return, suitable for a local chat input.
// false: type a visible transcript into a blank editor.
const bool SUBMIT_PROMPTS_TO_CHAT = true;
const bool TYPE_VISIBLE_SPEAKER_PREFIX = false;
const bool ENABLE_MOUSE_BEHAVIOR = true;
const bool ENABLE_MOUSE_CLICKS = false;

const int LINE_BUFFER_SIZE = 180;

enum MadnessMode {
  SANE,
  MEASURING,
  CONCERNED,
  PARANOID,
  MANIC,
  RECOVERY
};

struct ModeConfig {
  unsigned long minDurationMs;
  unsigned long maxDurationMs;

  int minCharDelayMs;
  int maxCharDelayMs;

  int typoChancePercent;
  int correctionChancePercent;
  int randomBackspaceChancePercent;

  unsigned long minPromptPauseMs;
  unsigned long maxPromptPauseMs;

  int longPauseChancePercent;
  unsigned long minLongPauseMs;
  unsigned long maxLongPauseMs;

  int walkAwayChancePercent;
  unsigned long minWalkAwayMs;
  unsigned long maxWalkAwayMs;

  int punctuationMutationChancePercent;
  int allCapsChancePercent;
  int lowercaseChancePercent;
  int titleCaseChancePercent;
  int mixedCapsChancePercent;
  int fragmentedLineChancePercent;
  int wordRepeatChancePercent;
  int indentationChancePercent;
  int charTransposeChancePercent;
  int wordTransposeChancePercent;
  int burstTypingChancePercent;

  unsigned long mouseMoveIntervalMinMs;
  unsigned long mouseMoveIntervalMaxMs;
  int mouseDistanceMin;
  int mouseDistanceMax;
  int clickChancePercent;
  int scrollChancePercent;
};

const char sanePrompt0[] PROGMEM = "The hotel is quiet tonight. I finally have time to work on the book.";
const char sanePrompt1[] PROGMEM = "The boiler behaves. The roof holds. The pages are blank.";
const char sanePrompt2[] PROGMEM = "The silence may help the writing, once I get used to it.";
const char sanePrompt3[] PROGMEM = "The off season has a discipline to it. No guests, no interruptions.";
const char sanePrompt4[] PROGMEM = "I checked the west wing. Just old pipes settling.";
const char sanePrompt5[] PROGMEM = "You might be right, perhaps I am working too hard.";
const char sanePrompt6[] PROGMEM = "Maybe a walk in the maze would ease my mind.";
const char sanePrompt7[] PROGMEM = "I missed that. I assume you meant I should keep a schedule.";
const char sanePrompt10[] PROGMEM = "The drive is clean with snow. No tracks since mine.";
const char sanePrompt13[] PROGMEM = "The book will come easier when the place feels less borrowed.";
const char sanePrompt14[] PROGMEM = "I checked {ROOM} after {CHORE}; the {THING} was where it belonged.";
const char sanePrompt15[] PROGMEM = "The {SOUND} near {ROOM} was probably only {WEATHER}.";
const char sanePrompt16[] PROGMEM = "I wrote about {ROOM}. It may become a chapter.";
const char sanePrompt17[] PROGMEM = "The {THING} belongs in {ROOM}. I wrote that down.";
const char sanePrompt18[] PROGMEM = "Nothing changed in {ROOM}, unless I am becoming too attentive.";
const char sanePrompt19[] PROGMEM = "After {CHORE}, I should write for one quiet hour.";

const char* const sanePrompts[] PROGMEM = {
  sanePrompt0, sanePrompt1, sanePrompt2, sanePrompt3,
  sanePrompt4, sanePrompt5, sanePrompt6, sanePrompt7,
  sanePrompt10, sanePrompt13, sanePrompt14, sanePrompt15,
  sanePrompt16, sanePrompt17, sanePrompt18, sanePrompt19
};

const char measuringPrompt0[] PROGMEM = "I wrote {PAGES} pages, if margin notes count.";
const char measuringPrompt1[] PROGMEM = "The room felt smaller after lunch. That cannot be right.";
const char measuringPrompt2[] PROGMEM = "If I check the boiler twice, is that diligence or nerves?";
const char measuringPrompt3[] PROGMEM = "You could be right, but how can I know for sure?";
const char measuringPrompt4[] PROGMEM = "The book needs order. The hotel needs order.";
const char measuringPrompt5[] PROGMEM = "I counted {PAGES} pages and only one good sentence.";
const char measuringPrompt6[] PROGMEM = "Maybe the lights make the halls look longer than they are.";
const char measuringPrompt7[] PROGMEM = "Was your reply about rest, or locking the pantry?";
const char measuringPrompt8[] PROGMEM = "I made three rounds and still forgot room 217.";
const char measuringPrompt12[] PROGMEM = "There are {PAGES} unused rooms. That should comfort me.";
const char measuringPrompt14[] PROGMEM = "I checked {ROOM}, then {OTHER_ROOM}, then {ROOM} again.";
const char measuringPrompt15[] PROGMEM = "{CHORE} took {PAGES} minutes longer than it should have.";
const char measuringPrompt16[] PROGMEM = "I moved the {THING} from {ROOM} to {OTHER_ROOM}. Why?";
const char measuringPrompt18[] PROGMEM = "{PAGES} ways to describe {SIGN}. None fit the book.";
const char measuringPrompt19[] PROGMEM = "If {ROOM} is locked, {OTHER_ROOM} should be quiet.";

const char* const measuringPrompts[] PROGMEM = {
  measuringPrompt0, measuringPrompt1, measuringPrompt2, measuringPrompt3,
  measuringPrompt4, measuringPrompt5, measuringPrompt6, measuringPrompt7,
  measuringPrompt8, measuringPrompt12, measuringPrompt14, measuringPrompt15,
  measuringPrompt16, measuringPrompt18, measuringPrompt19
};

const char concernedPrompt0[] PROGMEM = "There was music in the ballroom. It stopped at the door.";
const char concernedPrompt1[] PROGMEM = "The draft is blank, but I find words I do not remember.";
const char concernedPrompt2[] PROGMEM = "I describe the rooms so I cannot miss the obvious.";
const char concernedPrompt3[] PROGMEM = "Maybe the maze has an exit if I describe every wall.";
const char concernedPrompt4[] PROGMEM = "Do you think old places remember who stays in them?";
const char concernedPrompt6[] PROGMEM = "What if the work is not stalled? What if it is waiting?";
const char concernedPrompt7[] PROGMEM = "Say it plainly. I need the room to stop expanding.";
const char concernedPrompt8[] PROGMEM = "There was a wet footprint by the unused elevator.";
const char concernedPrompt9[] PROGMEM = "The pantry door was open again. I am sure I latched it.";
const char concernedPrompt10[] PROGMEM = "I heard laughter in the pipes. Ordinary, I suppose.";
const char concernedPrompt11[] PROGMEM = "The topiary looked closer to the path today.";
const char concernedPrompt13[] PROGMEM = "I found a sentence on the page before I sat down.";
const char concernedPrompt14[] PROGMEM = "Please say old buildings make sounds like voices.";
const char concernedPrompt15[] PROGMEM = "The {THING} should be in {ROOM}, not near {OTHER_ROOM}.";
const char concernedPrompt16[] PROGMEM = "I heard {SOUND} behind {ROOM}, but the door was latched.";
const char concernedPrompt17[] PROGMEM = "{SIGN} is easier to explain before sundown.";
const char concernedPrompt18[] PROGMEM = "I know {FEAR} is absurd, but it arrived fully formed.";

const char* const concernedPrompts[] PROGMEM = {
  concernedPrompt0, concernedPrompt1, concernedPrompt2, concernedPrompt3,
  concernedPrompt4, concernedPrompt6, concernedPrompt7,
  concernedPrompt8, concernedPrompt9, concernedPrompt10, concernedPrompt11,
  concernedPrompt13, concernedPrompt14, concernedPrompt15,
  concernedPrompt16, concernedPrompt17, concernedPrompt18
};

const char paranoidPrompt0[] PROGMEM = "The typewriter moved before I touched it.";
const char paranoidPrompt1[] PROGMEM = "They will ask if I finished. Not what the house did.";
const char paranoidPrompt2[] PROGMEM = "I can write another page. I can write another page.";
const char paranoidPrompt3[] PROGMEM = "You cannot see the maze, but you keep telling me where to turn.";
const char paranoidPrompt4[] PROGMEM = "I am not wasting time. The pages prove I was here.";
const char paranoidPrompt5[] PROGMEM = "I cannot read your answer. It came from somewhere warm.";
const char paranoidPrompt6[] PROGMEM = "Maybe if I fill every page, the doubt has nowhere to stand.";
const char paranoidPrompt7[] PROGMEM = "Every hallway is shorter before I stop trusting it.";
const char paranoidPrompt8[] PROGMEM = "The hotel knows when I stop looking.";
const char paranoidPrompt9[] PROGMEM = "Someone is correcting me from inside the walls.";
const char paranoidPrompt12[] PROGMEM = "The book is not mine when I read it aloud.";
const char paranoidPrompt13[] PROGMEM = "The elevator smells like rain and old flowers.";
const char paranoidPrompt14[] PROGMEM = "I keep finding proof after I decide not to look for it.";
const char paranoidPrompt15[] PROGMEM = "{ROOM} has moved into {OTHER_ROOM}.";
const char paranoidPrompt16[] PROGMEM = "The {THING} is listening from {ROOM}.";
const char paranoidPrompt17[] PROGMEM = "{SIGN} was waiting for me, not left by me.";
const char paranoidPrompt18[] PROGMEM = "You knew about {FEAR} before I typed it.";
const char paranoidPrompt19[] PROGMEM = "The {SOUND} keeps returning to {ROOM} when I look away.";

const char* const paranoidPrompts[] PROGMEM = {
  paranoidPrompt0, paranoidPrompt1, paranoidPrompt2, paranoidPrompt3,
  paranoidPrompt4, paranoidPrompt5, paranoidPrompt6, paranoidPrompt7,
  paranoidPrompt8, paranoidPrompt9, paranoidPrompt12, paranoidPrompt13,
  paranoidPrompt14, paranoidPrompt15, paranoidPrompt16, paranoidPrompt17,
  paranoidPrompt18, paranoidPrompt19
};

const char manicPrompt1[] PROGMEM = "ALL WORK AND NO REST MAKES JACK A DULL BOY.";
const char manicPrompt2[] PROGMEM = "All work and no play makes Jack a dull boy.";
const char manicPrompt3[] PROGMEM = "All fun and no play make Jack a dull boy.";
const char manicPrompt4[] PROGMEM = "The mirror is not a window. The mirror is not a window.";
const char manicPrompt6[] PROGMEM = "Why is the ballroom warm with every register closed?";
const char manicPrompt7[] PROGMEM = "The book is working. The book is working. The book is working.";
const char manicPrompt8[] PROGMEM = "ALL WORK ALL WINTER ALL JACK ALL WORK ALL WINTER ALL JACK";
const char manicPrompt9[] PROGMEM = "No play and all work makes Jack a dull boy.";
const char manicPrompt10[] PROGMEM = "All work makes Jack. No play makes Jack. Jack makes Jack.";
const char manicPrompt11[] PROGMEM = "The pages are full and the pages are blank and the pages are full.";
const char manicPrompt12[] PROGMEM = "I saw myself in the mirror and he was already typing.";
const char manicPrompt13[] PROGMEM = "THE HOTEL IS CLOSED THE HOTEL IS OPEN THE HOTEL IS CLOSED";
const char manicPrompt14[] PROGMEM = "The room corrected me. The room corrected me.";
const char manicPrompt15[] PROGMEM = "All work and no play makes Jack a dull boy all work and no play.";
const char manicPrompt16[] PROGMEM = "{ROOM} IS {OTHER_ROOM} AND {OTHER_ROOM} IS {ROOM}";
const char manicPrompt17[] PROGMEM = "{THING} {THING} {THING} IN {ROOM} IN {ROOM} IN {ROOM}";
const char manicPrompt19[] PROGMEM = "ALL {CHORE} AND NO PLAY MAKES JACK A DULL BOY.";
const char manicPrompt20[] PROGMEM = "{FEAR}. {FEAR}. {FEAR}.";

const char* const manicPrompts[] PROGMEM = {
  manicPrompt1, manicPrompt2, manicPrompt3,
  manicPrompt4, manicPrompt6, manicPrompt7,
  manicPrompt8, manicPrompt9, manicPrompt10, manicPrompt11,
  manicPrompt12, manicPrompt13, manicPrompt14, manicPrompt15,
  manicPrompt16, manicPrompt17, manicPrompt19, manicPrompt20
};

const char recoveryPrompt0[] PROGMEM = "You might be right, perhaps I am working too hard.";
const char recoveryPrompt1[] PROGMEM = "Maybe I should write one clean sentence and stop.";
const char recoveryPrompt2[] PROGMEM = "Speak plainly, please. I need to hear myself think.";
const char recoveryPrompt3[] PROGMEM = "I stepped away. The room did not follow me.";
const char recoveryPrompt4[] PROGMEM = "Maybe the draft is only a shadow of the work.";
const char recoveryPrompt6[] PROGMEM = "Maybe a walk in the maze would ease my mind.";
const char recoveryPrompt7[] PROGMEM = "The useful thought might fit in the margin.";
const char recoveryPrompt8[] PROGMEM = "The boiler is only a boiler. The wind is only wind.";
const char recoveryPrompt9[] PROGMEM = "I will make tea, sit down, and write one ordinary page.";
const char recoveryPrompt10[] PROGMEM = "No one is in the ballroom. I checked with the lights on.";
const char recoveryPrompt11[] PROGMEM = "The hotel feels smaller after sleep. That must be simple.";
const char recoveryPrompt12[] PROGMEM = "I can leave the page blank for one minute.";
const char recoveryPrompt13[] PROGMEM = "I am going to count the locks once, not twice.";
const char recoveryPrompt15[] PROGMEM = "The {THING} can stay where it is until morning.";
const char recoveryPrompt16[] PROGMEM = "If I hear {SOUND} again, I will write one sentence.";
const char recoveryPrompt17[] PROGMEM = "{SIGN} may have an ordinary explanation.";
const char recoveryPrompt19[] PROGMEM = "{WEATHER} makes old places theatrical. That is all.";

const char* const recoveryPrompts[] PROGMEM = {
  recoveryPrompt0, recoveryPrompt1, recoveryPrompt2, recoveryPrompt3,
  recoveryPrompt4, recoveryPrompt6, recoveryPrompt7,
  recoveryPrompt8, recoveryPrompt9, recoveryPrompt10, recoveryPrompt11,
  recoveryPrompt12, recoveryPrompt13, recoveryPrompt15,
  recoveryPrompt16, recoveryPrompt17, recoveryPrompt19
};

const char fragment0[] PROGMEM = "more pages";
const char fragment1[] PROGMEM = "no signal";
const char fragment2[] PROGMEM = "tap tap tap";
const char fragment3[] PROGMEM = "the house is hungry";
const char fragment4[] PROGMEM = "all work";
const char fragment5[] PROGMEM = "no play";
const char fragment6[] PROGMEM = "makes Jack";
const char fragment7[] PROGMEM = "a dull boy";
const char fragment8[] PROGMEM = "empty rooms";
const char fragment9[] PROGMEM = "empty pages";
const char fragment10[] PROGMEM = "proof by paper";
const char fragment11[] PROGMEM = "the window is full of snow";
const char fragment12[] PROGMEM = "room 217";
const char fragment13[] PROGMEM = "old music";
const char fragment14[] PROGMEM = "the boiler";
const char fragment15[] PROGMEM = "west wing";
const char fragment16[] PROGMEM = "red carpet";
const char fragment17[] PROGMEM = "locked pantry";
const char fragment18[] PROGMEM = "no guests";
const char fragment19[] PROGMEM = "write it again";
const char fragment20[] PROGMEM = "the maze knows";
const char fragment21[] PROGMEM = "warm ballroom";
const char fragment22[] PROGMEM = "{ROOM}";
const char fragment23[] PROGMEM = "{OTHER_ROOM}";
const char fragment24[] PROGMEM = "{THING}";
const char fragment25[] PROGMEM = "{SOUND}";
const char fragment26[] PROGMEM = "{SIGN}";
const char fragment27[] PROGMEM = "{FEAR}";
const char fragment28[] PROGMEM = "{ROOM} {OTHER_ROOM}";
const char fragment29[] PROGMEM = "{THING} in {ROOM}";

const char* const fragments[] PROGMEM = {
  fragment0, fragment1, fragment2, fragment3, fragment4, fragment5,
  fragment6, fragment7, fragment8, fragment9, fragment10, fragment11,
  fragment12, fragment13, fragment14, fragment15, fragment16, fragment17,
  fragment18, fragment19, fragment20, fragment21, fragment22, fragment23,
  fragment24, fragment25, fragment26, fragment27, fragment28, fragment29
};

const char walkAway0[] PROGMEM = "I am stepping away for a minute. Please keep the room quiet.";
const char walkAway1[] PROGMEM = "Maybe a walk in the maze would ease my mind.";
const char walkAway3[] PROGMEM = "Do not answer yet. Or answer softly. I cannot tell which helps.";
const char walkAway4[] PROGMEM = "I will come back when the hotel stops listening.";
const char walkAway5[] PROGMEM = "I am going to check the boiler and not think about the ballroom.";
const char walkAway7[] PROGMEM = "I need air. The maze is still air, but it is outside.";
const char walkAway8[] PROGMEM = "I am leaving the page face down until the room behaves.";
const char walkAway10[] PROGMEM = "I am going to {ROOM} for {CHORE}. Not {OTHER_ROOM}.";
const char walkAway12[] PROGMEM = "I need to stand somewhere I cannot hear {SOUND}.";
const char walkAway13[] PROGMEM = "I will come back after {WEATHER} settles against the windows.";

const char* const walkAwayPrompts[] PROGMEM = {
  walkAway0, walkAway1, walkAway3, walkAway4,
  walkAway5, walkAway7, walkAway8, walkAway10,
  walkAway12, walkAway13
};

const char room0[] PROGMEM = "the lobby";
const char room1[] PROGMEM = "the ballroom";
const char room2[] PROGMEM = "the west wing";
const char room3[] PROGMEM = "the pantry";
const char room4[] PROGMEM = "the boiler room";
const char room5[] PROGMEM = "room 217";
const char room6[] PROGMEM = "the hedge maze";
const char room7[] PROGMEM = "the service hall";
const char room8[] PROGMEM = "the writing room";
const char room9[] PROGMEM = "the kitchen";
const char room10[] PROGMEM = "the elevator";
const char room11[] PROGMEM = "the north stair";
const char room12[] PROGMEM = "the linen closet";
const char room13[] PROGMEM = "the glass corridor";
const char room14[] PROGMEM = "the red bathroom";
const char room15[] PROGMEM = "the staff quarters";
const char room16[] PROGMEM = "the roof access";
const char room17[] PROGMEM = "the old office";
const char room18[] PROGMEM = "the game room";
const char room19[] PROGMEM = "the topiary path";

const char* const rooms[] PROGMEM = {
  room0, room1, room2, room3, room4, room5, room6, room7, room8, room9,
  room10, room11, room12, room13, room14, room15, room16, room17, room18, room19
};

const char thing0[] PROGMEM = "the typewriter";
const char thing1[] PROGMEM = "the manuscript";
const char thing2[] PROGMEM = "the boiler gauge";
const char thing3[] PROGMEM = "the lobby clock";
const char thing4[] PROGMEM = "the radio";
const char thing5[] PROGMEM = "the pantry latch";
const char thing6[] PROGMEM = "the elevator doors";
const char thing7[] PROGMEM = "the snow shovel";
const char thing8[] PROGMEM = "the room key";
const char thing9[] PROGMEM = "the mirror";
const char thing10[] PROGMEM = "the ledger";
const char thing11[] PROGMEM = "the red carpet";
const char thing12[] PROGMEM = "the brass bell";
const char thing13[] PROGMEM = "the furnace door";
const char thing14[] PROGMEM = "the pencil";
const char thing15[] PROGMEM = "the ashtray";
const char thing16[] PROGMEM = "the snow boots";
const char thing17[] PROGMEM = "the service tray";
const char thing18[] PROGMEM = "the loose doorknob";
const char thing19[] PROGMEM = "the stack of paper";

const char* const things[] PROGMEM = {
  thing0, thing1, thing2, thing3, thing4, thing5, thing6, thing7, thing8, thing9,
  thing10, thing11, thing12, thing13, thing14, thing15, thing16, thing17, thing18, thing19
};

const char chore0[] PROGMEM = "checking the boiler";
const char chore1[] PROGMEM = "winding the lobby clock";
const char chore2[] PROGMEM = "locking the pantry";
const char chore3[] PROGMEM = "clearing snow from the steps";
const char chore4[] PROGMEM = "making the west wing round";
const char chore5[] PROGMEM = "testing the radio";
const char chore6[] PROGMEM = "counting the room keys";
const char chore7[] PROGMEM = "walking the maze path";
const char chore8[] PROGMEM = "checking the roof";
const char chore9[] PROGMEM = "reading the pressure gauge";
const char chore10[] PROGMEM = "closing the ballroom doors";
const char chore11[] PROGMEM = "dusting the writing desk";
const char chore12[] PROGMEM = "checking the linen closets";
const char chore13[] PROGMEM = "turning down the heat";
const char chore14[] PROGMEM = "writing before dinner";
const char chore15[] PROGMEM = "counting the stairs";

const char* const chores[] PROGMEM = {
  chore0, chore1, chore2, chore3, chore4, chore5, chore6, chore7,
  chore8, chore9, chore10, chore11, chore12, chore13, chore14, chore15
};

const char sound0[] PROGMEM = "music";
const char sound1[] PROGMEM = "pipes laughing";
const char sound2[] PROGMEM = "footsteps";
const char sound3[] PROGMEM = "the elevator bell";
const char sound4[] PROGMEM = "glass clinking";
const char sound5[] PROGMEM = "a typewriter key";
const char sound6[] PROGMEM = "someone breathing";
const char sound7[] PROGMEM = "the radio static";
const char sound8[] PROGMEM = "a door closing";
const char sound9[] PROGMEM = "whispering";
const char sound10[] PROGMEM = "a chair scraping";
const char sound11[] PROGMEM = "the boiler knocking";
const char sound12[] PROGMEM = "a party downstairs";
const char sound13[] PROGMEM = "snow against the glass";
const char sound14[] PROGMEM = "my name";
const char sound15[] PROGMEM = "a page turning";

const char* const sounds[] PROGMEM = {
  sound0, sound1, sound2, sound3, sound4, sound5, sound6, sound7,
  sound8, sound9, sound10, sound11, sound12, sound13, sound14, sound15
};

const char sign0[] PROGMEM = "a wet footprint";
const char sign1[] PROGMEM = "a warm chair";
const char sign2[] PROGMEM = "a fresh sentence";
const char sign3[] PROGMEM = "an open door";
const char sign4[] PROGMEM = "a moved key";
const char sign5[] PROGMEM = "a glass with lipstick";
const char sign6[] PROGMEM = "snow on the carpet";
const char sign7[] PROGMEM = "a light under the door";
const char sign8[] PROGMEM = "a second set of tracks";
const char sign9[] PROGMEM = "a page torn cleanly out";
const char sign10[] PROGMEM = "a mirror fogged from inside";
const char sign11[] PROGMEM = "a chair facing the wall";
const char sign12[] PROGMEM = "fresh ashes";
const char sign13[] PROGMEM = "a locked door standing open";
const char sign14[] PROGMEM = "a handprint in dust";
const char sign15[] PROGMEM = "a smell of flowers";

const char* const signs[] PROGMEM = {
  sign0, sign1, sign2, sign3, sign4, sign5, sign6, sign7,
  sign8, sign9, sign10, sign11, sign12, sign13, sign14, sign15
};

const char fear0[] PROGMEM = "the hotel remembers me";
const char fear1[] PROGMEM = "the rooms are changing places";
const char fear2[] PROGMEM = "the book is answering first";
const char fear3[] PROGMEM = "someone is using my hands";
const char fear4[] PROGMEM = "the maze is listening";
const char fear5[] PROGMEM = "the house wants witnesses";
const char fear6[] PROGMEM = "the snow is keeping count";
const char fear7[] PROGMEM = "the boiler knows when I lie";
const char fear8[] PROGMEM = "the mirrors are late";
const char fear9[] PROGMEM = "the doors close after I decide";
const char fear10[] PROGMEM = "the voices are rehearsing me";
const char fear11[] PROGMEM = "the hotel is less empty when I write";

const char* const fears[] PROGMEM = {
  fear0, fear1, fear2, fear3, fear4, fear5,
  fear6, fear7, fear8, fear9, fear10, fear11
};

const char weather0[] PROGMEM = "the wind";
const char weather1[] PROGMEM = "the snow";
const char weather2[] PROGMEM = "the storm";
const char weather3[] PROGMEM = "the cold";
const char weather4[] PROGMEM = "the pressure change";
const char weather5[] PROGMEM = "ice in the gutters";
const char weather6[] PROGMEM = "the mountain air";
const char weather7[] PROGMEM = "the thaw in the pipes";

const char* const weatherThings[] PROGMEM = {
  weather0, weather1, weather2, weather3,
  weather4, weather5, weather6, weather7
};

const int SANE_PROMPT_COUNT = sizeof(sanePrompts) / sizeof(sanePrompts[0]);
const int MEASURING_PROMPT_COUNT = sizeof(measuringPrompts) / sizeof(measuringPrompts[0]);
const int CONCERNED_PROMPT_COUNT = sizeof(concernedPrompts) / sizeof(concernedPrompts[0]);
const int PARANOID_PROMPT_COUNT = sizeof(paranoidPrompts) / sizeof(paranoidPrompts[0]);
const int MANIC_PROMPT_COUNT = sizeof(manicPrompts) / sizeof(manicPrompts[0]);
const int RECOVERY_PROMPT_COUNT = sizeof(recoveryPrompts) / sizeof(recoveryPrompts[0]);
const int FRAGMENT_COUNT = sizeof(fragments) / sizeof(fragments[0]);
const int WALK_AWAY_PROMPT_COUNT = sizeof(walkAwayPrompts) / sizeof(walkAwayPrompts[0]);
const int ROOM_COUNT = sizeof(rooms) / sizeof(rooms[0]);
const int THING_COUNT = sizeof(things) / sizeof(things[0]);
const int CHORE_COUNT = sizeof(chores) / sizeof(chores[0]);
const int SOUND_COUNT = sizeof(sounds) / sizeof(sounds[0]);
const int SIGN_COUNT = sizeof(signs) / sizeof(signs[0]);
const int FEAR_COUNT = sizeof(fears) / sizeof(fears[0]);
const int WEATHER_COUNT = sizeof(weatherThings) / sizeof(weatherThings[0]);

const ModeConfig shortModeConfigs[] PROGMEM = {
  {
    30000UL, 45000UL,
    70, 190,
    1, 90, 0,
    1800UL, 4500UL,
    8, 5000UL, 14000UL,
    4, 12000UL, 35000UL,
    2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
    20000UL, 45000UL,
    1, 4,
    0, 0
  },
  {
    45000UL, 70000UL,
    45, 150,
    3, 75, 0,
    2200UL, 7000UL,
    10, 7000UL, 18000UL,
    6, 15000UL, 45000UL,
    8, 1, 0, 1, 1, 2, 3, 2, 1, 1, 0,
    12000UL, 30000UL,
    2, 8,
    0, 0
  },
  {
    60000UL, 85000UL,
    30, 120,
    6, 55, 1,
    1800UL, 6000UL,
    14, 5000UL, 22000UL,
    9, 18000UL, 60000UL,
    16, 5, 3, 3, 4, 8, 8, 8, 4, 3, 5,
    7000UL, 18000UL,
    5, 18,
    1, 1
  },
  {
    70000UL, 95000UL,
    18, 95,
    10, 35, 3,
    900UL, 4500UL,
    18, 3000UL, 26000UL,
    12, 20000UL, 75000UL,
    28, 15, 8, 5, 8, 18, 16, 16, 8, 8, 14,
    4000UL, 12000UL,
    8, 28,
    2, 2
  },
  {
    65000UL, 90000UL,
    8, 70,
    14, 18, 6,
    300UL, 2500UL,
    22, 2000UL, 30000UL,
    15, 24000UL, 90000UL,
    40, 38, 18, 8, 12, 30, 26, 25, 14, 12, 28,
    2500UL, 9000UL,
    10, 36,
    4, 3
  },
  {
    40000UL, 70000UL,
    90, 240,
    2, 85, 0,
    3000UL, 9000UL,
    12, 8000UL, 24000UL,
    10, 18000UL, 65000UL,
    5, 0, 2, 2, 1, 2, 3, 2, 1, 0, 0,
    18000UL, 50000UL,
    1, 5,
    0, 0
  }
};

MadnessMode mode = SANE;
unsigned long modeStartMs = 0;
unsigned long currentModeDurationMs = 0;
unsigned long nextMouseMoveMs = 0;
unsigned long promptCount = 0;
int topicHoldRemaining = 0;
int currentRoomIndex = 0;
int alternateRoomIndex = 1;
int currentThingIndex = 0;
int alternateThingIndex = 1;
int currentChoreIndex = 0;
int currentSoundIndex = 0;
int currentSignIndex = 0;
int currentFearIndex = 0;
int currentWeatherIndex = 0;

bool randomChance(int percent) {
  return percent > 0 && random(100) < percent;
}

bool isAsciiAlpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

ModeConfig getConfigForMode(MadnessMode currentMode) {
  ModeConfig cfg;
  memcpy_P(&cfg, &shortModeConfigs[currentMode], sizeof(ModeConfig));

  if (!USE_SHORT_DEMO_TIMING) {
    cfg.minDurationMs *= 3;
    cfg.maxDurationMs *= 3;
    cfg.minPromptPauseMs *= 2;
    cfg.maxPromptPauseMs *= 2;
    cfg.minLongPauseMs *= 3;
    cfg.maxLongPauseMs *= 3;
    cfg.minWalkAwayMs *= 8;
    cfg.maxWalkAwayMs *= 4;
  }

  return cfg;
}

String readFlashLine(const char* const table[], int index) {
  char buffer[LINE_BUFFER_SIZE];
  PGM_P line = (PGM_P)pgm_read_ptr(&(table[index]));
  strncpy_P(buffer, line, LINE_BUFFER_SIZE - 1);
  buffer[LINE_BUFFER_SIZE - 1] = '\0';
  return String(buffer);
}

int randomDifferentIndex(int count, int currentIndex) {
  if (count < 2) {
    return 0;
  }

  int nextIndex = random(count - 1);
  if (nextIndex >= currentIndex) {
    nextIndex++;
  }

  return nextIndex;
}

int topicHoldMinForMode(MadnessMode currentMode) {
  switch (currentMode) {
    case SANE: return 3;
    case MEASURING: return 2;
    case CONCERNED: return 1;
    case PARANOID: return 1;
    case MANIC: return 0;
    case RECOVERY:
    default: return 2;
  }
}

int topicHoldMaxForMode(MadnessMode currentMode) {
  switch (currentMode) {
    case SANE: return 7;
    case MEASURING: return 5;
    case CONCERNED: return 4;
    case PARANOID: return 3;
    case MANIC: return 2;
    case RECOVERY:
    default: return 5;
  }
}

int topicPivotChanceForMode(MadnessMode currentMode) {
  switch (currentMode) {
    case SANE: return 6;
    case MEASURING: return 18;
    case CONCERNED: return 38;
    case PARANOID: return 58;
    case MANIC: return 68;
    case RECOVERY:
    default: return 12;
  }
}

void chooseNewTopic() {
  currentRoomIndex = random(ROOM_COUNT);
  alternateRoomIndex = randomDifferentIndex(ROOM_COUNT, currentRoomIndex);
  currentThingIndex = random(THING_COUNT);
  alternateThingIndex = randomDifferentIndex(THING_COUNT, currentThingIndex);
  currentChoreIndex = random(CHORE_COUNT);
  currentSoundIndex = random(SOUND_COUNT);
  currentSignIndex = random(SIGN_COUNT);
  currentFearIndex = random(FEAR_COUNT);
  currentWeatherIndex = random(WEATHER_COUNT);

  int minHold = topicHoldMinForMode(mode);
  int maxHold = topicHoldMaxForMode(mode);
  topicHoldRemaining = random(minHold, maxHold + 1);
}

void maybeAdvanceTopic() {
  if (topicHoldRemaining > 0) {
    topicHoldRemaining--;

    if ((mode == PARANOID || mode == MANIC) && randomChance(mode == MANIC ? 45 : 25)) {
      topicHoldRemaining += random(2, mode == MANIC ? 9 : 6);
      return;
    }

    if (!randomChance(topicPivotChanceForMode(mode))) {
      return;
    }
  } else if ((mode == PARANOID || mode == MANIC) && randomChance(mode == MANIC ? 55 : 30)) {
    topicHoldRemaining = random(3, mode == MANIC ? 12 : 8);
    return;
  }

  chooseNewTopic();
}

String currentRoom() {
  return readFlashLine(rooms, currentRoomIndex);
}

String alternateRoom() {
  return readFlashLine(rooms, alternateRoomIndex);
}

String currentThing() {
  return readFlashLine(things, currentThingIndex);
}

String alternateThing() {
  return readFlashLine(things, alternateThingIndex);
}

void swapTerms(String& line, String first, String second) {
  if (first.length() == 0 || second.length() == 0) {
    return;
  }

  if (line.indexOf(first) < 0 || line.indexOf(second) < 0) {
    return;
  }

  line.replace(first, "{SWAP}");
  line.replace(second, first);
  line.replace("{SWAP}", second);
}

String transposeTopicThings(String line) {
  int choice = random(4);

  if (choice == 0) {
    swapTerms(line, currentRoom(), alternateRoom());
  } else if (choice == 1) {
    swapTerms(line, currentThing(), alternateThing());
  } else if (choice == 2) {
    swapTerms(line, currentRoom(), currentThing());
  } else {
    swapTerms(line, alternateRoom(), alternateThing());
  }

  return line;
}

void scheduleMode(MadnessMode nextMode) {
  mode = nextMode;
  modeStartMs = millis();

  ModeConfig cfg = getConfigForMode(mode);
  currentModeDurationMs = random(cfg.minDurationMs, cfg.maxDurationMs + 1);
  nextMouseMoveMs = millis() + random(cfg.mouseMoveIntervalMinMs, cfg.mouseMoveIntervalMaxMs + 1);
}

void advanceMode() {
  int roll = random(100);

  switch (mode) {
    case SANE:
      scheduleMode(roll < 15 ? CONCERNED : MEASURING);
      break;
    case MEASURING:
      if (roll < 20) scheduleMode(SANE);
      else if (roll < 75) scheduleMode(CONCERNED);
      else scheduleMode(PARANOID);
      break;
    case CONCERNED:
      if (roll < 20) scheduleMode(RECOVERY);
      else if (roll < 40) scheduleMode(MEASURING);
      else scheduleMode(PARANOID);
      break;
    case PARANOID:
      if (roll < 20) scheduleMode(RECOVERY);
      else if (roll < 55) scheduleMode(CONCERNED);
      else scheduleMode(MANIC);
      break;
    case MANIC:
      if (roll < 30) scheduleMode(RECOVERY);
      else if (roll < 70) scheduleMode(PARANOID);
      else scheduleMode(CONCERNED);
      break;
    case RECOVERY:
    default:
      if (roll < 55) scheduleMode(SANE);
      else if (roll < 80) scheduleMode(MEASURING);
      else scheduleMode(CONCERNED);
      break;
  }
}

char randomLetterTypo(char c) {
  if (c >= 'a' && c <= 'z') {
    return char('a' + random(26));
  }

  if (c >= 'A' && c <= 'Z') {
    return char('A' + random(26));
  }

  return c;
}

char nearbyTypo(char c) {
  bool uppercase = (c >= 'A' && c <= 'Z');
  char lower = uppercase ? char(c + 32) : c;
  char typo;

  switch (lower) {
    case 'a': typo = random(2) ? 's' : 'q'; break;
    case 's': typo = random(2) ? 'a' : 'd'; break;
    case 'd': typo = random(2) ? 's' : 'f'; break;
    case 'f': typo = random(2) ? 'd' : 'g'; break;
    case 'j': typo = random(2) ? 'h' : 'k'; break;
    case 'k': typo = random(2) ? 'j' : 'l'; break;
    case 'l': typo = random(2) ? 'k' : ';'; break;
    case 'w': typo = random(2) ? 'q' : 'e'; break;
    case 'e': typo = random(2) ? 'w' : 'r'; break;
    case 'r': typo = random(2) ? 'e' : 't'; break;
    case 't': typo = random(2) ? 'r' : 'y'; break;
    case 'y': typo = random(2) ? 't' : 'u'; break;
    case 'u': typo = random(2) ? 'y' : 'i'; break;
    case 'i': typo = random(2) ? 'u' : 'o'; break;
    case 'o': typo = random(2) ? 'i' : 'p'; break;
    case 'p': typo = random(2) ? 'o' : '['; break;
    case 'n': typo = random(2) ? 'b' : 'm'; break;
    case 'm': typo = random(2) ? 'n' : ','; break;
    default: typo = randomLetterTypo(c); break;
  }

  if (uppercase && typo >= 'a' && typo <= 'z') {
    typo = char(typo - 32);
  }

  return typo;
}

String mutatePunctuation(String line) {
  while (line.endsWith(".") || line.endsWith("!") || line.endsWith("?")) {
    line.remove(line.length() - 1);
  }

  int choice = random(6);

  if (choice == 0) return line + ".";
  if (choice == 1) return line + "...";
  if (choice == 2) return line + "!!";
  if (choice == 3) return line + "?";
  if (choice == 4) return line + "???";

  return line;
}

String toTitleCaseLine(String line) {
  bool nextIsStart = true;

  for (int i = 0; i < line.length(); i++) {
    char c = line[i];

    if (c == ' ' || c == '\t' || c == '\n' || c == '-' || c == '/') {
      nextIsStart = true;
      continue;
    }

    if (nextIsStart && c >= 'a' && c <= 'z') {
      line[i] = char(c - 32);
    } else if (!nextIsStart && c >= 'A' && c <= 'Z') {
      line[i] = char(c + 32);
    }

    nextIsStart = false;
  }

  return line;
}

String toMixedCapsLine(String line) {
  for (int i = 0; i < line.length(); i++) {
    char c = line[i];

    if (c >= 'a' && c <= 'z' && randomChance(45)) {
      line[i] = char(c - 32);
    } else if (c >= 'A' && c <= 'Z' && randomChance(45)) {
      line[i] = char(c + 32);
    }
  }

  return line;
}

String fragmentLine(String line) {
  line.replace(". ", ".\n");
  line.replace("? ", "?\n");
  line.replace(" because ", "\nbecause ");
  line.replace(" but ", "\nbut ");
  line.replace(" and ", "\nand ");

  if (line.indexOf('\n') < 0 && line.length() > 34) {
    int splitAt = line.length() / 2;
    while (splitAt < line.length() && line[splitAt] != ' ') {
      splitAt++;
    }

    if (splitAt < line.length() - 1) {
      line.setCharAt(splitAt, '\n');
    }
  }

  return line;
}

String indentLine(String line) {
  int choice = random(4);

  if (choice == 0) return "  " + line;
  if (choice == 1) return "    " + line;
  if (choice == 2) return "\t" + line;

  return line;
}

String repeatWordMutation(String line) {
  int choice = random(7);

  if (choice == 0) {
    line.replace("pages", "pages pages");
    line.replace("Pages", "Pages Pages");
  } else if (choice == 1) {
    line.replace("rooms", "rooms rooms");
    line.replace("Rooms", "Rooms Rooms");
  } else if (choice == 2) {
    line.replace("book", "book book");
    line.replace("Book", "Book Book");
  } else if (choice == 3) {
    line.replace("hotel", "hotel hotel");
    line.replace("Hotel", "Hotel Hotel");
  } else if (choice == 4) {
    line.replace("proof", "proof proof");
    line.replace("Proof", "Proof Proof");
  } else if (choice == 5) {
    line.replace("Jack", "Jack Jack");
  } else {
    line += " again";
  }

  return line;
}

String transposeCharacters(String line) {
  if (line.length() < 4) return line;

  int i = random(1, line.length() - 2);

  if (line[i] == ' ' || line[i + 1] == ' ' || line[i] == '\n' || line[i + 1] == '\n') {
    return line;
  }

  char temp = line[i];
  line[i] = line[i + 1];
  line[i + 1] = temp;

  return line;
}

String transposeWords(String line) {
  int choice = random(6);

  if (randomChance((mode == SANE || mode == RECOVERY) ? 20 : 65)) {
    line = transposeTopicThings(line);
  }

  if (choice == 0) {
    line.replace("page count", "count page");
  } else if (choice == 1) {
    line.replace("more pages", "pages more");
  } else if (choice == 2) {
    line.replace("quiet room", "room quiet");
  } else if (choice == 3) {
    line.replace("useful work", "work useful");
  } else if (choice == 4) {
    line.replace("no play", "play no");
  } else {
    line.replace("dull boy", "boy dull");
  }

  return line;
}

String expandPlaceholders(String line) {
  if (line.indexOf("{ROOM}") >= 0) {
    line.replace("{ROOM}", currentRoom());
  }

  if (line.indexOf("{OTHER_ROOM}") >= 0) {
    line.replace("{OTHER_ROOM}", alternateRoom());
  }

  if (line.indexOf("{THING}") >= 0) {
    line.replace("{THING}", currentThing());
  }

  if (line.indexOf("{OTHER_THING}") >= 0) {
    line.replace("{OTHER_THING}", alternateThing());
  }

  if (line.indexOf("{CHORE}") >= 0) {
    line.replace("{CHORE}", readFlashLine(chores, currentChoreIndex));
  }

  if (line.indexOf("{SOUND}") >= 0) {
    line.replace("{SOUND}", readFlashLine(sounds, currentSoundIndex));
  }

  if (line.indexOf("{SIGN}") >= 0) {
    line.replace("{SIGN}", readFlashLine(signs, currentSignIndex));
  }

  if (line.indexOf("{FEAR}") >= 0) {
    line.replace("{FEAR}", readFlashLine(fears, currentFearIndex));
  }

  if (line.indexOf("{WEATHER}") >= 0) {
    line.replace("{WEATHER}", readFlashLine(weatherThings, currentWeatherIndex));
  }

  if (line.indexOf("{PAGES}") >= 0) {
    line.replace("{PAGES}", String(random(6L, 128L)));
  }

  if (line.indexOf("{NOTES}") >= 0) {
    line.replace("{NOTES}", String(promptCount));
  }

  return line;
}

String mutateLine(String line, const ModeConfig& cfg) {
  line = expandPlaceholders(line);

  if (randomChance(cfg.punctuationMutationChancePercent)) {
    line = mutatePunctuation(line);
  }

  if (randomChance(cfg.allCapsChancePercent)) {
    line.toUpperCase();
  } else if (randomChance(cfg.lowercaseChancePercent)) {
    line.toLowerCase();
  } else if (randomChance(cfg.titleCaseChancePercent)) {
    line = toTitleCaseLine(line);
  } else if (randomChance(cfg.mixedCapsChancePercent)) {
    line = toMixedCapsLine(line);
  }

  if (randomChance(cfg.wordRepeatChancePercent)) {
    line = repeatWordMutation(line);
  }

  if (randomChance(cfg.indentationChancePercent)) {
    line = indentLine(line);
  }

  if (randomChance(cfg.charTransposeChancePercent)) {
    line = transposeCharacters(line);
  }

  if (randomChance(cfg.wordTransposeChancePercent)) {
    line = transposeWords(line);
  }

  if (randomChance(cfg.fragmentedLineChancePercent)) {
    line = fragmentLine(line);
  }

  if (SUBMIT_PROMPTS_TO_CHAT) {
    line.replace("\n", " ");
    line.replace("\t", " ");
  }

  if (TYPE_VISIBLE_SPEAKER_PREFIX) {
    line = "Jack: " + line;
  }

  return line;
}

String choosePromptPart() {
  if ((mode == PARANOID || mode == MANIC) && randomChance(24)) {
    return readFlashLine(fragments, random(FRAGMENT_COUNT));
  }

  switch (mode) {
    case SANE:
      return readFlashLine(sanePrompts, random(SANE_PROMPT_COUNT));
    case MEASURING:
      return readFlashLine(measuringPrompts, random(MEASURING_PROMPT_COUNT));
    case CONCERNED:
      return readFlashLine(concernedPrompts, random(CONCERNED_PROMPT_COUNT));
    case PARANOID:
      return readFlashLine(paranoidPrompts, random(PARANOID_PROMPT_COUNT));
    case MANIC:
      return readFlashLine(manicPrompts, random(MANIC_PROMPT_COUNT));
    case RECOVERY:
    default:
      return readFlashLine(recoveryPrompts, random(RECOVERY_PROMPT_COUNT));
  }
}

int promptPartCountForMode() {
  int roll = random(100);

  switch (mode) {
    case SANE:
      return roll < 85 ? 2 : 3;
    case MEASURING:
      return roll < 55 ? 2 : 3;
    case CONCERNED:
      if (roll < 25) return 1;
      if (roll < 80) return random(2, 4);
      return random(4, 6);
    case PARANOID:
      if (roll < 32) return 1;
      if (roll < 70) return random(2, 5);
      return random(5, 8);
    case MANIC:
      if (roll < 35) return 1;
      if (roll < 72) return random(4, 8);
      return random(8, 12);
    case RECOVERY:
    default:
      return roll < 70 ? 1 : 2;
  }
}

int missingSeparatorChanceForMode() {
  switch (mode) {
    case SANE: return 1;
    case MEASURING: return 3;
    case CONCERNED: return 7;
    case PARANOID: return 15;
    case MANIC: return 24;
    case RECOVERY:
    default: return 2;
  }
}

String promptSeparator() {
  if (randomChance(missingSeparatorChanceForMode())) {
    return "";
  }

  if ((mode == PARANOID || mode == MANIC) && randomChance(10)) {
    return "  ";
  }

  return " ";
}

String choosePrompt() {
  promptCount++;
  maybeAdvanceTopic();

  int partCount = promptPartCountForMode();
  String prompt = "";
  String fixation = "";
  bool hyperfocus = (mode == PARANOID || mode == MANIC) && randomChance(mode == MANIC ? 35 : 18);

  for (int i = 0; i < partCount; i++) {
    if (i > 0) {
      if (!hyperfocus || randomChance(mode == MANIC ? 45 : 25)) {
        maybeAdvanceTopic();
      }
      prompt += promptSeparator();
    }

    String part;
    if (hyperfocus && i > 0 && randomChance(mode == MANIC ? 58 : 36)) {
      part = fixation;
    } else {
      part = choosePromptPart();
      if (fixation.length() == 0) {
        fixation = part;
      }
    }

    prompt += part;
  }

  return prompt;
}

void tapKey(uint8_t key) {
  Keyboard.press(key);
  Keyboard.release(key);
}

void typeCharWithTypo(char correctChar, int correctionChancePercent) {
  char wrongChar = nearbyTypo(correctChar);

  if (wrongChar == correctChar) {
    Keyboard.print(correctChar);
    return;
  }

  Keyboard.print(wrongChar);
  delay(random(80, 501));

  if (randomChance(correctionChancePercent)) {
    tapKey(KEY_BACKSPACE);
    delay(random(50, 251));
    Keyboard.print(correctChar);
  }
}

void typeLine(String line, const ModeConfig& cfg) {
  bool burstMode = randomChance(cfg.burstTypingChancePercent);

  for (int i = 0; i < line.length(); i++) {
    char c = line[i];

    if (c == '\n') {
      Keyboard.print(' ');
      delay(random(cfg.minCharDelayMs, cfg.maxCharDelayMs + 1));
      continue;
    }

    bool canRandomBackspace = i > 0 &&
                              c != ' ' &&
                              line[i - 1] != ' ' &&
                              line[i - 1] != '.' &&
                              line[i - 1] != '?' &&
                              line[i - 1] != '!';

    if (canRandomBackspace && randomChance(cfg.randomBackspaceChancePercent)) {
      tapKey(KEY_BACKSPACE);
      delay(random(50, 201));
    }

    if (randomChance(cfg.typoChancePercent) && isAsciiAlpha(c)) {
      typeCharWithTypo(c, cfg.correctionChancePercent);
    } else {
      Keyboard.print(c);
    }

    int charDelay = random(cfg.minCharDelayMs, cfg.maxCharDelayMs + 1);
    if (burstMode) {
      charDelay = max(5, charDelay / 3);
    }
    delay(charDelay);
  }

  tapKey(KEY_RETURN);

  if (!SUBMIT_PROMPTS_TO_CHAT) {
    tapKey(KEY_RETURN);
  }
}

void mouseMicroNudge(int maxDistance) {
  Mouse.move(random(-maxDistance, maxDistance + 1),
             random(-maxDistance, maxDistance + 1),
             0);
}

void mouseDrift(int maxDistance) {
  int dx = random(-2, 3);
  int dy = random(-2, 3);
  int steps = random(5, 12);

  for (int i = 0; i < steps; i++) {
    Mouse.move(dx, dy, 0);
    delay(random(30, 91));
  }
}

void mouseTwitchCorrection(int maxDistance) {
  int dx = random(-maxDistance, maxDistance + 1);
  int dy = random(-maxDistance, maxDistance + 1);

  Mouse.move(dx, dy, 0);
  delay(random(60, 151));
  Mouse.move(-dx / 2, -dy / 2, 0);
}

void mouseArc(int maxDistance) {
  int steps = random(8, 20);
  int direction = random(2) ? 1 : -1;

  for (int i = 0; i < steps; i++) {
    int dx = map(i, 0, steps, -2 * direction, 2 * direction);
    int dy = random(-2, 3);

    Mouse.move(dx, dy, 0);
    delay(random(25, 81));
  }
}

void performMouseBehavior(const ModeConfig& cfg) {
  if (!ENABLE_MOUSE_BEHAVIOR) {
    return;
  }

  int pattern = random(4);
  int distance = random(cfg.mouseDistanceMin, cfg.mouseDistanceMax + 1);

  if (pattern == 0) {
    mouseMicroNudge(distance);
  } else if (pattern == 1) {
    mouseDrift(distance);
  } else if (pattern == 2) {
    mouseTwitchCorrection(distance);
  } else {
    mouseArc(distance);
  }

  if (ENABLE_MOUSE_CLICKS && randomChance(cfg.clickChancePercent)) {
    Mouse.click(MOUSE_LEFT);
  }

  if (randomChance(cfg.scrollChancePercent)) {
    Mouse.move(0, 0, random(-1, 2));
  }
}

void maybeWalkAway(const ModeConfig& cfg) {
  if (!randomChance(cfg.walkAwayChancePercent)) {
    return;
  }

  String walkAwayLine = readFlashLine(walkAwayPrompts, random(WALK_AWAY_PROMPT_COUNT));
  typeLine(mutateLine(walkAwayLine, cfg), cfg);
  delay(random(cfg.minWalkAwayMs, cfg.maxWalkAwayMs + 1));
}

void setup() {
  Keyboard.begin();
  Mouse.begin();

  randomSeed(analogRead(A0) ^ micros());
  delay(STARTUP_DELAY_MS);

  chooseNewTopic();
  scheduleMode(SANE);
}

void loop() {
  ModeConfig cfg = getConfigForMode(mode);

  if (millis() - modeStartMs > currentModeDurationMs) {
    advanceMode();
    cfg = getConfigForMode(mode);
  }

  if (millis() > nextMouseMoveMs) {
    performMouseBehavior(cfg);
    nextMouseMoveMs = millis() + random(cfg.mouseMoveIntervalMinMs, cfg.mouseMoveIntervalMaxMs + 1);
  }

  String line = choosePrompt();
  typeLine(mutateLine(line, cfg), cfg);

  delay(random(cfg.minPromptPauseMs, cfg.maxPromptPauseMs + 1));

  if (randomChance(cfg.longPauseChancePercent)) {
    delay(random(cfg.minLongPauseMs, cfg.maxLongPauseMs + 1));
  }

  maybeWalkAway(cfg);
}
