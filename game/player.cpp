// NOTE: All player interaction is done through here

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
UpdatePlayerActions(player_actions *Actions)
{
    if(!Actions->Initialized)
    {
        // When we have the system, we just load the words here. For now let's hardcode
        // apple as the first word for the PLAYER_ACTION_ATTACK.

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

        const u32  InputBufferSize               = 6;
        const char InputBuffer[InputBufferSize]  = "apple";
        for(u32 InputIndex = 0; InputIndex < InputBufferSize; InputIndex++)
        {
            if(InputBuffer[InputIndex] == Action->Word[Action->WordAt])
            {
                PrintDebug("Correct letter typed\n");

                // Here you'd do the work to update whatever visuals we need or
                // sound or whatever.

                ++Action->WordAt;
                if(Action->WordAt == Action->WordSize)
                {
                    PrintDebug("WORD COMPLETED\n");
                }
            }
            else
            {
                break; // Means we are done matching the letters
            }
        }
    }
}
