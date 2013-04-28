/****************************************************************

 metabolism.c

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



/*NOBLEMAKE DEL=""*/

#include "entity.h"
#include "entity_internal.h"

#include <stdio.h>

/*NOBLEMAKE END=""*/

/*NOBLEMAKE VAR=""*/

/**
 * Returns a text description of the given vascular component
 * @param index Array index of the vascualr component
 * @param description Returned description
 */
void metabolism_vascular_description(n_int index, n_string description)
{
    n_string str[] =
    {
        "Ascending Aorta",
        "Thoracic Aorta",
        "Abdominal Aorta 1",
        "Abdominal Aorta 2",
        "Common iliac",
        "Femoral Artery",
        "Anterior Tibial Artery",
        "Brachiocephalic",
        "Brachial",
        "Right Common Carotid",
        "Left Common Carotid",
        "Left Brachial",
        "Right Radial",
        "Right Ulnar",
        "Right External Carotid",
        "Right Internal Carotid",
        "Left Internal Carotid",
        "Left External Carotid",
        "Left Radial",
        "Left Ulnar",
        "Coeliac",
        "Renal",
        "Superior Mesenteric",
        "Inferior Mesenteric",
        "Profundis",
        "Post Tibial",
        "Ant Tibial",
        "Proneal"
    };
    n_int location = 0;
    io_string_write(description, str[index], &location);
    description[location]='\0';
}


const n_string metabolism_text[] =
{
    "",
    "Protein",
    "Starch",
    "Fat",
    "Sugar",
    "Water",
    "Bile",
    "Glucose",
    "Muscle",
    "Amino Acids",
    "Glucogen",
    "Adrenalin",
    "Glycogen",
    "Ammonia",
    "Urea",
    "Lactate",
    "Oxygen",
    "CO2",
    "Fatty Acids",
    "Triglyceride",
    "Adipose",
    "Insulin",
    "ADP",
    "ATP",
    "Energy",
    "Heat",
    "Pyruvate",
    "Waste",
    "Leptin",
    "Ghrelin",
    "prolactin",
    "Milk",
    "Heart rate Hz",
    "Breathing rate Hz",
    "Thermoregulator",
    "Lung Capacity"
};

/**
 * Returns a description of the given metabolism index
 * @param index Index of the metabolism component
 * @return String describing the metabolism component
 */
n_string metabolism_description(n_int index)
{
    return metabolism_text[index];
}

#ifdef METABOLISM_ON

/**
 * Simulate a compartment containing a resistor, inductor and capacitor
 * @param I Current
 * @param vessel Pointer to the compartment
 */
static void metabolism_vascular_compartment(n_uint I, noble_vessel * vessel)
{
    const n_uint incr = 20;
    n_int Vind = 0;
    n_uint V;

    if ((I>0) && (vessel->flow_rate>0))
    {
        Vind = ((n_int)vessel->inertia*((n_int)I - (n_int)vessel->flow_rate)*1000/(n_int)I)/1000;
    }

    if (I > vessel->volume)
    {
        /** increase stored volume */
        if (vessel->volume < vessel->compliance) vessel->volume += incr;
        /** decrease flow rate */
        if (I > incr) I -= incr;
    }
    else
    {
        /** decrease stored volume */
        if (vessel->volume>incr) vessel->volume -= incr;
        /** increase flow rate */
        I += incr;
    }

    V = (I*vessel->resistance)>>12;
    if ((n_int)V + Vind > 0)
    {
        V = (n_uint)((n_int)V + Vind);
    }
    else
    {
        V = 0;
    }

    vessel->pressure = V;
    vessel->flow_rate = I;
}

/**
 * Adjust heart rate and vascular system
 * @param local_sim Pointer to the simulation
 * @param local_being Pointer to the being
 * @param response Positive (sympathetic) or negative (parasympathetic) response
 */
void metabolism_vascular_response(noble_simulation * local_sim, noble_being * local_being, n_int response)
{
    n_uint i;

    /** parasympathetic */
    if (response <= 0)
    {
        /** return towards a resting heart beat */
        n_uint hr_diff = GET_MT(local_being,METABOLISM_HEART_RATE) - HEART_RATE_RESTING;
        n_int mult = response;
        if (mult==0) mult=-1;
        GET_MT(local_being,METABOLISM_HEART_RATE) += (n_int)(1+(hr_diff>>2))*mult;

        for (i = 0; i < VASCULAR_SIZE; i++)
        {
            if (response != 0)
            {
                if (i < VASCULAR_SIZE_CORE)
                {
                    /** contract core vessels */
                    if (local_being->vessel[i].constriction > 1) local_being->vessel[i].constriction-=2;
                }
                else
                {
                    /** dilate peripheral vessels */
                    if (local_being->vessel[i].constriction < 254) local_being->vessel[i].constriction+=2;
                }
            }
            else
            {
                /** if the response is zero then return towards the default vessel radius */
                n_int v_diff =
                    (n_int)local_being->vessel[i].constriction -
                    (n_int)GET_MT(local_being,METABOLISM_THERMOREGULATOR);
                if (v_diff > 1) v_diff=1;
                if (v_diff < -1) v_diff=-1;
                local_being->vessel[i].constriction -= v_diff;
            }
        }
        return;
    }

    /** sympathetic */
    if (response > 0)
    {
        /** Range of possible heart rates */
        n_uint hr_range = HEART_RATE_MAX(local_sim,local_being) - HEART_RATE_RESTING;

        /** a target heart rate for this level of response,
        	with 16 levels possible (which could come from the genome) */
        n_uint hr_target = HEART_RATE_RESTING + ((hr_range*(n_uint)response)>>4);

        if (GET_MT(local_being,METABOLISM_HEART_RATE) < hr_target)
        {
            /** increase heart rate towards the target */
            GET_MT(local_being,METABOLISM_HEART_RATE) += (1+((hr_target - GET_MT(local_being,METABOLISM_HEART_RATE))>>2));
        }
        if (GET_MT(local_being,METABOLISM_HEART_RATE) > hr_target)
        {
            /** decrease heart rate towards the target */
            n_uint hr_diff = GET_MT(local_being,METABOLISM_HEART_RATE) - hr_target;
            GET_MT(local_being,METABOLISM_HEART_RATE) -= (n_int)(1+(hr_diff>>2));
        }

        for (i = 0; i < VASCULAR_SIZE; i++)
        {
            if (i < VASCULAR_SIZE_CORE)
            {
                /** dilate core vessels */
                if (local_being->vessel[i].constriction < 254) local_being->vessel[i].constriction+=2;
            }
            else
            {
                /** contract peripheral vessels */
                if (local_being->vessel[i].constriction > 1) local_being->vessel[i].constriction-=2;
            }
        }

    }
}

/**
 * Returns the radius of a vessel for the given being
 * @param local_being Pointer to the being
 * @param vessel_index Index number of the vessel
 * @return Radius of the vessel
 */
n_int metabolism_vascular_radius(noble_being * local_being, n_int vessel_index)
{
    return ((n_int)local_being->vessel[vessel_index].radius -
            (((n_int)local_being->vessel[vessel_index].constriction-VASCULAR_CONSTRICTION_ZERO)*
             (n_int)local_being->vessel[vessel_index].radius/256)) *
           (n_uint)local_being->height/BEING_MAX_HEIGHT;
}

/** metabolism state flags */
enum
{
    METABOLISM_STATE_ANY=1,
    METABOLISM_STATE_STORE=2,
    METABOLISM_STATE_RETRIEVE=4,
    METABOLISM_STATE_STARVE=8,
    METABOLISM_STATE_HUNGER=16,
    METABOLISM_STATE_LACTATION=32
};

static const n_int metabolic_pathway[] =
{
    /**Organ          Products                                                              Reactants                                                     Qty re.   Qty prod.  State*/
    ORGAN_STOMACH,    METABOLISM_AMINO_ACIDS,METABOLISM_GHRELIN,0,0,                        METABOLISM_PROTEIN,METABOLISM_BILE,0,0,                       1,1,0,0, 2,-10,0,0,  METABOLISM_STATE_ANY,
    ORGAN_STOMACH,    METABOLISM_GLUCOSE,METABOLISM_GHRELIN,0,0,                            METABOLISM_STARCH,METABOLISM_BILE,0,0,                        1,1,0,0, 2,-10,0,0,  METABOLISM_STATE_ANY,
    ORGAN_STOMACH,    METABOLISM_FATTY_ACIDS,METABOLISM_GHRELIN,0,0,                        METABOLISM_FAT,METABOLISM_BILE,0,0,                           1,1,0,0, 2,-10,0,0,  METABOLISM_STATE_ANY,
    ORGAN_STOMACH,    METABOLISM_GLUCOSE,METABOLISM_GHRELIN,0,0,                            METABOLISM_SUGAR,METABOLISM_BILE,0,0,                         2,1,0,0,  1,-5,0,0,  METABOLISM_STATE_ANY,

    ORGAN_TISSUE,     METABOLISM_ADRENALIN,0,0,0,                                           0,0,0,0,                                                      0,0,0,0,   2,0,0,0,  METABOLISM_STATE_RETRIEVE,
    ORGAN_PANCREAS_B, METABOLISM_INSULIN,0,0,0,                                             0,0,0,0,                                                      0,0,0,0,   4,0,0,0,  METABOLISM_STATE_STORE,
    ORGAN_LIVER,      METABOLISM_GLUCOGEN,0,0,0,                                            0,0,0,0,                                                      0,0,0,0,   2,0,0,0,  METABOLISM_STATE_STORE,
    ORGAN_TISSUE,     METABOLISM_GHRELIN,0,0,0,                                             0,0,0,0,                                                      0,0,0,0,   4,0,0,0,  METABOLISM_STATE_HUNGER,
    ORGAN_TISSUE,     METABOLISM_GHRELIN,0,0,0,                                             0,0,0,0,                                                      0,0,0,0,  -1,0,0,0,  METABOLISM_STATE_ANY,

    /** convert amino acids to muscle */
    ORGAN_MUSCLES,    METABOLISM_MUSCLE,0,0,0,                                              METABOLISM_AMINO_ACIDS,0,0,0,                                 8,0,0,0,   1,0,0,0,  METABOLISM_STATE_ANY,
    ORGAN_MUSCLES,    METABOLISM_AMINO_ACIDS,0,0,0,                                         METABOLISM_MUSCLE,0,0,0,                                      1,0,0,0,   8,0,0,0,  METABOLISM_STATE_STARVE,

    /** processing amino acids */
    ORGAN_TISSUE,     METABOLISM_GLUCOSE,METABOLISM_AMMONIA,0,0,                            METABOLISM_AMINO_ACIDS,0,0,0,                                 1,0,0,0,   1,1,0,0,  METABOLISM_STATE_ANY,
    ORGAN_KIDNEYS,    METABOLISM_UREA,METABOLISM_CO2,0,0,                                   METABOLISM_AMMONIA,0,0,0,                                     8,0,0,0,   1,8,0,0,  METABOLISM_STATE_ANY,

    /** storage of glucose as glycogen */
    ORGAN_LIVER,      METABOLISM_GLYCOGEN,0,0,0,                                            METABOLISM_GLUCOSE,METABOLISM_GLUCOGEN,0,0,                   5,1,0,0,   1,0,0,0,  METABOLISM_STATE_STORE,
    ORGAN_LIVER,      METABOLISM_GLUCOSE,0,0,0,                                             METABOLISM_GLYCOGEN,METABOLISM_ADRENALIN,0,0,                 4,4,0,0,  20,0,0,0,  METABOLISM_STATE_RETRIEVE,

    /** conversion of glucose to adipose tissue */
    ORGAN_PANCREAS_B, METABOLISM_FATTY_ACIDS,0,0,0,                                         METABOLISM_GLUCOSE,METABOLISM_INSULIN,0,0,                    1,0,0,0,   1,0,0,0,  METABOLISM_STATE_STORE,
    ORGAN_PANCREAS_B, METABOLISM_TRIGLYCERIDE,0,0,0,                                        METABOLISM_FATTY_ACIDS,METABOLISM_INSULIN,0,0,                2,2,0,0,   2,0,0,0,  METABOLISM_STATE_ANY,
    ORGAN_TISSUE,     METABOLISM_ADIPOSE,METABOLISM_LEPTIN,0,0,                             METABOLISM_TRIGLYCERIDE,0,0,0,                                2,0,0,0,   2,2,0,0,  METABOLISM_STATE_STORE,

    /** conversion of adipose tissue to glucose */
    ORGAN_TISSUE,     METABOLISM_TRIGLYCERIDE,METABOLISM_LEPTIN,0,0,                        METABOLISM_ADIPOSE,0,0,0,                                     2,0,0,0, 2,-16,0,0,  METABOLISM_STATE_RETRIEVE,
    ORGAN_PANCREAS_A, METABOLISM_FATTY_ACIDS,0,0,0,                                         METABOLISM_TRIGLYCERIDE,0,0,0,                                2,0,0,0,   2,0,0,0,  METABOLISM_STATE_RETRIEVE,
    ORGAN_PANCREAS_A, METABOLISM_GLUCOSE,0,0,0,                                             METABOLISM_FATTY_ACIDS,METABOLISM_BILE,0,0,                   1,1,0,0,   1,0,0,0,  METABOLISM_STATE_RETRIEVE,

    /** conversion between glucose and pyruvate */
    ORGAN_TISSUE,     METABOLISM_PYRUVATE,0,0,0,                                            METABOLISM_GLUCOSE,METABOLISM_ADP,0,0,                        1,2,0,0,   1,0,0,0,  METABOLISM_STATE_ANY,
    ORGAN_TISSUE,     METABOLISM_GLUCOSE,0,0,0,                                             METABOLISM_PYRUVATE,METABOLISM_ADP,0,0,                       4,8,0,0,   4,0,0,0,  METABOLISM_STATE_RETRIEVE,

    /** milk production and consumption */
    ORGAN_TISSUE,     METABOLISM_PROLACTIN,0,0,0,                                           0,0,0,0,                                                      0,0,0,0,   1,0,0,0,  METABOLISM_STATE_LACTATION,
    ORGAN_TISSUE,     METABOLISM_MILK,0,0,0,                                                METABOLISM_GLUCOSE,METABOLISM_PROLACTIN,METABOLISM_WATER,0,   1,8,8,0,   9,0,0,0,  METABOLISM_STATE_LACTATION,
    ORGAN_TISSUE,     METABOLISM_GLUCOSE,METABOLISM_WATER,0,0,                              METABOLISM_MILK,0,0,0,                                        1,0,0,0,   1,1,0,0,  METABOLISM_STATE_ANY,

    /** aerobic respiration */
    ORGAN_TISSUE,     METABOLISM_WATER,METABOLISM_CO2,METABOLISM_ATP,METABOLISM_HEAT,       METABOLISM_PYRUVATE,METABOLISM_OXYGEN,METABOLISM_ADP,0,       1,6,38,1,  1,6,6,38, METABOLISM_STATE_ANY,
    ORGAN_TISSUE,     METABOLISM_ENERGY,METABOLISM_ADP,METABOLISM_LACTATE,0,                METABOLISM_ATP,METABOLISM_WATER,0,0,                          38,6,0,0,  2,38,1,0, METABOLISM_STATE_ANY,
    ORGAN_LIVER,      METABOLISM_PYRUVATE,0,0,0,                                            METABOLISM_LACTATE,METABOLISM_OXYGEN,0,0,                     1,20,0,0,  1,1,0,0,  METABOLISM_STATE_ANY,

    0
};

/**
 * Returns if the given metabolism type is below its maximum capavity
 * @param local_being Pointer to the being
 * @param index Metabolism type index
 * @return Maximum value
 */
static n_int metabolism_below_capacity(noble_being * local_being, n_byte2 index)
{
    n_byte2 capacity = 1000;

    switch (index)
    {
    case METABOLISM_ENERGY:
        capacity = 3000;
        break;
    case METABOLISM_MUSCLE:
        capacity = 2000;
        break;
    case METABOLISM_UREA:
        capacity = 60000;
        break;
    case METABOLISM_ATP:
        capacity = 60000;
        break;
    case METABOLISM_ADP:
        capacity = 60000;
        break;
    case METABOLISM_ADRENALIN:
        capacity = ADRENALIN_MAX;
        break;
    case METABOLISM_LUNG_CAPACITY:
        capacity = MAX_LUNG_CAPACITY(local_being);
        break;
    case METABOLISM_BREATHING_RATE:
        capacity = BREATHING_RATE_MAX;
        break;
    }
    return (GET_MT(local_being,index)<capacity);
}

/**
 * Decay a reaction product amount
 * @param local_being Pointer to the being
 * @param pathway
 */ static void metabolism_decay(noble_being * local_being, n_byte2 pathway)
{
    if (GET_MT(local_being,pathway)>0)
    {
        GET_MT(local_being,pathway)--;
    }
}

/**
 * Set a given metabolism state as active or inactive
 * @param local_being Pointer to the being
 * @param state The bit to be set active or inactive
 * @param active Whether to set the state as active (1) or inactive (0)
 */ static void metabolism_set_state(noble_being * local_being, n_byte2 state, n_byte active)
{
    if (active!=0)
    {
        GET_MT(local_being,METABOLISM_STATE) |= state;
    }
    else
    {
        if (GET_MT(local_being,METABOLISM_STATE) & state)
        {
            GET_MT(local_being,METABOLISM_STATE) ^= state;
        }
    }
}

/**
 * Synthesis of milk
 * @param local_sim Pointer to the simulation
 * @param local_being Pointer to the being
 */ static void metabolism_milk_synthesis(noble_simulation * local_sim, noble_being * local_being)
{
    if (FIND_SEX(GET_I(local_being))==SEX_FEMALE)
    {
        n_uint today = TIME_IN_DAYS(local_sim->land->date);
        n_uint conception_date = TIME_IN_DAYS(local_being->date_of_conception);
        if ((conception_date > 0) &&
                (today >= conception_date) &&
                (TIME_IN_DAYS(local_sim->land->date) <=
                 conception_date + GESTATION_DAYS + CARRYING_DAYS))
        {
            /** activate lactation prior to weaning */
            metabolism_set_state(local_being,METABOLISM_STATE_LACTATION,1);
        }
        else
        {
            /** lactation deactivated */
            metabolism_set_state(local_being,METABOLISM_STATE_LACTATION,0);
            /** prolactin decays */
            metabolism_decay(local_being, METABOLISM_PROLACTIN);
        }
    }
}

/**
 * Regulation of hunger
 * @param local_sim Pointer to the simulation
 * @param local_being Pointer to the being
 */ static void metabolism_hunger(noble_simulation * local_sim, noble_being * local_being)
{
    /** leptin / ghrelin levels */
    if (GET_MT(local_being,METABOLISM_LEPTIN) > 10 + (GENE_FRAME(GET_G(local_being))*10))
    {
        /** disable the hunger flag if leptin is above a threshold */
        metabolism_set_state(local_being,METABOLISM_STATE_HUNGER,0);
    }
    else
    {
        /** enable the hunger flag if leptin falls below a threshold */
        metabolism_set_state(local_being,METABOLISM_STATE_HUNGER,1);
    }

    /** Produce bile if food is available */
    if (((GET_MT(local_being,METABOLISM_PROTEIN)>0) ||
            (GET_MT(local_being,METABOLISM_STARCH)>0) ||
            (GET_MT(local_being,METABOLISM_FAT)>0)) &&
            metabolism_below_capacity(local_being, METABOLISM_BILE))
    {
        GET_MT(local_being,METABOLISM_BILE) += 8;
    }
    else
    {
        /** decay bile */
        metabolism_decay(local_being, METABOLISM_BILE);
    }

    /** if glucose fall below a minimum threshold then enable
    	the starve flag which initiates conversion of muscle
    	back to amino acids and then to glucose */
    if (GET_MT(local_being,METABOLISM_GLUCOSE) <
            GLUCOSE_THRESHOLD_STARVE)
    {
        /** starving */
        metabolism_set_state(local_being,METABOLISM_STATE_STARVE,1);
    }
    else
    {
        /** no longer starving */
        metabolism_set_state(local_being,METABOLISM_STATE_STARVE,0);
    }
}

/**
 * Try to keep the glucose level within a range of tollerance
 * @param local_sim Pointer to the simulation
 * @param local_being Pointer to the being
 */ static void metabolism_glucose_homeostasis(noble_simulation * local_sim, noble_being * local_being)
{
    n_genetics * genetics = GET_G(local_being);

    /** if glucose falls below a minimum level then begin
    	converting glycogen and fat */
    if (GET_MT(local_being,METABOLISM_GLUCOSE) <
            GLUCOSE_THRESHOLD_MIN+(GENE_GLUCOSE_THRESHOLD_MIN(genetics)*10))
    {
        metabolism_set_state(local_being,METABOLISM_STATE_STORE,0);
        metabolism_set_state(local_being,METABOLISM_STATE_RETRIEVE,1);
    }
    else
    {
        /** If glucose level exceeds a threshold then begin
        	storing it as glycogen (glycogenesis) and fat */
        if (GET_MT(local_being,METABOLISM_GLUCOSE) >
                GLUCOSE_THRESHOLD_MAX+(GENE_GLUCOSE_THRESHOLD_MAX(genetics)*10))
        {
            metabolism_set_state(local_being,METABOLISM_STATE_RETRIEVE,0);
            metabolism_set_state(local_being,METABOLISM_STATE_STORE,1);
        }
        else
        {
            metabolism_set_state(local_being,METABOLISM_STATE_RETRIEVE,0);
            metabolism_set_state(local_being,METABOLISM_STATE_STORE,0);
        }

        /** adrenalin level decays */
        metabolism_decay(local_being, METABOLISM_ADRENALIN);
    }

    if (!(GET_MT(local_being,METABOLISM_STATE)&METABOLISM_STATE_STORE))
    {
        /** glucogen level decays */
        metabolism_decay(local_being, METABOLISM_GLUCOGEN);
        /** insulin level decays */
        metabolism_decay(local_being, METABOLISM_INSULIN);
    }
}

/**
 * Dump waste products
 * @param local_sim Pointer to the simulation
 * @param local_being Pointer to the being
 */ static void metabolism_waste_products(noble_simulation * local_sim, noble_being * local_being)
{
    n_genetics * genetics = GET_G(local_being);

    if (GET_MT(local_being,METABOLISM_UREA)>50+(GENE_BLADDER_CAPACITY(genetics)<<2))
    {
        GET_MT(local_being,METABOLISM_UREA) = 0;
    }
    if (GET_MT(local_being,METABOLISM_WASTE)>200+(GENE_WASTE_CAPACITY(genetics)<<6))
    {
        GET_MT(local_being,METABOLISM_WASTE) = 0;
    }
}

/**
 * Runs through the metabolic pathways and updates the amounts for each reaction product
 * @param local_sim Pointer to the simulation
 * @param local_being Pointer to the being
 */ static void metabolism_pathways(noble_simulation * local_sim, noble_being * local_being)
{
    n_byte2 i,j,p,mstate,index,reactant,product[METABOLISM_MAX_PRODUCTS];
    n_int increment;
    n_genetics * genetics = GET_G(local_being);
    const n_byte2 stride = 2 + (METABOLISM_MAX_PRODUCTS*2) + (METABOLISM_MAX_REACTANTS*2);

    /** update metabolic pathways */
    i = 0;
    while (metabolic_pathway[i * stride] > 0)
    {
        index = i * stride;

        /** skip inactive pathways */
        mstate = metabolic_pathway[index + 1 +
                                   (METABOLISM_MAX_PRODUCTS*2) +
                                   (METABOLISM_MAX_REACTANTS*2)];
        if ((mstate != METABOLISM_STATE_ANY) &&
                (!(mstate & GET_MT(local_being,METABOLISM_STATE))))
        {
            i++;
            continue;
        }

        /** products of the reaction */
        for (p = 0; p < METABOLISM_MAX_PRODUCTS; p++)
        {
            product[p] = metabolic_pathway[index+1+p];
        }

        /** are all the reactants present? */
        for (j = 0; j < METABOLISM_MAX_REACTANTS; j++)
        {
            reactant = metabolic_pathway[index+1+METABOLISM_MAX_PRODUCTS+j];
            if (reactant > 0)
            {
                if (GET_MT(local_being,reactant) <
                        metabolic_pathway[index+1+METABOLISM_MAX_PRODUCTS+METABOLISM_MAX_REACTANTS+j])
                {
                    break;
                }
            }
        }

        if (j>=METABOLISM_MAX_REACTANTS)
        {
            /** reduce reactant quantities */
            for (j = 0; j < METABOLISM_MAX_REACTANTS; j++)
            {
                reactant = metabolic_pathway[index+1+METABOLISM_MAX_PRODUCTS+j];
                if (reactant > 0)
                {
                    GET_MT(local_being,reactant) -=
                        metabolic_pathway[index+1+METABOLISM_MAX_PRODUCTS+METABOLISM_MAX_REACTANTS+j];
                }
            }
            /** increase the product quantities */
            for (p = 0; p < METABOLISM_MAX_PRODUCTS; p++)
            {
                increment =
                    metabolic_pathway[index + 1 + METABOLISM_MAX_PRODUCTS + p + (METABOLISM_MAX_REACTANTS*2)];

                if ((product[p] > 0) &&
                        ((increment<0) || ((increment>0) && metabolism_below_capacity(local_being,product[p]))))
                {

                    /** for particular reactions alter the number of products
                    	based upon genetics */
                    switch (product[p])
                    {
                    case METABOLISM_INSULIN:
                    {
                        increment = 2 + GENE_INSULIN_RATE(genetics);
                        break;
                    }
                    case METABOLISM_GLUCOGEN:
                    {
                        increment = 2 + GENE_GLUCOGEN_RATE(genetics);
                        break;
                    }
                    case METABOLISM_ADRENALIN:
                    {
                        increment = 2 + GENE_ADRENALIN_RATE(genetics);
                        break;
                    }
                    }

                    if (increment >= 0)
                    {
                        /** increase the value */
                        GET_MT(local_being,product[p]) += increment;
                    }
                    else
                    {
                        /** decrease the value */
                        if (GET_MT(local_being,product[p])>=-increment)
                        {
                            GET_MT(local_being,product[p]) += increment;
                        }
                        else
                        {
                            GET_MT(local_being,product[p]) = 0;
                        }
                    }

                    /** if food is digested then increment waste */
                    if (metabolic_pathway[index]==ORGAN_STOMACH)
                    {
                        GET_MT(local_being,METABOLISM_WASTE)++;
                    }
                }
            }
        }
        i++;
    }
}

/**
 * Biochemistry simulation
 * @param local_sim Pointer to the simulation
 * @param local_being Pointer to the being
 * @param core_temp Core temperature
 */ static void metabolism_chemistry(noble_simulation * local_sim, noble_being * local_being, n_uint core_temp)
{
    metabolism_milk_synthesis(local_sim, local_being);
    metabolism_hunger(local_sim, local_being);
    metabolism_glucose_homeostasis(local_sim, local_being);
    metabolism_pathways(local_sim, local_being);
    metabolism_waste_products(local_sim, local_being);
}

/**
 * Respiration system
 * @param local_sim Pointer to the simulation
 * @param local_being Pointer to the being
 * @param core_temp Core temperature
 */ static void metabolism_respiration(noble_simulation * local_sim, noble_being * local_being, n_uint core_temp)
{
    n_int pressure;
    n_uint lung_surface_area = (n_uint)GET_MT(local_being,METABOLISM_LUNG_CAPACITY)*5; /** approximation from a sphere */

    pressure = 16384-(weather_pressure(local_sim->weather,
                                       APESPACE_TO_MAPSPACE(local_being->x),
                                       APESPACE_TO_MAPSPACE(local_being->y))>>4);

    /** convert breathing rate into oxygen uptake */
    if (metabolism_below_capacity(local_being, METABOLISM_OXYGEN))
    {
        n_uint gas_pressure = ((((n_uint)pressure)>>4)*(lung_surface_area>>4))>>5;
        n_uint oxygen_absorbtion = 1 + ((gas_pressure * (GET_MT(local_being,METABOLISM_BREATHING_RATE)>>6))>>4);
        /*printf("gas %u %u\n", gas_pressure, oxygen_absorbtion);*/
        GET_MT(local_being,METABOLISM_OXYGEN) += (n_byte2)oxygen_absorbtion;
    }

    /** dump CO2 */
    if (GET_MT(local_being,METABOLISM_CO2) > 0)
    {
        n_uint gas_pressure = ((((n_uint)pressure)>>4)*(lung_surface_area>>4))>>6;
        n_uint co2_diffusion = 1 + ((gas_pressure * (GET_MT(local_being,METABOLISM_BREATHING_RATE)>>6))>>4);

        if (GET_MT(local_being,METABOLISM_CO2)>=(n_byte2)co2_diffusion)
        {
            GET_MT(local_being,METABOLISM_CO2) -= (n_byte2)co2_diffusion;
        }
        else
        {
            GET_MT(local_being,METABOLISM_CO2) = 0;
        }
    }

    /** if there is too much co2 then increase breathing rate,
    	for example after running */
    if (GET_MT(local_being,METABOLISM_CO2) > CO2_PANT)
    {
        /** breathe more often */
        if (GET_MT(local_being,METABOLISM_BREATHING_RATE) < BREATHING_RATE_MAX)
        {
            GET_MT(local_being,METABOLISM_BREATHING_RATE) += 20;
        }

        /* breathe more deeply */
        if (metabolism_below_capacity(local_being, METABOLISM_LUNG_CAPACITY))
        {
            GET_MT(local_being,METABOLISM_LUNG_CAPACITY) += 10;
        }
    }

    if (core_temp>CORE_TEMPERATURE)
    {
        /** increase breathing rate to remove excess heat */
        if (GET_MT(local_being,METABOLISM_BREATHING_RATE) < BREATHING_RATE_MAX)
        {
            GET_MT(local_being,METABOLISM_BREATHING_RATE) += 10;
        }

        /* breathe more deeply */
        if (metabolism_below_capacity(local_being, METABOLISM_LUNG_CAPACITY))
        {
            GET_MT(local_being,METABOLISM_LUNG_CAPACITY) += 5;
        }
    }
    if (core_temp<CORE_TEMPERATURE)
    {
        /** decrease breathing rate to preserve heat */
        if (GET_MT(local_being,METABOLISM_BREATHING_RATE) > BREATHING_RATE_MIN)
        {
            GET_MT(local_being,METABOLISM_BREATHING_RATE) -= 10;
        }

        /* decrease breathing depth */
        if (GET_MT(local_being,METABOLISM_LUNG_CAPACITY) >= MIN_LUNG_CAPACITY(local_being))
        {
            GET_MT(local_being,METABOLISM_LUNG_CAPACITY) -= 10;
        }
    }

    /** prevent ADP level from falling to zero */
    if (GET_MT(local_being,METABOLISM_ADP)<200)
    {
        GET_MT(local_being,METABOLISM_ADP)=200;
    }

    /** prevent water level from falling to zero */
    if (GET_MT(local_being,METABOLISM_WATER)<200)
    {
        GET_MT(local_being,METABOLISM_WATER)=200;
    }

    /** cool down */
    metabolism_decay(local_being, METABOLISM_HEAT);
}

/**
 * Updates body temperature and returns the core temperature in C x 1000
 * @param local_sim Pointer to the simulation
 * @param local_being Pointer to the being
 * @return Core temperature
 */ static n_uint metabolism_thermoregulation(noble_simulation * local_sim, noble_being * local_being)
{
    n_uint i,index,conduction,water_conduction=0,core_temp=0,diff;
    n_int local_z;
    n_vect2 slope_vector, location_vector;
    n_int ambient_temperature = weather_temperature(local_sim->land,
                                local_sim->weather,
                                APESPACE_TO_MAPSPACE(local_being->x),
                                APESPACE_TO_MAPSPACE(local_being->y));

    local_being->vessel[0].temperature =
        (n_uint)(CORE_TEMPERATURE +
                 (((n_int)GET_MT(local_being,METABOLISM_THERMOREGULATOR)-VASCULAR_CONSTRICTION_ZERO)*10)-
                 (n_int)(GET_MT(local_being,METABOLISM_BREATHING_RATE)*4));

    /** greater conductivity in water */
    land_vect2(&slope_vector, &local_z,local_sim->land, &location_vector);
    if (WATER_TEST(local_z,local_sim->land->tide_level))
    {
        /** A guess at sea temperature in an equatorial region.
        	Ideally this would come from a sea simulation */
        ambient_temperature -= 8000;
        water_conduction = 1;
    }

    index = 0;
    while (index < VASCULAR_SIZE)
    {
        n_uint paths = 0;
        for (i = 0; i < VASCULAR_SIZE; i++)
        {
            if (local_being->vessel[i].parent == index) paths++;
        }
        if (paths > 0)
        {
            for (i = 0; i < VASCULAR_SIZE; i++)
            {
                if (local_being->vessel[i].parent == index)
                {
                    /** Adjust temperature */
                    if (local_being->vessel[index].temperature > local_being->vessel[i].temperature)
                    {
                        diff = local_being->vessel[index].temperature - local_being->vessel[i].temperature;
                        local_being->vessel[i].temperature += 1 + (diff>>1);
                    }
                    else
                    {
                        diff = local_being->vessel[i].temperature - local_being->vessel[index].temperature;
                        local_being->vessel[i].temperature -= 1 + (diff>>1);
                    }

                    /** temperature loss from thermal conduction to the environment */
                    if (i < VASCULAR_SIZE_CORE)
                    {
                        if (i < (VASCULAR_SIZE_CORE>>1))
                        {
                            conduction = 6;
                        }
                        else
                        {
                            conduction = 3 - water_conduction;
                        }
                    }
                    else
                    {
                        conduction = 2 - water_conduction;
                    }
                    if (ambient_temperature < local_being->vessel[i].temperature)
                    {
                        diff = local_being->vessel[i].temperature - ambient_temperature;
                        local_being->vessel[i].temperature -= 1 + (diff>>conduction);
                    }
                    if (ambient_temperature > local_being->vessel[i].temperature)
                    {
                        diff = ambient_temperature - local_being->vessel[i].temperature;
                        local_being->vessel[i].temperature += 1 + (diff>>conduction);
                    }
                }
            }
        }
        /** update the core temperature */
        if (index < VASCULAR_SIZE_CORE) core_temp += local_being->vessel[index].temperature;
        index++;
    }

    /** Thermoregulation via altering the default vessel radius */
    core_temp /= VASCULAR_SIZE_CORE; /**< average core temperature */
    if (core_temp>CORE_TEMPERATURE)
    {
        /** vasodilation */
        if (GET_MT(local_being,METABOLISM_THERMOREGULATOR)>55) GET_MT(local_being,METABOLISM_THERMOREGULATOR)--;
    }
    if (core_temp<CORE_TEMPERATURE)
    {
        /** vasoconstriction */
        if (GET_MT(local_being,METABOLISM_THERMOREGULATOR)<200) GET_MT(local_being,METABOLISM_THERMOREGULATOR)++;
    }
    return core_temp;
}

/**
 * Update blood pressure and flow rate.
 * Based on "Simulating of Human Cardiovascular System
 * and Blood Vessel Obstruction Using Lumped Method" by
 * Mohammad Reza Mirzaee, Omid Ghasemalizadeh and Bahar Firoozabadi
 * In this model pressure is voltage and flow rate is current
 * @param local_sim Pointer to the simulation
 * @param local_being Pointer to the being
 */ static void metabolism_vascular(noble_simulation * local_sim, noble_being * local_being)
{
    n_uint i,area,I,index,radius,length,elasticity;

    metabolism_vascular_response(local_sim, local_being, 0);

    /** update parameters based of physiology */
    for (i = 0; i < VASCULAR_SIZE; i++)
    {
        /** radius taking into account constriction/dilation */
        radius = (n_uint)metabolism_vascular_radius(local_being, i);

        /** scale vessel length depending upon height */
        length = (n_uint)local_being->height*(n_uint)local_being->vessel[i].length/BEING_MAX_HEIGHT;

        /** elasticity taking into account hardening */
        elasticity =
            local_being->vessel[i].elasticity -
            local_being->vessel[i].hardening;

        /** cross sectional area of the vessel */
        area = 1 + ((3142 * radius * radius) >> 10);

        /** resistance to flow */
        local_being->vessel[i].resistance =
            8*length*3142*BLOOD_VISCOSITY /
            (area*area);

        /** inertia (inductance) */
        local_being->vessel[i].inertia =
            9*length*BLOOD_DENSITY /
            (4*area);

        /** Vessel compliance (capacitance)
        	How much of the flow can be stored */
        local_being->vessel[i].compliance =
            3*length*3142*
            radius*radius*radius /
            (2000000000*elasticity*(n_uint)local_being->vessel[i].thickness);
    }

    /** Convert heart rate to blood flow rate in ml per minute.
    	This is scaled by height to take account of different heart sizes */
    I = (n_uint)GET_MT(local_being,METABOLISM_HEART_RATE) * 100 *
        SECONDS_PER_SIMULATION_STEP * (n_uint)local_being->height /
        (1219*BEING_MAX_HEIGHT);
    metabolism_vascular_compartment(I, &local_being->vessel[0]);

    index = 0;
    while (index < VASCULAR_SIZE)
    {
        /** get total resistance */
        n_uint paths = 0, R = 0;
        for (i = 0; i < VASCULAR_SIZE; i++)
        {
            if (local_being->vessel[i].parent == index)
            {
                R += 1 + local_being->vessel[i].resistance;
                paths++;
            }
        }
        /** flow volume */
        if (R > 0)
        {
            I = local_being->vessel[index].flow_rate;
            for (i = 0; i < VASCULAR_SIZE; i++)
            {
                if (local_being->vessel[i].parent == index)
                {
                    metabolism_vascular_compartment(I*R/(1+local_being->vessel[i].resistance+R),
                                                    &local_being->vessel[i]);
                }
            }
        }
        index++;
    }
}

/**
 * Update metabolism
 * @param local_sim Pointer to the simulation
 * @param local_being Pointer to the being
 */ void metabolism_cycle(noble_simulation * local_sim, noble_being * local_being)
{
    n_uint core_temp=0;

    /** update vascular system */
    metabolism_vascular(local_sim,local_being);
    /** regulate body temperature */
    core_temp = metabolism_thermoregulation(local_sim, local_being);
    /** respiration */
    metabolism_respiration(local_sim, local_being, core_temp);
    /** biochemistry */
    metabolism_chemistry(local_sim, local_being, core_temp);
}

/**
 * Initialize vascular system.
 * Based on "Simulating of Human Cardiovascular System
 * and Blood Vessel Obstruction Using Lumped Method" by
 * Mohammad Reza Mirzaee, Omid Ghasemalizadeh and Bahar Firoozabadi
 * @param local_being Pointer to the being
 */ void metabolism_init(noble_being * local_being)
{
    n_uint i;

    /**
      l = length (cm x 1000)
      h = thickness (cm x 1000)
      r = radius (cm x 1000)
      E = elasticity
      P = parent index (beginning from 1)
     */
    const n_uint vascular_params[] =
    {
        /**  l      h       r      E    P */
        204,   163,   1450,   400,   0,   /**< 1  Ascending Aorta */
        18500,   124,   1070,   400,   1,   /**< 2  Thoracic Aorta */
        4300,   110,    870,   400,   2,   /**< 3  Abdominal Aorta */
        9600,    80,    570,   400,   3,   /**< 4  Abdominal Aorta */
        19200,    76,    519,   400,   4,   /**< 5  Common iliac */
        43200,    48,    240,   800,   5,   /**< 6  Femoral Artery */
        1500,    35,    129,  1600,   6,   /**< 7  Anterior Tibial Artery */
        2400,    85,    620,   400,   1,   /**< 8  Brachiocephalic */
        41000,    64,    400,   400,   8,   /**< 9  R Brachial */
        16800,    64,    400,   400,   8,   /**< 10 R Common Carotid */
        11000,    64,    400,   400,   1,   /**< 11 L Common Carotid */
        44400,    66,    419,   400,   1,   /**< 12 L Brachial */
        23200,    43,    159,   800,   9,   /**< 13 R Radial */
        22900,    47,    210,   800,   9,   /**< 14 R Ulnar */
        11300,    49,    249,   800,  10,   /**< 15 R External Carotid */
        17200,    54,    300,   800,  10,   /**< 16 R Internal Carotid */
        17200,    54,    300,   800,  11,   /**< 17 L Internal Carotid */
        11300,    49,    249,   800,  11,   /**< 18 L External Carotid */
        23200,    47,    210,   800,  12,   /**< 19 L Radial */
        22900,    43,    159,   800,  12,   /**< 20 L Ulnar */
        1000,    64,    390,   400,   2,   /**< 21 Coeliac */
        2700,    49,    259,   400,   3,   /**< 22 Renal */
        5400,    66,    430,   400,   3,   /**< 23 Sup Mesenteric */
        4500,    43,    159,   400,   4,   /**< 24 Inf Mesenteric */
        12100,    47,    229,  1600,   5,   /**< 25 Profundis */
        30600,    43,    180,  1600,   6,   /**< 26 Post Tibial */
        29500,    30,     99,  1600,   7,   /**< 27 Ant Tibial */
        31300,    35,    129,  1600,   7   /**< 28 Proneal */
    };

    for (i=0; i<VASCULAR_SIZE; i++)
    {
        local_being->vessel[i].length = (n_byte2)vascular_params[i*5];
        local_being->vessel[i].thickness = (n_byte2)vascular_params[i*5+1];
        local_being->vessel[i].radius = (n_byte2)vascular_params[i*5+2];
        local_being->vessel[i].elasticity = (n_byte2)vascular_params[i*5+3];
        if (vascular_params[i*5+4] > 0)
        {
            local_being->vessel[i].parent = (n_byte2)vascular_params[i*5+4]-1;
        }
        else
        {
            local_being->vessel[i].parent = 0;
        }
        local_being->vessel[i].volume = 0;
        local_being->vessel[i].inertia = 0;
        local_being->vessel[i].pressure = 0;
        local_being->vessel[i].temperature = CORE_TEMPERATURE;
        local_being->vessel[i].flow_rate = 0;
        local_being->vessel[i].constriction = VASCULAR_CONSTRICTION_ZERO;
        local_being->vessel[i].hardening = 0;
    }

    for (i = 0; i < METABOLISM_SIZE; i++)
    {
        GET_MT(local_being,i)=0;
    }

    /** Start heartbeat */
    GET_MT(local_being,METABOLISM_HEART_RATE) = 1219;
    GET_MT(local_being,METABOLISM_LUNG_CAPACITY) = MIN_LUNG_CAPACITY(local_being);
    GET_MT(local_being,METABOLISM_THERMOREGULATOR) = VASCULAR_CONSTRICTION_ZERO;
    GET_MT(local_being,METABOLISM_BREATHING_RATE) = BREATHING_RATE_MIN;

    /** chemistry */
    GET_MT(local_being,METABOLISM_HEAT)=1000;
    GET_MT(local_being,METABOLISM_ENERGY)=1000;
    GET_MT(local_being,METABOLISM_WATER)=1000;
    GET_MT(local_being,METABOLISM_PROTEIN)=1000;
    GET_MT(local_being,METABOLISM_STARCH)=1000;
    GET_MT(local_being,METABOLISM_FAT)=1000;
    GET_MT(local_being,METABOLISM_ADP)=1000;
}

/**
 * Suckling behavior
 * @param sim Pointer to the simulation
 * @param child Pointer to the child being
 * @param mother Pointer to the mother being
 */ void metabolism_suckle(noble_simulation * sim, noble_being * child, noble_being * mother)
{
    n_byte2 suckling_rate = 1 + GENE_SUCKLING_RATE(GET_G(child));

    metabolism_vascular_response(sim, mother, VASCULAR_PARASYMPATHETIC);
    metabolism_vascular_response(sim, child, VASCULAR_PARASYMPATHETIC);
    if (GET_MT(mother,METABOLISM_MILK) >= suckling_rate)
    {
        GET_MT(mother,METABOLISM_MILK) -= suckling_rate;
        GET_MT(child,METABOLISM_MILK) += suckling_rate;
    }
}

/**
 * Ingest various food types
 * @param local_being Pointer to the being
 * @param food_type Type of food being eaten
 */ void metabolism_eat(noble_being * local_being,
                        n_byte food_type)
{
    n_int i;
    n_byte2 qty[6];

    for (i=0; i<6; i++)
    {
        qty[i]=0;
    }

    switch (food_type)
    {
    case FOOD_VEGETABLE:
        qty[METABOLISM_WATER] = 200;
        qty[METABOLISM_PROTEIN] = 40;
        qty[METABOLISM_STARCH] = 110;
        break;
    case FOOD_FRUIT:
        qty[METABOLISM_WATER] = 200;
        qty[METABOLISM_PROTEIN] = 30;
        qty[METABOLISM_STARCH] = 90;
        qty[METABOLISM_SUGAR] = 60;
        break;
    case FOOD_SHELLFISH:
        qty[METABOLISM_WATER] = 100;
        qty[METABOLISM_PROTEIN] = 60;
        qty[METABOLISM_STARCH] = 60;
        qty[METABOLISM_FAT] = 10;
        break;
    case FOOD_SEAWEED:
        qty[METABOLISM_WATER] = 40;
        qty[METABOLISM_PROTEIN] = 40;
        qty[METABOLISM_STARCH] = 40;
        break;
    }

    for (i=0; i<5; i++)
    {
        if (GET_MT(local_being,i) < metabolism_below_capacity(local_being, i))
        {
            GET_MT(local_being,i) += qty[i];
        }
    }
}

#endif
