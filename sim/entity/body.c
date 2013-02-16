/****************************************************************

 body.c

 =============================================================

 Copyright 1996-2013 Tom Barbalet. All rights reserved.

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.

 This software and Noble Ape are a continuing work of Tom Barbalet,
 begun on 13 June 1996. No apes or cats were harmed in the writing
 of this software.

 ****************************************************************/

#ifndef	_WIN32
#include "../noble/noble.h"
#include "../universe/universe.h"
#else
#include "..\noble\noble.h"
#include "..\universe\universe.h"
#endif

#include "entity_internal.h"
#include "entity.h"

static void body_action_give(noble_simulation * sim, noble_being * local, noble_being * other, n_byte2 carrying)
{
    n_byte hand = BODY_RIGHT_HAND;

    if (carrying == 0)
    {
        hand = BODY_LEFT_HAND;
        carrying = OBJECTS_CARRIED(local,hand);
    }
    if ((carrying != 0) &&
            ((OBJECTS_CARRIED(other,BODY_LEFT_HAND)==0) ||
             (OBJECTS_CARRIED(other,BODY_RIGHT_HAND)==0)))
    {
        GET_A(local,ATTENTION_BODY) = BODY_RIGHT_HAND;
        GET_A(other,ATTENTION_BODY) = BODY_RIGHT_HAND;
        
        episodic_interaction(sim, local, other, EVENT_GIVEN, EPISODIC_AFFECT_ZERO, carrying);
        episodic_interaction(sim, other, local, EVENT_GIVEN_BY, AFFECT_RECEIVE, carrying);
        
        OBJECTS_DROP(local,hand);
        if (OBJECTS_CARRIED(other,BODY_RIGHT_HAND)==0)
        {
            OBJECT_TAKE(other,BODY_RIGHT_HAND,carrying);
        }
        else
        {
            OBJECT_TAKE(other,BODY_LEFT_HAND,carrying);
        }
    }
}

static void body_action_bash(noble_simulation * sim, noble_being * local, noble_being * other, n_byte2 carrying)
{
    n_byte hand = BODY_RIGHT_HAND;
    n_int  index, hit = 0;
    social_link * graph;

    if (carrying == 0)
    {
        hand = BODY_LEFT_HAND;
        carrying = OBJECTS_CARRIED(local,hand);
    }
    if (carrying!=0)
    {
        if ((carrying & INVENTORY_BRANCH) || (carrying & INVENTORY_ROCK))
        {
            GET_A(local,ATTENTION_BODY) = BODY_RIGHT_HAND;
            GET_A(other,ATTENTION_BODY) = BODY_BACK;
            index = get_social_link(other,local,sim);
            if (index>-1)
            {
                graph = GET_SOC(sim, other);
                if (!graph) return;
                if (graph[index].friend_foe>1) graph[index].friend_foe-=2;
            }
            if ((carrying & INVENTORY_ROCK) && (math_random(other->seed)>THROW_ACCURACY))
            {
                hit=1;
                GET_E(other) -= SQUABBLE_ENERGY_ROCK_HURL;
                GET_IN(sim).average_energy_output += SQUABBLE_ENERGY_ROCK_HURL;
            }
            if ((carrying & INVENTORY_BRANCH) && (math_random(other->seed)>WHACK_ACCURACY))
            {
                hit=1;
                GET_E(other) -= SQUABBLE_ENERGY_BRANCH_WHACK;
                GET_IN(sim).average_energy_output += SQUABBLE_ENERGY_BRANCH_WHACK;
            }
        }
        if (carrying & INVENTORY_BRANCH)
        {
            if (hit!=0)
            {
                episodic_interaction(sim, local, other, EVENT_WHACKED, EPISODIC_AFFECT_ZERO, 0);
                episodic_interaction(sim, other, local, EVENT_WHACKED_BY, AFFECT_WHACKED, 0);
            }
        }
        if (carrying & INVENTORY_ROCK)
        {
            episodic_interaction(sim, local, other, EVENT_HURLED, EPISODIC_AFFECT_ZERO, 0);
            if (hit!=0)
            {
                episodic_interaction(sim, other, local, EVENT_HURLED_BY, AFFECT_HURL, 0);
            }
        }
    }

}

static void body_action_interactive(noble_simulation * sim, noble_being * local, noble_being * other,
                                    n_byte local_attention, n_byte other_attention, n_byte kind)
{
    GET_A(local,ATTENTION_BODY) = local_attention;
    GET_A(other,ATTENTION_BODY) = other_attention;
    episodic_interaction(sim, local, other, kind, EPISODIC_AFFECT_ZERO, 0);
    episodic_interaction(sim, other, local, kind+1, EPISODIC_AFFECT_ZERO, 0);
}

/*** This block should also be the same function ***/

static void body_action_interactive_change(noble_simulation * sim, noble_being * local, noble_being * other,
        n_byte local_attention, n_byte other_attention, n_byte kind, n_byte positive, n_int affect)
{
    n_int index;
    GET_A(local,ATTENTION_BODY) = local_attention;
    GET_A(other,ATTENTION_BODY) = other_attention;
    index = get_social_link(other,local,sim);
    if (index>-1)
    {
        social_link * graph = GET_SOC(sim, other);
        if (!graph) return;
        if (positive)
        {
            if (graph[index].friend_foe<255) graph[index].friend_foe++;
        }
        else
        {
            if (graph[index].friend_foe>0) graph[index].friend_foe--;
        }
    }
    
    episodic_interaction(sim, local, other, kind, EPISODIC_AFFECT_ZERO, 0);
    episodic_interaction(sim, other, local, kind+1, affect, 0);
}

static void body_action_hand_object(noble_simulation * sim, noble_being * local, n_byte2 carrying, n_byte hand, n_byte kind)
{
    if (carrying == 0)
    {
        hand = BODY_LEFT_HAND;
        carrying = OBJECTS_CARRIED(local,hand);
    }
    if (carrying!=0)
    {
        if (carrying & INVENTORY_BRANCH)
        {            
            episodic_self(sim, local, kind, EPISODIC_AFFECT_ZERO, INVENTORY_BRANCH);
        }
        else
        {
            if (carrying & INVENTORY_TWIG)
            {
                episodic_self(sim, local, kind, EPISODIC_AFFECT_ZERO, INVENTORY_TWIG);
            }
            else
            {
                if (carrying & INVENTORY_SPEAR)
                {
                    episodic_self(sim, local, kind, EPISODIC_AFFECT_ZERO, INVENTORY_SPEAR);
                }
            }
        }
    }
}

static void body_action_jab(noble_simulation * sim, noble_being * local, n_byte2 carrying, n_byte hand)
{
    n_byte carrying2 = OBJECTS_CARRIED(local,BODY_LEFT_HAND);
    if ((carrying & INVENTORY_SPEAR) ||
            (carrying2 & INVENTORY_SPEAR))
    {
        n_int loc_f = GET_F(local);
        n_int az;
        n_vect2 location_vector,facing_vector,slope_vector;

        vect2_byte2(&location_vector, (n_byte2 *)&GET_X(local));
        vect2_direction(&facing_vector,(n_byte)loc_f,4);
        land_vect2(&slope_vector,&az,sim->land,&location_vector);

        if ((az > WATER_MAP) && (az < TIDE_MAX))
        {
            /* some probability of spearing a fish */
            if (math_random(local->seed)<FISHING_PROB)
            {
                /* carry fish */
                if (carrying & INVENTORY_SPEAR)
                {
                    OBJECT_TAKE(local,BODY_LEFT_HAND,INVENTORY_FISH);
                }
                else
                {
                    OBJECT_TAKE(local,hand,INVENTORY_FISH);
                }
                episodic_self(sim, local, EVENT_FISH, AFFECT_FISH, 0);
            }
        }
    }
}

static void body_action_bash_objects(noble_simulation * sim, noble_being * local, n_byte2 carrying, n_byte hand)
{
    n_byte carrying2 = OBJECTS_CARRIED(local,BODY_LEFT_HAND);
    if ((carrying & INVENTORY_ROCK) && (carrying2 & INVENTORY_ROCK))
    {
        /* bash two rocks to make a scraper */
        OBJECTS_DROP(local,hand);
        OBJECT_TAKE(local,hand,INVENTORY_SCRAPER);
    }
    if (((carrying & INVENTORY_ROCK) && (carrying2 & INVENTORY_NUT)) ||
            ((carrying & INVENTORY_NUT) && (carrying2 & INVENTORY_ROCK)))
    {
        /* bash nut with a rock */
        if (carrying & INVENTORY_NUT)
        {
            OBJECTS_DROP(local,hand);
            OBJECT_TAKE(local,hand,INVENTORY_NUT_CRACKED);
        }
        else
        {
            OBJECTS_DROP(local,BODY_LEFT_HAND);
            OBJECT_TAKE(local,BODY_LEFT_HAND,INVENTORY_NUT_CRACKED);
        }
    }
    if (((carrying & INVENTORY_BRANCH) && (carrying2 & INVENTORY_SCRAPER)) ||
            ((carrying & INVENTORY_SCRAPER) && (carrying2 & INVENTORY_BRANCH)))
    {
        /* use a scraper to make a spear */
        if (carrying & INVENTORY_BRANCH)
        {
            OBJECTS_DROP(local,hand);
            OBJECT_TAKE(local,hand,INVENTORY_SPEAR);
        }
        else
        {
            OBJECTS_DROP(local,BODY_LEFT_HAND);
            OBJECT_TAKE(local,BODY_LEFT_HAND,INVENTORY_SPEAR);
        }
    }
    if (((carrying & INVENTORY_BRANCH) && (carrying2 & INVENTORY_NUT)) ||
            ((carrying & INVENTORY_NUT) && (carrying2 & INVENTORY_BRANCH)))
    {
        /* whack nut with a branch */
        if (carrying & INVENTORY_NUT)
        {
            OBJECTS_DROP(local,hand);
            OBJECT_TAKE(local,hand,INVENTORY_NUT_CRACKED);
        }
        else
        {
            OBJECTS_DROP(local,BODY_LEFT_HAND);
            OBJECT_TAKE(local,BODY_LEFT_HAND,INVENTORY_NUT_CRACKED);
        }
    }
}


static void body_action_chew(noble_simulation * sim, noble_being * local, n_byte2 carrying, n_byte hand)
{
    if (!((carrying & INVENTORY_GRASS) ||
            (carrying & INVENTORY_TWIG) ||
            (carrying & INVENTORY_FISH) ||
            (carrying & INVENTORY_NUT_CRACKED)))
    {
        hand = BODY_LEFT_HAND;
        carrying = OBJECTS_CARRIED(local,hand);
    }
    if ((carrying & INVENTORY_GRASS) ||
            (carrying & INVENTORY_TWIG) ||
            (carrying & INVENTORY_FISH) ||
            (carrying & INVENTORY_NUT_CRACKED))
    {
        if (hand == BODY_RIGHT_HAND)
        {
            carrying |= 1;
        }
        episodic_self(sim, local,EVENT_CHEW, EPISODIC_AFFECT_ZERO, carrying);
    }
    if (carrying & INVENTORY_GRASS)
    {
        CONSUME(local,ENERGY_GRASS,FOOD_VEGETABLE);
        OBJECTS_DROP(local,hand);
    }
    else
    {
        if (carrying & INVENTORY_FISH)
        {
            /* consume fish */
            CONSUME(local,ENERGY_FISH,FOOD_SHELLFISH);
            OBJECTS_DROP(local,hand);
        }
        else
        {
            if (carrying & INVENTORY_NUT_CRACKED)
            {
                /* consume nut */
                CONSUME(local,ENERGY_NUT,FOOD_VEGETABLE);
                OBJECTS_DROP(local,hand);
            }
        }
    }
}

static void body_action_swap_hands(noble_simulation * sim, noble_being * local, n_byte2 carrying, n_byte hand)
{
    if ((carrying != 0) && (OBJECTS_CARRIED(local,BODY_LEFT_HAND)==0))
    {
        OBJECTS_DROP(local,hand);
        OBJECT_TAKE(local,BODY_LEFT_HAND,carrying);
    }
    else
    {
        if ((carrying == 0) && (OBJECTS_CARRIED(local,BODY_LEFT_HAND)!=0))
        {
            carrying = OBJECTS_CARRIED(local,BODY_LEFT_HAND);
            OBJECTS_DROP(local,BODY_LEFT_HAND);
            OBJECT_TAKE(local,hand,carrying);
        }
    }
}

static void body_action_drop(noble_simulation * sim, noble_being * local, n_byte2 carrying, n_byte hand)
{
    if (carrying == 0)
    {
        hand = BODY_LEFT_HAND;
        carrying = OBJECTS_CARRIED(local,hand);
    }
    if (carrying != 0)
    {
        OBJECTS_DROP(local,hand);
        episodic_self(sim, local, EVENT_DROP, EPISODIC_AFFECT_ZERO, carrying);
    }
}

static void body_action_pickup(noble_simulation * sim, noble_being * local, n_byte2 carrying, n_byte hand)
{
    if ((carrying != 0) && (GET_PS(local)>=POSTURE_CROUCHING))
    {
        hand = BODY_LEFT_HAND;
        carrying = OBJECTS_CARRIED(local,hand);
    }
    if (carrying == 0)
    {
        n_int loc_f = GET_F(local);
        n_int az;
        n_vect2 location_vector,facing_vector,slope_vector;
        vect2_byte2(&location_vector, (n_byte2 *)&GET_X(local));
        vect2_direction(&facing_vector,(n_byte)loc_f,4);
        land_vect2(&slope_vector,&az,sim->land,&location_vector);

        if (az > WATER_MAP)
        {
            if (az > TIDE_MAX)
            {
                n_int grass, trees, bush;
                food_values(sim->land, sim->weather, local->x, local->y, &grass, &trees, &bush);

                if ((grass>bush) && (grass>trees))
                {
                    OBJECT_TAKE(local,hand, INVENTORY_GRASS);
                    episodic_self(sim, local, EVENT_PICKUP,EPISODIC_AFFECT_ZERO, INVENTORY_GRASS);
                }
                if ((trees>grass) && (trees>bush))
                {
                    if (GET_PS(local) < POSTURE_UPRIGHT)
                    {
                        OBJECT_TAKE(local,hand, INVENTORY_BRANCH);
                        episodic_self(sim, local, EVENT_PICKUP,EPISODIC_AFFECT_ZERO, INVENTORY_BRANCH);
                    }
                    else
                    {
                        OBJECT_TAKE(local,hand, INVENTORY_NUT);
                        episodic_self(sim, local, EVENT_PICKUP,EPISODIC_AFFECT_ZERO, INVENTORY_NUT);
                    }
                }
                if ((bush>grass) && (bush>trees))
                {
                    OBJECT_TAKE(local,hand, INVENTORY_TWIG);
                    episodic_self(sim, local, EVENT_PICKUP,EPISODIC_AFFECT_ZERO, INVENTORY_TWIG);
                }
            }
            else
            {
                OBJECT_TAKE(local,hand, INVENTORY_ROCK);
                episodic_self(sim, local, EVENT_PICKUP,EPISODIC_AFFECT_ZERO, INVENTORY_ROCK);
            }
        }
    }
}

void social_action(
    noble_simulation * sim,
    noble_being * local,
    noble_being * other,
    n_byte action)
{
    n_byte2 carrying;
    n_byte hand = BODY_RIGHT_HAND;

    if (being_awake_local(sim, local) == FULLY_ASLEEP)
    {
        return;
    }

    carrying = OBJECTS_CARRIED(local,hand);
    if (other == 0L)
    {
        /* individual action */
        switch(action%INDIVIDUAL_ACTIONS)
        {
        case ACTION_JAB:
            body_action_jab(sim, local, carrying, hand);
            break;
        case ACTION_BASH_OBJECTS:
            body_action_bash_objects(sim, local, carrying, hand);
            break;
        case ACTION_CHEW:
            body_action_chew(sim, local, carrying, hand);
            break;
        case ACTION_BRANDISH:
            body_action_hand_object(sim, local, carrying, hand, EVENT_BRANDISH);
            break;
        case ACTION_DRAG:
            body_action_hand_object(sim, local, carrying, hand, EVENT_DRAG);
            break;
        case ACTION_SWAP_HANDS:
            body_action_swap_hands(sim, local, carrying, hand);
            break;
        case ACTION_DROP:
            body_action_drop(sim, local, carrying, hand);
            break;
        case ACTION_PICKUP:
            body_action_pickup(sim, local, carrying, hand);
            break;
        }
    }
    else
    {
        /* social action */
        switch(action%SOCIAL_ACTIONS)
        {
        case ACTION_PROD:
            body_action_interactive_change(sim, local, other, BODY_RIGHT_HAND, BODY_FRONT,
                                           EVENT_PRODDED, 0, AFFECT_PRODDED);
            break;
        case ACTION_HUG:
            body_action_interactive_change(sim, local, other, BODY_FRONT, BODY_FRONT,
                                           EVENT_HUGGED, 1, AFFECT_HUGGED);
            break;
        case ACTION_SMILE:
            body_action_interactive_change(sim, local, other, BODY_TEETH, BODY_TEETH,
                                           EVENT_SMILED, 1, AFFECT_SMILED);
            break;
        case ACTION_GLOWER:
            body_action_interactive_change(sim, local, other, BODY_HEAD, BODY_HEAD,
                                           EVENT_GLOWERED, 0, AFFECT_GLOWER);
            break;
        case ACTION_TICKLE:
            body_action_interactive(sim, local, other, BODY_RIGHT_HAND, BODY_FRONT, EVENT_TICKLED);
            break;
        case ACTION_POINT:
            body_action_interactive(sim, local, other, BODY_RIGHT_HAND, BODY_RIGHT_HAND, EVENT_POINT);
            break;
        case ACTION_PAT:
            body_action_interactive(sim, local, other, BODY_RIGHT_HAND, BODY_BACK, EVENT_PATTED);
            break;
        case ACTION_BASH:
            body_action_bash(sim, local, other, carrying);
            break;
        case ACTION_GIVE:
            body_action_give(sim, local, other, carrying);
            break;
        }
    }
}


n_int genetics_compare(n_genetics * genetics_a, n_genetics * genetics_b)
{
    n_int loop = 0;
    while (loop < CHROMOSOMES)
    {
        if (genetics_b == 0L)
        {
            if (genetics_a[loop] != 0)
            {
                return 0;
            }
        }
        else if (genetics_a[loop] != genetics_b[loop])
        {
            return 0;
        }
        loop++;
    }
    return 1;
}

void genetics_set(n_genetics * genetics_a, n_genetics * genetics_b)
{
    n_int loop = 0;
    while (loop < CHROMOSOMES)
    {
        genetics_a[loop] = genetics_b[loop];
        loop++;
    }
}

void genetics_zero(n_genetics * genetics_a)
{
    n_int loop = 0;
    while (loop < CHROMOSOMES)
    {
        genetics_a[loop] = 0;
        loop++;
    }
}

static n_int genetics_unique(noble_simulation * local, n_genetics * genetics)
{
    n_uint         loop = 0;
    noble_being	* local_being = local->beings;

    if (local->num == 0)
    {
        return 1;
    }

    while (loop < local->num)
    {
        if (genetics_compare(local_being[loop].new_genetics, genetics))
        {
            return 0;
        }
        loop++;
    }
    return 1;
}

/* Returns the 2 bit gene value from the given point within a diploid chromosome.
 This includes mutations */
static n_int genetics_child_gene(n_int chromosome, n_int point, n_byte2 mutation_prob, n_byte2 * random)
{
    n_byte2 mutation_type;
    n_int child_gene = 0;

    math_random3(random);
    if (random[0] < mutation_prob)
    {
        mutation_type = (random[1] & 7);
        switch(mutation_type)
        {
        case MUTATION_MATERNAL:
            child_gene = DIPLOID(
                             (math_random(random) & 3),
                             ((CHROMOSOME_FROM_FATHER(chromosome) >> point ) & 3));
            break;
        case MUTATION_PATERNAL:
            child_gene = DIPLOID(
                             ((CHROMOSOME_FROM_MOTHER(chromosome) >> point ) & 3),
                             (math_random(random) & 3));
            break;
        case MUTATION_MATERNAL_DUPLICATE:
            child_gene = DIPLOID(
                             ((CHROMOSOME_FROM_MOTHER(chromosome) >> point ) & 3),
                             ((CHROMOSOME_FROM_MOTHER(chromosome) >> point ) & 3));
            break;
        case MUTATION_PATERNAL_DUPLICATE:
            child_gene = DIPLOID(
                             ((CHROMOSOME_FROM_FATHER(chromosome) >> point ) & 3),
                             ((CHROMOSOME_FROM_FATHER(chromosome) >> point ) & 3));
            break;
        default:
            math_random3(random);
            child_gene = DIPLOID(
                             (random[0] & 3), (random[1] & 3));
        }
    }
    else
    {
        /* normal gene */
        if ((math_random(random) & 1)!=0)
        {
            child_gene = DIPLOID(
                             ((CHROMOSOME_FROM_MOTHER(chromosome) >> point ) & 3),
                             ((CHROMOSOME_FROM_FATHER(chromosome) >> point ) & 3));
        }
        else
        {
            child_gene = DIPLOID(
                             ((CHROMOSOME_FROM_FATHER(chromosome) >> point ) & 3),
                             ((CHROMOSOME_FROM_MOTHER(chromosome) >> point ) & 3));
        }
    }
    return child_gene;
}

/* Performs crossover and mutation */
static n_genetics	genetics_crossover(n_genetics mother, n_genetics father, n_byte2 * random)
{
    n_int loop = 0;
    n_genetics result = 0;
    n_int point, point2, parent;
    n_int deletion_point = 16;
    n_byte2 prob;

    /* randomly select a crossover point */
    n_int crossover_point = (math_random(random) >> 13) << 1;

    /* gene insertion/deletion */
    if (math_random(random) < MUTATION_DELETION_PROB)
    {
        deletion_point = (math_random(random) >> 13) << 1;
    }

    point = point2 = crossover_point - 8;
    while(loop< 16)
    {
        if (loop == deletion_point) point2 -= 2;

        /* equal genetic contribution from mother and father */
        if (point2 < 0)
        {
            point2 += 16;
        }
        else
        {
            if (point2 > 15) point2 -= 16;
        }

        if (loop < 8)
        {
            parent=father;
            /* higher mutation probability in males */
            prob = MUTATION_CROSSOVER_PROB*50;
        }
        else
        {
            parent=mother;
            prob = MUTATION_CROSSOVER_PROB;
        }

        result |= ( genetics_child_gene(parent, point2, prob, random) << point );
        loop += 2;
        point += 2;
        point2 += 2;
    }
    return result;
}

/* Mutates a single chromosome, without crossover */
static n_genetics	genetics_mutate(n_genetics chromosome, n_byte2 * random)
{
    n_genetics result = 0;
    n_int point = 0;
    n_int loop = 0;
    n_int deletion_point = 16;

    /* gene insertion/deletion */
    if (math_random(random) < MUTATION_DELETION_PROB)
    {
        deletion_point = (math_random(random) >> 13) << 1;
    }

    point = 0;
    while(loop< 16)
    {
        if (loop == deletion_point)
        {
            point -= 2;
            if (point < 0)
            {
                point += 16;
            }
        }
        if (point > 15) point -= 16;

        result |= ( genetics_child_gene(chromosome, point, MUTATION_CROSSOVER_PROB, random) << point );
        loop += 2;
        point += 2;
    }
    return result;
}

/* Transposes segments of the genome between chromosomes or within the same chromosome.
 This is the main cause of variation between siblings. */
static void genetics_transpose(noble_being * local, n_byte2 * local_random)
{
    math_random3(local_random);
    
    if (local_random[0] < MUTATION_TRANSPOSE_PROB)
    {
        /* number of bits to transpose */
        /* chromosome numbers */
        /* locations within the chromosomes */
        n_byte source_offset = (local_random[0]>>8)&31;
        n_byte dest_offset   = local_random[1]&31;
        /* whether to invert the sequence */
        n_byte inversion     = (local_random[0]>>13) & 1;
        n_byte source_ch     = (local_random[1]>>5) % CHROMOSOMES;
        n_byte dest_ch       = (local_random[1]>>7) % CHROMOSOMES;
        n_int  ctr1          = source_offset;
        n_byte p             = 0;
        math_random3(local_random);
        while (p < (local_random[1]&15))
        {
            n_int ctr2;
            ctr1 = (ctr1 & 31);
            
            if (inversion==0)
            {
                ctr2=(n_int)(dest_offset+p);
            }
            else
            {
                /* inverted sequence */
                ctr2=(n_int)dest_offset-p+32;
            }
            ctr2 = (ctr2 & 31);
            /* clear destination bit */
            if ((GET_G(local)[dest_ch] & (1<<ctr2)) != 0)
            {
                GET_G(local)[dest_ch] ^= (1 << ctr2);
            }
            /* set destination bit */
            if ((GET_G(local)[source_ch] & (1<<ctr1)) != 0)
            {
                GET_G(local)[dest_ch] |= (1 << ctr2);
            }
            p++;
            ctr1++;
        }
    }
}

void body_genetics(noble_simulation * sim, noble_being * local, noble_being * mother, n_byte2 * local_random)
{
    n_c_uint ch;
    n_byte sex = 2;

    /* determine the sex */
    math_random3(local_random);
    sex |= (local_random[0]&1);

    do
    {
        /* crossover and mutation */
        for (ch = 0; ch < CHROMOSOMES; ch++)
        {
            if (ch != CHROMOSOME_Y)
            {
                GET_G(local)[ch] = genetics_crossover(GET_G(mother)[ch], mother->father_new_genetics[ch], local_random);
            }
        }
        /* Y chromosome does not undergo crossover and passes from father to son */
        if (sex != SEX_FEMALE)
        {
            GET_I(local) = genetics_mutate(mother->father_new_genetics[CHROMOSOME_Y], local_random);
        }
        else
        {
            GET_I(local) = genetics_mutate(mother->mother_new_genetics[CHROMOSOME_Y], local_random);
        }
        /* transpose genes between chromosomes */
        genetics_transpose(local, local_random);
        /* align the sex genetics */
        GET_I(local) &= ~1;
        GET_I(local) |= sex;
    }
    while (genetics_unique(sim, GET_G(local)) == 0);
}

void body_genome_random(noble_simulation * sim, noble_being * local, n_byte2 * local_random)
{
    do
    {
        n_int ch,loop,gene;

        for (ch = 0; ch < CHROMOSOMES; ch++)
        {
            GET_G(local)[ch] = 0;
            for (loop=0; loop<16; loop+=2)
            {
                math_random3(local_random);
                gene = DIPLOID((local_random[0] & 3), (local_random[1] & 3));
                GET_G(local)[ch] |= ( gene << loop );
            }
        }
        /* align the sex genetics */
        GET_G(local)[CHROMOSOME_Y] |= 2;
    }
    while (genetics_unique(sim, GET_G(local)) == 0);
}

void body_genome(n_byte maternal, n_genetics * genome, n_byte * genome_str)
{
    n_byte string_point = 0;
    n_int ch, value;

    n_byte nucleotide[] = { 'A', 'T', 'C', 'G' };
    for (ch = 0; ch < CHROMOSOMES; ch++)
    {
        n_byte gene_point = 0;
        while (gene_point < 16)
        {
            if (maternal!=0)
            {
                value = ( CHROMOSOME_FROM_MOTHER(genome[ch]) >> gene_point ) & 3;
            }
            else
            {
                value = ( CHROMOSOME_FROM_FATHER(genome[ch]) >> gene_point ) & 3;
            }
            genome_str[string_point++] = nucleotide[value];
            gene_point += 2;
        }
    }
    genome_str[string_point] = 0;
}

