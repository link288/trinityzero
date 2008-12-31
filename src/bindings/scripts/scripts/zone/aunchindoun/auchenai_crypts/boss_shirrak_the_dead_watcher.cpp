/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* ScriptData
Name: Boss_Shirrak_the_dead_watcher
%Complete: 80
Comment: InhibitMagic should stack slower far from the boss, proper Visual for Focus Fire, heroic implemented
Category: Auchindoun, Auchenai Crypts
EndScriptData */

#include "precompiled.h"

#define SPELL_INHABITMAGIC          32264	            
#define SPELL_ATTRACTMAGIC          32265
#define N_SPELL_CARNIVOROUSBITE     36383
#define H_SPELL_CARNIVOROUSBITE     39382
#define SPELL_CARNIVOROUSBITE       (HeroicMode?H_SPELL_CARNIVOROUSBITE:N_SPELL_CARNIVOROUSBITE)

#define ENTRY_FOCUS_FIRE            18374

#define N_SPELL_FIERY_BLAST         32302
#define H_SPELL_FIERY_BLAST         38382
#define SPELL_FIERY_BLAST           (HeroicMode?H_SPELL_FIERY_BLAST:N_SPELL_FIERY_BLAST)
#define SPELL_FOCUS_FIRE_VISUAL     42075 //need to find better visual

struct TRINITY_DLL_DECL boss_shirrak_the_dead_watcherAI : public ScriptedAI
{
    boss_shirrak_the_dead_watcherAI(Creature *c) : ScriptedAI(c)
    {
        HeroicMode = m_creature->GetMap()->IsHeroic();
        Reset();
    }

    uint32 Inhabitmagic_Timer;
    uint32 Attractmagic_Timer;
    uint32 Carnivorousbite_Timer;
    uint32 FocusFire_Timer;
    bool HeroicMode;
    Unit *focusedTarget;

    void Reset()
    {       
        Inhabitmagic_Timer = 3000;
        Attractmagic_Timer = 28000;
        Carnivorousbite_Timer = 10000;
        FocusFire_Timer = 17000;
        focusedTarget = NULL;
    }

    void Aggro(Unit *who)
    { }

    void JustSummoned(Creature *summoned)
    {
        if (summoned && summoned->GetEntry() == ENTRY_FOCUS_FIRE)
        {
            summoned->CastSpell(summoned,SPELL_FOCUS_FIRE_VISUAL,false);
            summoned->setFaction(m_creature->getFaction());
            summoned->SetLevel(m_creature->getLevel());
            summoned->addUnitState(UNIT_STAT_ROOT);

            if(focusedTarget)
                summoned->AI()->AttackStart(focusedTarget);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!m_creature->SelectHostilTarget() || !m_creature->getVictim() )
            return;

        //Inhabitmagic_Timer
        if (Inhabitmagic_Timer < diff)
        {
            DoCast(m_creature,SPELL_INHABITMAGIC);
            Inhabitmagic_Timer = 2000+(rand()%2000);
        }else Inhabitmagic_Timer -= diff;

        //Attractmagic_Timer
        if (Attractmagic_Timer < diff)
        {
            DoCast(m_creature,SPELL_ATTRACTMAGIC);
            Attractmagic_Timer = 30000;
        }else Attractmagic_Timer -= diff;

        //Carnivorousbite_Timer
        if (Carnivorousbite_Timer < diff)
        {
            DoCast(m_creature,SPELL_CARNIVOROUSBITE);
            Carnivorousbite_Timer = 10000;
        }else Carnivorousbite_Timer -= diff;

        //FocusFire_Timer
        if (FocusFire_Timer < diff)
        {
            // Summon Focus Fire & Emote
            Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0);
            if (target && target->GetTypeId() == TYPEID_PLAYER)
            {
                focusedTarget = target;
                m_creature->SummonCreature(ENTRY_FOCUS_FIRE,target->GetPositionX(),target->GetPositionY(),target->GetPositionZ(),0,TEMPSUMMON_TIMED_DESPAWN,5500);

                // Emote
                std::string *emote = new std::string("focuses his energy on ");
                emote->append(target->GetName());
                DoTextEmote(emote->c_str(),NULL,true);
                delete emote;

                FocusFire_Timer = 15000+(rand()%5000);
            }
        }else FocusFire_Timer -= diff;

        DoMeleeAttackIfReady();
    }
}; 

CreatureAI* GetAI_boss_shirrak_the_dead_watcher(Creature *_Creature)
{
    return new boss_shirrak_the_dead_watcherAI (_Creature);
}

struct TRINITY_DLL_DECL mob_focus_fireAI : public ScriptedAI
{
    mob_focus_fireAI(Creature *c) : ScriptedAI(c)
    {
        HeroicMode = m_creature->GetMap()->IsHeroic();
        Reset();
    }

    bool HeroicMode;
    uint32 FieryBlast_Timer;
    bool fiery1, fiery2, fiery3;

    void Reset()
    {
        FieryBlast_Timer = 3000+(rand()%1000);
        fiery1 = fiery2 = true;
    }

    void Aggro(Unit *who)
    { }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!m_creature->SelectHostilTarget() || !m_creature->getVictim() )
            return;

        //FieryBlast_Timer
        if (fiery2 && FieryBlast_Timer < diff)
        {
            DoCast(m_creature,SPELL_FIERY_BLAST);

            if(fiery1) fiery1 = false;
            else if(fiery2) fiery2 = false;

            FieryBlast_Timer = 1000;
        }else FieryBlast_Timer -= diff;

        //DoMeleeAttackIfReady();
    }
}; 

CreatureAI* GetAI_mob_focus_fire(Creature *_Creature)
{
    return new mob_focus_fireAI (_Creature);
}

void AddSC_boss_shirrak_the_dead_watcher()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_shirrak_the_dead_watcher";
    newscript->GetAI = &GetAI_boss_shirrak_the_dead_watcher;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_focus_fire";
    newscript->GetAI = &GetAI_mob_focus_fire;
    newscript->RegisterSelf();
}