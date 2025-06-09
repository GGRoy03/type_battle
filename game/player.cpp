struct player
{
    f32 Health;
    f32 Mana;
    u32 Level;
};

// NOTE: So let's say the player has two types of actions. Attack and heal.

struct player_attack_action
{
    f32 DamageDealt;
};

struct player_heal_action
{
    f32 HealAmount;
};

// So every action types has an active word, a word queue, which is a circular buffer.
// Words are streamed from a list on disk.

enum PLAYER_ACTION_TYPE
{
    PLAYER_ACTION_ATTACK,
    PLAYER_ACTION_HEAL,

    PLAYER_ACTION_COUNT,
};

#define WORDS_PER_QUEUE 10
#define MAXIMUM_WORD_SIZE 16

struct action_word
{
    char Word[MAXIMUM_WORD_SIZE];
    u32  WordAt;
    u32  WordSize;
};

struct word_queue
{
    u32         CurrentWordIndex;
    u32         CompletedThisFrame;
    action_word Words[WORDS_PER_QUEUE];
};

struct player_actions
{
    bool       Initialized;
    word_queue WordQueue[PLAYER_ACTION_COUNT];
};

// This takes in a buffer of inputs, and try to match the words, if the word is
// completed do the action. So how do I represent a word. It's length, completed count,
// well the actual word. They must have a length.
internal void
UpdatePlayerActions(player_actions *Actions, game_controller_input *Input)
{
    if(!Actions->Initialized)
    {

        const char* TestWord = "apple";
        memcpy(Actions->WordQueue[PLAYER_ACTION_ATTACK].Words[0].Word,
               TestWord, sizeof(TestWord));
        Actions->WordQueue[PLAYER_ACTION_ATTACK].Words[0].WordSize = 5;
        Actions->WordQueue[PLAYER_ACTION_ATTACK].Words[0].WordAt   = 0;

        Actions->Initialized = true;
    }

    for(u32 Type = PLAYER_ACTION_ATTACK; Type < PLAYER_ACTION_COUNT; Type++)
    {
        word_queue  *Queue  = Actions->WordQueue + Type;
        action_word *Action = Queue->Words + Queue->CurrentWordIndex;

        u32 InputIndex = GET_LAST_WRITTEN_INDEX(Input);
        for(u32 Index = 0; Index < Input->RecordedCharacters; Index++)
        {
            if(Input->CharBuffer[InputIndex] == Action->Word[Action->WordAt])
            {
                PrintDebug("Correct letter typed\n");

                ++Action->WordAt;
                if(Action->WordAt == Action->WordSize)
                {
                    PrintDebug("WORD COMPLETED\n");
                }

                InputIndex = MOVE_INPUT_HEAD_TO_NEXT(Input);
            }
            else
            {
                break; // Means we are done matching the letters
            }
        }
    }
}
