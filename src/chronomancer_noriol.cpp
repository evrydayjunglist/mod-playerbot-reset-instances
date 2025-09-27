#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"    // For AddGossipItemFor, SendGossipMenuFor, CloseGossipMenuFor
#include "Player.h"
#include "GossipDef.h"
#include "Chat.h"
#include "Config.h"
#include "Creature.h"
#include "Group.h"
#include "PlayerbotAI.h"
#include "PlayerbotMgr.h"

class chronomancer_noriol : public CreatureScript
{
public:
    chronomancer_noriol() : CreatureScript("chronomancer_noriol") { }

    struct chronomancer_noriolAI : public ScriptedAI
    {
        chronomancer_noriolAI(Creature* creature) : ScriptedAI(creature) { }

    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new chronomancer_noriolAI(creature);
    }

void GossipSetText(Player* player, std::string message, uint32 textID)
{
    WorldPacket data(SMSG_NPC_TEXT_UPDATE, 100);
    data << textID;
    for (uint8 i = 0; i < MAX_GOSSIP_TEXT_OPTIONS; ++i)
    {
        data << float(0);
        data << message;
        data << message;
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
    }
    player->GetSession()->SendPacket(&data);
}

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (!sConfigMgr->GetOption<bool>("Chronomancer.EnableModule", true))
        {
            ChatHandler(player->GetSession()).SendSysMessage("Chronomancer Noriol's powers are currently dormant.");
            return true;
        }

        std::string introText;
        std::string answerText;
        std::string boostAnswerText;

        introText = "Time is a spiral. Care to rewind your fate?";
        answerText = "Please unravel the threads of fate.";
        boostAnswerText = "Please elevate the fate of my companions.";

        // GossipItem -> Answer
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, answerText, GOSSIP_SENDER_MAIN, 1);
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, boostAnswerText, GOSSIP_SENDER_MAIN, 2);
        // Intro message from NPC
        GossipSetText(player, introText, DEFAULT_GOSSIP_MESSAGE);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        // Instance reset choice
        if (action == 1)
        {
            if (sConfigMgr->GetOption<bool>("Chronomancer.EnableGoldCost", true))
            {
                uint32 goldCost = sConfigMgr->GetOption<uint32>("Chronomancer.GoldCostAmount", 10);
                uint32 costInCopper = goldCost * 10000;

                if (player->GetMoney() < costInCopper)
                {
                    creature->Say("Time magic isn't free. Come back with more gold, friend.", LANG_UNIVERSAL);
                    CloseGossipMenuFor(player);
                    return true;
                }

                player->ModifyMoney(-static_cast<int32>(costInCopper));
                char msg[256];
                snprintf(msg, sizeof(msg), "You pay %u gold to Chronomancer Noriol.", goldCost);
                ChatHandler(player->GetSession()).SendSysMessage(msg);
            }

            creature->Say("Time bends to my will. Be still... and begin anew.", LANG_UNIVERSAL);
            creature->CastSpell(player, 52759, true);
            creature->HandleEmoteCommand(EMOTE_ONESHOT_SPELL_CAST_OMNI);

            // Reset instance bindings for the main player (both non-raid and raid)
            uint32 diff = MAX_DIFFICULTY;
            for (uint8 i = 0; i < diff; ++i)
            {
                BoundInstancesMap const& m_boundInstances = sInstanceSaveMgr->PlayerGetBoundInstances(player->GetGUID(), Difficulty(i));
                for (BoundInstancesMap::const_iterator itr = m_boundInstances.begin(); itr != m_boundInstances.end();)
                {
                    if (itr->first != player->GetMapId())
                    {
                        sInstanceSaveMgr->PlayerUnbindInstance(player->GetGUID(), itr->first, Difficulty(i), true, player);
                        itr = m_boundInstances.begin();
                    }
                    else
                        ++itr;
                }
            }


            // Also reset instance bindings for all playerbots in the player's group
            if (Group* group = player->GetGroup())
            {
                for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
                {
                    Player* member = itr->GetSource();
                    if (!member || member == player)
                        continue;


                    for (uint8 i = 0; i < diff; ++i)
                    {
                        BoundInstancesMap const& m_boundInstances = sInstanceSaveMgr->PlayerGetBoundInstances(member->GetGUID(), Difficulty(i));
                        for (BoundInstancesMap::const_iterator itr = m_boundInstances.begin(); itr != m_boundInstances.end();)
                        {
                            if (itr->first != member->GetMapId())
                            {
                                sInstanceSaveMgr->PlayerUnbindInstance(member->GetGUID(), itr->first, Difficulty(i), true, member);
                                itr = m_boundInstances.begin();
                            }
                            else
                                ++itr;
                        }
                    }
                    char botMsg[256];
                    snprintf(botMsg, sizeof(botMsg), "%s's timeline has also been reset.", member->GetName().c_str());
                    ChatHandler(player->GetSession()).SendSysMessage(botMsg);
                }
            }
            ChatHandler(player->GetSession()).SendSysMessage("Your timeline has been reset.");
        }
        // PlayerBot boost
        else if (action == 2)
        {
            uint32 goldCost = sConfigMgr->GetOption<uint32>("Chronomancer.BotBoostCostAmount", 1000);
            uint32 costInCopper = goldCost * 10000;

            // Iterate bot by bot if any
            if (Group* group = player->GetGroup())
            {
                for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
                {
                    Player* member = itr->GetSource();
                    if (!member || member == player)
                        continue;

                    if (member->GetLevel() >= 80)
                        continue;

                    // OK, we've got a playerbot, and not level 80, proceed.

                    // No need to go further if we're broke...
                    if (player->GetMoney() < costInCopper)
                    {
                        creature->Say("Time magic isn't free. Come back with more gold, friend.", LANG_UNIVERSAL);
                        CloseGossipMenuFor(player);
                        return true;
                    }

                    member->GiveLevel(80);
                    player->ModifyMoney(-static_cast<int32>(costInCopper));
                    char botMsg[256];
                    snprintf(botMsg, sizeof(botMsg), "%s's fate has been augmented.", member->GetName().c_str());
                    ChatHandler(player->GetSession()).SendSysMessage(botMsg);
                }
            }
            ChatHandler(player->GetSession()).SendSysMessage("The timelines have been altered.");
        }
        // End dialog actions
        CloseGossipMenuFor(player);
        return true;
    }
};

void Addmod_playerbot_reset_instancesScripts()
{
    new chronomancer_noriol();
}
